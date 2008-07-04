/***************************************************************************
       OverViewWidget.h  -  horizontal slider with overview over a signal
                             -------------------
    begin                : Tue Oct 21 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#ifndef _OVER_VIEW_WIDGET_H_
#define _OVER_VIEW_WIDGET_H_

#include "config.h"

#include <QBitmap>
#include <QSize>
#include <QTimer>
#include <QWidget>

#include <kdemacros.h>

#include "libgui/ImageView.h"
#include "libgui/OverViewCache.h"

class QMouseEvent;
class QResizeEvent;
class SignalManager;
class Track;

class KDE_EXPORT OverViewWidget : public ImageView
{
    Q_OBJECT
public:

    OverViewWidget(SignalManager &signal, QWidget *parent = 0);

    /** Destructor */
    virtual ~OverViewWidget();

    /** minimum size of the widget, @see QWidget::minimumSize() */
    virtual QSize minimumSize() const;

    /** optimal size for the widget, @see QWidget::sizeHint() */
    virtual QSize sizeHint() const;

public slots:

    /**
     * Sets new range parameters of the slider, using a scale that is calculated
     * out of the slider's maximum position. All parameters are given in the
     * user's coordinates/units (e.g. samples).
     * @param offset index of the first visible sample
     * @param viewport width of the visible area
     * @param total width of the whole signal
     */
    void setRange(unsigned int offset, unsigned int viewport,
                  unsigned int total);

protected:

    /** refreshes the bitmap when resized */
    void resizeEvent(QResizeEvent *);

    /**
     * On mouse move:
     * move the current viewport center to the clicked position.
     */
    void mouseMoveEvent(QMouseEvent *);

    /**
     * On single-click with the left mouse button:
     * move the current viewport center to the clicked position.
     */
    void mousePressEvent(QMouseEvent *);

    /**
     * On double click with the left mouse button, without shift:
     * move the current viewport center to the clicked position, like
     * on a single-click, but also zoom in (by sending "zoomin()").
     *
     * When double clicked with the left mouse button with shift:
     * The same as above, but zoom out instead of in (by sending "zoomout()").
     */
    void mouseDoubleClickEvent(QMouseEvent *e);

protected slots:

    /** refreshes all modified parts of the bitmap */
    void refreshBitmap();

    /**
     * connected to the m_repaint_timer, called when it has
     * elapsed and the signal has to be repainted
     */
    void overviewChanged();

signals:

    /**
     * Will be emitted if the slider position has changed. The value
     * is in user's units (e.g. samples).
     */
    void valueChanged(unsigned int new_value);

    /** emitted for zooming in and out via command */
    void sigCommand(const QString &command);

protected:

    /**
     * Converts a pixel offset within the overview's drawing area
     * into the user's coordinate system.
     * @param pixels the pixel coordinate [0...width-1]
     * @return an offset [0..length-1]
     */
    int pixels2offset(int pixels);

private:

    /** index of the first visible sample */
    unsigned int m_view_offset;

    /** width of the visible area [samples] */
    unsigned int m_view_width;

    /** length of the whole area [samples] */
    unsigned int m_signal_length;

    /** last emitted offset (for avoiding duplicate events) */
    unsigned int m_last_offset;

    /** cache with overview data */
    OverViewCache m_cache;

    /** timer for limiting the number of repaints per second */
    QTimer m_repaint_timer;

};

#endif // _OVER_VIEW_WIDGET_H_
