#!/bin/bash
## @file build_packages.sh
## @brief builds packages indicated by input file
## @details input file needs to have one package per line; the package recipe file indicates what steps to use to build that package
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

#set -x
if [[ $# -ne 1 ]]; then
  echo "Usage: $0 <file with list of packages, one package per line>"
  exit -1
fi
while IFS= read -r line
do
  arr=($line)
  PACKAGE_NAME=${arr[0]}
  $CLFS/scripts/run_recipe_file.sh $PACKAGE_NAME $CLFS/packages/$PACKAGE_NAME/recipe ${arr[1]} ${arr[2]} ${arr[3]}
  if [ -f "${CLFSPROJ}/status/this_file_indicates_an_error_occured" ]; then echo "exiting build"; exit $?; fi
done < $1