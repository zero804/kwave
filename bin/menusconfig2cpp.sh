#!/bin/sh
############################################################################
#   menusconfig2cpp - stores entries menusconfig into a cpp file
#                            -------------------
#   begin                : Tue Mar 14 2000
#   copyright            : (C) 2000 by Thomas Eschenbacher
#   email                : Thomas.Eschenbacher@gmx.de
############################################################################
#
############################################################################
#                                                                          #
#    This program is free software; you can redistribute it and/or modify  #
#    it under the terms of the GNU General Public License as published by  #
#    the Free Software Foundation; either version 2 of the License, or     #
#    (at your option) any later version.                                   #
#                                                                          #
############################################################################
#
# 2000-03-14 by Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
# 2002-02-08 fixed support for macros
#
# This very kwave-specific script filters the entries of the menu
# configuration, creates a cpp file with a list of menu tokens
# and encapsulates each of them with the i18n() macro. The resulting
# file will not produce any executable code but all of the menu entries
# will get into the files for internationalization.
#
# parameters:
# $1 full path to the "menus.config" file
# $2 output file
#
# NOTE: this should be regarded to be a quick hack, only few error checking
#       is performed !!!

# uncomment this for debugging
# set -x

cat > $2 << EOF_HEADER
/*
 * this file was automatically generated by "`basename $0`"
 * at "`(LANG=en;date)`" -> DO NOT EDIT !
 */

#include <klocale.h> // (for i18n macro)

static void dummy(const char * /*string_to_be_internationalized */) { }

void __menusconfig_2_cpp_internationalize_all()
{
EOF_HEADER

cat $1 | awk ' \
    { level=0; } \
    { pos=0; } \
    { line=""; \
    for (pos=0; pos < length($0); pos++) { \
        c = substr($0,pos,1); \
	if (c == "(") level++; \
	else if (c == ")") level--; \
	else if (level == 1) { \
	    if (c == ",,") level=1; \
	    else line = line c; \
	} \
    } \
    if (length(line) > 0) { \
	nparams=split(line, params, ","); \
	param = params[2]; \
        pos = index(param, "#"); \
	if (pos != 0) param = substr(param,0,pos-1); \
        ntokens = split(param, tokens, "/"); \
        for (j=1; j <= ntokens; j++) { \
            if (length(tokens[j]) != 0) print tokens[j]; \
	} \
    } \
    }' \
    | \
   sort | \
   uniq |
   awk '{print("    dummy(i18n(\"" $0 "\"));") }' \
   >> $2

cat >> $2 << EOF_FOOT
}
/* end of file */
EOF_FOOT
# EOF
