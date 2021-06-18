#!/bin/bash
## @file common.sh
## @brief variables and functions common to all the build scripts
## @details The main frunction is standard_get_and_unzip which all the download scripts use. This also sets variables to point to all the project 
# directories based on the project directory that is assumed to have been set prior to doing anything with this project.
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

#set +h
umask 022
LC_ALL=POSIX
export LC_ALL 
CLFS_SCRIPTS_DIR="${CLFS}/scripts"
CLFS_PACKAGES_DIR="${CLFS}/packages"
CLFS_SOURCE_DIR="${CLFS}/sources"
CLFS_PATCH_DIR="${CLFS}/patches"
CLFS_BUILD_DIR="${CLFS}/../build/build"
CLFS_SYSROOT_DIR="${XTOOLS}/${CLFS_HOST}/${CLFS_TARGET}/sysroot"
CLFS_CROSSTOOLS_DIR="${XTOOLS}"
CLFS_TARGETFS_DIR="${CLFSPROJ}/targetfs"
CLFS_STATUS_DIR="${CLFSPROJ}/status"
CLFS_PATCHING_DIR="${CLFSPROJ}/patching"
CLFS_ERROR_FILE="${CLFSPROJ}/status/this_file_indicates_an_error_occured"

clean_project() {
  rm -rf ${CLFSPROJ}/build
  rm -rf ${CLFSPROJ}/crosstools
  rm -rf ${CLFSPROJ}/targetfs
  rm -rf ${CLFSPROJ}/status
}

set_status() {
  PACKAGE=$1
  STEP=$2
  MESSAGE=$3
  if [ ! -f ${CLFS_STATUS_DIR}/$PACKAGE ]; then mkdir -p ${CLFS_STATUS_DIR}/$PACKAGE; fi
  echo $MESSAGE >> ${CLFS_STATUS_DIR}/$PACKAGE/$STEP
}

check_status() {
  if [ -f $CLFS_STATUS_DIR/$1/$2 ]; then
    echo true
  else
    echo false
  fi
}

clear_status() {
  PACKAGE=$1
  STEP=$2
  rm -f ${CLFS_STATUS_DIR}/$PACKAGE/$STEP
}

add_include_path() {
  . ${CLFSPROJ}/config.sh
  CFLAGS="${CFLAGS} -I$1"
  eval "sed 's?CFLAGS=\".*\"?CFLAGS=\"$CFLAGS\"?' ${CLFSPROJ}/config.sh > ${CLFS_STATUS_DIR}/config2.tmp"
  cp ${CLFS_STATUS_DIR}/config2.tmp ${CLFSPROJ}/config.sh
  rm ${CLFS_STATUS_DIR}/config2.tmp
}

clear_include_path() {
  . ${CLFSPROJ}/config.sh
  CFLAGS=$CFLAGS_DEFAULT
  eval "sed 's?CFLAGS=\".*\"?CFLAGS=\"$CFLAGS\"?' ${CLFSPROJ}/config.sh > ${CLFS_STATUS_DIR}/config2.tmp"
  cp ${CLFS_STATUS_DIR}/config2.tmp ${CLFSPROJ}/config.sh
  rm ${CLFS_STATUS_DIR}/config2.tmp
}

standard_get_and_unzip() (
  if ($USE_EXISTING_SOURCE); then
    if [ -f $CLFS_SOURCE_DIR/$SOURCE_FILE ]; then
        do_download=false
    else
        do_download=true
    fi
  else
    do_download=true
    if [ -f $CLFS_SOURCE_DIR/$SOURCE_FILE ]; then
      rm -rf $CLFS_SOURCE_DIR/$SOURCE_FILE
    fi
  fi
  if ($do_download); then
    echo "Downloading $PACKAGE_NAME from $DOWNLOAD_URL"
    cd $CLFS_SOURCE_DIR
    wget -c $DOWNLOAD_URL
  else
    echo "Using local source package $SOURCE_FILE"
  fi
  if ($USE_EXISTING_PACKAGE); then
    if [ -d $CLFS_BUILD_DIR/$PACKAGE_NAME ]; then
        do_unzip=false
    else
        do_unzip=true
    fi
  else
    do_unzip=true
    if [ -d $CLFS_BUILD_DIR/$PACKAGE_NAME ]; then
      rm -rf $CLFS_BUILD_DIR/$PACKAGE_NAME
    fi
  fi
  if ($do_unzip); then
    echo "Unzipping $PACKAGE_NAME from $SOURCE_FILE"
    cd $CLFS_BUILD_DIR
    tar -xf $CLFS_SOURCE_DIR/$SOURCE_FILE 
    mv $PACKAGE_DIRECTORY $PACKAGE_NAME
  else
    echo "Using existing package for $PACKAGE_NAME"
  fi  
)