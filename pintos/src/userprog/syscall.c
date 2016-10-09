#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

struct lock fileLock;

void
syscall_init (void) 
{
  lock_init(&fileLock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
  thread_exit ();
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
  return
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

void
isValidPointer (void *requested_mem)
{
  if(*requested_mem == NULL || !is_user_vaddr(requested_mem))
  {
    exit ();
  }
}