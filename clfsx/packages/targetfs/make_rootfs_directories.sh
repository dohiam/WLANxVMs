#!/bin/bash
## @file targetfs/make_rootfs_directories.sh
## @brief creates the targetfs folder structure, making it ready for files to go into
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

# if argument passed then recipe is run for a specific purpose so skip out
if [[ $# -gt 0 ]]; then exit 0; fi
echo "making rootfs directories"

. ${CLFS}/scripts/common.sh

CD=`dirname "$0"`
PACKAGE_NAME=`basename $CD`
STEP=`basename $0`
touch $CLFS_ERROR_FILE
rm -rf ${CLFS_TARGETFS_DIR}
mkdir -p ${CLFS_TARGETFS_DIR}
mkdir -p ${CLFS_TARGETFS_DIR}/bin
mkdir -p ${CLFS_TARGETFS_DIR}/boot
mkdir -p ${CLFS_TARGETFS_DIR}/dev
mkdir -p ${CLFS_TARGETFS_DIR}/etc/rc.d/start
mkdir -p ${CLFS_TARGETFS_DIR}/home
mkdir -p ${CLFS_TARGETFS_DIR}/lib/firmware
mkdir -p ${CLFS_TARGETFS_DIR}/lib/modules
mkdir -p ${CLFS_TARGETFS_DIR}/mnt
mkdir -p ${CLFS_TARGETFS_DIR}/opt
mkdir -p ${CLFS_TARGETFS_DIR}/proc
mkdir -p ${CLFS_TARGETFS_DIR}/sbin
mkdir -p ${CLFS_TARGETFS_DIR}/srv
mkdir -p ${CLFS_TARGETFS_DIR}/sys
mkdir -p ${CLFS_TARGETFS_DIR}/var/cache
mkdir -p ${CLFS_TARGETFS_DIR}/var/lib
mkdir -p ${CLFS_TARGETFS_DIR}/var/local
mkdir -p ${CLFS_TARGETFS_DIR}/var/lock
mkdir -p ${CLFS_TARGETFS_DIR}/var/log
mkdir -p ${CLFS_TARGETFS_DIR}/var/opt
mkdir -p ${CLFS_TARGETFS_DIR}/var/run
mkdir -p ${CLFS_TARGETFS_DIR}/var/spool
mkdir -p ${CLFS_TARGETFS_DIR}/var/tmp
mkdir -p ${CLFS_TARGETFS_DIR}/root
mkdir -p ${CLFS_TARGETFS_DIR}/tmp
sudo install -dv -m 0750 ${CLFS_TARGETFS_DIR}/root
sudo install -dv -m 1777 ${CLFS_TARGETFS_DIR}/{var/,}tmp
mkdir -p ${CLFS_TARGETFS_DIR}/usr/local/bin
mkdir -p ${CLFS_TARGETFS_DIR}/usr/local/include
mkdir -p ${CLFS_TARGETFS_DIR}/usr/local/lib
mkdir -p ${CLFS_TARGETFS_DIR}/usr/local/sbin
mkdir -p ${CLFS_TARGETFS_DIR}/usr/local/share
mkdir -p ${CLFS_TARGETFS_DIR}/usr/local/src
mkdir -p ${CLFS_TARGETFS_DIR}/etc/network/if-post-up.d
mkdir -p ${CLFS_TARGETFS_DIR}/etc/network/if-post-down.d
mkdir -p ${CLFS_TARGETFS_DIR}/etc/network/if-pre-up.d
mkdir -p ${CLFS_TARGETFS_DIR}/etc/network/if-pre-down.d
mkdir -p ${CLFS_TARGETFS_DIR}/etc/network/if-up.d
mkdir -p ${CLFS_TARGETFS_DIR}/etc/network/if-down.d
mkdir -p ${CLFS_TARGETFS_DIR}/usr/share/udhcpc
cd ${CLFS_TARGETFS_DIR}
LOG="${CLFS_STATUS_DIR}/${PACKAGE_NAME}/logfile.txt"
mkdir -p "${CLFS_STATUS_DIR}/${PACKAGE_NAME}"
touch $LOG
chmod ugo+rw $LOG
ln -svf proc/mounts etc/mtab  > $LOG 2>&1
cp ${CD}/files/.passwd ${CLFS_TARGETFS_DIR}/etc/passwd
cp ${CD}/files/.group ${CLFS_TARGETFS_DIR}/etc/group
touch ${CLFS_TARGETFS_DIR}/var/log/lastlog 
chmod -v 664 ${CLFS_TARGETFS_DIR}/var/log/lastlog  > $LOG 2>&1
cp ${CD}/files/.fstab ${CLFS_TARGETFS_DIR}/etc/fstab
cp ${CD}/files/.mdev ${CLFS_TARGETFS_DIR}/etc/mdev.conf
cp ${CD}/files/.profile ${CLFS_TARGETFS_DIR}/etc/profile
cp ${CD}/files/.inittab ${CLFS_TARGETFS_DIR}/etc/inittab
cp ${CD}/files/.hostname ${CLFS_TARGETFS_DIR}/etc/HOSTNAME
cp ${CD}/files/.hosts ${CLFS_TARGETFS_DIR}/etc/hosts
cp ${CD}/files/.interfaces ${CLFS_TARGETFS_DIR}/etc/network/interfaces
cp ${CD}/files/.default.script ${CLFS_TARGETFS_DIR}/usr/share/udhcpc/default.script
chmod +x ${CLFS_TARGETFS_DIR}/usr/share/udhcpc/default.script
cd ${CLFS_TARGETFS_DIR}/dev
sudo mknod -m 622 console c 5 1
sudo mknod -m 622 tty0 c 4 0
set_status $PACKAGE_NAME $STEP "completed_successfully"
rm -f $CLFS_ERROR_FILE