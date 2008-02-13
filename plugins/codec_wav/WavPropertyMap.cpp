/*************************************************************************
     WavPropertyMap.cpp  -  map for translating properties to chunk names
                             -------------------
    begin                : Sat Jul 06 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "WavPropertyMap.h"

//***************************************************************************
WavPropertyMap::WavPropertyMap()
{
    insert("AUTH", INF_AUTHOR        ); // author's name
    insert("ANNO", INF_ANNOTATION    ); // annotations
    insert("IARL", INF_ARCHIVAL      ); // archival location
    insert("IART", INF_PERFORMER     ); // performer
    insert("ICMS", INF_COMMISSIONED  ); // commissioned
    insert("ICMT", INF_COMMENTS	     ); // comments
    insert("ICOP", INF_COPYRIGHT     ); // copyright
    insert("(c) ", INF_COPYRIGHT     ); // copyright
    insert("ICRD", INF_CREATION_DATE ); // creation date (iso)
    insert("IENG", INF_ENGINEER	     ); // engineer
    insert("IGNR", INF_GENRE	     ); // genre
    insert("IKEY", INF_KEYWORDS	     ); // keywords
    insert("IMED", INF_MEDIUM	     ); // medium
    insert("INAM", INF_NAME	     ); // name
    insert("IPRD", INF_PRODUCT	     ); // product
    insert("ISFT", INF_SOFTWARE	     ); // software
    insert("ISRC", INF_SOURCE	     ); // source
    insert("ISRF", INF_SOURCE_FORM   ); // source form
    insert("ITCH", INF_TECHNICAN     ); // technican
    insert("ISBJ", INF_SUBJECT	     ); // subject
}

//***************************************************************************
QByteArray WavPropertyMap::findProperty(const FileProperty property)
{
    return (values().contains(property)) ? key(property) : "";
}

//***************************************************************************
bool WavPropertyMap::containsProperty(const FileProperty property)
{
    return (findProperty(property).length() != 0);
}

//***************************************************************************
//***************************************************************************
