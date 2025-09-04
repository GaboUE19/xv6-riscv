# Tarea 0 –  GRUPO H


## Pasos seguidos
1. Activé virtualización en BIOS/UEFI y habilité WSL2 en Windows.
2. Instalé Ubuntu (WSL) y configuré usuario.
3. Instalé varias dependencias en ubunto  como sudo apt install -y qemu-system-misc 
4. accedi a mi repo de windows "/mnt/c/Users/almen/OneDrive/Escritorio/universidad/10mo semestre/Sistemas operativos/xv6-riscv"
5. cree mi rama con  git checkout -b almen_t0
6. ejecute  make qemu
7. hice las pruebas correspondientes( ls, echo  "Hola xv6", cat README)
8. sali de XV6 para crear el informe

##Problemas y soluciones
- “fatal: detected dubious ownership …”*: agregué safe.director en Git.
- No corrio el make quemu asi que instale `gcc-riscv64-unknown-elf

## Conclusión
xv6 se compila y ejecuta correctamente en QEMU bajo WSL2.
