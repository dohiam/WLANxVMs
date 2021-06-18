#!/bin/bash

ap_pane="0.{top-left}"
server_pane="0.{top-right}"
client1_pane="0.{bottom-left}"
client2_pane="0.{bottom-right}"

get_client_name() {
	client_text=`tmux list-clients`
	words=( $client_text )
	client_name=${words[0]}
}

cmd_ap() {
	tmux send-keys -t ${client_name}${ap_pane} $1 C-m	
}

cmd_server() {
	tmux send-keys -t ${client_name}${server_pane} $1 C-m 	
}

cmd_client1() {
	tmux send-keys -t ${client_name}${client1_pane} $1 C-m 	
}


cmd_client2() {
	tmux send-keys -t ${client_name}${client2_pane} $1 C-m 	
}


########################################################################################
get_client_name
cmd_server "./tools/bcaster/bin/Release/bcaster"
cmd_ap "./runvm.sh Space 1"
cmd_client1 "./runvm.sh Space 2"
sleep 15
cmd_ap "root"
cmd_client1 "root"
sleep 1
cmd_ap "./start_vsock"
cmd_client1 "./start_vsock"
cmd_client2 "telnet Space localhost Space 6662"
sleep 1
cmd_ap "./start_ap"
cmd_client1 "./start_hwsim0"
cmd_client2 "root"
sleep 1
cmd_ap "./ci Space c"
cmd_client2 ""
sleep 1
cmd_client1 "./ci Space c"
sleep 5
cmd_client2 "./start_sta1"
#sleep 10
#cmd_client2 "iwlist Space wlan1 Space scanning"
#sleep 10
#cmd_client2 "iwconfig"
#sleep 5
#cmd_client2 "ping Space 172.16.16.1"

