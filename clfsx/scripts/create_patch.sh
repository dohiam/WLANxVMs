#!/bin/bash
## @file create_patch.sh
## @brief diffs indicated input files to create patch file in the third argument
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in roocrediffs release mlme.c with the one in the linux package folder to create a patch file

#set -x
if [[ $# -ne 3 ]]; then
  echo "Usage: $0 <old_filename> <new_filename> <patch_filename>"
  exit -1
fi
diff -uNr $1 $2 > $3