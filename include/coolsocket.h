/*
* Copyright (C) 2019 Veli Tasalı
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#pragma once

#include <QDataStream>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <iostream>
#include <utility>

using namespace std;

namespace CoolSocket {
	extern const char *HEADER_KEYWORD_LENGTH;
	extern const char *HEADER_DIVIDER;
	extern const int HEADER_HEAP_SIZE;
	extern const quint16 TIMEOUT_NONE;
	extern const quint16 TIMEOUT_DEFAULT;

	class Response;

	class Connection;

	class Client;

	class Server;
}

class CoolSocket::Server : public QTcpServer {
Q_OBJECT

	int m_timeout = TIMEOUT_NONE;
	quint16 m_port = 0;
	QHostAddress m_hostAddress;

public:
	friend class Connection;

	friend class Response;

	explicit Server(const QHostAddress &hostAddress, quint16 port = 0, int timeout = TIMEOUT_NONE,
	                QObject *parent = nullptr);

	~Server() override;

	QHostAddress hostAddress() const;

	quint16 port() const;

	void setHostAddress(const QHostAddress &hostAddress);

	void setPort(quint16 port);

	void setTimeout(int timeout);

	bool start();

	int timeout();
};

class CoolSocket::Connection : public QObject {
Q_OBJECT

	int m_timeout = 2000;
	QTcpSocket *m_socket;

public:
	explicit Connection(QTcpSocket *socket, int msecTimeout = 2000, QObject *parent = nullptr);

	~Connection() override;

	void setTimeout(int msecs);

	QTcpSocket *socket();

	int timeout();

public slots:

	void reply(const QJsonObject &reply);

	void reply(const QString & reply);

	void reply(const QByteArray &reply);

	Response receive();
};

class CoolSocket::Response {
public:
	QString msg;
	QJsonObject headerIndex;
	size_t length = 0;

	Response() = default;

	QJsonObject asJson() const;
};

class CoolSocket::Client : public QObject {
Q_OBJECT

public:
	static Connection *openConnection(const QHostAddress &hostName, quint16 port, int timeoutMSeconds = TIMEOUT_DEFAULT,
	                                  QObject *sender = nullptr);
};