/***************************************************************************
         GotoDialog.cpp  -  dialog for selecting a position
                             -------------------
    begin                : Sat Dec 06 2008
    copyright            : (C) 2008 by Thomas Eschenbacher
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

#include "GotoDialog.h"

//***************************************************************************
Kwave::GotoDialog::GotoDialog(QWidget *widget, Mode mode, sample_index_t pos,
                              double sample_rate, sample_index_t signal_length)
    :QDialog(widget), Ui::GotoDlg()
{
    setupUi(this);
    setModal(true);

    if (select_pos) {
        select_pos->init(mode, pos, sample_rate, 0, signal_length);
        select_pos->setTitle(QString());
    }

    setMinimumSize(sizeHint());
    setFixedSize(sizeHint());
}

//***************************************************************************
Kwave::GotoDialog::~GotoDialog()
{
}

//***************************************************************************
void Kwave::GotoDialog::setMode(Kwave::SelectTimeWidget::Mode new_mode)
{
    if (select_pos) select_pos->setMode(new_mode);
}

//***************************************************************************
#include "GotoDialog.moc"
//***************************************************************************
//***************************************************************************
