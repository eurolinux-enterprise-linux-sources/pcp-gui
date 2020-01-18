/*
 * Copyright (c) 2006-2007, Aconex.  All Rights Reserved.
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */
#include "main.h"
#include <pmtime.h>

#include <QtGui/QMessageBox>
#include <QtGui/QApplication>
#include <QtNetwork/QHostAddress>

TimeControl::TimeControl() : QProcess(NULL)
{
    my.tcpPort = -1;
    my.tzLength = 0;
    my.tzData = NULL;
    my.bufferLength = sizeof(PmTime::Packet);

    my.buffer = (char *)malloc(my.bufferLength);
    my.livePacket = (PmTime::Packet *)malloc(sizeof(PmTime::Packet));
    my.archivePacket = (PmTime::Packet *)malloc(sizeof(PmTime::Packet));
    if (!my.buffer || !my.livePacket || !my.archivePacket)
	nomem();
    my.livePacket->magic = PmTime::Magic;
    my.livePacket->source = PmTime::HostSource;
    my.liveState = TimeControl::Disconnected;
    my.liveSocket = new QTcpSocket(this);
    connect(my.liveSocket, SIGNAL(connected()),
				SLOT(liveSocketConnected()));
    connect(my.liveSocket, SIGNAL(disconnected()),
				SLOT(liveCloseConnection()));
    connect(my.liveSocket, SIGNAL(readyRead()),
				SLOT(liveProtocolMessage()));

    my.archivePacket->magic = PmTime::Magic;
    my.archivePacket->source = PmTime::ArchiveSource;
    my.archiveState = TimeControl::Disconnected;
    my.archiveSocket = new QTcpSocket(this);
    connect(my.archiveSocket, SIGNAL(connected()),
				SLOT(archiveSocketConnected()));
    connect(my.archiveSocket, SIGNAL(disconnected()),
				SLOT(archiveCloseConnection()));
    connect(my.archiveSocket, SIGNAL(readyRead()),
				SLOT(archiveProtocolMessage()));
}

void TimeControl::quit()
{
    disconnect(this, SIGNAL(finished(int, QProcess::ExitStatus)), this,
		    SLOT(endTimeControl()));
    if (my.liveSocket)
	disconnect(my.liveSocket, SIGNAL(disconnected()), this,
			SLOT(liveCloseConnection()));
    if (my.archiveSocket)
	disconnect(my.archiveSocket, SIGNAL(disconnected()), this,
			SLOT(archiveCloseConnection()));
    if (my.liveSocket) {
	my.liveSocket->close();
	my.liveSocket = NULL;
    }
    if (my.archiveSocket) {
	my.archiveSocket->close();
	my.archiveSocket = NULL;
    }
    terminate();
}

void TimeControl::init(int port, bool live,
		struct timeval *interval, struct timeval *position,
		struct timeval *starttime, struct timeval *endtime,
		QString tzstring, QString tzlabel)
{
    struct timeval now;
    int tzlen = tzstring.length(), lablen = tzlabel.length();

    my.tzLength = tzlen+1 + lablen+1;
    my.tzData = (char *)realloc(my.tzData, my.tzLength);
    if (!my.tzData)
	nomem();

    my.livePacket->length = my.archivePacket->length =
				sizeof(PmTime::Packet) + my.tzLength;
    my.livePacket->command = my.archivePacket->command = PmTime::Set;
    my.livePacket->delta = my.archivePacket->delta = *interval;
    if (live) {
	my.livePacket->position = *position;
	my.livePacket->start = *starttime;
	my.livePacket->end = *endtime;
	memset(&my.archivePacket->position, 0, sizeof(struct timeval));
	memset(&my.archivePacket->start, 0, sizeof(struct timeval));
	memset(&my.archivePacket->end, 0, sizeof(struct timeval));
    } else {
	__pmtimevalNow(&now);
	my.archivePacket->position = *position;
	my.archivePacket->start = *starttime;
	my.archivePacket->end = *endtime;
	my.livePacket->position = now;
	my.livePacket->start = now;
	my.livePacket->end = now;
    }
    strncpy(my.tzData, (const char *)tzstring.toAscii(), tzlen+1);
    strncpy(my.tzData + tzlen+1, (const char *)tzlabel.toAscii(), lablen+1);

    if (port < 0) {
	startTimeServer();
    } else {
	my.tcpPort = port;
	liveConnect();
	archiveConnect();
    }
}

void TimeControl::addArchive(
		struct timeval starttime, struct timeval endtime,
		QString tzstring, QString tzlabel, bool atEnd)
{
    PmTime::Packet *message;
    int tzlen = tzstring.length(), lablen = tzlabel.length();
    int sz = sizeof(PmTime::Packet) + tzlen + 1 + lablen + 1;

    if (my.archivePacket->position.tv_sec == 0) {	// first archive
	my.archivePacket->position = atEnd ? endtime : starttime;
	my.archivePacket->start = starttime;
	my.archivePacket->end = endtime;
    }

    if ((message = (PmTime::Packet *)malloc(sz)) == NULL)
	nomem();
    *message = *my.archivePacket;
    message->command = PmTime::Bounds;
    message->length = sz;
    message->start = starttime;
    message->end = endtime;
    strncpy((char *)message->data, (const char *)tzstring.toAscii(), tzlen+1);
    strncpy((char *)message->data + tzlen+1,
				(const char *)tzlabel.toAscii(), lablen+1);
    if (my.archiveSocket->write((const char *)message, sz) < 0)
	QMessageBox::warning(0,
		QApplication::tr("Error"),
		QApplication::tr("Cannot update pmtime boundaries."),
		QApplication::tr("Quit") );
    free(message);
}

void TimeControl::liveConnect()
{
    console->post("Connecting to pmtime, live source");
    my.liveSocket->connectToHost(QHostAddress::LocalHost, my.tcpPort);
}

void TimeControl::archiveConnect()
{
    console->post("Connecting to pmtime, archive source");
    my.archiveSocket->connectToHost(QHostAddress::LocalHost, my.tcpPort);
}

void TimeControl::showLiveTimeControl(void)
{
    my.livePacket->command = PmTime::GUIShow;
    my.livePacket->length = sizeof(PmTime::Packet);
    if (my.liveSocket->write((const char *)my.livePacket,
					sizeof(PmTime::Packet)) < 0)
	QMessageBox::warning(0,
                QApplication::tr("Error"),
                QApplication::tr("Cannot get pmtime to show itself."),
                QApplication::tr("Quit") );
}

void TimeControl::showArchiveTimeControl(void)
{
    my.archivePacket->command = PmTime::GUIShow;
    my.archivePacket->length = sizeof(PmTime::Packet);
    if (my.archiveSocket->write((const char *)my.archivePacket,
					sizeof(PmTime::Packet)) < 0)
	QMessageBox::warning(0,
    		QApplication::tr("Error"),
    		QApplication::tr("Cannot get pmtime to show itself."),
    		QApplication::tr("Quit") );
}

void TimeControl::hideLiveTimeControl()
{
    my.livePacket->command = PmTime::GUIHide;
    my.livePacket->length = sizeof(PmTime::Packet);
    if (my.liveSocket->write((const char *)my.livePacket,
					sizeof(PmTime::Packet)) < 0)
	QMessageBox::warning(0,
		QApplication::tr("Error"),
		QApplication::tr("Cannot get pmtime to hide itself."),
		QApplication::tr("Quit") );
}

void TimeControl::hideArchiveTimeControl()
{
    my.archivePacket->command = PmTime::GUIHide;
    my.archivePacket->length = sizeof(PmTime::Packet);
    if (my.archiveSocket->write((const char *)my.archivePacket,
					sizeof(PmTime::Packet)) < 0)
	QMessageBox::warning(0,
		QApplication::tr("Error"),
		QApplication::tr("Cannot get pmtime to hide itself."),
		QApplication::tr("Quit") );
}

void TimeControl::endTimeControl(void)
{
    QMessageBox::warning(0,
		QApplication::tr("Error"),
		QApplication::tr("Time Control process pmtime has exited."),
		QApplication::tr("Quit") );
    exit(-1);
}

void TimeControl::liveCloseConnection()
{
    console->post("Disconnected from pmtime, live source");
    QMessageBox::warning(0,
		QApplication::tr("Error"),
		QApplication::tr("Connection to pmtime lost (live mode)."),
		QApplication::tr("Quit") );
    quit();
}

void TimeControl::archiveCloseConnection()
{
    console->post("Disconnected from pmtime, archive source");
    QMessageBox::warning(0,
		QApplication::tr("Error"),
		QApplication::tr("Connection to pmtime lost (archive mode)."),
		QApplication::tr("Quit") );
    quit();
}

void TimeControl::liveSocketConnected()
{
    if (my.liveSocket->write((const char *)my.livePacket,
				sizeof(PmTime::Packet)) < 0) {
	QMessageBox::critical(0,
		QApplication::tr("Fatal error"),
		QApplication::tr(
			"Failed socket write in live pmtime negotiation."),
		QApplication::tr("Quit") );
	exit(1);
    }
    if (my.liveSocket->write((const char *)my.tzData, my.tzLength) < 0) {
	QMessageBox::critical(0,
		QApplication::tr("Fatal error"),
		QApplication::tr(
			"Failed to send timezone in live pmtime negotiation."),
		QApplication::tr("Quit"));
	exit(1);
    }
    my.liveState = TimeControl::AwaitingACK;
}

void TimeControl::archiveSocketConnected()
{
    if (my.archiveSocket->write((const char *)my.archivePacket,
				sizeof(PmTime::Packet)) < 0) {
	QMessageBox::critical(0,
		QApplication::tr("Fatal error"),
		QApplication::tr(
			"Failed socket write in archive pmtime negotiation."),
		QApplication::tr("Quit") );
	exit(1);
    }
    if (my.archiveSocket->write((const char *)my.tzData, my.tzLength) < 0) {
	QMessageBox::critical(0,
		QApplication::tr("Fatal error"),
		QApplication::tr(
			"Failed timezone send in archive pmtime negotiation."),
		QApplication::tr("Quit") );
	exit(1);
    }
    my.archiveState = TimeControl::AwaitingACK;
}

//
// Start a shiny new pmtime process.
// The one process serves time for all (live and archive) tabs.
// We do have to specify which form will be used first, however.
//
void TimeControl::startTimeServer()
{
    QStringList arguments;

    if (pmDebug & DBG_TRACE_TIMECONTROL)
	arguments << "-D" << "all";
    connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), this,
		    SLOT(endTimeControl()));
    connect(this, SIGNAL(readyReadStandardOutput()), this,
		    SLOT(readPortFromStdout()));
    start("pmtime", arguments);
}

//
// When pmtime starts in "port probe" mode, port# is written to
// stdout.  We can only complete negotiation once we have that...
//
void TimeControl::readPortFromStdout(void)
{
    bool ok;
    QString data = readAllStandardOutput();

    my.tcpPort = data.remove("port=").toInt(&ok, 10);
    if (!ok) {
	QMessageBox::critical(0,
    	QApplication::tr("Fatal error"),
    	QApplication::tr("Bad port number from pmtime program."),
    	QApplication::tr("Quit") );
	exit(1);
    }

    liveConnect();
    archiveConnect();
}

void TimeControl::protocolMessage(bool live,
	PmTime::Packet *packet, QTcpSocket *socket, ProtocolState *state)
{
    int sts, need = sizeof(PmTime::Packet), offset = 0;
    PmTime::Packet *msg;

    // Read one pmtime packet, handling both small reads and large packets
    for (;;) {
	sts = socket->read(my.buffer + offset, need);
	if (sts < 0) {
	    QMessageBox::critical(0,
		QApplication::tr("Fatal error"),
		QApplication::tr("Failed socket read in pmtime transfer."),
		QApplication::tr("Quit") );
	    exit(1);
	}
	else if (sts != need) {
	    need -= sts;
	    offset += sts;
	    continue;
	}

	msg = (PmTime::Packet *)my.buffer;
	if (msg->magic != PmTime::Magic) {
	    QMessageBox::critical(0,
		QApplication::tr("Fatal error"),
		QApplication::tr("Bad client message magic number."),
		QApplication::tr("Quit") );
	    exit(1);
	}
	if (msg->length > my.bufferLength) {
	    my.bufferLength = msg->length;
	    my.buffer = (char *)realloc(my.buffer, my.bufferLength);
	    if (!my.buffer)
		nomem();
	    msg = (PmTime::Packet *)my.buffer;
	}
	if (msg->length > (uint)offset + sts) {
	    offset += sts;
	    need = msg->length - offset;
	    continue;
	}
	break;
    }

#if DESPERATE
    console->post(PmChart::DebugProtocol,
		  "TimeControl::protocolMessage: recv pos=%s state=%d",
		  timeString(tosec(packet->position)), *state);
#endif

    switch (*state) {
    case TimeControl::AwaitingACK:
	if (msg->command != PmTime::ACK) {
	    QMessageBox::critical(0,
		QApplication::tr("Fatal error"),
		QApplication::tr("Initial ACK not received from pmtime."),
		QApplication::tr("Quit") );
	    exit(1);
	}
	if (msg->source != packet->source) {
	    QMessageBox::critical(0,
		QApplication::tr("Fatal error"),
		QApplication::tr("pmtime not serving same metric source."),
		QApplication::tr("Quit") );
	    exit(1);
	}
	*state = TimeControl::ClientReady;
	if (msg->length > packet->length) {
	    packet = (PmTime::Packet *)realloc(packet, msg->length);
	    if (!packet)
		nomem();
	}
	//
	// Note: we drive the local state from the time control values,
	// and _not_ from the values that we initially sent to it.
	//
	memcpy(packet, msg, msg->length);
	pmchart->VCRMode(live, msg, true);
	break;

    case TimeControl::ClientReady:
	if (msg->command == PmTime::Step) {
	    pmchart->step(live, msg);
	    msg->command = PmTime::ACK;
	    msg->length = sizeof(PmTime::Packet);
	    sts = socket->write((const char *)msg, msg->length);
	    if (sts < 0 || sts != (int)msg->length) {
		QMessageBox::critical(0,
			QApplication::tr("Fatal error"),
			QApplication::tr("Failed pmtime write for STEP ACK."),
			QApplication::tr("Quit") );
		exit(1);
	    }
	} else if (msg->command == PmTime::VCRMode ||
		   msg->command == PmTime::VCRModeDrag ||
		   msg->command == PmTime::Bounds) {
	    pmchart->VCRMode(live, msg, msg->command == PmTime::VCRModeDrag);
	} else if (msg->command == PmTime::TZ) {
	    pmchart->timeZone(live, msg, (char *)msg->data);
	}
	break;

    default:
	QMessageBox::critical(0,
		QApplication::tr("Fatal error"),
		QApplication::tr("Protocol error with pmtime."),
		QApplication::tr("Quit") );
	// fall through
    case TimeControl::Disconnected:
	exit(1);
    }
}

void TimeControl::protocolMessageLoop(bool live,
	PmTime::Packet *packet, QTcpSocket *socket, ProtocolState *state)
{
    do {
	protocolMessage(live, packet, socket, state);
    } while (socket->bytesAvailable() >= (int)sizeof(PmTime::Packet));
}
