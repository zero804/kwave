/***************************************************************************
          TrackPixmap.h  -  buffered pixmap for displaying a kwave track
                             -------------------
    begin                : Tue Mar 20 2001
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

#ifndef _TRACK_PIXMAP_H_
#define _TRACK_PIXMAP_H_

#include "config.h"
#include <qmemarray.h>
#include <qbitarray.h>
#include <qcolor.h>
#include <qmutex.h>
#include <qobject.h>
#include <qpixmap.h>

#include "libkwave/Sample.h"

class Track;

/**
 * The TrackPixmap is a graphical representation of a track's sample
 * data. It is connected directly to a Track object so that it is able
 * to get any needed sample data on it's own. It provides internal
 * caching mechanisms for reducing (slow) accesses to the track; this
 * is especially needed for speeding up the handling large wav files.
 *
 * @note The sample ranges in this class are a kind of "virtual", it is
 *       possible that the length of this pixmap in samples is larger
 *       than the track. However, this should not do any harm and might
 *       even be useful if a track grows.
 *
 * @todo If the "interpolated mode" is used, the sample buffer should
 *       contain some samples before and some samples after the current
 *       view. (m_extra_samples, calculated in set_zoom, !=0 only in
 *       interpolation mode, ignored in all other modes.
 *
 * @todo Check setOffset()
 * @todo optimizations if zoom factor is multiple of last zoom factor
 * @todo optimizations in slotSamplesDeleted and slotSamplesInserted if
 *       parts of the current buffers can be re-used
 *
 * @author Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
 */
class TrackPixmap : public QObject, public QPixmap
{
    Q_OBJECT

public:
    /** Default constructor */
    TrackPixmap(Track &track);

    /** Destructor */
    virtual ~TrackPixmap();

    /**
     * Resize the pixmap.
     * @param width new width in pixels
     * @param height new height in pixels
     * @see QPixmap::resize()
     */
    void resize(int width, int height);

    /**
     * Repaints the current pixmap. After the repaint the signal is no
     * longer in status "modified". If it was not modified before, this
     * is a no-op.
     */
    void repaint();

    /**
     * Returns "true" if the buffer has changed and the pixmap has to
     * be re-painted.
     */
    bool isModified();

signals:

    /** Emitted if the content of the pixmap was modified. */
    void sigModified();

public slots:

    /**
     * Sets a new sample offset and moves the signal display
     * left or right. Only the new areas that were moved in
     * will be redrawn.
     * @param offset index of the first visible sample
     */
    void setOffset(unsigned int offset);

    /**
     * Sets a new zoom factor in samples per pixel. This normally
     * affects the number of visible samples and a redraw of
     * the current view.
     */
    void setZoom(double zoom);

private slots:

    /**
     * Connected to the track's sigSamplesInserted.
     * @param src source track
     * @param offset position from which the data was inserted
     * @param length number of samples inserted
     * @see Track::sigSamplesInserted
     * @internal
     */
    void slotSamplesInserted(Track &src, unsigned int offset,
                             unsigned int length);

    /**
     * Connected to the track's sigSamplesDeleted.
     * @param src source track
     * @param offset position from which the data was removed
     * @param length number of samples deleted
     * @see Track::sigSamplesDeleted
     * @internal
     */
    void slotSamplesDeleted(Track &src, unsigned int offset,
                            unsigned int length);

    /**
     * Connected to the track's sigSamplesModified
     * @param src source track
     * @param offset position from which the data was modified
     * @param length number of samples modified
     * @see Track::sigSamplesModified
     * @internal
     */
    void slotSamplesModified(Track &src, unsigned int offset,
                             unsigned int length);

private:

    /**
     * Resizes the current buffer and sets all new entries to
     * invalid (if any).
     */
    void resizeBuffer();

    /**
     * Sets the current buffer to "invalid" state. Note: this does
     * not include any resize!
     */
    void invalidateBuffer();

    /**
     * Adapts the current mode and size of the buffers and fills all
     * areas that do not contain valid data with fresh samples. In other
     * words: it makes the buffer "valid" and consistent.
     * @return true if the buffer content has changed, false if no
     *         invalid samples were found
     */
    bool validateBuffer();

    /**
     * Draws the signal as an overview with multiple samples per
     * pixel.
     * @param p reference to a QPainter
     * @param middle the y position of the zero line [pixels]
     * @param height the height of the pixmap [pixels]
     * @param first the offset of the first pixel
     * @param last the offset of the last pixel
     */
    void drawOverview(QPainter &p, int middle, int height,
	int first, int last);

    /**
     * Calculates the parameters for interpolation of the graphical
     * display when zoomed in. Allocates (new) buffer for the
     * filter coefficients of the low pass filter used for interpolation.
     * @see #interpolation_alpha
     */
    void calculateInterpolation();

    /**
     * Draws the signal and interpolates the pixels between the
     * samples. The interpolation is done by using a simple FIR
     * lowpass filter.
     * @param p reference to a QPainter
     * @param width the width of the pixmap in pixels
     * @param middle the y position of the zero line in the drawing
     *               area [pixels]
     * @param height the height of the drawing are [pixels]
     * @see #calculateInterpolation()
     */
    void drawInterpolatedSignal(QPainter &p, int width, int middle,
	int height);

    /**
     * Draws the signal and connects the pixels between the samples
     * by using a simple poly-line. This gets used if the current zoom
     * factor is not suitable for either an overview nor an interpolated
     * signal display.
     * @param p reference to a QPainter
     * @param width the width of the pixmap in pixels
     * @param middle the y position of the zero line in the drawing
     *               area [pixels]
     * @param height the height of the drawing are [pixels]
     */
    void drawPolyLineSignal(QPainter &p, int width, int middle, int height);

    /**
     * Converts a pixel offset into a sample offset.
     */
    inline unsigned int pixels2samples(int pixels) {
	return (unsigned int)(pixels*m_zoom);
    }

    /**
     * Converts a sample offset into a pixel offset.
     */
    inline int samples2pixels(unsigned int samples) {
	if (m_zoom==0.0) return 0;
	return (int)(samples / m_zoom);
    }

    /**
     * Converts the offset and length of an overlapping region into buffer
     * indices, depending on the current mode. If the given region does
     * not overlap at all, the length of the area will be set to zero.
     * The length will be truncated to the end of the current buffer(s).
     * @note If the resulting or given length is zero, the offset value
     *       is not necessarily valid and should be ignored!
     * @param offset reference to the source sample index, will be converted
     *               into buffer index
     * @param length reference to the length in samples, will be converted
     *               to the number of buffer indices
     */
    void convertOverlap(unsigned int &offset, unsigned int &length);

    /**
     * Reference to the track with our sample data.
     */
    Track &m_track;

    /**
     * Index of the first sample. Needed for converting pixel
     * positions into absolute sample numbers. This is always
     * in units of samples, independend of the current mode!
     */
    unsigned int m_offset;

    /**
     * Zoom factor in samples/pixel. Needed for converting
     * sample indices into pixels and vice-versa.
     */
    double m_zoom;

    /**
     * If true, we are in min/max mode. This means that m_sample_buffer
     * is not used and m_minmax_buffer contains is used instead.
     */
    bool m_minmax_mode;

    /**
     * Array with samples needed for drawing when not in min/max mode.
     * This might sometimes include one sample before or after the
     * current view.
     */
    QMemArray<sample_t> m_sample_buffer;

    /**
     * Array with minimum sample values, if in min/max mode.
     */
    QMemArray<sample_t> m_min_buffer;

    /**
     * Array with maximum sample values, if in min/max mode.
     */
    QMemArray<sample_t> m_max_buffer;

    /** Indicates that the buffer content was modified */
    bool m_modified;

    /**
     * Array with one bit for each position in the internal
     * buffers. If the bit corresponding to a certain buffer
     * index is set to "1", the position contains valid data,
     * if "0" the content of the buffer is invalid.
     */
    QBitArray m_valid;

    /** Mutex for exclusive access to the buffers. */
    QMutex m_lock_buffer;

    /**
     * order of the low pass filter used for interpolation
     */
    int m_interpolation_order;

    /**
     * Buffer for filter coefficients of the low pass used for
     * interpolation.
     * @see #calculateInterpolation()
     */
    float *m_interpolation_alpha;

    /** Background color */
    QColor m_color_background;

    /** Color for samples */
    QColor m_color_sample;

    /** Color for interpolated samples */
    QColor m_color_interpolated;

    /** Color for the zero line, used areas */
    QColor m_color_zero;

    /** Color of the zero line, unused areas */
    QColor m_color_zero_unused;

};

#endif /* _TRACK_PIXMAP_H_ */
