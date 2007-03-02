/*************************************************************************
      KwaveFileDialog.h  -  enhanced KFileDialog
                             -------------------
    begin                : Thu May 30 2002
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

#ifndef _KWAVE_FILE_DIALOG_H_
#define _KWAVE_FILE_DIALOG_H_

#include "config.h"
#include <qobject.h>
#include <qstring.h>
#include <kfiledialog.h>

class QWidget;

/**
 * @class KwaveFileDialog
 * An improved version of KFileDialog that does not forget the last
 * directory and pre-selects the last file extension.
 */
class KwaveFileDialog: public KFileDialog
{
    Q_OBJECT
public:
    /**
     * Constructor.
     * @see KFileFialog
     */
    KwaveFileDialog(const QString& startDir, const QString& filter,
                    QWidget *parent, const char *name, bool modal,
                    const QString last_url = 0,
                    const QString last_ext = 0,
                    QWidget *widget = 0);

    /** Destructor */
    virtual ~KwaveFileDialog() {} ;

    /**
     * Returns the last used extension, including "*."
     */
    QString selectedExtension();

protected:

    /** load last settings */
    void loadConfig(const QString &section);

protected slots:

    /** save current settings */
    void saveConfig();

private:

    /** name of the group in the config file */
    QString m_config_group;

    /** last opened URL */
    QString m_last_url;

    /** extension of the last selected single URL or file */
    QString m_last_ext;

};

#endif /* _KWAVE_FILE_DIALOG_H_ */
