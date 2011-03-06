/*************************************************************************
   AudiofileDecoder.cpp  -  import through libaudiofile
                             -------------------
    begin                : Tue May 28 2002
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

#include "config.h"
#include <stdlib.h>

extern "C" {
#include <audiofile.h>
}

#include <QtGlobal>

#include <klocale.h>
#include <kmimetype.h>

#include "libkwave/ConfirmCancelProxy.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/Signal.h"
#include "libkwave/Writer.h"
#include "libkwave/VirtualAudioFile.h"

#include "AudiofileDecoder.h"

#define CHECK(cond) Q_ASSERT(cond); if (!(cond)) { src.close(); return false; }

//***************************************************************************
AudiofileDecoder::AudiofileDecoder()
    :Decoder(), m_source(0), m_src_adapter(0)
{
    /* defined in RFC 1521 */
    addMimeType("audio/basic",
                i18n("NeXT, Sun Audio"),
                "*.au; *.AU; *.snd; *.SND");

    /* some others, mime types might be wrong (I found no RFC or similar)  */
    addMimeType("audio/x-aifc",
                i18n("Compressed Audio Interchange Format"),
                "*.aifc; *.AIFC");
    addMimeType("audio/x-aiff", /* included in KDE */
                i18n("Audio Interchange Format"),
                "*.aif; *.AIF; *.aiff; *.AIFF");
    addMimeType("audio/x-ircam",
                i18n("Berkeley, IRCAM, Carl Sound Format"),
                "*.sf; *.SF");
}

//***************************************************************************
AudiofileDecoder::~AudiofileDecoder()
{
    if (m_source) close();
    if (m_src_adapter) delete m_src_adapter;
}

//***************************************************************************
Decoder *AudiofileDecoder::instance()
{
    return new AudiofileDecoder();
}

//***************************************************************************
bool AudiofileDecoder::open(QWidget *widget, QIODevice &src)
{
    info().clear();
    Q_ASSERT(!m_source);
    if (m_source) qWarning("AudiofileDecoder::open(), already open !");

    // try to open the source
    if (!src.open(QIODevice::ReadOnly)) {
	qWarning("failed to open source !");
	return false;
    }

    // source successfully opened
    m_source = &src;
    m_src_adapter = new VirtualAudioFile(*m_source);

    Q_ASSERT(m_src_adapter);
    if (!m_src_adapter) return false;

    m_src_adapter->open(m_src_adapter, 0);

    AFfilehandle fh = m_src_adapter->handle();
    if (!fh || (m_src_adapter->lastError() >= 0)) {
	QString reason;

	switch (m_src_adapter->lastError()) {
	    case AF_BAD_NOT_IMPLEMENTED:
	        reason = i18n("Format or function is not implemented");
	        break;
	    case AF_BAD_MALLOC:
	        reason = i18n("Out of memory");
	        break;
	    case AF_BAD_HEADER:
	        reason = i18n("File header is damaged");
	        break;
	    case AF_BAD_CODEC_TYPE:
	        reason = i18n("Invalid codec type");
	        break;
	    case AF_BAD_OPEN:
	        reason = i18n("Opening the file failed");
	        break;
	    case AF_BAD_READ:
	        reason = i18n("Read access failed");
	        break;
	    case AF_BAD_SAMPFMT:
	        reason = i18n("Invalid sample format");
	        break;
	    default:
		reason = reason.number(m_src_adapter->lastError());
	}

	QString text= i18n("An error occurred while opening the "\
	    "file:\n'%1'", reason);
	Kwave::MessageBox::error(widget, text);

	return false;
    }

    unsigned int length = afGetFrameCount(fh, AF_DEFAULT_TRACK);
    unsigned int tracks = afGetVirtualChannels(fh, AF_DEFAULT_TRACK);
    unsigned int bits = 0;
    unsigned int rate = 0;
    int sample_format;
    afGetVirtualSampleFormat(fh, AF_DEFAULT_TRACK, &sample_format,
	reinterpret_cast<int *>(&bits));
    
    // get sample rate, with fallback to 8kHz
    rate = static_cast<int>(afGetRate(fh, AF_DEFAULT_TRACK));
    if (rate < 1) {
	qWarning("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"\
	         "WARNING: file has no sample rate!\n"\
	         "         => using 8000 samples/sec as fallback\n"\
	         "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	rate = 8000;
    }

    QString sample_format_name;
    switch (sample_format) {
	case AF_SAMPFMT_TWOSCOMP:
	    sample_format_name = "Linear Two's Complement";
	    break;
	case AF_SAMPFMT_UNSIGNED:
	    sample_format_name = "Unsigned Integer";
	    break;
	case AF_SAMPFMT_FLOAT:
	    sample_format_name = "32-bit IEEE Floating-Point";
	    break;
	case AF_SAMPFMT_DOUBLE:
	    sample_format_name = "64-bit IEEE Double-Precision Floating-Point";
	    break;
	default:
	    sample_format_name = "(unknown)";
    }
    if (static_cast<signed int>(bits) < 0) bits = 0;
    
    int compression = afGetCompression(fh, AF_DEFAULT_TRACK); // just for debug

    info().setRate(rate);
    info().setBits(bits);
    info().setTracks(tracks);
    info().setLength(length);
    qDebug("-------------------------");
    qDebug("info:");
    qDebug("compression = %d", compression);
    qDebug("channels    = %d", info().tracks());
    qDebug("rate        = %0.0f", info().rate());
    qDebug("bits/sample = %d", info().bits());
    qDebug("length      = %lu samples", 
	   static_cast<unsigned long int>(info().length()));
    qDebug("format      = %d (%s)", sample_format,
                                    sample_format_name.toLocal8Bit().data());
    qDebug("-------------------------");

    // set up libaudiofile to produce Kwave's internal sample format
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    afSetVirtualByteOrder(fh, AF_DEFAULT_TRACK, AF_BYTEORDER_BIGENDIAN);
#else
    afSetVirtualByteOrder(fh, AF_DEFAULT_TRACK, AF_BYTEORDER_LITTLEENDIAN);
#endif
    afSetVirtualSampleFormat(fh, AF_DEFAULT_TRACK,
	AF_SAMPFMT_TWOSCOMP, SAMPLE_STORAGE_BITS);

    return true;
}

//***************************************************************************
bool AudiofileDecoder::decode(QWidget */*widget*/, Kwave::MultiWriter &dst)
{
    Q_ASSERT(m_src_adapter);
    Q_ASSERT(m_source);
    if (!m_source) return false;
    if (!m_src_adapter) return false;

    AFfilehandle fh = m_src_adapter->handle();
    Q_ASSERT(fh);
    if (!fh) return false;

    unsigned int frame_size = static_cast<unsigned int>(
	afGetVirtualFrameSize(fh, AF_DEFAULT_TRACK, 1));

    // allocate a buffer for input data
    const unsigned int buffer_frames = (8*1024);
    int32_t *buffer =
	static_cast<int32_t *>(malloc(buffer_frames * frame_size));
    Q_ASSERT(buffer);
    if (!buffer) return false;

    // read in from the audiofile source
    const unsigned int tracks = info().tracks();
    sample_index_t rest = info().length();
    while (rest) {
	unsigned int frames = buffer_frames;
	if (frames > rest) frames = rest;
	unsigned int buffer_used = afReadFrames(fh,
	    AF_DEFAULT_TRACK, reinterpret_cast<char *>(buffer), frames);

	// break if eof reached
	Q_ASSERT(buffer_used);
	if (!buffer_used) break;
	rest -= buffer_used;

	// split into the tracks
	int32_t *p = buffer;
	unsigned int count = buffer_used;
	while (count--) {
	    for (unsigned int track = 0; track < tracks; track++) {
		int32_t s = *p++;

		// adjust precision
		if (SAMPLE_STORAGE_BITS != SAMPLE_BITS) {
		    s /= (1 << (SAMPLE_STORAGE_BITS-SAMPLE_BITS));
		}

		// the following cast is only necessary if
		// sample_t is not equal to a u_int32_t
		*(dst[track]) << static_cast<sample_t>(s);
	    }
	}

	// abort if the user pressed cancel
	if (dst.isCanceled()) break;
    }

    // return with a valid Signal, even if the user pressed cancel !
    if (buffer) free(buffer);
    return true;
}

//***************************************************************************
void AudiofileDecoder::close()
{
    if (m_src_adapter) delete m_src_adapter;
    m_src_adapter = 0;
    m_source = 0;
}

//***************************************************************************
//***************************************************************************
