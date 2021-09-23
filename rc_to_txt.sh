#!/bin/bash

#iconv "$1" -f UTF-16 -t UTF-8 | grep " IDS_" | sort | awk 'BEGIN {id = ""; str = ""} {id = id $1 ",\n"; $1=""; str = str substr($0, 1, length($0)-1) ",\n"} END { print id; print str;}'
iconv "$1" -f UTF-16 -t UTF-8 | sort | grep -e " IDS_" | awk '
BEGIN{ id = "enum: int {\n"; str = "void InitResStr()\n{\n"; }
{
    id = id "    " $1 ",\n"; 
    idds = $1; $1=""; 
    str = str "    res_str[" idds "] = QObject::tr(" substr($0, 1, length($0)-1) ");\n"; 
}
 
END{ print "#include <QHash>\n#include <QObject>\n"; print id "};\n\nQHash <int, const char *> res_str;\n\n"; print str; print "}";}'
