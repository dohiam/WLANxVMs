#!/bin/bash
## @file strace/config.sh
## @brief set the source URL & file, and target build directory for strace
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

PACKAGE_NAME="strace"
VERSION="-5.6"
TAR=".tar.xz"
DOWNLOAD_URL="https://github.com/strace/strace/releases/download/v5.6/strace-5.6.tar.xz"
SOURCE_FILE="$PACKAGE_NAME$VERSION$TAR"
PACKAGE_DIRECTORY="$PACKAGE_NAME$VERSION"
