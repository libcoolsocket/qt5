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

#include "coolsocket.h"

namespace CoolSocket {
	const char *HEADER_KEYWORD_LENGTH = "length";
	const char *HEADER_DIVIDER = "\nHEADER_END\n";
	const int HEADER_HEAP_SIZE = 8196;
	const quint16 TIMEOUT_NONE = -1;
	const quint16 TIMEOUT_DEFAULT = 5000;
}

CoolSocket::Server::Server(const QHostAddress &hostAddress, quint16 port, int timeout, QObject *parent)
		: QTcpServer(parent)
{
	setTimeout(timeout);
	setHostAddress(hostAddress);
	setPort(port);
}

CoolSocket::Server::~Server()
{
	close();
}

bool CoolSocket::Server::start()
{
	return listen(hostAddress(), port());
}

QHostAddress CoolSocket::Server::hostAddress() const
{
	return m_hostAddress;
}

quint16 CoolSocket::Server::port() const
{
	return m_port;
}

void CoolSocket::Server::setHostAddress(const QHostAddress &hostAddress)
{
	m_hostAddress = hostAddress;
}

void CoolSocket::Server::setPort(quint16 port)
{
	m_port = port;
}

void CoolSocket::Server::setTimeout(int timeout)
{
	m_timeout = timeout;
}

int CoolSocket::Server::timeout()
{
	return m_timeout;
}

void CoolSocket::Connection::reply(const QJsonObject &reply)
{
	this->reply(QJsonDocument(reply).toJson());
}

void CoolSocket::Connection::reply(const QString &reply)
{
	this->reply(reply.toUtf8());
}

void CoolSocket::Connection::reply(const QByteArray &reply)
{
	QJsonObject headerIndex{
			{HEADER_KEYWORD_LENGTH, reply.length()}
	};

	socket()->write(QJsonDocument(headerIndex).toJson());
	socket()->write(HEADER_DIVIDER);
	socket()->flush();

	socket()->write(reply);
	socket()->flush();

	while (socket()->bytesToWrite() != 0) {
		if (!socket()->waitForBytesWritten(timeout() < 1000 ? 1000 : timeout())) {
			qInfo() << this << "Timed out !!!";
			throw exception();
		}
	}
}

CoolSocket::Response CoolSocket::Connection::receive()
{
	clock_t lastDataAvailable = clock();
	size_t headerPosition = string::npos;
	CoolSocket::Response response;
	QByteArray headerData;

	while (socket()->isReadable()) {
		if (headerPosition == string::npos) {
			if (socket()->waitForReadyRead(2000)) {
				headerData.append(socket()->readAll());
				lastDataAvailable = clock();
			}

			headerPosition = headerData.indexOf(HEADER_DIVIDER);

			if (headerPosition != string::npos) {
				size_t dividerOccupiedSize = strlen(HEADER_DIVIDER) + headerPosition;

				if (headerData.length() > dividerOccupiedSize)
					response.msg.append(headerData.right(dividerOccupiedSize));

				headerData.resize(headerPosition);

				response.headerIndex = QJsonDocument::fromJson(headerData)
						.object();

				if (response.headerIndex.contains(HEADER_KEYWORD_LENGTH)) {
					response.length = response.headerIndex.value(HEADER_KEYWORD_LENGTH)
							.toVariant()
							.toUInt();
				} else
					break;

			}

			if (headerData.length() > HEADER_HEAP_SIZE) {
				qCritical() << this << "Header exceeds heap size:" << headerData.length();
				throw exception();
			}
		} else {
			if (socket()->waitForReadyRead(2000)) {
				response.msg.append(socket()->readAll());
				lastDataAvailable = clock();
			}

			if (response.msg.length() >= response.length)
				break;
		}

		if (timeout() >= 0 && (clock() - lastDataAvailable) > timeout())
			throw exception();
	}

	return response;
}

CoolSocket::Connection::Connection(QTcpSocket *socket, int msecTimeout, QObject *parent)
		: QObject(parent)
{
	m_socket = socket;
	m_timeout = msecTimeout;
}

CoolSocket::Connection::~Connection()
{
	if (this->m_socket == nullptr)
		return;

	if (this->m_socket->isOpen())
		this->m_socket->close();

	delete m_socket;
}

void CoolSocket::Connection::setTimeout(int msecs)
{
	this->m_timeout = msecs;
}

QTcpSocket *CoolSocket::Connection::socket()
{
	return m_socket;
}

int CoolSocket::Connection::timeout()
{
	return m_timeout;
}

CoolSocket::Connection *CoolSocket::Client::openConnection(const QHostAddress &hostName,
                                                           quint16 port,
                                                           int timeoutMSeconds,
                                                           QObject *sender)
{
	auto *socket = new QTcpSocket;
	auto *connection = new CoolSocket::Connection(socket, timeoutMSeconds, sender);

	QTcpSocket::connect(sender, SIGNAL(destroyed()), connection, SLOT(deleteLater()));

	socket->connectToHost(hostName, port);

	while (QAbstractSocket::SocketState::ConnectingState == socket->state())
		socket->waitForConnected(timeoutMSeconds);

	if (QAbstractSocket::SocketState::ConnectedState != socket->state())
		throw exception();

	return connection;
}

QJsonObject CoolSocket::Response::asJson() const
{
	return QJsonDocument::fromJson(msg.toUtf8()).object();
}
