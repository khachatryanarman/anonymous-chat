#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QString>
#include <QHostAddress>
#include <map>
#include <assert.h>
#include "server.h"
#include "clientmaster.h"
	
int Server::ID=0;
void Server::notifyOtherClients(std::pair<QString, int> name)
{
	QString newUser = "0^";
	QString id = QString::number(name.second);
	newUser.append(name.first+"^"+id);
	QByteArray array = newUser.toUtf8();
	
	for (std::map<std::pair<QString, int>,ClientMaster*>::iterator it=m_cmPointersMap.begin(); it != m_cmPointersMap.end(); ++it) {
		if ( it->first != name && it->first.first !="" ) {
			it->second->m_socket->write(array);
		}
	}
}

bool Server::checkIfUserOnline(QString username)
{
	QString ID = username.section("^", 4, 4);
	QString name = username.section("^", 3, 3);
	int id = ID.toInt();
	std::pair <QString,int> idusername;
    idusername = std::make_pair(name, id);

	if (m_cmPointersMap.find(idusername) == m_cmPointersMap.end()) {
 		return false;
	} else {
		return true;   
	}
}

void Server::clientSendMessage(QString& message)
{
	if (checkIfUserOnline(message)) {
		QString ID = message.section("^", 4, 4);
		QString username = message.section("^", 3, 3);
		int id = ID.toInt();
		std::pair <QString,int> idusername;
		idusername = std::make_pair(username, id);
		ClientMaster* p_cm = m_cmPointersMap.find(idusername)->second;
		QByteArray arraymsg = message.toUtf8();
		assert(0 != p_cm);
		if (p_cm->m_socket->write(arraymsg) == -1) {
			qDebug()<<"can't write to socket\n";	
		}
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
	ClientMaster* clientm = new ClientMaster(this, psocket, Server::ID);
	std::pair<QString, int> idAndUsername = std::make_pair("", Server::ID);
	m_cmPointersMap.insert(std::make_pair(idAndUsername, clientm));
	QObject::connect(clientm, SIGNAL(newInitClientConnected(QString&, int)), this, SLOT(initNewClient(QString&, int)));
	QObject::connect(clientm, SIGNAL(disconnected(ClientMaster*)), this, SLOT(clientDisconnected(ClientMaster*)));
	QObject::connect(clientm, SIGNAL(messageToSend(QString&)), this, SLOT(clientSendMessage(QString&)));
	Server::ID++;
}

void Server::initNewClient(QString& username, int ID)
{
	std::pair<QString, int> ID_and_username = std::make_pair("", ID);
	std::map<std::pair<QString, int>, ClientMaster*>::iterator it = m_cmPointersMap.find(ID_and_username);
	if (it != m_cmPointersMap.end()) {
		ID_and_username.first = username;
		m_cmPointersMap.insert(std::make_pair(ID_and_username, it->second));
		m_cmPointersMap.erase(it);
		notifyOtherClients(ID_and_username);
	}
}

void Server::clientDisconnected(ClientMaster* client)
{
	QString disconnectedUser = "1^";
	int clientID = client->m_ID_of_client;
	QString id = QString::number(clientID);
	disconnectedUser.append(client->m_name_of_client + "^" + id);
	QByteArray array = disconnectedUser.toLocal8Bit();
	qDebug()<<"client "+client->m_name_of_client+":was disconnected";
	
	std::pair<QString, int> idAndUsername = std::make_pair(client->m_name_of_client, clientID);
	
	std::map<std::pair<QString, int>, ClientMaster*>::iterator it = m_cmPointersMap.find(idAndUsername);
	if (it !=m_cmPointersMap.end() && it->first.first != "") {
		delete it->second;	 
		m_cmPointersMap.erase(idAndUsername);
		if (!m_cmPointersMap.empty()) {
			for (std::map<std::pair<QString, int>,ClientMaster*>::iterator it=m_cmPointersMap.begin(); it != m_cmPointersMap.end(); ++it) {
    	    	it->second->m_socket->write(array);
   			}
		}
	}
}

QByteArray Server::getOnlineUsersList(int id)
{
	QString msg = "5^";
	QByteArray data;
	if (!m_cmPointersMap.empty()) {
		for (std::map<std::pair<QString,int>, ClientMaster*>::iterator it=m_cmPointersMap.begin(); it != m_cmPointersMap.end(); ++it) {
			if (it->first.second != id && it->first.first != "") {
				QString ID = QString::number(it->first.second);
    			msg.append(it->first.first + "^" + ID + "^");
			}
    	}
		data = msg.toLocal8Bit();
	}
	return data;
}

Server::Server()
	: QObject(),
	 m_server(0)
{
	m_server = new QTcpServer;
	m_server->listen(QHostAddress::LocalHost, 3333);
	QObject::connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnectedClient()));
	qDebug()<<"Trying to connect..." << m_server->isListening();
}

Server::~Server()
{	

}
