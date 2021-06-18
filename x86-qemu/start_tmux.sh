#!/bin/bash
## @file start_tmux.sh
## @brief starts the main window where the simulation runs, which uses tmux to have multiple coordinated windows
#

tmux kill-session -t ci
tmux new -s ci -d
tmux split-window 
tmux split-window 
tmux split-window 
tmux select-layout tiled
tmux attach
