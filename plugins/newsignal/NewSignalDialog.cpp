/***************************************************************************
    NewSignalDialog.cpp  -  dialog for the "newsignal" plugin
                             -------------------
    begin                : Wed Jul 18 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qtimer.h>

#include <klocale.h>
#include <knuminput.h>

#include "libkwave/FileFormat.h"
#include "NewSignalDialog.h"

/** size of all header and structure information of a canonical wav file */
#define WAV_FILE_OVERHEAD (sizeof(wav_header_t) + 8)

//***************************************************************************
NewSignalDialog::NewSignalDialog(QWidget *parent, unsigned int samples,
	unsigned int rate, unsigned int bits, unsigned int tracks,
	bool by_time)
    :NewSigDlg(parent, 0, true), m_timer(this), m_recursive(false)
{
    ASSERT(btOK);
    ASSERT(cbSampleRate);
    ASSERT(sbResolution);
    ASSERT(sbTracks);
    ASSERT(rbTime);
    ASSERT(lblTracksVerbose);
    ASSERT(sbHours);
    ASSERT(sbMinutes);
    ASSERT(sbSeconds);
    ASSERT(rbSamples);
    ASSERT(edSamples);
    ASSERT(slideLength);
    ASSERT(lblFileSize);

    if (!ok()) return;

    edSamples->setFormat("%0.0f");
    edSamples->setRange(0.0, (double)UINT_MAX, 1.0, false);

    // connect the timer for the sample edit
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(checkNewSampleEdit()));

    // emulate the behaviour of an exclusive selection
    connect(rbSamples, SIGNAL(toggled(bool)),
            this, SLOT(rbSamplesToggled(bool)));
    connect(rbTime, SIGNAL(toggled(bool)),
            this, SLOT(rbTimeToggled(bool)));

    // connect the file format controls
    connect(cbSampleRate, SIGNAL(textChanged(const QString&)),
            this, SLOT(sampleRateChanged(const QString&)));
    connect(sbTracks, SIGNAL(valueChanged(int)),
            this, SLOT(tracksChanged(int)));
    connect(sbResolution, SIGNAL(valueChanged(int)),
            this, SLOT(checkTimeAndLengthInfo(int)));

    // connect the time controls
    connect(sbSeconds, SIGNAL(valueChanged(int)),
            this, SLOT(timeChanged(int)));
    connect(sbMinutes, SIGNAL(valueChanged(int)),
            this, SLOT(timeChanged(int)));
    connect(sbHours, SIGNAL(valueChanged(int)),
            this, SLOT(timeChanged(int)));

    // selection by number of samples
    connect(slideLength,SIGNAL(valueChanged(int)),
            this, SLOT(setLengthPercentage(int)));

    // selection by percentage of maximum possible length
    connect(edSamples, SIGNAL(valueChanged(double)),
            this, SLOT(samplesChanged(double)));

    // pre-initialize the size
    setFixedSize(minimumSize());

    // initialize the controls
    rbSamples->setChecked(!by_time);
    rbTime->setChecked(by_time);
    edSamples->setValue(samples);
    cbSampleRate->setEditText(QString::number(rate));
    sbResolution->setValue(bits);
    sbTracks->setValue(tracks);

    tracksChanged(0);
    checkTimeAndLengthInfo(0);

    // that dialog is big enough, limit it to it's minimum size
    setFixedSize(minimumSize());
}

//***************************************************************************
int NewSignalDialog::exec()
{
    return (!ok()) ? Rejected : (NewSigDlg::exec());
}

//***************************************************************************
bool NewSignalDialog::ok()
{
    return(btOK && cbSampleRate && sbResolution && sbTracks && rbTime &&
	lblTracksVerbose && sbHours && sbMinutes && sbSeconds &&
	rbSamples && edSamples && slideLength && lblFileSize);
}

//***************************************************************************
unsigned int NewSignalDialog::samples()
{
    return (unsigned int)floor(edSamples->value());
}

//***************************************************************************
double NewSignalDialog::rate()
{
    bool ok;
    double r = cbSampleRate->currentText().toDouble(&ok);
    if (!ok) r = 0;
    return r;
}

//***************************************************************************
void NewSignalDialog::checkNewSampleEdit()
{
    static double last_samples = -1.0;
    if (edSamples->value() != last_samples) {
	last_samples = edSamples->value();
	samplesChanged(last_samples);
    }
}

//***************************************************************************
unsigned int NewSignalDialog::tracks()
{
    return sbTracks->value();
}

//***************************************************************************
unsigned int NewSignalDialog::bitsPerSample()
{
    int res = sbResolution->value();
    if (res < 8) res = 8;
    return res;
}

//***************************************************************************
bool NewSignalDialog::byTime()
{
    return rbTime->isChecked();
}

//***************************************************************************
double NewSignalDialog::maxSamples()
{
    unsigned int bytes_per_sample = bitsPerSample() >> 3;
    unsigned int max_file_size = UINT_MAX;
    max_file_size -= WAV_FILE_OVERHEAD;

    return floor(max_file_size / tracks() / bytes_per_sample);
}

//***************************************************************************
void NewSignalDialog::rbSamplesToggled(bool)
{
    if (rbSamples->isChecked() == rbTime->isChecked())
	rbTime->setChecked(!rbSamples->isChecked());
}

//***************************************************************************
void NewSignalDialog::rbTimeToggled(bool)
{
    if (rbTime->isChecked() == rbSamples->isChecked())
	rbSamples->setChecked(!rbTime->isChecked());
	
    if (rbTime->isChecked()) {
	m_timer.stop();
    } else {
	// activate the sample edit timer
	m_timer.start(100, false);
    }
}

//***************************************************************************
void NewSignalDialog::checkTimeAndLengthInfo(int)
{
    (rbTime->isChecked()) ? timeChanged(0) : samplesChanged(0);
}

//***************************************************************************
void NewSignalDialog::timeChanged(int)
{
    if (m_recursive) return; // don't do recursive processing
    if (!rbTime->isChecked()) return;
    if (!rate() || !tracks() || (bitsPerSample() < 8)) return;
    m_recursive = true;

    // get current time and correct wrap-overs
    int seconds = sbSeconds->value();
    int minutes = sbMinutes->value();
    int hours = sbHours->value();
    if ((seconds < 0) && ((minutes > 0) || (hours > 0)) ) {
	sbSeconds->setValue(59);
	sbMinutes->stepDown();
	minutes--;
    } else if (seconds < 0) {
	sbSeconds->setValue(0);
    } else if (seconds > 59) {
	sbSeconds->setValue(0);
	sbMinutes->stepUp();
	minutes++;
    }

    if ((minutes < 0) && (hours > 0)) {
	sbMinutes->setValue(59);
	sbHours->stepDown();
	hours--;
    } else if (minutes < 0) {
	sbMinutes->setValue(0);
    } else if (minutes > 59) {
	sbMinutes->setValue(0);
	sbHours->stepUp();
	hours++;
    }
    seconds = sbSeconds->value();
    minutes = sbMinutes->value();
    hours = sbHours->value();
    minutes += 60 * hours;
    seconds += 60 * minutes;

    // limit the current number of samples
    double max_samples = maxSamples();
    double samples = ceil((double)seconds * rate());
    if (samples > max_samples) {
	// wrap down to the maximum allowed number of samples
	samples =  max_samples;
	setHMS(samples);
    }

    // update the other controls
    edSamples->setValue(samples);
    slideLength->setValue((int)(100.0 * samples / max_samples));
    updateFileSize();
    btOK->setEnabled(samples > 0.0);

    m_recursive = false;
}

//***************************************************************************
void NewSignalDialog::samplesChanged(double)
{
    if (m_recursive) return; // don't do recursive processing
    if (!rbSamples->isChecked()) return;
    m_recursive = true;

    double samples = edSamples->value();
    double max_samples = maxSamples();

    if (samples > max_samples) {
	samples = max_samples;
	edSamples->setValue(samples);
    }

    // update the other controls
    setHMS(samples);
    slideLength->setValue((int)(100.0 * samples / max_samples));
    updateFileSize();
    btOK->setEnabled(samples > 0.0);

    m_recursive = false;
}

//***************************************************************************
void NewSignalDialog::sampleRateChanged(const QString&)
{
    checkTimeAndLengthInfo(0);
}

//***************************************************************************
void NewSignalDialog::tracksChanged(int)
{
    switch (tracks()) {
	case 1:
	    lblTracksVerbose->setText(i18n("(Mono)"));
	    break;
	case 2:
	    lblTracksVerbose->setText(i18n("(Stereo)"));
	    break;
	case 4:
	    lblTracksVerbose->setText(i18n("(Quadro)"));
	    break;
	default:
	    lblTracksVerbose->setText("");
	    break;
    }
    checkTimeAndLengthInfo(0);
}

//***************************************************************************
void NewSignalDialog::updateFileSize()
{
    double samples = edSamples->value();
    double mbytes = samples * (double)tracks() *
                    (double)(bitsPerSample() >> 3) + WAV_FILE_OVERHEAD;
    mbytes /= 1024.0; // to kilobytes
    mbytes /= 1024.0; // to megabytes

    QString str_bytes = "";
    if (mbytes >= 10.0) {
	str_bytes.sprintf("%0.1f",mbytes);
    } else {
	str_bytes.sprintf("%0.3f",mbytes);
    }

    lblFileSize->setText(i18n("(Resulting file size: %1 MB)").arg(str_bytes));
}

//***************************************************************************
void NewSignalDialog::setLengthPercentage(int percent)
{
    if (m_recursive) return; // don't do recursive processing
    if (rate() <= 0) return;
    m_recursive = true;

    double samples = maxSamples() * (double)percent / 100.0;

    // update the other controls
    edSamples->setValue(samples);
    setHMS(samples);
    updateFileSize();
    btOK->setEnabled(samples > 0.0);

    m_recursive = false;
}

//***************************************************************************
void NewSignalDialog::setHMS(const double &samples)
{
    double rate = this->rate();
    if (rate <= 0.0) return;

    int total_sec = (int)ceil(samples / rate);
    int hours   = total_sec / (60*60);
    int minutes = (total_sec / 60) % 60;
    int seconds = total_sec % 60;

    sbHours->setValue(hours);
    sbMinutes->setValue(minutes);
    sbSeconds->setValue(seconds);
}

//***************************************************************************
//***************************************************************************
