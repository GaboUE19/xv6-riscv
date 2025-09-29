// kernel/sysproc.c
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

// ---------------- syscalls ya existentes ----------------

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
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
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

// Nota: en tu árbol, sbrk recibe 2 arg: n y t (SBRK_EAGER / lazy)
uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0){
    if(growproc(n) < 0){
      return -1;
    }
  } else {
    // Asignación perezosa: solo aumenta sz; vmfault() asignará memoria.
    if(addr + n < addr)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
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
  argint(0, &pid);
  return kkill(pid);
}

// ticks desde el arranque
uint64
sys_uptime(void)
{
  uint xticks;
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// ---------------- NUEVAS syscalls ----------------

// getppid(): retorna el PID del padre o -1 si no existe.
uint64
sys_getppid(void)
{
  struct proc *p = myproc();
  int ppid = -1;

  acquire(&p->lock);
  if(p->parent){
    acquire(&p->parent->lock);
    ppid = p->parent->pid;
    release(&p->parent->lock);
  }
  release(&p->lock);
  return ppid;
}

// getancestor(n): 0->yo, 1->padre, 2->abuelo, ...; -1 si no existe.
// OJO: en esta versión de xv6, argint() no devuelve valor.
uint64
sys_getancestor(void)
{
  int n;

  // leer argumento 0
  argint(0, &n);

  // validar
  if(n < 0)
    return -1;

  struct proc *cur = myproc();
  acquire(&cur->lock);

  while(n > 0){
    if(cur->parent == 0){
      release(&cur->lock);
      return -1;
    }
    struct proc *par = cur->parent;
    acquire(&par->lock);
    release(&cur->lock);
    cur = par;
    n--;
  }

  int pid = cur->pid;
  release(&cur->lock);
  return pid;
}

