#!/bin/bash
## @file CUSTOMIN/CUSTOM_ACTION.sh
## @brief just redirects back to the projec directory's custom_actions 
## @details Rationale: all build actions are invoked from clfsx/packages directories, but it would be better if any 
# custom actions didn't have to be copied into this area but rather could stay in the project area. This redirect allows for that.
# Note that this is different from CUSTOM/CUSTOM_ACTION in that this version is to be used when a minimal environment is needed (set by the recipe file).
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

$CLFSPROJ/custom_actions/$1.sh $2 $3