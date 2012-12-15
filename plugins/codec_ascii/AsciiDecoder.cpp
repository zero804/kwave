/*************************************************************************
       AsciiDecoder.cpp  -  decoder for ASCII data
                             -------------------
    begin                : Sun Dec 03 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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

#include <QtCore/QDateTime>
#include <QtCore/QLatin1Char>
#include <QtCore/QTextStream>

#include <klocale.h>
#include <kmimetype.h>

#include "libkwave/MessageBox.h"
#include "libkwave/MultiWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/String.h"
#include "libkwave/Writer.h"

#include "AsciiCodecPlugin.h"
#include "AsciiDecoder.h"

//***************************************************************************
Kwave::AsciiDecoder::AsciiDecoder()
    :Kwave::Decoder(), m_source(0), m_dest(0)
{
    LOAD_MIME_TYPES;
}

//***************************************************************************
Kwave::AsciiDecoder::~AsciiDecoder()
{
    if (m_source) close();
}

//***************************************************************************
Kwave::Decoder *Kwave::AsciiDecoder::instance()
{
    return new Kwave::AsciiDecoder();
}

//***************************************************************************
bool Kwave::AsciiDecoder::open(QWidget *widget, QIODevice &src)
{
    metaData().clear();
    Q_ASSERT(!m_source);
    if (m_source) qWarning("AsciiDecoder::open(), already open !");

    // try to open the source
    if (!src.open(QIODevice::ReadOnly)) {
	qWarning("failed to open source !");
	return false;
    }

    // take over the source
    m_source = &src;

    /********** Decoder setup ************/
    qDebug("--- AsciiDecoder::open() ---");

    QTextStream source(m_source);

    // read in all metadata until start of samples, EOF or user cancel
    qDebug("AsciiDecoder::open(...)");
    unsigned int linenr = 0;
    while (!source.atEnd()) {
	QString line = source.readLine().simplified();
	qDebug("META %5u %s", linenr++, DBG(line));
	if (!line.length())
	    continue; // skip empty line
	if (line.startsWith(QLatin1Char('#')) &&
	    !line.startsWith(META_PREFIX))
	    continue; // skip comment lines

	if (!line.startsWith(META_PREFIX)) {
	    // reached end of metadata
	    break;
	}
    }

    qDebug("--- THE ASCII DECODER IS NOT FUNCTIONAL YET ---");
    qDebug("---           sorry :-(                     ---");
    Kwave::MessageBox::sorry(widget,
	i18n("This is not implemented yet."),
	i18n("Sorry"));
    return false;

//     return true;
}

//***************************************************************************
bool Kwave::AsciiDecoder::decode(QWidget * /* widget */,
                                 Kwave::MultiWriter &dst)
{
    Q_ASSERT(m_source);
    if (!m_source) return false;

    m_dest = &dst;
    QTextStream source(m_source);

    // read in all remaining data until EOF or user cancel
    qDebug("AsciiDecoder::decode(...)");
    unsigned int linenr = 0;
    while (!source.atEnd() && !dst.isCanceled()) {
	QString line = source.readLine();
	qDebug("DATA %5u %s", linenr++, DBG(line));

	if (linenr > 50) break;
    }

    m_dest = 0;
    Kwave::FileInfo info(metaData());
    info.setLength(dst.last() ? (dst.last() + 1) : 0);
    metaData().replace(info);

    // return with a valid Signal, even if the user pressed cancel !
    return true;
}

//***************************************************************************
void Kwave::AsciiDecoder::close()
{
    m_source = 0;
}

//***************************************************************************
//***************************************************************************
