#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include <stdlib.h>
#include <stdbool.h>
#include "threads/interrupt.h"
typedef int pid_t;
void syscall_init (void);
void halt(void);
void exit(int status);
int exec(const char *cmd_line);
pid_t wait(pid_t pid);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open(const char *file);
int filesize(int fd);
int read(int fd, void *buffer, unsigned size);
int write(int fd, const void *buffer, unsigned size);
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);
void isValidPointer(void * requested_mem);
struct file* fdToFile(int fd);
void RARG(struct intr_frame *f, int *arg, int a);
int uskrt(const void* usr);
void buffCheck(void* bSt, unsigned s);

//stands for babysitter, as in it watches children.
#endif /* userprog/syscall.h */

