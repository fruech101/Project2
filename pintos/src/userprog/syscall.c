#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <user/syscall.h>
#include <string.h>
#include "devices/input.h"
#include "devices/shutdown.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/interrupt.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"

static void syscall_handler (struct intr_frame *);

struct lock fileLock;

static void syscall_handler (struct intr_frame *f);
void isValidPointer (void *requested_mem);
void buffCheck(void * bSt, unsigned s);
int uskrt(const void* usr);
void RARG(struct intr_frame *f, int *arg, int a );
struct file* fdToFile(int fd);

void
syscall_init (void) 
{
  lock_init(&fileLock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  typedef int syscall_function (int, int, int);
  struct syscall
  {
    size_t argc;
    syscall_function *func;
  };

  static const struct syscall systable[]=
  {
   {0, (syscall_function *) halt},
   {1, (syscall_function *) exit},
   {1, (syscall_function *) exec},
   {1, (syscall_function *) wait},
   {2, (syscall_function *) create},
   {1, (syscall_function *) remove},
   {1, (syscall_function *) open},
   {1, (syscall_function *) filesize},
   {3, (syscall_function *) read},
   {3, (syscall_function *) write},
   {2, (syscall_function *) seek},
   {1, (syscall_function *) tell},
   {1, (syscall_function *) close},
  };
  const struct syscall *sc;
  unsigned int call_nr;
  int args[3];
  memcpy(&call_nr, f->esp, sizeof(&f->esp));
  //if invalid, exit
  if(call_nr >= sizeof(systable)/sizeof(*systable))
    thread_exit ();
  sc=systable+call_nr;
  //RARG(f, arg, call_nr); //get the arguments into arg[];

  ASSERT (sc->argc <= sizeof(args) / sizeof (*args)); 
  memset (args, 0, sizeof(args));
  memcpy(args, (uint32_t *) f->esp + 1, sizeof( *args * sc->argc));

  f->eax = sc->func(args[0], args[1], args[3]);
}

void
halt (void)
{
  shutdown_power_off();
}

void
exit (int status)
{
  enum intr_level old_level = intr_disable ();
  struct thread * par = thread_current()->parent;
  struct thread * child_ptr;

  //find the calling thread in the parent's child list
  struct list_elem * e;
  for (e=list_begin(&par->children); e != list_end(&par->children); e = list_next(e))
  {
    child_ptr = list_entry(e, struct thread, child_elem);
    if (child_ptr == thread_current())
    {
      par->child=child_ptr;
    }
  }

  //if the parent thread waits on the child, return child's status to parent
  if (par->child->is_waited_on)
  {
    par->status = status;
  }
  thread_exit ();
  intr_set_level (old_level);
}

pid_t exec (const char * cmd_line)
{
  struct thread * cur = thread_current();
  pid_t pid = process_execute (cmd_line);
  struct thread * child_ptr;

  //find the child thread in this thread's calling list
  struct list_elem *e;
  for (e = list_begin (&cur->children); e != list_end (&cur->children); e = list_next(e))
  {
    child_ptr = list_entry(e, struct thread, child_elem);
    if (child_ptr->tid == pid)
    {
      cur->child = child_ptr;
    }
  }

  //wait for child to load
  if(cur->child->is_waited_on)
  {
    sema_down (&cur->child->load_wait);
  }
  return pid;
}

int wait(pid_t pid)
{
 //not yet implemented
 return -1;
}

bool create(const char *file, unsigned initial_size)
{
  lock_acquire(&fileLock);// needed for atomicity. we need it here
  bool win=filesys_create(file, initial_size); // Create the file using the given function
  lock_release(&fileLock);
  return win; // #winning
}

bool remove(const char *file)
{
  lock_acquire(&fileLock);
  bool win=filesys_remove(file);
  lock_release(&fileLock);
  return win;
}

int open(const char * file) 
{
  lock_acquire(&fileLock);
  struct file *opened = filesys_open(file);
  if (opened==NULL)
  {
    lock_release(&fileLock);
    return -1;
  }
//not done yet

}

//Uses the file_length call and the fdToFile translator to get the file's size returns -1 if it fails
int filesize(int fd)
{
  lock_acquire(&fileLock);
  struct file *entry=fdToFile(fd);
  if(entry==NULL)
  {
    lock_release(&fileLock);
    return -1;
  }
  int ret=file_length(entry);
  lock_release(&fileLock);
  return ret;
}

int read (int fd, void * buffer, unsigned size)
{
  uint8_t * casted = (uint8_t *)buffer;
  if(fd == 0) //if it is keyboard input
  {
    for (int i = 0; i < (int)size; i++)
    {
      casted[i] = input_getc();
      buffer = casted;
    }
    return size;
  }
  lock_acquire(&fileLock);
  struct file *entry=fdToFile(fd);
  if(entry==NULL)
  {
    lock_release(&fileLock);
    return -1;
  }
  int ret = file_read(entry, buffer, size);
  lock_release(&fileLock);
  return ret;
}

int write (int fd, const void * buffer, unsigned size)
{
  if(fd == 1) //if it is console output
  {
    putbuf((char *)buffer, size);
    return size;
  }
  lock_acquire(&fileLock);
  struct file *entry=fdToFile(fd);
  if(entry==NULL)
  {
    lock_release(&fileLock);
    return -1;
  }
  int ret = file_write(entry, buffer, size);
  lock_release(&fileLock);
  return ret;
}

void seek(int fd, unsigned position)
{
  lock_acquire(&fileLock);
  struct file *entry=fdToFile(fd);
  if(entry==NULL)
  {
    lock_release(&fileLock);
    return;
  }
  file_seek(entry, position);
  lock_release(&fileLock);
}


unsigned tell(int fd)
{
  lock_acquire(&fileLock);
  struct file *entry=fdToFile(fd);
  if(!entry)//Pointers are false if null, thank you stackOverflow!
  {
    lock_release(&fileLock);
    return -1;
  }
  unsigned ret=file_tell(entry);
  lock_release(&fileLock);
  return ret;
}

void close(int fd)
{



}


//Quality of Life section
void
isValidPointer (void *requested_mem)
{
  if(requested_mem == NULL || !is_user_vaddr(requested_mem))
  {
    exit (-1);
  }
}
//rolls through buffer that starts at bSt (buffer Start) and is of size s, checking if it is valid
void
buffCheck(void * bSt, unsigned s)
{
 char* lSt=(char *)bSt; //local copy of bSt so there is no unintentional alterations
 for(int i=0; i<(int)s; i++)
 {
  isValidPointer(lSt);
  lSt++;
 }

}
//USer to KeRnel pointer Translator.
//takes user pointer usr and returns the kernel address using given pagedir functions.
int uskrt(const void* usr)
{
  isValidPointer(usr);
  void *kpt = pagedir_get_page(thread_current()->pagedir, usr);
  if(!kpt)
  {
   exit(-1);  
  }
  return (int) kpt;
}
//Returns ARGuments
void RARG(struct intr_frame *f, int *arg, int a )
{
  int *hld; //holds things
  for(int i=0; i<a; a++)
    {
     hld=(int *) f->esp+a+1;
     isValidPointer((void *) hld);
     arg[i]=*hld;
    }
}
/*gets file with file descriptor fd from the process's open_files list.*/
struct file* fdToFile(int fd)
{
  struct thread *cur=thread_current();
  struct list_elem *e;
  struct file * tf;
  for(e=list_begin(&cur->open_files); e!=list_end(&cur->open_files); e=list_next(e))
  {
   tf=list_entry(e, struct file, open_files_elem);
   if(tf->fd==fd)
   {
    return tf->file;     
   }
  }
}
