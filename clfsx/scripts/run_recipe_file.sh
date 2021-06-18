#!/bin/bash
## @file run_recipe_file.sh
## @brief runs script files as indicated by the recipe file, stops on error
## @details Note that there are two ways to run a steo. One is to run it in an environment set up for only the cross compilation tools ("minimal") and the second is to
## run it in the normal regular host environment (with some additional environmental variables to point to the right project directories.)
## Also note that the script to execute is just the step name with .sh added to it.
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

. $CLFS/scripts/common.sh
#set -x
PACKAGE_NAME=$1
RECIPE_FILE=$2

while IFS="," read STEP ENVIRONMENT
  do
    STEP="${STEP//[[:space:]]/}"
    ENVIRONMENT="${ENVIRONMENT//[[:space:]]/}"
	if [[ $PACKAGE_NAME == "CLFSPROJ" ]]; then
      cd ${CLFSPROJ}
	  ./${STEP}
      if [ -f $CLFS_ERROR_FILE ]; then
        echo "exiting $PACKAGE_NAME $STEP due to failure"
        exit -1
      fi
    else 
      if [ $ENVIRONMENT == "minimal" ]; then
        $CLFS_SCRIPTS_DIR/run_minimal.sh "$CLFS_PACKAGES_DIR/$PACKAGE_NAME/$STEP.sh $3 $4 $5"
      else
        cd $CLFS_PACKAGES_DIR/$PACKAGE_NAME
        ($CLFS_PACKAGES_DIR/$PACKAGE_NAME/$STEP.sh  $3 $4 $5)
      fi
      if [ -f $CLFS_ERROR_FILE ]; then
        echo "exiting $PACKAGE_NAME $STEP due to failure"
        exit -1
      fi
	fi # CLFSPROJ check
  done < $RECIPE_FILE #reading STEP ENVIRONMENT
