/*************************************************************************
          Record-ALSA.h  -  device for audio recording via ALSA
                             -------------------
    begin                : Sun Jul 24 2005
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

#ifndef _RECORD_ALSA_H_
#define _RECORD_ALSA_H_

#include "config.h"
#ifdef HAVE_ALSA_SUPPORT

/*
 * use the new ALSA HW/SW params API, needed to compile under SuSE-9.0
 * (workaround as seen in http://www.linuxjournal.com/article/6735)
 */
#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API

#include <alsa/asoundlib.h>
#include <qstring.h>
#include <qmap.h>
#include <qvaluelist.h>
#include "libkwave/CompressionType.h"
#include "libkwave/SampleFormat.h"

#include "RecordDevice.h"

/* forward declaration, needed for list of supported formats */
struct alsa_sample_format_t;

class RecordALSA: public RecordDevice
{
public:

    /** Constructor */
    RecordALSA();

    /** Destructor */
    virtual ~RecordALSA();

    /**
     * Open the record device.
     * @param dev path of the record device
     * @return file descriptor >= 0 or negative error code if failed
     */
    virtual int open(const QString &dev);

    /**
     * Read the raw audio data from the record device.
     * @param buffer pointer to the array of bytes to receive the audio data
     * @param length size of the buffer
     * @return number of bytes read, zero or negative if failed
     */
    virtual int read(char *buffer, unsigned int length);


    /** Close the device */
    virtual int close();

    /** return a string list with supported device names */
    virtual QStringList supportedDevices();

    /**
     * Detect the minimum and maximum number of tracks.
     * If the detection fails, minimum and maximum are set to zero.
     * @param min receives the lowest supported number of tracks
     * @param max receives the highest supported number of tracks
     * @return zero or positive number if ok, negative error number if failed
     */
    virtual int detectTracks(unsigned int &min, unsigned int &max);

    /**
     * Try to set a new number of tracks.
     * @note the device must be open
     * @param tracks the number of tracks to be set, can be modified and
     *        decreased to the next supported number of tracks if the
     *        underlying driver supports that.
     * @return zero on success, negative error code if failed
     */
    virtual int setTracks(unsigned int &tracks);

    /** Returns the current number of tracks */
    virtual int tracks();

    /** get a list of supported sample rates */
    virtual QValueList<double> detectSampleRates();

    /**
     * Try to set a new sample rate.
     * @param new_rate the sample rate to be set [samples/second], can
     *        be modified and rounded up/down to the nearest supported
     *        sample rate if the underlying driver supports that.
     * @return zero on success, negative error code if failed
     */
    virtual int setSampleRate(double &new_rate);

    /** Returns the current sample rate of the device */
    virtual double sampleRate();

    /**
     * Gets a list of supported compression types. If no compression is
     * supported, the list might be empty.
     */
    virtual QValueList<int> detectCompressions();

    /**
     * Try to set a new compression type.
     * @param new_compression the identifier of the new compression
     * @return zero on success, negative error code if failed
     * @see class CompressionType
     */
    virtual int setCompression(int new_compression);

    /** Returns the current compression type (0==none) */
    virtual int compression();

    /**
     * Detect a list of supported bits per sample.
     * @note this depends on the compression type
     * @return a list of bits per sample, empty if failed
     */
    virtual QValueList <unsigned int> supportedBits();

    /**
     * Set the resolution in bits per sample
     * @param new_bits resolution [bits/sample]
     */
    virtual int setBitsPerSample(unsigned int new_bits);

    /**
     * Returns the current resolution in bits per sample or a negative
     * error code if failed
     */
    virtual int bitsPerSample();

    /**
     * Gets a list of supported sample formats.
     * @note this depends on the current setting of the compression!
     */
    virtual QValueList<SampleFormat::sample_format_t> detectSampleFormats();

    /**
     * Try to set a new sample format (signed/unsigned)
     * @param new_format the identifier for the new format
     * @return zero on success, negative error code if failed
     * @see class SampleFormat
     */
    virtual int setSampleFormat(SampleFormat::sample_format_t new_format);

    /** Returns the current sample format (signed/unsigned) */
    virtual SampleFormat::sample_format_t sampleFormat();

    /** Returns the current endianness (big/little) */
    virtual byte_order_t endianness();

private:

    /**
     * Walk through the list of all known formats and collect the
     * ones that are supported into "m_supported_formats".
     */
    void detectSupportedFormats();

    /**
     * Initialize the ALSA device with current parameters and
     * prepare it for recording.
     * @return zero on success or negative error code
     *         -EINVAL or -EIO
     */
    int initialize();

    /**
     * create a ALSA device format (enum) from parameters.
     * @param compression the compression type
     * @see CompressionType
     * @param bits the number of bits per sample, related
     *        to the decoded stream
     * @param sample_format the sample format, as defined in
     *        libaudiofile (signed or unsigned)
     * @return the index of the best matching format within the list
     *         of known formats, or -1 if no match was found
     */
    int mode2format(int compression, int bits,
                    SampleFormat::sample_format_t sample_format);

    /** scan all ALSA devices, re-creates m_device_list */
    void scanDevices();

    /**
     * Translate a verbose device name into a ALSA hardware device name.
     *
     * @param name verbose name of the device
     * @return device name that can be used for snd_pcm_open()
     */
    QString alsaDeviceName(const QString &name);

private:

    /** Handle of the source device or zero if not open */
    snd_pcm_t *m_handle;

    /**
     * dictionary for translating verbose device names
     * into ALSA hardware device names
     */
    static QMap<QString, QString> m_device_list;

    /** number of tracks [0...N-1] */
    unsigned int m_tracks;

    /** sample rate  */
    double m_rate;

    /** compression mode */
    int m_compression;

    /** resolution [bits per sample] */
    unsigned int m_bits_per_sample;

    /** sample format (signed int, unsigned int, float, ... */
    SampleFormat::sample_format_t m_sample_format;

    /**
     * list of supported formats of the current device, indices in
     * the global list of known formats.
     * Only valid after a successful call to "open()",
     * otherwise empty
     */
    QValueList<int> m_supported_formats;

    /** true if initialize() has been successfully been run */
    bool m_initialized;

};

#endif /* HAVE_ALSA_SUPPORT */

#endif /* _RECORD_OSS_H_ */