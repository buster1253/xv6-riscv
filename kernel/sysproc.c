#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


// exec_mem

int
fn1(void(*print)(char *, ...)) {
    /*print("abc\n");*/
    return 1;
}

uint64
sys_exec_mem(void) {
    // refer to the kernel pagetable (defined in kernel/vm.c)
    extern pagetable_t kernel_pagetable;
    pagetable_t pt = kernel_pagetable; // instead of uvmcreate();
    // allocate a page that will be used to store the function
    void *mem = kalloc();

    uint64 va = 0x89000000;
    // map the new page as READ and EXECUTE
    if (mappages(pt, va, PGSIZE, (uint64)mem, PTE_R | PTE_W | PTE_X) < 0) {
        printf("mappages failed\n");
        return 0;
    }

    int fsize = (void *)&sys_exec_mem - (void *)&fn1;
    void *dst = memmove((void*)va, (const void *)&fn1, fsize);

    // Instead of copying the code of fn1, just write two (compact, 16 bit) "ret" machine
    // instructions to the virtual page
    /*printf("printf - dst = %x\n", dst - (void*)&printf);*/
    int (*f)(void(*)(char *, ...));
    f = dst;

    // verify that the function was correctly copied
    if (memcmp(dst, f, fsize) != 0) {
        printf("moved memory not equal to original\n");
        return -1;
    }

    // execute native and memory bound function
    printf("native ret: %d\n", fn1(&printf));
    printf("mem ret:    %d\n", f(&printf));

    return 0;
}
