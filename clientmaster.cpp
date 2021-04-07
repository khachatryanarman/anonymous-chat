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
	emit disconnected(this);
}

void ClientMaster::parseUsernames(QString& strToParse)
{
	assert(0 != m_server);
	QString username = strToParse.section('^', 1, 1);
	m_name_of_client = username; 
	QByteArray array = username.toLocal8Bit();

	emit newInitClientConnected(username, m_ID_of_client);
	
	QByteArray onlineUsersList = m_server->getOnlineUsersList(m_ID_of_client);
	m_socket->write(onlineUsersList);
}

void ClientMaster::parseSendMessage(QString& strToParse)
{
	assert(0 != m_server);
	int count = strToParse.count('^');
	QString receiver = strToParse.section('^', 2, 2);
	QString receiverID = strToParse.section('^', 3, 3);
	QString msg = strToParse.section('^', 4, count);
	QString ID = QString::number(m_ID_of_client);
	QString str = "4^"+m_name_of_client+"^"+ID+"^"+receiver+
	"^"+receiverID+"^"+ msg;
	emit messageToSend(str);
}

void ClientMaster::parseGetOnlineUsers()
{
	assert(0 != m_server);
	QByteArray onlineUsersList = m_server->getOnlineUsersList(m_ID_of_client);
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

ClientMaster::ClientMaster(Server* p_server, QTcpSocket* p_socket, int ID)
	:m_socket(p_socket),
	m_server(p_server),
	m_ID_of_client(ID)
{
	QObject::connect(m_socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));	
	QObject::connect(m_socket, SIGNAL(readyRead()),this, SLOT(readFromSocket()));
	m_ID_of_client = ID;
}

ClientMaster::~ClientMaster()
{

}
