/***************************************************************************
              Track.cpp  -  collects one or more stripes in one track
			     -------------------
    begin                : Feb 10 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mt/Mutex.h"
#include "mt/MutexGuard.h"
#include "mt/MutexSet.h"

#include "libkwave/SampleInputStream.h"
#include "libkwave/Stripe.h"
#include "libkwave/Track.h"

//***************************************************************************
Track::Track()
    :m_stripes(), m_lock_stripes()
{
}

//***************************************************************************
Track::Track(unsigned int length)
    :m_stripes(), m_lock_stripes()
{
    appendStripe(length);
}

//***************************************************************************
Stripe *Track::appendStripe(unsigned int length)
{
    MutexGuard lock(m_lock_stripes);

    Stripe *s = new Stripe(length);
    ASSERT(s);
    if (s) m_stripes.append(s);
    debug("Track::appendStripe(%d): new stripe at %p", length, s);

    return s;
}

//***************************************************************************
//void Track::appendSamples(const QArray<sample_t> &samples)
//{
//    if (!samples.size()) return; // nothing to do
//
//    // find the last stripe
//    MutexGuard lock(m_lock_stripes);
//    Stripe *s = 0;
//    if (m_stripes.count() == 0) {
//	// no stripe yet, add a new one
//	s = new Stripe(samples);
//	ASSERT(s);
//	if (!s || (s->length() < samples.count()) ) {
//	    warning("allocating first stripe of track failed.");
//	    return;
//	}
//    } else {
//	// get the last stripe
//	s = m_stripes.last();
//    }
//
//    ASSERT(s);
//    if (!s) {
//	warning("no last stripe for appending data?");
//	return;
//    }
//
//    // append samples to the last stripe
//    *s << samples;
//
//}

//***************************************************************************
SampleInputStream *Track::openInputStream(InsertMode mode,
	unsigned int left, unsigned int right)
{
    MutexGuard lock(m_lock_stripes);
    MutexSet stripe_locks;
    QList<Stripe> stripes;

    switch (mode) {
	case Append: {
	    debug("Track::openInputStream(apppend)");
	
	    // create a new stripe
	    Stripe *s = new Stripe(0);
	    ASSERT(s);
	    if (!s) return 0;
	
	    // add to our stripes list and lock it
	    m_stripes.append(s);
	    stripe_locks.addLock(s->mutex());

	    // use new created stripe as start
            stripes.append(s);
	    break;
	}
	case Insert:
	    debug("Track::openInputStream(insert, %u)", left);
	    break;
	case Overwrite:
	    debug("Track::openInputStream(overwrite, %u, %u)", left, right);
	    break;
    }

    return new SampleInputStream(*this, stripes,
	stripe_locks, mode, left, right);
}

//***************************************************************************
//***************************************************************************
