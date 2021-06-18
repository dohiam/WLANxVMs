#!/bin/bash
## @file tcpdump/config.sh
## @brief set the source URL & file, and target build directory for tcpdump
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

PACKAGE_NAME="tcpdump"
VERSION="-4.9.3"
TAR=".tar.gz"
DOWNLOAD_URL="https://www.tcpdump.org/release/tcpdump-4.9.3.tar.gz"
SOURCE_FILE="$PACKAGE_NAME$VERSION$TAR"
PACKAGE_DIRECTORY="$PACKAGE_NAME$VERSION"
