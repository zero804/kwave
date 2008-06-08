/***************************************************************************
    SelectRangePlugin.h  -  Plugin for selecting a range of samples
                             -------------------
    begin                : Sat Jun 15 2002
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

#ifndef _SELECT_RANGE_PLUGIN_H_
#define _SELECT_RANGE_PLUGIN_H_

#include "config.h"

#include <QObject>
#include <QString>
#include <QStringList>

#include "libkwave/KwavePlugin.h"
#include "libgui/SelectTimeWidget.h"

class QStringList;

class SelectRangePlugin: public KwavePlugin
{
    Q_OBJECT

public:

    /** Constructor */
    SelectRangePlugin(const PluginContext &context);

    /** Destructor */
    virtual ~SelectRangePlugin();

    /**
     * Shows a dialog for selecting the range and emits a command
     * for applying the selection if OK has been pressed.
     * @see KwavePlugin::setup
     */
    virtual QStringList *setup(QStringList &previous_params);

    /**
     * selects the range
     * @see KwavePlugin::start()
     */
    virtual int start(QStringList &params);

protected:

    /** Reads values from the parameter list */
    int interpreteParameters(QStringList &params);

private:

    /** selected mode for start: by time, samples, percentage */
    SelectTimeWidget::Mode m_start_mode;

    /** selected mode for range: by time, samples, percentage */
    SelectTimeWidget::Mode m_range_mode;

    /** start in milliseconds, samples or percents */
    double m_start;

    /** range in milliseconds, samples or percents */
    double m_range;

};

#endif /* _SELECT_RANGE_PLUGIN_H_ */
