/***************************************************************************
     PlayBackTypesMap.h  -  map for playback methods
			     -------------------
    begin                : Sat Feb 05 2005
    copyright            : (C) 2005 by Thomas Eschenbacher
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
#ifndef _PLAY_BACK_TYPES_MAP_H_
#define _PLAY_BACK_TYPES_MAP_H_

#include "config.h"
#include "libkwave/TypesMap.h"
#include "PlayBackParam.h"

#undef HAVE_ARTS_SUPPORT // ###

class PlayBackTypesMap: public TypesMap<unsigned int, playback_method_t>
{
public:
    /** fill function for the map */
    virtual void fill();
};

#endif /* _PLAY_BACK_TYPES_MAP_H_ */
