#!/usr/bin/expect
set timeout 10
spawn telnet localhost 4444
expect ">" 
send "reset init\n"
expect ">"
send "flash write_image erase [lindex $argv 0] 0x8000000\n"
expect ">"
send "reset halt\n"
expect ">"
send "arm semihosting enable\n"
expect ">"
send "exit\n"
expect off

