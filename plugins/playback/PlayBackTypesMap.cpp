/***************************************************************************
    PlayBackTypesMap.cpp  -  map for playback methods
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

#include "config.h"

#include <qstring.h>
#include <klocale.h>

#include "PlayBackTypesMap.h"

//***************************************************************************
void PlayBackTypesMap::fill()
{
    unsigned int index = 0;
    QString name = "";

#ifdef HAVE_ARTS_SUPPORT
    name = "aRts (Analog Realtime Synthesizer)";
    append(index++, PLAYBACK_ARTS, "arts", name);
#if 0
    i18n("aRts (Analog Realtime Synthesizer)");
#endif
#endif /* HAVE_OSS_SUPPORT */

#ifdef HAVE_ALSA_SUPPORT
    name = "ALSA (Advanced Linux Sound Architecture)";
    append(index++, PLAYBACK_ALSA, "alsa", name);
#if 0
    i18n("ALSA (Advanced Linux Sound Architecture)");
#endif
#endif /* HAVE_ALSA_SUPPORT */

#ifdef HAVE_OSS_SUPPORT
    name = "OSS (Open Sound System)";
    append(index++, PLAYBACK_OSS, "oss", name);
#if 0
    i18n("OSS (Open Sound System)");
#endif
#endif /* HAVE_OSS_SUPPORT */

   Q_ASSERT(index);
   if (!index) qWarning("no playback method defined!");
}

//***************************************************************************
//***************************************************************************
