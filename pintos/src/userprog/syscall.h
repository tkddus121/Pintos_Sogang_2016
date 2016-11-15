#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "lib/user/syscall.h"

typedef int pid_t;


/* prj2_1 */

void syscall_init (void);

void halt(void);
void exit(int status);

pid_t exec(const char *cmd);

int wait(pid_t pid);
int open(const char *file);
int filesize(int fd);
int read(int fd,void *buffer,unsigned size);
int write(int fd,const void *buffer,unsigned size);

void seek(int fd,unsigned position);
unsigned tell(int fd);
void close(int fd);

bool create(const char *file, unsigned initial_size);
bool remove(const char *file);

void check_addr_val(const void *addr);

/* Add new system calls */
int fibonacci(int n);
int sum_of_four_integers(int a,int b,int c,int d);


#endif /* userprog/syscall.h */
