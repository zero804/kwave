/***************************************************************************
         ZeroPlugin.cpp  -  wipes out the selected range of samples to zero
                             -------------------
    begin                : Fri Jun 01 2001
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

#include "config.h"
#include <math.h>
#include <klocale.h> // for the i18n macro

#include <QList>
#include <QStringList>

#include "libkwave/MultiTrackWriter.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/SelectTimeWidget.h" // for selection mode

#include "ZeroPlugin.h"

KWAVE_PLUGIN(ZeroPlugin,"zero","2.1","Thomas Eschenbacher");

#define ZERO_COUNT (64 * 1024)

//***************************************************************************
ZeroPlugin::ZeroPlugin(const PluginContext &context)
    :Kwave::Plugin(context), m_zeroes()
{
     i18n("zero");
}

//***************************************************************************
ZeroPlugin::~ZeroPlugin()
{
}

//***************************************************************************
void ZeroPlugin::run(QStringList params)
{
    QList<unsigned int> tracks;
    unsigned int first = 0;
    unsigned int last  = 0;

    UndoTransactionGuard undo_guard(*this, i18n("silence"));

    MultiTrackWriter *writers = 0;

    /*
     * new mode: insert a range filled with silence:
     * -> usage: zero(<mode>, <range>)
     */
    if (params.count() == 2) {
	// get the current selection
	selection(&tracks, &first, &last, false);

	// mode for the time (like in selectrange plugin)
	bool ok = true;
	int mode = params[0].toInt(&ok);
	Q_ASSERT(ok);
	if (!ok) return;
	Q_ASSERT((mode == static_cast<int>(SelectTimeWidget::byTime)) ||
	         (mode == static_cast<int>(SelectTimeWidget::bySamples)) ||
	         (mode == static_cast<int>(SelectTimeWidget::byPercents)));
	if ((mode != static_cast<int>(SelectTimeWidget::byTime)) &&
	    (mode != static_cast<int>(SelectTimeWidget::bySamples)) &&
	    (mode != static_cast<int>(SelectTimeWidget::byPercents)))
	{
	    return;
	}

	// length of the range of zeroes to insert
	unsigned int time = params[1].toUInt(&ok);
	Q_ASSERT(ok);
	if (!ok) return;

	// convert from time to samples
	unsigned int length = SelectTimeWidget::timeToSamples(
	    static_cast<SelectTimeWidget::Mode>(mode),
	    time, signalRate(), signalLength());

	// some sanity check
	Q_ASSERT(length);
	Q_ASSERT(!tracks.isEmpty());
	if (!length || tracks.isEmpty()) return; // nothing to do

	last  = first + length - 1;
	writers = new MultiTrackWriter(signalManager(),
	    tracks, Insert, first, last);
    } else {
	writers = new MultiTrackWriter(signalManager(), Overwrite);
    }

    Q_ASSERT(writers);
    if (!writers) return; // out-of-memory

    // break if aborted
    if (!writers->tracks()) return;

    // connect the progress dialog
    connect(writers, SIGNAL(progress(unsigned int)),
	    this,    SLOT(updateProgress(unsigned int)),
	    Qt::BlockingQueuedConnection);

    first = (*writers)[0]->first();
    last  = (*writers)[0]->last();
    unsigned int count = writers->tracks();

    // get the buffer with zeroes for faster filling
    if (m_zeroes.size() != ZERO_COUNT) {
	m_zeroes.resize(ZERO_COUNT);
	m_zeroes.fill(0);
    }
    Q_ASSERT(m_zeroes.size() == ZERO_COUNT);

    // loop over the sample range
    while ((first <= last) && !shouldStop()) {
	unsigned int rest = last - first + 1;
	if (rest < m_zeroes.size()) m_zeroes.resize(rest);

	// loop over all writers
	unsigned int w;
	for (w = 0; w < count; w++) {
	    *((*writers)[w]) << m_zeroes;
	}

	first += m_zeroes.size();
    }

    delete writers;
}

//***************************************************************************
#include "ZeroPlugin.moc"
//***************************************************************************
//***************************************************************************
