#!/usr/bin/expect -f
set dest [lindex $argv 0];
set timeout -1
spawn telnet localhost $dest
match_max 100000
expect -exact "\[clfs\] login: "
send -- "root\r"
expect "~ # "
send -- "poweroff\r"
expect "~ # Connection closed by foreign host."
expect eof
