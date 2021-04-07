#ifndef CLIENTMASTER_H
#define CLIENTMASTER_H
#include <QByteArray>
#include <QString>
#include <QDebug>
#include <QTcpSocket>
#include <QObject>

class Server;
class ClientMaster : public QObject
{
	Q_OBJECT
	public: 
		QTcpSocket* m_socket;
		Server* m_server;
		int m_ID_of_client;
		QString m_name_of_client;
		void parseUsernames(QString&);
		void parseSendMessage(QString&);
		void parseGetOnlineUsers();	
	public: 
		ClientMaster(Server*, QTcpSocket*, int ID);
		~ClientMaster();
	signals:
		void newInitClientConnected(QString&, int);
		void disconnected(ClientMaster*);
		void messageToSend(QString&); 
	public slots:
	    void readFromSocket();
		void socketDisconnected();
};
#endif
