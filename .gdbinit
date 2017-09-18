add-auto-load-safe-path /home/sunqi/CLionProjects/mcu/.gdbinit
define target hookpost-remote
file "./build/mcu.elf"
monitor reset
monitor halt
break main
end
