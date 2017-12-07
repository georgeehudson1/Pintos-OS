#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include  <user/syscall.h>
#include  "devices/input.h"
#include  "devices/shutdown.h"
#include  "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/interrupt.h"
#include  "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"

#define ARG_CODE 0
#define ARG_1 4
#define ARG_2 8
#define ARG_3 12
#define EXIT_ERROR -1


static void syscall_handler (struct intr_frame *);

void syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/*load stack start
process to take load data from the stack for processing*/
static uint32_t load_stack(struct intr_frame *f, int offset)
{
    return *((uint32_t*)(f->esp + offset));
}

//end

// halt (0) start
//halt terminates the system
static void handle_halt(void)
{
    printf("handle_halt\n");
    shutdown_power_off();
}

//end

//exit (1) start
//exit terminates the currently running thread
static void handle_exit(int status)
{
    struct thread * current = thread_current();
    printf ("%s: exit(%d)\n", current->name, status);
    thread_exit();
}

//end

//wait (3) start
//wait tells the currently running process to wait
int handle_wait(tid_t pid)
{
  return process_wait(pid);
}
//end

//write (9) start
//write is the functin to deal with input via the buffer
static int handle_write(int fd, const void * buffer, unsigned int length)
{
    //if the file is 1 then add the buffer
    if (fd == STDOUT_FILENO)
    {
	     putbuf((const char *)buffer, (size_t)length);
    }
    else
    {
	     printf("handle_write does not support fd output\n");
    }

    return length;
}

//end

syscall_handler (struct intr_frame *f)
{
//Extracts the SYS_Call_NO form the stack
int code = (int)load_stack(f, ARG_CODE);
switch(code)
{
  case SYS_HALT: /* Halts this process */
  {
    handle_halt();
    break;
  }
case SYS_EXIT:                  /* Terminate this process. */
  {
    handle_exit(load_stack(f, ARG_1));
    break;
  }
case SYS_WAIT:                   /* Wait for a child process to die. */
{
  f->eax = handle_wait(code);
  break;
}

case SYS_WRITE:                 /* Write to a file. */
{
  int result = handle_write((int)load_stack(f,ARG_1),(void *)load_stack(f, ARG_2),(unsigned int)load_stack(f, ARG_3));

	// set return value
	f->eax = result;
	break;
}

default:
  //default argument for if the system call is finished
  printf("SYS_CALL (%d) not implemented\n", code);
  thread_exit();
}
// end of switch
}
