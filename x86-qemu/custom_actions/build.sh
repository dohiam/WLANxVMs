#!/bin/bash
## @file custom_actions/build.sh
## @brief builds the custom package specified in the first argument
## @details TBD
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

#set -x

CLFS_ERROR_FILE="${CLFSPROJ}/status/this_file_indicates_an_error_occured"
touch $CLFS_ERROR_FILE
case $1 in
  "nl") cd ${CLFSPROJ}/custom_packages/nl
        make $2
		;;
  "gnl") cd ${CLFSPROJ}/custom_packages/gnl
        make $2
		;;
  "gnl2") cd ${CLFSPROJ}/custom_packages/gnl2
        make $2
		;;
  "wmedium") cd ${CLFSPROJ}/custom_packages/wmedium
        make $2
		if [ -z ${2+x} ]; then 
  		cp ${CLFSPROJ}/custom_packages/wmedium/wmediumd/wmediumd /home/david/xc/x86-qemu/custom_packages/wmedium/wmediumd/codeblocks/wmediumd/bin/Debug/wmediumd
		  cp ${CLFSPROJ}/custom_packages/wmedium/wmediumd/wmediumd.o /home/david/xc/x86-qemu/custom_packages/wmedium/wmediumd/codeblocks/wmediumd/bin/Debug/wmediumd.o
		  cp ${CLFSPROJ}/custom_packages/wmedium/wmediumd/config.o /home/david/xc/x86-qemu/custom_packages/wmedium/wmediumd/codeblocks/wmediumd/bin/Debug/config.o
		  cp ${CLFSPROJ}/custom_packages/wmedium/wmediumd/probability.o /home/david/xc/x86-qemu/custom_packages/wmedium/wmediumd/codeblocks/wmediumd/bin/Debug/probability.o
		  cp ${CLFSPROJ}/custom_packages/wmedium/wmediumd/mac_address.o /home/david/xc/x86-qemu/custom_packages/wmedium/wmediumd/codeblocks/wmediumd/bin/Debug/mac_address.o
    fi
		;;
  "bear_linux") cd ${CLFSPROJ}/build/linux
  		${CLFSPROJ}/../x86-qemu/custom_actions/bear_linux
		;;
  "lisa") cd ${CLFSPROJ}/custom_packages/lisa
  		make clean
  		make 
		;;
  "ci") echo "copying prebuilt cross_injector "
#  		cp ${CLFSPROJ}/custom_packages/cross_injector/bin/Debug/cross_injector ${CLFSPROJ}/targetfs/root/ci
  		cp ${CLFSPROJ}/custom_packages/cross_injector/bin/Release/cross_injector ${CLFSPROJ}/targetfs/root/ci
		;;

  "hwsim") echo "building hwsim " $2
  		cd ${CLFSPROJ}/custom_packages/mac80211_hwsim
  		make $2
		;;

  *) echo "Invalid action. Do you need to update $0 after adding a new custom package?"
     exit -1
	 ;;
esac
if [ $? -ne 0 ]; then echo "exiting due to failure"; exit $?; fi
rm -f $CLFS_ERROR_FILE  
