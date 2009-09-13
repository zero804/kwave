/***************************************************************************
            KwaveDrag.h  -  Drag&Drop container for Kwave's audio data
			     -------------------
    begin                : Jan 24 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KWAVE_DRAG_H_
#define _KWAVE_DRAG_H_

#include "config.h"

#include <QByteArray>
#include <QDrag>
#include <QObject>
#include <QString>

#include <kdemacros.h>

class QMimeData;
class QWidget;

class FileInfo;
class MultiTrackReader;
class SignalManager;

/**
 * Simple class for drag & drop of wav data.
 * @todo support for several codecs
 * @todo the current storage mechanism is straight-forward and stupid, it
 *       should be extended to use virtual memory
 */
class KDE_EXPORT KwaveDrag: public QDrag
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @see QDragObject
     */
    KwaveDrag(QWidget *dragSource = 0);

    /** Destructor */
    virtual ~KwaveDrag();

    /**
     * Encodes wave data received from a MultiTrackReader into a byte
     * array that is compatible with the format of a wav file.
     * @param widget the widget used for displaying error messages
     * @param src source of the samples
     * @param info information about the signal, sample rate, resolution etc
     * @return true if successful
     */
    bool encode(QWidget *widget, MultiTrackReader &src, FileInfo &info);

    /** Returns true if the mime type of the given source can be decoded */
    static bool canDecode(const QMimeData *data);

    /**
     * Decodes the encoded byte data of the given mime source and
     * initializes a MultiTrackReader.
     * @param widget the widget used for displaying error messages
     * @param e mime source
     * @param sig signal that receives the mime data
     * @param pos position within the signal where to insert the data
     * @return number of decoded samples if successful, zero if failed
     */
    static unsigned int decode(QWidget *widget, const QMimeData *e,
                               SignalManager &sig, unsigned int pos);

};

#endif /* _KWAVE_DRAG_H_ */
