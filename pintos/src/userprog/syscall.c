#include "userprog/syscall.h"
#include <stdio.h>
#include <stdlib.h>
#include <list.h>
#include <stdbool.h>

#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "devices/shutdown.h"
#include "process.h"
#include "threads/vaddr.h" 
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "devices/input.h"


#include "../filesys/file.h"
#include "../filesys/filesys.h"
#include "../filesys/inode.h"


/* prj2_2 */
static struct lock open_lock;
int sys_fd = 2;

struct file_nth* find_by_fd(int fd);
static struct file* find_nth_file(int file_num);
static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
	/* prj2_2 */
	lock_init( &open_lock);
	sys_fd = 2;
	/*   */

  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{

//	hex_dump(f->esp,f->esp,0x90,true);
//


	switch( *(uint32_t *)(f->esp)  )
	{
		case SYS_HALT:
			halt();
			break;
		case SYS_EXIT:
			check_addr_val(f->esp + 4);
			exit( *(uint32_t *)(f->esp + 4));
			break;
		case SYS_EXEC:
			check_addr_val(f->esp + 4);
			f-> eax = exec((const char*)*(uint32_t*)(f->esp + 4));/////////try
			break;
		case SYS_WAIT:
			check_addr_val(f->esp + 4);
			f-> eax = wait( (pid_t)*(uint32_t *)(f->esp +4)); // return value to eax
			break;
		case SYS_READ:
			check_addr_val(f->esp + 20); check_addr_val(f->esp + 24); check_addr_val(f->esp + 28);
			lock_acquire( &me_lock );
			f->eax = read((int)*(uint32_t *)(f->esp + 20), (void *)*(uint32_t *)(f->esp + 24), (unsigned)*((uint32_t *)(f->esp + 28)));
			lock_release( &me_lock);
			break;
		case SYS_WRITE:
			check_addr_val(f->esp + 20); check_addr_val(f->esp + 24); check_addr_val(f->esp + 28);
			lock_acquire( &me_lock );
			f->eax = write((int)*(uint32_t *)(f->esp + 20), (void *)*(uint32_t *)(f->esp + 24), (unsigned)*((uint32_t *)(f->esp + 28)));
			lock_release( &me_lock);
			break;
		case SYS_FIBONACCI:
			check_addr_val(f->esp + 4);
			f->eax = fibonacci( (int)*(uint32_t*)(f->esp + 4));
			break;
		case SYS_SUMofABCD:
			{
				// check address validate.
				check_addr_val(f->esp +24);
				check_addr_val(f->esp +28);
				check_addr_val(f->esp +32);
				check_addr_val(f->esp +36);

				int a = (int)*(uint32_t *)(f->esp +24);
				int b = (int)*(uint32_t *)(f->esp +28);
				int c = (int)*(uint32_t *)(f->esp +32);
				int d = (int)*(uint32_t *)(f->esp +36);

				f->eax = sum_of_four_integers(a,b,c,d);
			}
			break;
		case SYS_SEEK:
			check_addr_val(f->esp + 16);
			check_addr_val(f->esp + 20);

			seek( (int)*(uint32_t *)(f->esp + 16),(unsigned)*(uint32_t *)(f->esp + 20));
			break;
		case SYS_TELL:
			check_addr_val(f->esp + 4);
			f->eax = tell( (int)*(uint32_t*)(f->esp + 4));
			break;
		case SYS_CLOSE:
			check_addr_val(f->esp + 4);
			close( (int)*(uint32_t *)(f->esp + 4));
			break;
		case SYS_CREATE:
			check_addr_val(f->esp + 16);
			check_addr_val(f->esp + 20);

			lock_acquire(&me_lock);
			f->eax = create( (const char*)*(uint32_t*)(f->esp + 16), (unsigned)*(uint32_t*)(f->esp + 20));
			lock_release(&me_lock);

			break;
		case SYS_REMOVE:		
			check_addr_val(f->esp + 4);


			lock_acquire(&me_lock);
			f->eax = remove( (const char*)*(uint32_t*)(f->esp + 4));	
			lock_release(&me_lock);
			break;
		case SYS_OPEN:
			check_addr_val(f->esp + 4);
			f->eax = open( (const char*)*(uint32_t*)(f->esp + 4));
			break;
		case SYS_FILESIZE:
			check_addr_val(f->esp + 4);
			f->eax = filesize( (int)*(uint32_t*)(f->esp + 4));
			break;
	}

	
 
	//thread_exit ();
}
void halt(void)
{
	shutdown_power_off();
}

void exit(int status)
{
	printf("%s: exit(%d)\n", thread_name(), status);
	thread_current()->exit_status = status;/////////
	thread_exit();
}

pid_t exec( const char *cmd)
{
	return process_execute(cmd);
}

int wait( pid_t pid)
{
	return process_wait(pid);
}

int read(int fd, void* buffer, unsigned size)
{
	int i , ret = 0;

	//check valid address
	if( buffer >= PHYS_BASE )
		exit(-1);

	if( fd == 0)// STDIN
	{		
		for( i = 0 ; i < (int)size ; ++i)
		{
			*(uint8_t*)(buffer + i) = input_getc();	
		}

		if( i !=(int) size)
			return -1;
		else
			return size;
	}
	else // STDOUT 
	{
		struct file_nth *fp = NULL;
		if(fd > 1)
		{
			fp = find_by_fd(fd);

			if( fp != NULL)
			{
				ret = file_read(fp->f, (void*)buffer,size);
			}

		}
		else
			exit(-1);

		return ret;

	}
	return -1;


}
int write(int fd, const void *buffer,unsigned size)
{

	unsigned ret = 0;
	
	if(buffer >= PHYS_BASE || !is_user_vaddr( buffer + size -1) || buffer == NULL )
		exit(-1);

	if( fd == 1)
	{
		putbuf(buffer,size);
		return size;
	}
	else
	{
		struct file_nth *fp = NULL;
		
		if( fd > 1)
		{
			fp = find_by_fd(fd);
			if( fp != NULL)
			{
				ret = file_write(fp->f,buffer,size);
			}
			return ret;
		}
		else
			exit(-1);

		return ret;
	}
	return -1;
}

void check_addr_val(const void *addr)
{
	// in thread/vaddr.h
	// It can filter bad address.
	if(!is_user_vaddr(addr))
	{
		exit(-1);
	}
}
int fibonacci(int n)
{
	
	int fibo1 = 1,fibo2 = 1;
	int i,tmp;
	if( n == 1 || n == 2 )
		return 1;
	else
	{

		for(i = 2 ; i < n ;i++)
		{
			fibo1 = fibo1 + fibo2;
			tmp = fibo1;
			fibo1 = fibo2;
			fibo2 = tmp;
		}

	}

	return fibo2;
}

int sum_of_four_integers(int a,int b,int c, int d)
{
	return a + b + c + d;
}
/* prj2_2 */

bool create(const char *file, unsigned initial_size)
{
	if( file == NULL || file + initial_size -1 >= PHYS_BASE )
		exit(-1);

	bool ret_val = filesys_create(file,initial_size);

	return ret_val;

}
bool remove(const char *file)
{
	if(file == NULL)
		exit(-1);

	bool ret_val = filesys_remove(file);
	
	return ret_val;

}

int open(const char *file)
{

	struct file_nth* fp; 
	struct thread* cur = thread_current();

	if( file == NULL) 
		exit(-1);

	fp = (struct file_nth*)malloc(sizeof(struct file_nth));

	if( list_size (&cur->file_list) > FILE_OPEN_NUM )
		return -1;

	lock_acquire( &open_lock);
	fp->f = filesys_open(file);
	lock_release(&open_lock);

	if(fp->f == NULL)
		return -1;


	//check file is exist,if it exists file_deny_write
	if( thread_exist_check (file) )
	{
		file_deny_write(fp->f);
	}

	//give sys_fd each files. 
	//This step, make each file atomical thread using lock.
	
	fp->fd = sys_fd++;

	//add current thread
	list_push_back ( &cur->file_list , &fp->file_elem);

	return fp->fd;



}
void 
close(int fd)
{
	struct thread* cur = thread_current();
	struct list *file_lst = &(cur->file_list);
	struct file_nth* fp = NULL;
	struct list_elem *e = NULL;

	for ( e = list_begin( file_lst );
			e != list_end( file_lst );
			e = list_next (e))
	{
		fp = list_entry( e, struct file_nth , file_elem);

		if( fp->fd == fd)
		{
			if( fp->fd == NULL)
				exit(-1);

			file_close( fp->f );
			list_remove(e);

			if( fp== NULL)
				;
			else
				free(fp);
			
			break;

		}
	}
	
}


int filesize(int fd){

	struct list_elem *e;
	struct file_nth *t = NULL;
	struct thread *cur = thread_current();

	for( e = list_begin (&cur->file_list);
			e != list_end(&cur->file_list);
			e = list_next(e))
	{
		t = list_entry(e , struct file_nth , file_elem);
		if( t ->fd == fd)
		{

			if (t->fd == NULL)
				exit(-1);
			return file_length(t->f);
		}
	}


	return -1;
}


void 
seek(int fd,unsigned position)
{
	struct file *f = find_nth_file(fd);
	if( f == NULL)
		exit(-1);

	lock_acquire(&open_lock);
	file_seek(f,position);
	lock_release(&open_lock);

}

//find file in list
static struct file* 
find_nth_file(int file_num)
{
	struct thread *cur = thread_current();
	struct list_elem *e, *found = NULL;

	for( e = list_begin(&cur->file_list);
			e != list_end(&cur->file_list);
			e = list_next(e))
	{
		if(list_entry(e, struct file_nth,file_elem)->fd == file_num)
		{
			found = e;
			break;
		}

	}

	if( found == NULL)return NULL;
	

	return list_entry(found,struct file_nth,file_elem)->f;

}
struct file_nth* find_by_fd( int fd)
{
	struct list_elem *e;
	struct file_nth *t = NULL;
	struct thread *cur = thread_current();

	for( e = list_begin (&cur->file_list);
			e != list_end(&cur->file_list);
			e = list_next(e))
	{
		t = list_entry(e , struct file_nth , file_elem);
		if( t ->fd == fd)
			return t;
	}
	return t;


}

unsigned tell(int fd)
{
	struct list_elem *e;
	struct file_nth *t = NULL;
	struct thread *cur = thread_current();

	for( e = list_begin (&cur->file_list);
			e != list_end(&cur->file_list);
			e = list_next(e))
	{
		t = list_entry(e , struct file_nth , file_elem);
		if( t ->fd == fd)
			return file_tell(t->f);
	}

	return -1;
}


