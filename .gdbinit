define target hookpost-remote
#file "~/CLionProjects/mcu/build/mcu.elf"
set print pretty on
monitor reset
monitor halt
load
break main
end
