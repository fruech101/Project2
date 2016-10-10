#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
void halt(void);
void exit(int status);
pid_t exec(const char *cmd_line);
int wait(pid_t pid);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open(const char *file);
int filesize(int fd);
int read(int fd, void *buffer, unsigned size);
int write(int fd, const void *buffer, unsigned size);
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);
void isValidPointer(void * requested_mem)
struct file* fdToFile(int fd)
//stands for babysitter, as in it watches children.
struct bs
{
pid_t pid;
bool iwo;//Is Waited On
int sh;//exit status holder
int lh;//load status holder 0 = unloaded, 1 = loaded, 2 = failed
struct list_elem elem;//needs to be listable.
}
#endif /* userprog/syscall.h */

