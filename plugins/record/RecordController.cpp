/*************************************************************************
   RecordController.cpp  -  controller/state maching for the audio recorder
                             -------------------
    begin                : Sat Oct 04 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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
#include "RecordController.h"
#include "RecordState.h"

//***************************************************************************
static const char *state2str(const RecordState state)
{
    switch (state) {
	case REC_EMPTY:			return "REC_EMPTY";
	case REC_BUFFERING:		return "REC_BUFFERING";
	case REC_WAITING_FOR_TRIGGER:	return "REC_WAITING_FOR_TRIGGER";
	case REC_PRERECORDING:		return "REC_PRERECORDING";
	case REC_RECORDING:		return "REC_RECORDING";
	case REC_PAUSED:		return "REC_PAUSED";
	case REC_DONE:			return "REC_DONE";
    }
    return "-INVALID-";
}

//***************************************************************************
RecordController::RecordController()
    :QObject(), m_state(REC_EMPTY), m_trigger_set(false),
     m_use_prerecording(false)
{
}

//***************************************************************************
RecordController::~RecordController()
{
}

//***************************************************************************
void RecordController::actionReset()
{
    qDebug("RecordController::actionReset");
    switch (m_state) {
	case REC_EMPTY:
	    // already empty, nothing to do
	    break;
	case REC_BUFFERING:
	case REC_WAITING_FOR_TRIGGER:
	case REC_PRERECORDING:
	    // abort, change to REC_EMPTY
	    emit sigStopRecord();
	    break;
	case REC_RECORDING:
	    // fall back to REC_WAITING_FOR_TRIGGER
	    emit stateChanged(m_state = REC_WAITING_FOR_TRIGGER);
	    break;
	case REC_PAUSED:
	    // stop recording and reset
	    emit sigReset();
	    break;
	case REC_DONE:
	    emit sigReset();
	    break;
    }
}

//***************************************************************************
void RecordController::actionStop()
{
    qDebug("RecordController::actionStop");
    switch (m_state) {
	case REC_EMPTY:
	case REC_DONE:
	    // already stopped, nothing to do
	    break;
	case REC_BUFFERING:
	case REC_WAITING_FOR_TRIGGER:
	case REC_PRERECORDING:
	case REC_RECORDING:
	case REC_PAUSED:
	    // abort, change to REC_EMPTY or REC_DONE
	    emit sigStopRecord();
	    break;
    }
}

//***************************************************************************
void RecordController::actionPause()
{
    qDebug("RecordController::actionPause");
    switch (m_state) {
	case REC_EMPTY:
	case REC_DONE:
	    // what do you want ?
	    break;
	case REC_BUFFERING:
	case REC_WAITING_FOR_TRIGGER:
	case REC_PRERECORDING:
	    // this should never happen
	    qWarning("RecordController::actionPause(): "
	             "state = %s ???", state2str(m_state));
	    break;
	case REC_RECORDING:
	    // pause recording
	    emit sigStopRecord();
	    break;
	case REC_PAUSED:
	    // continue recording
	    emit sigStartRecord();
	    break;
    }
}

//***************************************************************************
void RecordController::actionStart()
{
    qDebug("RecordController::actionStart");
    switch (m_state) {
	case REC_EMPTY:
	case REC_DONE:
	case REC_BUFFERING:
	case REC_PRERECORDING:
	case REC_WAITING_FOR_TRIGGER:
	    // interprete this as manual trigger
	    emit sigStartRecord();
	    break;
	case REC_PAUSED:
	    // interprete this as "continue"
	    emit sigStartRecord();
	    break;
	case REC_RECORDING:
	    // already recording...
	    break;
    }
}

//***************************************************************************
void RecordController::deviceRecordStarted()
{
    qDebug("RecordController::deviceRecordStarted");
    switch (m_state) {
	case REC_EMPTY:
	case REC_PAUSED:
	    // continue, pre-recording or trigger
	    qDebug("RecordController::deviceRecordStarted -> REC_BUFFERING");
	    emit stateChanged(m_state = REC_BUFFERING);
	    break;
	case REC_BUFFERING:
	case REC_WAITING_FOR_TRIGGER:
	case REC_PRERECORDING:
	case REC_RECORDING:
	case REC_DONE:
	    // this should never happen
	    qWarning("RecordController::deviceRecordStarted(): "
	             "state = %s ???", state2str(m_state));
	    break;
    }
}

//***************************************************************************
void RecordController::deviceBufferFull()
{
    qDebug("RecordController::deviceBufferFull");
    switch (m_state) {
	case REC_EMPTY:
	case REC_WAITING_FOR_TRIGGER:
	case REC_PRERECORDING:
	case REC_RECORDING:
	case REC_PAUSED:
	case REC_DONE:
	    // this should never happen
	    qWarning("RecordController::deviceBufferFull(): "
	             "state = %s ???", state2str(m_state));
	    break;
	case REC_BUFFERING:
	    if (m_trigger_set) {
		// trigger was set
		qDebug("RecordController::deviceBufferFull -> REC_WAITING_FOR_TRIGGER");
		m_state = REC_WAITING_FOR_TRIGGER;
	    } else if (m_use_prerecording) {
		// prerecording was set
		qDebug("RecordController::deviceBufferFull -> REC_PRERECORDING");
		m_state = REC_PRERECORDING;
	    } else {
		// default: just start recording
		qDebug("RecordController::deviceBufferFull -> REC_RECORDING");
		m_state = REC_RECORDING;
	    }
	    emit stateChanged(m_state);
	    break;
    }
}

//***************************************************************************
void RecordController::deviceTriggerReached()
{
    qDebug("RecordController::deviceTriggerReached");
    switch (m_state) {
	case REC_EMPTY:
	case REC_BUFFERING:
	case REC_PRERECORDING:
	case REC_RECORDING:
	case REC_PAUSED:
	case REC_DONE:
	    // this should never happen
	    qWarning("RecordController::deviceTriggerReached(): "
	             "state = %s ???", state2str(m_state));
	    break;
	case REC_WAITING_FOR_TRIGGER:
	    Q_ASSERT(m_trigger_set);
	    if (m_use_prerecording) {
		// prerecording was set
		qDebug("RecordController::deviceTriggerReached -> REC_PRERECORDING");
		m_state = REC_PRERECORDING;
	    } else {
		// default: just start recording
		m_state = REC_RECORDING;
		qDebug("RecordController::deviceTriggerReached -> REC_RECORDING");
	    }
	    emit stateChanged(m_state);
	    break;
    }
}

//***************************************************************************
void RecordController::deviceRecordStopped()
{
    qDebug("RecordController::deviceRecordStopped");
    switch (m_state) {
	case REC_EMPTY:
	case REC_DONE:
	    // this should never happen
	    qWarning("RecordController::deviceRecordStopped(): "
	             "state = %s ???", state2str(m_state));
	    break;
	case REC_BUFFERING:
	case REC_PRERECORDING:
	case REC_WAITING_FOR_TRIGGER:
	    // abort, no real data preoduced
	    qDebug("RecordController::deviceRecordStopped -> REC_EMPTY");
	    emit stateChanged(m_state = REC_EMPTY);
	    break;
	case REC_RECORDING:
	case REC_PAUSED:
	    // recording done
	    qDebug("RecordController::deviceRecordStopped -> REC_DONE");
	    emit stateChanged(m_state = REC_DONE);
	    break;
    }
}

//***************************************************************************
//***************************************************************************
