/***************************************************************************
     AboutKwaveDialog.h  -  dialog for Kwave's "Help-About"
                             -------------------
    begin                : Sun Feb 10 2002
    copyright            : (C) 2002 by Ralf Waspe
    email                : rwaspe@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _ABOUT_KWAVE_DIALOG_H_
#define _ABOUT_KWAVE_DIALOG_H_

#include <kaboutdata.h>
#include <kdialog.h>

#include "KwaveAboutDialogBase.uih.h"
#include "LogoWidget.h"

/**
 * @class AboutKwaveDialog
 * Dialog for Help/About
 */
class AboutKwaveDialog : public KwaveAboutDialogBase
{
    Q_OBJECT

public:
    /** Constructor */
    AboutKwaveDialog(QWidget *parent);

    /** destructor */
    virtual ~AboutKwaveDialog();

};

#endif  /* _ABOUT_KWAVE_DIALOG_H_ */
