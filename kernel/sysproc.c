#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if (argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0; // not reached
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
  if (argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if (argint(0, &n) < 0)
    return -1;

  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if (argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (myproc()->killed)
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// #ifdef LAB_PGTBL
int sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  uint64 va;
  int npages;
  uint64 ua;
  uint64 a;
  struct proc *p = myproc(); 

  argaddr(0, &va);
  argint(1, &npages);
  argaddr(2, &ua);
  uint64 bitmasks[npages/64+1];
  uint64 mask = 1L;
  memset(bitmasks, 0, sizeof bitmasks);

  //go through PTE, unset PTE_A if set
  uint64 downva = PGROUNDDOWN(va);
  int index = 0;
  pte_t *pte;
  for(a=downva; a<downva+npages*PGSIZE; a+=PGSIZE){
    pte = walk(p->pagetable, a, 0);
    if(*pte & PTE_A){
      *pte &= ~PTE_A;
      bitmasks[index] |= mask;
    }
    mask <<= 1;
    if(mask == 0){
      index++;
      mask = 1L;
    }
  }

  //copy bit stream to userspace
  if(copyout(p->pagetable, ua, (char*)bitmasks, npages/8+8) < 0){
    return -1;
  }
  return 0;
}
// #endif

uint64
sys_kill(void)
{
  int pid;

  if (argint(0, &pid) < 0)
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
