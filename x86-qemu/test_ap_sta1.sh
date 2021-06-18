#!/bin/bash

server_pane="0.0"
client_pane="0.1"

get_client_name() {
	client_text=`tmux list-clients`
	words=( $client_text )
	client_name=${words[0]}
}

cmd_server() {
	tmux send-keys -t ${client_name}${server_pane} $1 C-m	

}

cmd_client() {
	tmux send-keys -t ${client_name}${client_pane} $1 C-m 	

}

########################################################################################
get_client_name
cmd_server "./runvm.sh Space 1"
cmd_client "./runvm.sh Space 2"
sleep 7
cmd_server "root"
cmd_client "root"
sleep 1
cmd_server "./start_vsock"
cmd_client "./start_vsock"
sleep 1
cmd_server "./start_ap"
#cmd_client "./start_sta1"
cmd_client "./start_hwsim0"
cmd_client "ifconfig Space wlan0 Space down"
sleep 1
cmd_server "./ci Space c"
sleep 1
cmd_client "./ci Space c"

