/*************************************************************************
           ZeroPlugin.h  -  wipes out the selected range of samples to zero
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

#ifndef _ZERO_PLUGIN_H_
#define _ZERO_PLUGIN_H_

#include <config.h>

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "libkwave/Plugin.h"
#include "libkwave/SampleArray.h"
#include "libkwave/Sample.h"

namespace Kwave
{
    /**
     * @class ZeroPlugin
     * This is a very simple plugin that blanks the currently selected range of
     * samples with zeroes.
     */
    class ZeroPlugin: public Kwave::Plugin
    {
	Q_OBJECT

    public:

	/** Constructor */
	ZeroPlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~ZeroPlugin();

	/** Returns the name of the plugin. */
	virtual QString name() const;

	/**
	 * Fills the selected area with zeroes
	 * @param params list of strings with parameters
	 */
	virtual void run(QStringList params);

    private:

	/** use an array with zeroes for faster filling */
	Kwave::SampleArray m_zeroes;

    };
}

#endif /* _ZERO_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
