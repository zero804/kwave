/*************************************************************************
        WavDecoder.cpp  -  decoder for wav data
                             -------------------
    begin                : Sun Mar 10 2002
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
#include <klocale.h>
#include <kmimetype.h>

#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/Signal.h"

#include "WavDecoder.h"
#include "WavFileFormat.h"

#define CHECK(cond) ASSERT(cond); if (!(cond)) { src.close(); return false; }

/***************************************************************************/
WavDecoder::WavDecoder()
    :Decoder(), m_source(0)
{
    LOAD_MIME_TYPES;
}

/***************************************************************************/
WavDecoder::~WavDecoder()
{
    if (m_source) close();
}

/***************************************************************************/
Decoder *WavDecoder::instance()
{
    return new WavDecoder();
}

/***************************************************************************/
bool WavDecoder::open(QIODevice &src)
{
    info().clear();
    ASSERT(!m_source);
    if (m_source) warning("WavDecoder::open(), already open !");

    // try to open the source
    if (!src.open(IO_ReadOnly)) {
	warning("failed to open source !");
	return false;
    }

    // source successfully opened
    m_source = &src;

    unsigned int rate = 0;
    unsigned int bits = 0;
    char c;

    // get the encoded block of data from the mime source
    CHECK(src.size() > sizeof(wav_header_t)+8);

    wav_header_t header;
    unsigned int datalen = src.size() - (sizeof(wav_header_t) + 8);
    unsigned int src_pos = 0;

    // get the header
    src.readBlock((char *)&header, sizeof(wav_header_t));
#if defined(IS_BIG_ENDIAN)
    header.filelength = bswap_32(header.filelength);
    header.fmtlength = bswap_32(header.fmtlength);
    header.mode = bswap_16(header.mode);
    header.channels = bswap_16(header.channels);
    header.rate = bswap_32(header.rate);
    header.AvgBytesPerSec = bswap_32(header.AvgBytesPerSec);
    header.BlockAlign = bswap_16(header.BlockAlign);
    header.bitspersample = bswap_16(header.bitspersample);
#endif
    const unsigned int tracks = header.channels;
    rate = header.rate;
    bits = header.bitspersample;
    const unsigned int bytes = (bits >> 3);

    // some sanity checks
    CHECK(header.AvgBytesPerSec == rate * bytes * tracks);
    CHECK(static_cast<unsigned int>(header.BlockAlign) == bytes*tracks);
    CHECK(!strncmp((char*)&(header.riffid), "RIFF", 4));
    CHECK(!strncmp((char*)&(header.wavid), "WAVE", 4));
    CHECK(!strncmp((char*)&(header.fmtid), "fmt ", 4));
    CHECK(header.filelength == (datalen + sizeof(wav_header_t)));
    CHECK(header.fmtlength == 16);
    CHECK(header.mode == 1); /* currently only mode 1 is supported :-( */

    src_pos += sizeof(wav_header_t);
    c = src.getch(); CHECK(c == 'd');
    c = src.getch(); CHECK(c == 'a');
    c = src.getch(); CHECK(c == 't');
    c = src.getch(); CHECK(c == 'a');
    c = src.getch(); CHECK(c == static_cast<char>( datalen        & 0xFF));
    c = src.getch(); CHECK(c == static_cast<char>((datalen >>  8) & 0xFF));
    c = src.getch(); CHECK(c == static_cast<char>((datalen >> 16) & 0xFF));
    c = src.getch(); CHECK(c == static_cast<char>((datalen >> 24) & 0xFF));

    info().setRate(rate);
    info().setBits(bits);
    info().setTracks(tracks);
    info().setLength(datalen / bytes / tracks);

    return true;
}

/***************************************************************************/
bool WavDecoder::decode(MultiTrackWriter &dst)
{
    ASSERT(m_source);
    if (!m_source) return false;

    const unsigned int bits   = info().bits();
    const unsigned int tracks = info().tracks();
    const unsigned int bytes = (bits >> 3);

    // fill the signal with data
    const __uint32_t sign = 1 << (24-1);
    const unsigned int negative = ~(sign - 1);
    const unsigned int shift = 24-bits;

    unsigned int rest = info().length();
    while (rest--) {
	__uint32_t s = 0; // raw 32bit value
	for (unsigned int track = 0; track < tracks; track++) {
	    SampleWriter *stream = dst[track];
	
	    if (bytes == 1) {
		// 8-bit files are always unsigned !
		s = (static_cast<__uint8_t>(m_source->getch() - 128))<<shift;
	    } else {
		// >= 16 bits is signed
		s = 0;
		for (register unsigned int byte = 0; byte < bytes; byte++) {
		    s |= (static_cast<__uint8_t>(m_source->getch())
		        << ((byte << 3) + shift));
		}
		// sign correcture for negative values
		if (s & sign) s |= negative;
	    }
	
	    // the following cast is only necessary if
	    // sample_t is not equal to a 32bit int
	    *stream << static_cast<sample_t>(s);
	}
	
	// abort if the user pressed cancel
	if (dst.isCancelled()) break;
    }

    // return with a valid Signal, even if the user pressed cancel !
    return true;
}

/***************************************************************************/
void WavDecoder::close()
{
    m_source = 0;
}

/* ###

below comes some old code from the SignalManager. It's features should
get merged into the new code again...

__uint32_t SignalManager::findChunk(QFile &sigfile, const char *chunk,
	__uint32_t offset)
{
    char current_name[16];
    __uint32_t length = 0;
    int len;

    ASSERT(sizeof(length) == 4);
    ASSERT(sizeof(int) == 4);

    sigfile.at(offset);
    while (!sigfile.atEnd()) {
	// get name of the chunk
	len = sigfile.readBlock((char*)(&current_name), 4);
	if (len < 4) {
	    debug("findChunk('%s'): not found, reached EOF while reading name",
	    	chunk);
	    return 0; // reached EOF
	}

	// get length of the chunk
	len = sigfile.readBlock((char*)(&length), sizeof(length));
	if (len < 4) {
	    debug("findChunk('%s'): not found, reached EOF :-(", chunk);
	    return 0; // reached EOF
	}
#ifdef IS_BIG_ENDIAN
	length = bswap_32(length);
#endif

	// chunk found !
	if (strncmp(chunk, current_name, 4) == 0) return length;

	// not found -> skip
	sigfile.at(sigfile.at()+length);
    };

    debug("findChunk('%s'): not found :-(", chunk);
    return 0;
}

int SignalManager::loadWavChunk(QFile &sigfile, unsigned int length,
                                unsigned int channels, int bits)
{
    unsigned int bufsize = 64 * 1024 * sizeof(sample_t);
    unsigned char *loadbuffer = 0;
    int bytes = bits >> 3;
    unsigned int sign = 1 << (24-1);
    unsigned int negative = ~(sign - 1);
    unsigned int shift = 24-bits;
    unsigned int bytes_per_sample = bytes * channels;
    unsigned int max_samples = bufsize / bytes_per_sample;
    long int start_offset = sigfile.at();

    debug("SignalManager::loadWavChunk(): offset     = %d", sigfile.at());
    debug("SignalManager::loadWavChunk(): length     = %d samples", length);
    debug("SignalManager::loadWavChunk(): tracks     = %d", channels);
    debug("SignalManager::loadWavChunk(): resoultion = %d bits/sample", bits);

    ASSERT(bytes);
    ASSERT(channels);
    ASSERT(length);
    if (!bytes || !channels || !length) return -EINVAL;

    // try to allocate memory for the load buffer
    // if failed, try again with the half buffer size as long
    // as <1kB is not reached (then we are really out of memory)
    while (loadbuffer == 0) {
	if (bufsize < 1024) {
	    debug("SignalManager::loadWavChunk:not enough memory for buffer");
	    return -ENOMEM;
	}
	loadbuffer = new unsigned char[bufsize];
	if (!loadbuffer) bufsize >>= 1;
    }

    // check if the file is large enough for "length" samples
    size_t file_rest = sigfile.size() - sigfile.at();
    if (length > file_rest/bytes_per_sample) {
	debug("SignalManager::loadWavChunk: "\
	      "length=%d, rest of file=%d",length,file_rest);
	KMessageBox::error(m_parent_widget,
	    i18n("Error in input: file is smaller than stated "\
	    "in the header. \n"\
	    "File will be truncated."));
	length = file_rest/bytes_per_sample;
    }

    QList<SampleWriter> samples;
    samples.setAutoDelete(true);

    for (unsigned int track = 0; track < channels; track++) {
	SampleWriter *s = 0;
	Track *new_track = m_signal.appendTrack(length);
	ASSERT(new_track);
	if (new_track && (new_track->length() >= length)) {
	    s = openSampleWriter(track, Overwrite);
	    ASSERT(s);
	}
	
	if (!s) {
	    KMessageBox::sorry(m_parent_widget, i18n("Out of Memory!"));
	    return -ENOMEM;
	}
	samples.append(s);
    }

    // now the signal is considered not to be empty
    m_empty = false;

    //prepare and show the progress dialog
    FileProgress *dialog = new FileProgress(m_parent_widget,
	m_name, file_rest, length, m_rate, bits, channels);
    ASSERT(dialog);

    // prepare the loader loop
    int percent_count = length / 100;

    // debug("sign=%08X, negative=%08X, shift=%d",sign,negative,shift);

    for (unsigned int pos = 0; pos < length; ) {
	// break the loop if the user has pressed "cancel"
	if (dialog && dialog->isCancelled()) break;
	
	// limit reading to end of wav chunk length
	if ((pos + max_samples) > length) max_samples=length-pos;
	
	// read the samples into a temporary buffer
	int read_samples = sigfile.readBlock(
	    reinterpret_cast<char *>(loadbuffer),
	    bytes_per_sample*max_samples
	) / bytes_per_sample;
	percent_count -= read_samples;
	
	// debug("read %d samples", read_samples);
	if (read_samples <= 0) {
	    warning("SignalManager::loadWavChunk:EOF reached?"\
		    " (at sample %ld, expected length=%d",
		    sigfile.at() / bytes_per_sample - start_offset, length);
	    break;
	}
	
	unsigned char *buffer = loadbuffer;
	__uint32_t s = 0; // raw 32bit value
	while (read_samples--) {
	    for (register unsigned int channel = 0;
		 channel < channels;
		channel++)
	    {
		SampleWriter *stream = samples.at(channel);
		
		if (bytes == 1) {
		    // 8-bit files are always unsigned !
		    s = (*(buffer++) - 128) << shift;
		} else {
		    // >= 16 bits is signed
		    s = 0;
		    for (register int byte = 0; byte < bytes; byte++) {
			s |= *(buffer++) << ((byte << 3) + shift);
		    }
		    // sign correcture for negative values
		    if ((unsigned int)s & sign)
			s |= negative;
		}
		
		// the following cast is only necessary if
		// sample_t is not equal to a 32bit int
		sample_t sample = static_cast<sample_t>(s);
		
		*stream << sample;
	    }
	    pos++;
	}
	
	if (dialog && (percent_count <= 0)) {
	    percent_count = length / 100;
	    dialog->setValue(pos * bytes_per_sample);
	}
    }

    // close all sample input streams
    samples.clear();

    if (dialog) delete dialog;
    if (loadbuffer) delete[] loadbuffer;
    return 0;
}
### */

//    /**
//     * Try to find a chunk within a RIFF file. If the chunk
//     * was found, the current position will be at the start
//     * of the chunk's data.
//     * @param sigfile file to read from
//     * @param chunk name of the chunk
//     * @param offset the file offset for start of the search
//     * @return the size of the chunk, 0 if not found
//     */
//    __uint32_t findChunk(QFile &sigfile, const char *chunk,
//	__uint32_t offset = 12);
//
//    /**
//     * Imports ascii file with one sample per line and only one
//     * track. Everything that cannot be parsed by strod will be ignored.
//     * @return 0 if succeeded or error number if failed
//     */
//    int loadAscii();
//
//    /**
//     * Loads a .wav-File.
//     * @return 0 if succeeded or a negative error number if failed:
//     *           -ENOENT if file does not exist,
//     *           -ENODATA if the file has no data chunk or is zero-length,
//     *           -EMEDIUMTYPE if the file has an invalid/unsupported format
//     */
//    int loadWav();
//
//    /**
//     * Reads in the wav data chunk from a .wav-file. It creates
//     * a new empty Signal for each track and fills it with
//     * data read from an opened file. The file's actual position
//     * must already be set to the correct position.
//     * @param sigin reference to the already opened file
//     * @param length number of samples to be read
//     * @param tracks number of tracks [1..n]
//     * @param number of bits per sample [8,16,24,...]
//     * @return 0 if succeeded or error number if failed
//     */
//    int loadWavChunk(QFile &sigin, unsigned int length,
//                     unsigned int tracks, int bits);
//
//    /**
//     * Writes the chunk with the signal to a .wav file (not including
//     * the header!).
//     * @param sigout reference to the already opened file
//     * @param offset start position from where to save
//     * @param length number of samples to be written
//     * @param bits number of bits per sample [8,16,24,...]
//     * @return 0 if succeeded or error number if failed
//     */
//    int writeWavChunk(QFile &sigout, unsigned int offset, unsigned int length,
//                      unsigned int bits);

/* ###
int SignalManager::loadWav()
{
    wav_fmt_header_t fmt_header;
    int result = 0;
    __uint32_t num;
    __uint32_t length;

    ASSERT(m_closed);
    ASSERT(m_empty);

    QFile sigfile(m_name);
    if (!sigfile.open(IO_ReadOnly)) {
	KMessageBox::error(m_parent_widget,
		i18n("File does not exist !"));
	return -ENOENT;
    }

    // --- check if the file starts with "RIFF" ---
    num = sigfile.size();
    length = findChunk(sigfile, "RIFF", 0);
    if ((length == 0) || (sigfile.at() != 8)) {
	KMessageBox::error(m_parent_widget,
	    i18n("File is no RIFF File !"));
	// maybe recoverable...
    } else if (length+8 != num) {
//	KMessageBox::error(m_parent_widget,
//	    i18n("File has incorrect length! (maybe truncated?)"));
	// will be warned anyway later...
	// maybe recoverable...
    } else {
	// check if the chunk data contains "WAVE"
	char file_type[16];
	num = sigfile.readBlock((char*)(&file_type), 4);
	if ((num != 4) || strncmp("WAVE", file_type, 4)) {
	    KMessageBox::error(m_parent_widget,
		i18n("File is no WAVE File !"));
	    // maybe recoverable...
	}
    }

    // ------- read the "fmt " chunk -------
    ASSERT(sizeof(fmt_header) == 16);
    num = findChunk(sigfile, "fmt ");
    if (num != sizeof(fmt_header)) {
	debug("SignalManager::loadWav(): length of fmt chunk = %d", num);
	KMessageBox::error(m_parent_widget,
	    i18n("File does not contain format information!"));
	return -EMEDIUMTYPE;
    }
    num = sigfile.readBlock((char*)(&fmt_header), sizeof(fmt_header));
#ifdef IS_BIG_ENDIAN
    fmt_header.length = bswap_32(fmt_header.length);
    fmt_header.mode = bswap_16(fmt_header.mode);
    fmt_header.channels = bswap_16(fmt_header.channels);
    fmt_header.rate = bswap_32(fmt_header.rate);
    fmt_header.AvgBytesPerSec = bswap_32(fmt_header.AvgBytesPerSec);
    fmt_header.BlockAlign = bswap_32(fmt_header.BlockAlign);
    fmt_header.bitspersample = bswap_16(fmt_header.bitspersample);
#endif
    if (fmt_header.mode != 1) {
	KMessageBox::error(m_parent_widget,
	    i18n("File must be uncompressed (Mode 1) !"),
	    i18n("Sorry"), 2);
	return -EMEDIUMTYPE;
    }

    m_rate = fmt_header.rate;
    m_signal.setBits(fmt_header.bitspersample);

    // ------- search for the data chunk -------
    length = findChunk(sigfile, "data");
    if (!length) {
	warning("length = 0, but file size = %u, current pos = %u",
	    sigfile.size(), sigfile.at());
	
	if (sigfile.size() - sigfile.at() > 0) {
	    if (KMessageBox::warningContinueCancel(m_parent_widget,
		i18n("File is damaged: the file header reports zero length\n"\
		     "but the file seems to contain some data. \n\n"\
		     "Kwave can try to recover the file, but the\n"\
		     "result might contain trash at it's start and/or\n"\
		     "it's end. It is strongly advisable to edit the\n"\
		     "damaged parts manually and save the file again\n"\
		     "to fix the problem.\n\n"\
		     "Try to recover?"),
		     i18n("Damaged File"),
		     i18n("&Recover")) == KMessageBox::Continue)
	    {
		length = sigfile.size() - sigfile.at();
	    } else return -EMEDIUMTYPE;
	} else {
	    KMessageBox::error(m_parent_widget,
	        i18n("File does not contain data!"));
	    return -EMEDIUMTYPE;
	}
    }

    length = (length/(fmt_header.bitspersample/8))/fmt_header.channels;
    switch (fmt_header.bitspersample) {
	case 8:
	case 16:
	case 24:
	    ASSERT(m_empty);
	    // currently the signal should be closed and empty
	    // now make it opened but empty
	    m_closed = false;
	    emitStatusInfo();
	    result = loadWavChunk(sigfile, length,
				  fmt_header.channels,
				  fmt_header.bitspersample);
	    break;
	default:
	    KMessageBox::error(m_parent_widget,
		i18n("Sorry only 8/16/24 Bits per Sample"\
		" are supported !"), i18n("Sorry"), 2);
	    result = -EMEDIUMTYPE;
    }

    if (result == 0) {
	debug("SignalManager::loadWav(): successfully opened");
    }

    return result;
}
### */

/***************************************************************************/
/***************************************************************************/
