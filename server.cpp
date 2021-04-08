#include <QTcpSocket>
#include <QTcpServer>
#include <QDebug>
#include <QString>
#include <QHostAddress>
#include <map>
#include <assert.h>
#include "server.h"
#include "clientmaster.h"
	
int Server::clientCounter = 0;
void Server::notifyOtherClients(QString& username, int id)
{
	std::map<int, ClientMaster*>::iterator it = m_clientMasterMap.find(id);
	if ( it != m_clientMasterMap.end() ) {
		QString str = "3^";
		QByteArray data = str.toLocal8Bit();
		it->second->sendMessage(data);
		it->second->getSocket()->waitForBytesWritten(1000);
	}
	QString newUser = "0^";
	QString ID = QString::number(id);
	newUser.append(username + '^' + ID);
	QByteArray array = newUser.toUtf8();
	for (std::map<int, ClientMaster*>::iterator it = m_clientMasterMap.begin(); it != m_clientMasterMap.end(); ++it) {
		if ( it->first != id && it->second->getName() != "" ) {
			it->second->sendMessage(array);
		}
	}
}

bool Server::checkIfUserOnline(int ID)
{
	if ( m_clientMasterMap.find(ID) == m_clientMasterMap.end() ) {
		return false;
	} else {
		return true;   
	}
}

void Server::clientSendMessage(int ID, QString& message)
{
	if ( checkIfUserOnline(ID) ) {
		ClientMaster* p_cm = m_clientMasterMap.find(ID)->second;
		QByteArray arraymsg = message.toUtf8();
		assert(0 != p_cm);
		p_cm->sendMessage(arraymsg);
	} else {
		qDebug()<<"can't send the message\n";
	}
}

void Server::newConnectedClient()
{
	assert(0 != m_server);
	qDebug()<<"new client has been connected:\n";
	QTcpSocket* psocket = m_server->nextPendingConnection();
	
	assert(0 != psocket);
	qDebug()<<"Client with id: "<<Server::clientCounter;
	ClientMaster* clientm = new ClientMaster(this, psocket, Server::clientCounter);
	m_clientMasterMap.insert(std::make_pair(Server::clientCounter, clientm));
	QObject::connect(clientm, SIGNAL(newInitClientConnected(QString&, int)), this, SLOT(notifyOtherClients(QString&, int)));
	QObject::connect(clientm, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
	QObject::connect(clientm, SIGNAL(messageToSend(int, QString&)), this, SLOT(clientSendMessage(int, QString&)));
	++Server::clientCounter;
}

void Server::clientDisconnected()
{
	ClientMaster* pClient = dynamic_cast<ClientMaster*>(QObject::sender());
	assert(0 != pClient);
	int clientID = pClient->getId();
	QString id = QString::number(clientID);
	QString disconnectedUser = "1^";
	disconnectedUser.append(pClient->getName() + '^' + id);
	QByteArray array = disconnectedUser.toLocal8Bit();
	std::map<int, ClientMaster*>::iterator it = m_clientMasterMap.find(clientID);
	if ( it != m_clientMasterMap.end() && it->second->getName() != "" ) {
		delete it->second;	 
		m_clientMasterMap.erase(clientID);	 
		for (std::map<int, ClientMaster*>::iterator it = m_clientMasterMap.begin(); it != m_clientMasterMap.end(); ++it) {
    	    it->second->sendMessage(array);
   		}
	}
}

QByteArray Server::getOnlineUsersList(int id)
{
	QString msg = "5^";
	for (std::map<int, ClientMaster*>::iterator it = m_clientMasterMap.begin(); it != m_clientMasterMap.end(); ++it) {
		if ( it->first != id && it->second->getName() != "" ) {
			QString ID = QString::number(it->first);
    		msg.append(it->second->getName() + "^" + ID + "^");
		}
    }
	return msg.toLocal8Bit();
}

Server::Server()
	: QObject()
	, m_server(0)
{
	m_server = new QTcpServer;
	m_server->listen(QHostAddress::LocalHost, 3333);
	QObject::connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnectedClient()));
	qDebug()<<"Trying to connect..." << m_server->isListening();
}

Server::~Server()
{	
	assert(0 != m_server);
	m_server->deleteLater();
}
