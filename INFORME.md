# Tarea 0 –  GRUPO H

## Computador 1
## Pasos seguidos
1. Activé virtualización en BIOS/UEFI y habilité WSL2 en Windows.
2. Instalé Ubuntu (WSL) y configuré usuario.
3. Instalé varias dependencias en ubunto  como sudo apt install -y qemu-system-misc 
4. accedi a mi repo de windows "/mnt/c/Users/almen/OneDrive/Escritorio/universidad/10mo semestre/Sistemas operativos/xv6-riscv"
5. cree mi rama con  git checkout -b almen_t0
6. ejecute  make qemu
7. hice las pruebas correspondientes( ls, echo  "Hola xv6", cat README)
8. sali de XV6 para crear el informe

## Problemas y soluciones
- “fatal: detected dubious ownership …”*: agregué safe.director en Git.
- No corrio el make quemu asi que instale `gcc-riscv64-unknown-elf

## Conclusión
xv6 se compila y ejecuta correctamente en QEMU bajo WSL2.

## Computador 2

1. Instalé **Ubuntu en Windows** mediante WSL con:
   wsl --install
2. Actualicé los paquetes del sistema con: sudo apt update && sudo apt upgrade -y
3. Instalé las dependencias iniciales: sudo apt install build-essential gdb qemu-system-misc -y
4. Me ubiqué en la dirección del repositorio: cd /mnt/c/Users/gabri/Desktop/GitHubDesktop/xv6-riscv
5. Intenté compilar xv6 con make pero apareció el error "Couldn't find a riscv64 version of GCC/binutils".
6. Instalé el compilador cruzado de RISC-V: sudo apt install gcc-riscv64-unknown-elf qemu-system-misc -y
7. Limpié compilaciones previas con make clean
8. Compilé con make y luego ejecuté xv6 en QEMU con make qemu

## Problemas encontrados y soluciones

Problema 1: Al compilar, el sistema mostraba errores en kernel/entry.S (instrucciones como la, li, csrr no reconocidas).
Causa: No estaba instalado el compilador cruzado para RISC-V.
Solución: Instalé el paquete gcc-riscv64-unknown-elf con apt.

Problema 2: Al principio pensé que el gcc estándar servía, pero luego confirmé que debía instalar el toolchain de RISC-V. Con esa corrección, la compilación fue exitosa.
