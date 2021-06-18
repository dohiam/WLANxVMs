#!/bin/bash
## @file libnl/config.sh
## @brief set the source URL & file, and target build directory for libnl
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

PACKAGE_NAME="libnl"
VERSION="-3.2.25"
TAR=".tar.gz"
DOWNLOAD_URL="http://www.infradead.org/~tgr/libnl/files/libnl-3.2.25.tar.gz"
SOURCE_FILE="$PACKAGE_NAME$VERSION$TAR"
PACKAGE_DIRECTORY="$PACKAGE_NAME$VERSION"
