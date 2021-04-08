#include <QDebug>
#include <QByteArray>
#include <QObject>
#include <QString>
#include "server.h"
#include "clientmaster.h"
#include "utility"
#include <unistd.h>
#include <assert.h>
	
void ClientMaster::socketDisconnected()
{
	emit disconnected();
}

void ClientMaster::parseUsernames(QString& strToParse)
{
	assert(0 != m_server);
	QString username = strToParse.section('^', 1, 1);
	m_name = username; 
	QByteArray array = username.toLocal8Bit();
	emit newInitClientConnected(username, m_ID);
	QByteArray onlineUsersList = m_server->getOnlineUsersList(m_ID);
	m_socket->write(onlineUsersList);
}

void ClientMaster::parseSendMessage(QString& strToParse)
{
	int count = strToParse.count('^');
	QString receiver = strToParse.section('^', 2, 2);
	QString receiverID = strToParse.section('^', 3, 3);
	QString msg = strToParse.section('^', 4, count);
	QString ID = QString::number(m_ID);
	QString str = "4^" + m_name + "^" + ID + "^" + receiver +
	"^" + receiverID + "^" + msg;
	emit messageToSend(receiverID.toInt(), str);
}

void ClientMaster::sendMessage(QByteArray& bytearray)
{
	assert(0 != m_socket);
	m_socket->write(bytearray);
}

void ClientMaster::parseGetOnlineUsers()
{
	assert(0 != m_server);
	QByteArray onlineUsersList = m_server->getOnlineUsersList(m_ID);
	m_socket->write(onlineUsersList);
}

void ClientMaster::readFromSocket()
{
	assert(0 != m_server);
	QByteArray array = m_socket->readAll();
	qDebug()<<"raw str from socket: " + array;
	QString somemsg = QString(array);
	QString command = somemsg.section('^', 0, 0);
	int commandID = command.toInt();	
	switch (commandID) {
		case 3: parseUsernames(somemsg);
			break;
		case 4: parseSendMessage(somemsg);
			break;
		case 5: parseGetOnlineUsers();	
			break;
	}
}

int ClientMaster::getId()
{
	return m_ID;
}

QString ClientMaster::getName()
{
	return m_name;
}

QTcpSocket* ClientMaster::getSocket()
{
	return m_socket;
}

ClientMaster::ClientMaster(Server* p_server, QTcpSocket* p_socket, int ID)
	:m_socket(p_socket)
	 , m_server(p_server)
	 , m_ID(ID)
	 , m_name("")
{
	assert(0 != m_socket);
	QObject::connect(m_socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));	
	QObject::connect(m_socket, SIGNAL(readyRead()), this, SLOT(readFromSocket()));
}

ClientMaster::~ClientMaster()
{	
	m_socket->deleteLater();
}
