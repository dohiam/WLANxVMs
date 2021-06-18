#!/bin/bash
## @file gdb/config.sh
## @brief set the source URL & file, and target build directory for gdb
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

PACKAGE_NAME="gdb"
VERSION="-8.2"
TAR=".tar.xz"
DOWNLOAD_URL="https://ftp.gnu.org/gnu/gdb/gdb-8.2.tar.xz"
SOURCE_FILE="$PACKAGE_NAME$VERSION$TAR"
PACKAGE_DIRECTORY="$PACKAGE_NAME$VERSION"
