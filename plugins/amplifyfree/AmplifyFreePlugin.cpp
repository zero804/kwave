/***************************************************************************
  AmplifyFreePlugin.cpp  -  Plugin for free amplification curves
                             -------------------
    begin                : Sun Sep 02 2001
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
#include <errno.h>
#include <string.h> // ###
#include <stdio.h> // ###

#include <qarray.h> // ###
#include <qstringlist.h>
#include <klocale.h>

#include <arts/artsflow.h>
#include <arts/connect.h>
#include <arts/objectmanager.h> // ###
#include <arts/stdsynthmodule.h> // ###

#include "libkwave/InsertMode.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Parser.h"
#include "libkwave/Sample.h" // ###
#include "libkwave/SampleReader.h" // ###
#include "libkwave/SampleWriter.h" // ###

#include "kwave/PluginManager.h"
#include "kwave/UndoTransactionGuard.h"

#include "AmplifyFreePlugin.h"
#include "AmplifyFreeDialog.h"

KWAVE_PLUGIN(AmplifyFreePlugin,"amplifyfree","Thomas Eschenbacher");

//***************************************************************************
AmplifyFreePlugin::AmplifyFreePlugin(PluginContext &context)
    :KwavePlugin(context), m_params(), m_stop(false)
{
}

//***************************************************************************
AmplifyFreePlugin::~AmplifyFreePlugin()
{
}

//***************************************************************************
int AmplifyFreePlugin::interpreteParameters(QStringList &params)
{
    // all params are curve params
    m_params = params;
    return 0;
}

//***************************************************************************
QStringList *AmplifyFreePlugin::setup(QStringList &previous_params)
{
    // try to interprete the previous parameters
    interpreteParameters(previous_params);

    // create the setup dialog
    AmplifyFreeDialog *dialog = new AmplifyFreeDialog(parentWidget());
    ASSERT(dialog);
    if (!dialog) return 0;

    if (!m_params.isEmpty()) dialog->setParams(m_params);

    QStringList *list = new QStringList();
    ASSERT(list);
    if (list && dialog->exec()) {
	// user has pressed "OK"
	QString cmd = dialog->getCommand();
	Parser p(cmd);
	while (!p.isDone()) *list << p.nextParam();
	emitCommand(cmd);
    } else {
	// user pressed "Cancel"
	if (list) delete list;
	list = 0;
    }

    if (dialog) delete dialog;
    return list;
};

// example_add_impl.cc

using namespace Arts;

#include "kwave_sample_source.h"
#include "kwave_sample_sink.h"
#include <flowsystem.h>
#include <stdsynthmodule.h>
#include "libkwave/Sample.h" // ###
#include "libkwave/SampleReader.h" // ###

class Kwave_SampleSource_impl
    :public Kwave_SampleSource_skel, Arts::StdSynthModule
{
public:

     Kwave_SampleSource_impl()
	:Kwave_SampleSource_skel(), Arts::StdSynthModule(),
	reader(0)
     { }

     Kwave_SampleSource_impl(SampleReader *rdr)
	:Kwave_SampleSource_skel(), Arts::StdSynthModule(),
	reader(rdr)
    { }

    void calculateBlock(unsigned long samples)
    {
	unsigned long i;
	sample_t sample = 0;
	
	if (reader && !(reader->eof())) {
	    for(i=0;i < samples;i++) {
		*reader >> sample;
		source[i] = sample / double(1 << 23);
		if (reader->eof()) {
		    break;
		}
	    }
	}
	
	for (; i < samples; i++) {
	    source[i] = 0;
	}
	
	if (!reader || reader->eof()) {
//	    emit sigEof();
	}
    }

protected:
    SampleReader *reader;

};

class Kwave_SampleSink_impl
    :public Kwave_SampleSink_skel, Arts::StdSynthModule
{
public:

	void calculateBlock(unsigned long samples)
	{
//		if(samples > maxsamples)
//		{
//			maxsamples = samples;
//			if(outblock) delete[] outblock;
//			outblock = new uchar[maxsamples * 4]; // 2 channels, 16 bit
//		}
//
//		convert_stereo_2float_i16le(samples,left,right, outblock);
//		fwrite(outblock, 1, 4 * samples,out);
	}

	/*
	 * this is the most tricky part here - since we will run in a context
	 * where no audio hardware will play the "give me more data role",
	 * we'll have to request things ourselves (requireFlow() tells the
	 * flowsystem that more signal flow should happen, so that
	 * calculateBlock will get called
	 */
	void goOn()
	{
		_node()->requireFlow();
	}

};

REGISTER_IMPLEMENTATION(Kwave_SampleSource_impl);
REGISTER_IMPLEMENTATION(Kwave_SampleSink_impl);


//***************************************************************************
void AmplifyFreePlugin::run(QStringList)
{
    unsigned int first, last;

//    UndoTransactionGuard undo_guard(*this, i18n("amplify free"));
    m_stop = false;

    MultiTrackReader source;

    unsigned int input_length = selection(&first, &last);
    if (first == last) {
	input_length = signalLength()-1;
	first = 0;
	last = input_length-1;
    }
    openMultiTrackReader(source, selectedTracks(), first, last);

    fprintf(stderr, "---%s:%d---\n",__FILE__, __LINE__);

//    QArray<sample_t> m_zeroes;
//    unsigned int ZERO_COUNT = 65536;
//
//    MultiTrackWriter writers;
//    manager().openMultiTrackWriter(writers, Overwrite);
//
//    // break if aborted
//    if (writers.isEmpty()) return;
//
//    unsigned int first = writers[0]->first();
//    unsigned int last  = writers[0]->last();
//    unsigned int count = writers.count();
//
//    // get the buffer with zeroes for faster filling
//    if (m_zeroes.count() != ZERO_COUNT) {
//	m_zeroes.resize(ZERO_COUNT);
//	m_zeroes.fill(0);
//    }
//
//    // loop over the sample range
//    while ((first <= last) && (!m_stop)) {
//	unsigned int rest = last - first + 1;
//	if (rest < m_zeroes.count()) m_zeroes.resize(rest);
//	
//	// loop over all writers
//	unsigned int w;
//	for (w=0; w < count; w++) {
//	    *(writers[w]) << m_zeroes;
//	}
//	
//	first += m_zeroes.count();
//    }

    fprintf(stderr, "---%s:%d---\n",__FILE__, __LINE__);
    Arts::Dispatcher dispatcher;
    dispatcher.lock();

    fprintf(stderr, "---%s:%d---\n",__FILE__, __LINE__);
//    for (track=0; track < tracks; track++) {
	SampleReader *reader = source.at(0);
	
    fprintf(stderr, "---%s:%d---\n",__FILE__, __LINE__);

    Kwave_SampleSource src_adapter = Kwave_SampleSource::_from_base(
	new Kwave_SampleSource_impl(reader));

    fprintf(stderr, "---%s:%d---\n",__FILE__, __LINE__);

    Arts::Synth_PLAY      play;

//    setValue(freq1, 440.0);	  // set frequencies
//    Arts::connect(freq1, sin1);	  // object connection
//    Arts::connect(sin1, play, "invalue_left");
//    Arts::connect(sin2, play, "invalue_right");

    fprintf(stderr, "---%s:%d\n",__FILE__, __LINE__);
    Arts::connect(src_adapter, play, "invalue_left");
    fprintf(stderr, "---%s:%d\n",__FILE__, __LINE__);

    src_adapter.start();
    play.start();

    dispatcher.run();

    fprintf(stderr, "---%s:%d\n",__FILE__, __LINE__);

//    writers.setAutoDelete(true);
//    writers.clear();

    fprintf(stderr, "---%s:%d---\n",__FILE__, __LINE__);
    dispatcher.unlock();

    close();
}

//***************************************************************************
int AmplifyFreePlugin::stop()
{
    m_stop = true;
    return KwavePlugin::stop();
}

//***************************************************************************
//***************************************************************************
