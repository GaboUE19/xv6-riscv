# Informe – Tarea 1: Nuevas llamadas al sistema en xv6 


#integrantes: Almendra Aedo, Gabriel Basualto (GRUPO H)


## Modificaciones realizadas

1. **`kernel/syscall.h`** → se agregaron `SYS_getppid` y `SYS_getancestor`.
2. **`kernel/syscall.c`** → se registraron en la tabla de syscalls.
3. **`kernel/sysproc.c`** → implementación de ambas funciones con locks para evitar inconsistencias.
4. **`user/user.h`** y **`user/usys.pl`** → prototipos y stubs de usuario.
5. **`user/yosoytupadre.c`** → programa de prueba.
6. **`Makefile`** → añadido `yosoytupadre` a `UPROGS`.

## Dificultades y soluciones
- **`argint` definido como `void`**: no se podía usar en un `if`. → Cambio a `argint(0, &n); if (n < 0) return -1;`.
- **Errores por duplicar funciones**: inicialmente se declaró `sys_getancestor` dos veces. → Se dejó una única definición correcta.


## Conclusión

Las llamadas al sistema fueron implementadas y probadas exitosamente. `getppid` y `getancestor` funcionan según lo esperado, devolviendo los PIDs correctos y manejando casos inválidos. El sistema xv6 quedó extendido con nuevas funcionalidades listas para usarse.

