/***************************************************************************
          StreamWriter.h - adapter between writers and sample source
			     -------------------
    begin                : Sun Aug 23 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
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

#ifndef _STREAM_WRITER_H_
#define _STREAM_WRITER_H_

#include "config.h"

#include <QObject>

#include <kdemacros.h>

#include "libkwave/Writer.h"

class Track;

namespace Kwave {

    class SampleArray;

    /**
     * @class StreamWriter
     * Input stream for transferring samples into a Track.
     *
     * @warning THIS CLASS IS NOT THREADSAFE! It is intended to be owned by
     *          and used from only one thread.
     */
    class KDE_EXPORT StreamWriter: public Kwave::Writer
    {
	Q_OBJECT
    public:

	/**
	 * Constructor
	 */
	StreamWriter();

	/**
	 * Destructor.
	 */
	virtual ~StreamWriter();

	/**
	 * Flush the content of a buffer. Normally the buffer is the
	 * internal intermediate buffer used for single-sample writes.
	 * When using block transfers, the internal buffer is bypassed
	 * and the written block is passed instead.
	 * @internal
	 * @param buffer reference to the buffer to be flushed
	 * @param count number of samples in the buffer to be flushed,
	 *              will be internally set to zero if successful
	 * @return true if successful, false if failed (e.g. out of memory)
	 */
	 virtual bool write(const Kwave::SampleArray &buffer,
	                    unsigned int &count);

    signals:

	/** emits a block with sine wave data */
	void output(Kwave::SampleArray data);

    };

}

#endif /* _STREAM_WRITER_H_ */
