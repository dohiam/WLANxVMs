#!/bin/bash
## @file x86-qemu/get_ci_symbols.sh
## @brief displays symbol table for the cross injector debug executable
## @details intended purpose is to help with debugging things like segfaults
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

objdump -t custom_packages/cross_injector/bin/Debug/cross_injector > objdata.txt
vi objdata.txt
