#!/bin/bash
## @file x86-qemu/env-vars.sh
## @brief sets environment variables for a project
## @details configure for your root directories
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

echo "setting exports for ${USERNAME} to ${PWD}"
export CLFS="${PWD}/clfsx"
export CLFSPROJ="${PWD}/x86-qemu"
export XTOOLS="${PWD}/x-tools"
