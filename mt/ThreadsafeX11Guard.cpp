/***************************************************************************
  ThreadsafeX11Guard.cpp -  guard for using X11 from a worker thread
			     -------------------
    begin                : Sun Jun 03 2001
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

#include <qapplication.h> // for flushX and syncX
#include <qmutex.h>

#include "mt/MutexGuard.h"
#include "mt/Semaphore.h"
#include "mt/ThreadsafeX11Guard.h"

//***************************************************************************

/* static initializer for the global/static X11 lock */
QMutex ThreadsafeX11Guard::m_lock_X11;

/* static initializer for the internal/static X11 lock */
QMutex ThreadsafeX11Guard::m_internal_lock;

/* static initializer for the lock for protection of recursion and owner */
QMutex ThreadsafeX11Guard::m_lock_recursion;

/* static initializer for the thread that currently has locked X11 */
pthread_t ThreadsafeX11Guard::m_pid_owner = pthread_self();

/* static initializer for the thread that owns X11 (main thread) */
pthread_t ThreadsafeX11Guard::m_pid_x11 = pthread_self();

/* static initializer for the counter of recursive X11 locks. */
unsigned int ThreadsafeX11Guard::m_recursion_level = 0;

//***************************************************************************
ThreadsafeX11Guard::ThreadsafeX11Guard()
    :QObject(), m_sem_x11_locked(), m_sem_x11_done(),
     m_sem_x11_unlocked(), m_spx_X11_request(this, SLOT(lockX11()))
{
    if (m_pid_x11 == pthread_self()) {
	return;
    }

    {
	MutexGuard lock(m_lock_recursion);
	if (m_pid_owner == pthread_self()) {
	    // recursive enter
	    m_recursion_level++;
	    return;
	}
    }

    // lock the X11 system, no others may use it now
    m_internal_lock.lock();

    // activate the X11 thread
    m_spx_X11_request.AsyncHandler();

    // sometimes the X11 / GUI thread needs an extra wakeup
    Q_ASSERT(qApp);
    if (qApp) qApp->wakeUpGuiThread();

    // wait until the X11 thread is locked and suspended
    m_sem_x11_locked.wait();

    // now we are the only owner of X11
    {
	MutexGuard lock(m_lock_recursion);
	m_pid_owner = pthread_self();
	m_recursion_level = 1;
    }
}

//***************************************************************************
ThreadsafeX11Guard::~ThreadsafeX11Guard()
{
    if (m_pid_x11 == pthread_self()) {
	return;
    }

    {
	MutexGuard lock(m_lock_recursion);

	// decrease the recursion level (should always be at least 1)
	Q_ASSERT(m_recursion_level);
	if (m_recursion_level) m_recursion_level--;

	// break if only recursion decreased and zero not reached
	if (m_recursion_level) return;
    }

    // flush all X11 events
    QApplication::flushX();

    // let the X11 thread continue
    m_sem_x11_done.post();

    // wait until X11 is unlocked again
    m_sem_x11_unlocked.wait();

    // release the control over X11
    {
	MutexGuard lock(m_lock_recursion);
	m_pid_owner = 0;
	m_recursion_level = 0;
    }

    // sometimes the X11 / GUI thread needs an extra wakeup
    Q_ASSERT(qApp);
    if (qApp) qApp->wakeUpGuiThread();

    // release the X11 system and let others lock it
    m_internal_lock.unlock();
}

//***************************************************************************
void ThreadsafeX11Guard::lockX11()
{
    // lock the global/static X11 mutex
    m_lock_X11.lock();

    // be sure there are no other X11 transactions we interrupt
    // so better give them a chance to complete within the
    // context of our thread
    QApplication::syncX();
    QApplication::flushX();

    // notify the waiting worker thread that X11 is locked now
    m_sem_x11_locked.post();

    // suspend until the worker thread is done with X11
    m_sem_x11_done.wait();

    // unlock X11 again
    m_lock_X11.unlock();

    // notify the worker thread that X11 is available/unlocked again
    m_sem_x11_unlocked.post();
}

//***************************************************************************
//***************************************************************************
