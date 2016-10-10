#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <stdlib.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "filesys/filesys.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "threads/thread.h"
#include "filesys/file.h"
#include "threads/synch.h"
#include <stdbool.h>
#include "devices/input.h"
#include "devices/shutdown.h"
#include "user/syscall.h"
#include "filesys/filesys.h"
static void syscall_handler (struct intr_frame *);

struct lock fileLock;

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
    int argc;
    syscall_function *func;
  };
  const struct syscall systable[]=
  {
   (0,(syscall_function *) halt),
   (1,(syscall_function *) exit),
   (1,(syscall_function *) exec),
   (1,(syscall_function *) wait),
   (2,(syscall_function *) create),
   (1,(syscall_function *) remove),
   (1,(syscall_function *) open),
   (1,(syscall_function *) filesize),
   (3,(syscall_function *) read),
   (3,(syscall_function *) write),
   (2,(syscall_function *) seek),
   (1,(syscall_function *) tell),
   (1,(syscall_function *) close),
   };
  struct syscall *sc;
  unsigned call_nr;
  int args[3];
  copy_in(&call_nr, f->esp, sizeof call_nr);
  //if invalid, exit
  if(call_nr >= sizeof systable/sizeof *systable)
  thread_exit ();
  sc=systable+call_nr;
  RARG(f, args, call_nr);
  f->eax =sc->func(arg[0],arg[1],arg[3]);

  
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
  if (*parent->status == THREAD_RUNNING)
  {
    parent->status = status;
  }
  thread_exit ();
  intr_set_level (old level);
}

pid_t exec (const char * cmd_line)
{
  struct thread * cur = thread_current();
  pit_t pid = process_execute (cmd_line);
  struct thread * child_pointer;
  for (e=list_begin(&cur->children), e != list_end(&cur->children), e = list_next(e))
  {
    if(e->pid == pid)
    {
      child_pointer == e->pid;
    }
  }

  while (!(struct process)child_pointer->load ())
}

pid_t wait(pid_t pid)
{
 if()
 return -1;
}

bool create(const char *file, unsigned initial_size)
{
 lock_acquire(&fileLock);// needed for atomicity. we need it here
 bool win=filesys_create(file, initial_size); // Create the file using the given function
 lock_release(&fileLock);
 return win;
}

bool remove(const char *file)
{
lock_acquire(&fileLock);
bool win=filesys_remove(file);
lock_release(&fileLock);
return win;
}
int open(const char *file)
{
 lock_acquire(&fileLock);
 struct file *cf=filesys_open(file);
 if(!cf)
  {
   lock_release(&fileLock);
   return -1;    
  }
  
  lock_relase(&fileLock);
}
void close(int fd)
{



}


struct file* fdToFile(int fd)
{
  struct thread cur=current_thread)();
  struct list_elem e;
  for(e=list_begin(&cur->open_files),e!=list_end(&cur->open_files), e=list_next(e))
  {
   struct file tf=list_entry(e, struct file, elem);
   if(tf->fd==fd)
   {
    return tf->file;     
   }
  }
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
 unsigned ret=file_tell(fd);
 lock_release(&fileLock);
 return ret;
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
 for(int i=0; i<s; i++)
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
void RARG(struct intr_frame *f, int *arg, int a)
{
  int *hld; //holds things
  for(int i=0; i<a; a++)
    {
     hld=(int *) f->esp+a+1;
     isValidPointer((const void *) hld);
     arg[i]=hld;
    }
}