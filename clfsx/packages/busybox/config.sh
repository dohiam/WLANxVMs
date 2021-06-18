#!/bin/bash
## @file busybox/config.sh
## @brief set the source URL & file, and target build directory for bootscripts
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

PACKAGE_NAME="busybox"
VERSION="-1.24.2"
TAR=".tar.bz2"
DOWNLOAD_URL="https://busybox.net/downloads/busybox-1.24.2.tar.bz2"
SOURCE_FILE="$PACKAGE_NAME$VERSION$TAR"
PACKAGE_DIRECTORY="$PACKAGE_NAME$VERSION"
