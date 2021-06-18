#!/bin/bash
## @file iana-etc/config.sh
## @brief set the source URL & file, and target build directory for iana-etc
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

PACKAGE_NAME="iana-etc"
VERSION="-2.30"
TAR=".tar.bz2"
DOWNLOAD_URL="http://sethwklein.net/iana-etc-2.30.tar.bz2"
SOURCE_FILE="$PACKAGE_NAME$VERSION$TAR"
PACKAGE_DIRECTORY="$PACKAGE_NAME$VERSION"
PATCH_URL="http://patches.clfs.org/embedded-dev/iana-etc-2.30-update-2.patch"
PATCH_FILE="iana-etc-2.30-update-2.patch"