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
	private: 
		QTcpSocket* m_socket;
		Server* m_server;
		int m_ID;
		QString m_name;
	public:
		void parseUsernames(QString&);
		void parseSendMessage(QString&);
		void parseGetOnlineUsers();
		void sendMessage(QByteArray&);	
		int getId();
		QString getName();
		QTcpSocket* getSocket();
 
		ClientMaster(Server*, QTcpSocket*, int ID);
		~ClientMaster();
	signals:
		void newInitClientConnected(QString&, int);
		void disconnected();
		void messageToSend(int, QString&); 
	public slots:
	    void readFromSocket();
		void socketDisconnected();
};
#endif
