# Tarea 1 — Grupo H
## Integrantes: Almendra Aedo - Gabriel Basualto

## Introducción
El sistema operativo **xv6** es una versión educativa de Unix desarrollada para la arquitectura **RISC-V**.

El objetivo de esta tarea fue extender xv6 mediante la implementación de dos nuevas llamadas al sistema: `getppid()` y `getancestor(int n)`. Posteriormente, se debía probar su correcto funcionamiento a través de un programa de usuario denominado **`yosoytupadre`**.

---

## Conceptos teóricos

- **Syscall (llamada al sistema):** mecanismo que permite que un programa en espacio de usuario solicite servicios al kernel.  
- **PID / PPID:** identificadores de procesos. `PID` corresponde al proceso actual, mientras que `PPID` identifica al proceso padre.  
- **Ancestro de proceso:** proceso que forma parte de la cadena de padres hasta llegar a `init`.  
- **User space vs Kernel space:** separación entre ejecución de programas de usuario y operaciones privilegiadas del kernel.  

### Archivos clave de xv6
| Archivo       | Función principal |
|---------------|------------------|
| `syscall.h`   | Define los números de las syscalls. |
| `syscall.c`   | Contiene la tabla que enlaza números de syscall con funciones del kernel. |
| `sysproc.c`   | Implementa la lógica de llamadas al sistema relacionadas con procesos. |
| `user.h`      | Contiene prototipos accesibles para los programas de usuario. |
| `usys.pl`     | Genera wrappers de syscalls para el espacio de usuario. |
| `Makefile`    | Define qué programas de usuario se compilan en la imagen del SO. |

---

## Desarrollo

### Implementación de `getppid`
Se agregó la función en `sysproc.c`:

```c
uint64
sys_getppid(void) {
  struct proc *p = myproc();
  if(p->parent)
    return p->parent->pid;
  return -1;
}
```

### Implementación de getancestor(int n) (también en sysproc.c)

```c
uint64
sys_getancestor(void) {
  int n;
  if(argint(0, &n) < 0) return -1;

  struct proc *p = myproc();
  if(n < 0) return -1;

  while(n > 0 && p->parent){
    p = p->parent;
    n--;
  }
  return (n == 0) ? p->pid : -1;
}
```

## Cambios en el sistema

| Archivo       | Modificación realizada                                        |
|---------------|---------------------------------------------------------------|
| `syscall.h`   | Añadidos `SYS_getppid` y `SYS_getancestor`.                   |
| `syscall.c`   | Se declararon `extern` y se agregaron en la tabla `syscalls[]`.|
| `sysproc.c`   | Implementación de ambas funciones.                            |
| `user.h`      | Prototipos visibles en espacio de usuario.                    |
| `usys.pl`     | Se añadieron `entry("getppid")` y `entry("getancestor")`.     |
| `Makefile`    | Se agregó `_yosoytupadre` en `UPROGS`.                        |


### Programa de prueba

yosoytupadre.c:

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void) {
  printf("Mi PID=%d, PPID=%d\n", getpid(), getppid());
  printf("Ancestro 0: %d\n", getancestor(0));
  printf("Ancestro 1: %d\n", getancestor(1));
  printf("Ancestro 2: %d\n", getancestor(2));
  printf("Ancestro 99: %d\n", getancestor(99));
  exit(0);
}

```
## Problemas encontrados

| Problema                                | Causa                           | Solución                                                   |
|-----------------------------------------|---------------------------------|------------------------------------------------------------|
| Toolchain RISC-V no detectado           | `riscv64-unknown-elf-gcc` no instalado | Instalación y configuración correcta del compilador         |
| Error en tabla `syscalls[]`             | Falta de comas o llaves         | Revisión y corrección de sintaxis                          |
| `sys_getppid` / `sys_getancestor` no declarados | Faltaba `extern` en `syscall.c` | Se agregaron declaraciones externas                        |
| `argint`, `argaddr`, `argstr` como `void` | Prototipos incorrectos en `defs.h` | Se corrigieron a `int` y se ajustaron las implementaciones |

### Ejecución en xv6:

$ yosoytupadre
Mi PID=3, PPID=2
Ancestro 0: 3
Ancestro 1: 2
Ancestro 2: 1
Ancestro 99: -1

![Imagen de WhatsApp 2025-09-29 a las 11 43 36_d6ef8488](https://github.com/user-attachments/assets/5d8bcd7b-3c59-4aac-a672-682739c2bdbb)


## Conclusión

La tarea permitió comprender de forma práctica cómo xv6 maneja las llamadas al sistema y la gestión de procesos. Se logró extender el sistema operativo correctamente, añadiendo las funciones getppid() y getancestor(), además de probarlas con un programa de usuario.

El resultado obtenido demuestra que los cambios cumplen la especificación solicitada. Asimismo, los problemas enfrentados durante el desarrollo contribuyeron a profundizar el entendimiento de la relación entre prototipos, declaraciones externas y consistencia del código en xv6.


## Evidencia

Ruteo entre archivos modificados:
![Imagen de WhatsApp 2025-09-29 a las 11 49 29_80f0bf75](https://github.com/user-attachments/assets/4bc2a02b-5115-4854-ae5b-16828f78d1ab)

Makefile:
![Imagen de WhatsApp 2025-09-29 a las 11 16 05_e9fcc23d](https://github.com/user-attachments/assets/ae65c03b-4e55-43a5-b20e-af28c9834c05)

Resolución de errores de declaración (de void a int por características de xv6):
![Imagen de WhatsApp 2025-09-29 a las 11 37 02_9ab6ed6f](https://github.com/user-attachments/assets/5d6a6f43-b612-4e03-82e9-13ca85e54356)

