#!/usr/bin/expect -f
## @file send_cmd.sh
## @brief used to send commands to a given tmux window to drive an overall simulation
#
set dest 666[lindex $argv 0];
set a1 [lindex $argv 1];
set a2 [lindex $argv 2];
set a3 [lindex $argv 2];
set timeout -1
spawn telnet localhost $dest
match_max 100000
expect -exact "\[clfs\] login: "
send -- "root\r"
expect "~ # "
send -- "$a1 $a2 $a3 \r"
expect "~ # "
send -- "exit\r"
expect eof

