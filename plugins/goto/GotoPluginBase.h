/***************************************************************************
       GotoPluginBase.h  -  base class for the goto plugin
                             -------------------
    begin                : Thu May 12 2011
    copyright            : (C) 2011 by Thomas Eschenbacher
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

#ifndef _GOTO_PLUGIN_BASE_H_
#define _GOTO_PLUGIN_BASE_H_

#include "config.h"

#include <QObject>
#include <QString>
#include <QStringList>

#include "libkwave/KwavePlugin.h"
#include "libgui/SelectTimeWidget.h"

class QStringList;

class GotoPluginBase: public Kwave::Plugin
{
    Q_OBJECT

public:

    /** Constructor */
    GotoPluginBase(const PluginContext &context);

    /** Destructor */
    virtual ~GotoPluginBase();

    /**
     * Shows a dialog for selecting the range and emits a command
     * for applying the selection if OK has been pressed.
     * @see Kwave::Plugin::setup
     */
    virtual QStringList *setup(QStringList &previous_params);

    /**
     * selects the position
     * @see Kwave::Plugin::start()
     */
    virtual int start(QStringList &params);

protected:

    /** Returns the command to be emitted */
    virtual QString command() const = 0;
    
    /** Returns the title of the dialog */
    virtual QString title() const = 0;

    /** Reads values from the parameter list */
    int interpreteParameters(QStringList &params);

private:

    /** selected mode for position: by time, samples, percentage */
    SelectTimeWidget::Mode m_mode;

    /** position in milliseconds, samples or percents */
    unsigned int m_position;
};

#endif /* _GOTO_PLUGIN_BASE_H_ */