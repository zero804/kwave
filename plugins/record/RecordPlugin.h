/*************************************************************************
         RecordPlugin.h  -  plugin for recording audio data
                             -------------------
    begin                : Wed Jul 09 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#ifndef _RECORD_PLUGIN_H_
#define _RECORD_PLUGIN_H_

#include "config.h"
#include "libkwave/KwavePlugin.h"
#include "RecordController.h"
#include "RecordParams.h"
#include "RecordState.h"

class QStringList;
class RecordDevice;
class RecordDialog;
class RecordThread;

class RecordPlugin: public KwavePlugin
{
    Q_OBJECT
public:

    /** Constructor */
    RecordPlugin(PluginContext &c);

    /** Destructor */
    virtual ~RecordPlugin();

    /** @see KwavePlugin::setup() */
    virtual QStringList *setup(QStringList &previous_params);

signals:

    /** emitted when the record buffer has been filled */
    void sigBufferFull();

    /** emitted when the trigger level has been reached */
    void sigTriggerReached();

    /** emitted when the recording has started */
    void sigStarted();

    /** emitted when the recording has been stopped */
    void sigStopped();

protected slots:

    /**
     * command for resetting all recorded stuff for starting again
     */
    void resetRecording();

    /**
     * command for starting the recording, completion is
     * signalled with sigStarted()
     */
    void startRecording();

    /**
     * command for stopping recording, completion is
     * signalled with sigStopped()
     */
    void stopRecording();

    /**
     * called when the recording engine has changed it's state
     */
    void stateChanged(RecordState state);

private slots:

    // setup functions

    /** select a new record device */
    void changeDevice(const QString &dev);

    /** select a new number of tracks (channels) */
    void changeTracks(unsigned int new_tracks);

    /** select a new sample rate [samples/second] */
    void changeSampleRate(double new_rate);

    /** change compression type */
    void changeCompression(int new_compression);

    /** select a new resolution [bits/sample] */
    void changeBitsPerSample(unsigned int new_bits);

    /** select a new sample format */
    void changeSampleFormat(int new_format);

private:

    /** global state of the plugin */
    RecordState m_state;

    /** record control: pre-record enabled */
    bool m_prerecord_enabled;

    /** device used for recording */
    RecordDevice *m_device;

    /** setup dialog */
    RecordDialog *m_dialog;

    /** the thread for recording */
    RecordThread *m_thread;

};

#endif /* _RECORD_PLUGIN_H_ */
