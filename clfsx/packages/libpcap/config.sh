#!/bin/bash
## @file libpcap/config.sh
## @brief set the source URL & file, and target build directory for libpcap
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

PACKAGE_NAME="libpcap"
VERSION="-1.9.1"
TAR=".tar.gz"
DOWNLOAD_URL="https://www.tcpdump.org/release/libpcap-1.9.1.tar.gz"
SOURCE_FILE="$PACKAGE_NAME$VERSION$TAR"
PACKAGE_DIRECTORY="$PACKAGE_NAME$VERSION"
