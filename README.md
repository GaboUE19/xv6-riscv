Tarea 2 — Grupo H
Integrantes: Almendra Aedo - Gabriel Basualto
Introducción

El sistema operativo xv6 es una versión educativa de Unix desarrollada para la arquitectura RISC-V.

El objetivo de esta tarea fue modificar el planificador de procesos de xv6 reemplazando el algoritmo Round Robin por Lottery Scheduling, un método probabilístico que asigna la CPU en función de un número de “tickets” que cada proceso posee.

Además, se debía implementar una nueva llamada al sistema denominada settickets(int n) para permitir que los procesos de usuario ajusten su cantidad de tickets, y un contador de ejecución cpu_slices que registre cuántas veces el scheduler selecciona cada proceso.

Para validar el funcionamiento, se desarrolló un programa de usuario llamado demo, que crea múltiples procesos con distinta cantidad de tickets, permitiendo observar la proporcionalidad entre los tickets y el uso de CPU.

Conceptos teóricos

Scheduler: componente del sistema operativo que decide qué proceso se ejecutará en la CPU.

Round Robin: algoritmo que asigna la CPU de manera equitativa por turnos fijos a todos los procesos listos.

Lottery Scheduling: asigna a cada proceso un número de “boletos” (tickets); el scheduler realiza sorteos aleatorios, donde la probabilidad de ganar es proporcional a la cantidad de tickets.

Syscall: interfaz que permite a los programas de usuario solicitar servicios al kernel. En este caso, settickets() permite modificar el número de tickets del proceso.

Proporcionalidad: la fracción del tiempo de CPU que recibe un proceso tiende a ser proporcional a su cantidad de tickets a lo largo del tiempo.

Archivos clave modificados
Archivo	Función principal
proc.h	Estructura struct proc, donde se agregaron los campos tickets y cpu_slices.
proc.c	Implementación del scheduler y del manejo de procesos (allocproc, kexit, etc.).
sysproc.c	Implementación de la syscall settickets().
syscall.h	Asignación del número de syscall a SYS_settickets.
syscall.c	Asociación de la syscall con su función del kernel.
user.h	Prototipo de la función settickets() accesible desde usuario.
usys.pl	Generación automática del stub entry("settickets").
Makefile	Inclusión del programa de prueba _demo.
user/demo.c	Programa de usuario para probar el funcionamiento del Lottery Scheduling.
Desarrollo
1. Campos agregados al proceso

En kernel/proc.h, dentro de la estructura struct proc, se añadieron los campos:

int tickets;       // Cantidad de tickets asignados al proceso
int cpu_slices;    // Número de veces que el proceso fue seleccionado por el scheduler


Inicializados en allocproc() (archivo proc.c):

p->tickets = 100;
p->cpu_slices = 0;

2. Nueva syscall settickets(int n)

En kernel/sysproc.c se implementó la función:

uint64
sys_settickets(void)
{
  int n;
  argint(0, &n);  // Obtiene el argumento desde espacio de usuario
  if(n < 1) n = 1;

  struct proc *p = myproc();
  acquire(&p->lock);
  p->tickets = n;
  release(&p->lock);

  return 0;
}


Se completó el registro de la syscall:

syscall.h: #define SYS_settickets 24

syscall.c: extern uint64 sys_settickets(void); y [SYS_settickets] sys_settickets,

user.h: int settickets(int);

usys.pl: entry("settickets");

3. Modificación del scheduler

El algoritmo Round Robin fue reemplazado por Lottery Scheduling, implementado dentro de scheduler() en kernel/proc.c.
El nuevo flujo consiste en:

Sumar tickets: recorre todos los procesos RUNNABLE y suma sus tickets.

Elegir ganador: genera un número aleatorio r entre 1 y total_tickets.

Seleccionar proceso: recorre nuevamente acumulando tickets hasta alcanzar r, eligiendo ese proceso para ejecutar.

Actualizar contadores: incrementa cpu_slices del proceso ganador y lo ejecuta.

Fragmento principal del código:

for(;;){
  intr_on();

  int total = 0;
  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->state == RUNNABLE)
      total += (p->tickets < 1 ? 1 : p->tickets);
    release(&p->lock);
  }

  if(total == 0)
    continue;

  static uint randstate = 1;
  randstate = randstate * 1664525 + 1013904223;
  int r = (randstate % total) + 1;

  int acc = 0;
  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->state == RUNNABLE){
      acc += (p->tickets < 1 ? 1 : p->tickets);
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

4. Registro de ejecución

Dentro de kexit() (en proc.c), se imprimió al finalizar cada proceso:

printf("proc pid=%d tickets=%d slices=%d\n", p->pid, p->tickets, p->cpu_slices);


Esto permitió visualizar en la consola del kernel cuántas veces fue elegido cada proceso.

5. Programa de prueba demo.c
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
      settickets(50*(i+1));
      burn(40000000);
      printf("Proceso %d terminado (tickets=%d)\n", getpid(), 50*(i+1));
      exit(0);
    }
  }

  for(int k = 0; k < N; k++)
    wait(0);

  exit(0);
}


El programa crea 10 procesos hijos con tickets crecientes desde 50 hasta 500 y los hace ejecutar una carga de CPU idéntica.

Resultados

Ejecución del comando en xv6:

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

Interpretación
Tickets	Slices	Relación aproximada
50	5	1×
100	10	2×
200	22	4×
400–500	50–55	8–10×

Se observa que los procesos con más tickets reciben más CPU, cumpliendo el comportamiento proporcional esperado del Lottery Scheduling.
Las variaciones menores son producto de la naturaleza aleatoria del algoritmo.

Problemas encontrados
Problema	Causa	Solución
Error argint no retornaba valor	En la versión actual argint() devuelve void.	Se eliminó la comparación < 0 y se usó argint(0, &n); directamente.
Conflictos en scheduler()	Se dejaron bloques del Round Robin junto con el Lottery.	Se eliminó el loop original y se mantuvo solo el nuevo.
Error de formato en printf	cpu_slices era uint64 y printf esperaba int.	Se cambió su tipo a int en proc.h.
Kernel panic por llaves mal cerradas	Se revisaron las funciones y se equilibraron los {}.	Corrección manual del bloque de código.
Conclusión

La implementación del Lottery Scheduling en xv6 permitió comprender a fondo el funcionamiento del planificador de procesos y el manejo de concurrencia en el kernel.

El algoritmo desarrollado reemplazó exitosamente al esquema Round Robin, otorgando tiempo de CPU en proporción al número de tickets asignados a cada proceso.

Los resultados obtenidos confirman el correcto funcionamiento del nuevo scheduler y la syscall settickets(), cumpliendo todos los requerimientos de la tarea y demostrando la proporcionalidad esperada entre tickets y tiempo de ejecución.
