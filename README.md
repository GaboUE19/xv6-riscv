# Tarea 2 — Grupo H  
## Integrantes: Almendra Aedo - Gabriel Basualto  

## Introducción  

El sistema operativo **xv6** es una versión educativa de Unix desarrollada para la arquitectura **RISC-V**.  

El objetivo de esta tarea fue **modificar el planificador de procesos de xv6** reemplazando el algoritmo Round Robin por **Lottery Scheduling**, un método probabilístico que asigna la CPU en función de un número de “tickets” que cada proceso posee.  

Además, se debía implementar una nueva llamada al sistema denominada `settickets(int n)` para permitir que los procesos de usuario ajusten su cantidad de tickets, y un contador de ejecución `cpu_slices` que registre cuántas veces el scheduler selecciona cada proceso.  

Para validar el funcionamiento, se desarrolló un programa de usuario llamado **`demo`**, que crea múltiples procesos con distinta cantidad de tickets, permitiendo observar la proporcionalidad entre los tickets y el uso de CPU.

---

## Conceptos teóricos  

- **Scheduler:** componente del sistema operativo que decide qué proceso se ejecutará en la CPU.  
- **Round Robin:** algoritmo que asigna la CPU de manera equitativa por turnos fijos a todos los procesos listos.  
- **Lottery Scheduling:** asigna a cada proceso un número de “boletos” (tickets); el scheduler realiza sorteos aleatorios, donde la probabilidad de ganar es proporcional a la cantidad de tickets.  
- **Syscall:** interfaz que permite a los programas de usuario solicitar servicios al kernel. En este caso, `settickets()` permite modificar el número de tickets del proceso.  
- **Proporcionalidad:** la fracción del tiempo de CPU que recibe un proceso tiende a ser proporcional a su cantidad de tickets a lo largo del tiempo.

### Archivos clave modificados

| Archivo        | Función principal |
|----------------|------------------|
| `proc.h`       | Estructura `struct proc`, donde se agregaron los campos `tickets` y `cpu_slices`. |
| `proc.c`       | Implementación del scheduler y del manejo de procesos (`allocproc`, `kexit`, etc.). |
| `sysproc.c`    | Implementación de la syscall `settickets()`. |
| `syscall.h`    | Asignación del número de syscall a `SYS_settickets`. |
| `syscall.c`    | Asociación de la syscall con su función del kernel. |
| `user.h`       | Prototipo de la función `settickets()` accesible desde usuario. |
| `usys.pl`      | Generación automática del stub `entry("settickets")`. |
| `Makefile`     | Inclusión del programa de prueba `_demo`. |
| `user/demo.c`  | Programa de usuario para probar el funcionamiento del Lottery Scheduling. |

---

## Desarrollo  

### 1. Campos agregados al proceso  

En `kernel/proc.h`, dentro de la estructura `struct proc`, se añadieron los campos:

```c
int tickets;       // Cantidad de tickets asignados al proceso
int cpu_slices;    // Número de veces que el proceso fue seleccionado por el scheduler
```
Inicialización en allocproc() (kernel/proc.c):

```c
p->tickets = 100;
p->cpu_slices = 0;
```

### 2) Syscall settickets(int n)

Implementación en kernel/sysproc.c:

```c
uint64
sys_settickets(void)
{
  int n;
  argint(0, &n);        // En esta versión de xv6, argint no retorna valor
  if(n < 1) n = 1;

  struct proc *p = myproc();
  acquire(&p->lock);
  p->tickets = n;
  release(&p->lock);

  return 0;
}
```

Registro:

```c
kernel/syscall.h:

#define SYS_settickets 24
```

kernel/syscall.c:

```c

extern uint64 sys_settickets(void);
...
[SYS_settickets] sys_settickets,
```

user/user.h:

```c
int settickets(int);
```

user/usys.pl:

```c
entry("settickets");
```
### 3) Scheduler por Lottery

En kernel/proc.c::scheduler() se reemplaza Round Robin por Lottery:

```c
for(;;){
  intr_on();

  // 1) Sumar tickets de RUNNABLE
  int total = 0;
  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->state == RUNNABLE){
      int t = (p->tickets < 1) ? 1 : p->tickets;
      total += t;
    }
    release(&p->lock);
  }
  if(total == 0)
    continue;

  // 2) Número aleatorio en [1, total]
  static uint randstate = 1;
  randstate = randstate * 1664525 + 1013904223;
  int r = (randstate % total) + 1;

  // 3) Seleccionar ganador por acumulación
  int acc = 0;
  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->state == RUNNABLE){
      int t = (p->tickets < 1) ? 1 : p->tickets;
      acc += t;
      if(acc >= r){
        p->state = RUNNING;
        p->cpu_slices++;
        c->proc = p;
        swtch(&c->context, &p->context);
        c->proc = 0;
        release(&p->lock);
        break;
      }
    }
    release(&p->lock);
  }
}
```
### 4) Log de ejecución al terminar

En kernel/proc.c::kexit() se agregó antes de marcar ZOMBIE:

```c
printf("proc pid=%d tickets=%d slices=%d\n",
       p->pid, p->tickets, p->cpu_slices);
```

### 5) Programa de prueba demo.c
```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static void burn(unsigned iters){
  volatile uint x = 0;
  for (unsigned i = 0; i < iters; i++)
    x = x * 1664525u + 1013904223u;
}

int main(int argc, char **argv){
  int N = 10;
  if(argc > 1) N = atoi(argv[1]);

  for(int i = 0; i < N; i++){
    int pid = fork();
    if(pid == 0){
      settickets(50*(i+1));   // 50, 100, 150, ..., 500
      burn(40000000);
      printf("Proceso %d terminado (tickets=%d)\n", getpid(), 50*(i+1));
      exit(0);
    }
  }

  for(int k = 0; k < N; k++)
    wait(0);

  exit(0);
}
```

Makefile: añadir _demo a UPROGS.

## Resultados

Ejecución en xv6:
```c
$ demo 10
Proceso 7 terminado (tickets=200)
proc pid=7 tickets=200 slices=22
Proceso 13 terminado (tickets=500)
proc pid=13 tickets=500 slices=53
Proceso 11 terminado (tickets=450)
proc pid=11 tickets=450 slices=50
Proceso 4 terminado (tickets=50)
proc pid=4 tickets=50 slices=5
...
```
## Interpretación:

| Tickets | Slices | Relación aproximada |
| ------- | ------ | ------------------- |
| 50      | 5      | 1×                  |
| 100     | 10     | 2×                  |
| 200     | 22     | 4×                  |
| 400–500 | 50–55  | 8–10×               |

Se observa que los procesos con más tickets reciben más CPU, cumpliendo el comportamiento proporcional esperado del Lottery Scheduling.
Las variaciones menores son producto de la naturaleza aleatoria del algoritmo.

## Problemas encontrados

| Problema                             | Causa                                                    | Solución                                                                |
| ------------------------------------ | -------------------------------------------------------- | ----------------------------------------------------------------------- |
| Error `argint` no retornaba valor    | En la versión actual `argint()` devuelve `void`.         | Se eliminó la comparación `< 0` y se usó `argint(0, &n);` directamente. |
| Conflictos en `scheduler()`          | Se dejaron bloques del Round Robin junto con el Lottery. | Se eliminó el loop original y se mantuvo solo el nuevo.                 |
| Error de formato en `printf`         | `cpu_slices` era `uint64` y `printf` esperaba `int`.     | Se cambió su tipo a `int` en `proc.h`.                                  |
| Kernel panic por llaves mal cerradas | Se revisaron las funciones y se equilibraron los `{}`.   | Corrección manual del bloque de código.                                 |

## Conclusión

La implementación del Lottery Scheduling en xv6 permitió comprender a fondo el funcionamiento del planificador de procesos y el manejo de concurrencia en el kernel.

El algoritmo desarrollado reemplazó exitosamente al esquema Round Robin, otorgando tiempo de CPU en proporción al número de tickets asignados a cada proceso.

Los resultados obtenidos confirman el correcto funcionamiento del nuevo scheduler y la syscall settickets(), cumpliendo todos los requerimientos de la tarea y demostrando la proporcionalidad esperada entre tickets y tiempo de ejecución.


## Evidencia

1. Ejecución del comando demo en xv6:
<img width="1920" height="1032" alt="image" src="https://github.com/user-attachments/assets/8c054d6e-78f2-4a7a-bff7-3e289c930cfd" />


2. Scheduler modificado en proc.c:
![7cdd0527cf324090807b4d6188ba390f 1](https://github.com/user-attachments/assets/9dec8362-e470-4268-a409-0aaf254b0729)


3. Resultado del kernel al finalizar cada proceso:
![Imagen de WhatsApp 2025-10-30 a las 11 18 27_f9bc4fea](https://github.com/user-attachments/assets/37c35b40-645d-47fa-98ea-064006bdde8e)




