/*************************************************************************
          OggEncoder.h  -  encoder for Ogg/Vorbis data
                             -------------------
    begin                : Tue Sep 10 2002
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

#ifndef _OGG_ENCODER_H_
#define _OGG_ENCODER_H_

#include "config.h"
#include "libkwave/Encoder.h"

class QWidget;

class OggEncoder: public Encoder
{
public:
    /** Constructor */
    OggEncoder();

    /** Destructor */
    virtual ~OggEncoder();

    /** Returns a new instance of the encoder */
    virtual Encoder *instance();

    /**
     * Encodes a signal into a stream of bytes.
     * @param widget a widget that can be used for displaying
     *        message boxes or dialogs
     * @param src MultiTrackReader used as source of the audio data
     * @param dst file or other source to receive a stream of bytes
     * @param info information about the file to be saved
     * @return true if succeeded, false on errors
     */
    virtual bool encode(QWidget *widget, MultiTrackReader &src,
                        QIODevice &dst, FileInfo &info);


};

#endif /* _OGG_ENCODER_H_ */
