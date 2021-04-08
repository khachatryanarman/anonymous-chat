#ifndef SERVER_H
#define SERVER_H
#include <QObject>

class ClientMaster;
class QTcpServer;
class QTcpSocket;
class QByteArray;

class Server : public QObject
{
	Q_OBJECT 
	private:
		static int clientCounter;
		std::map<int, ClientMaster*> m_clientMasterMap;
		QTcpServer* m_server;
	public:
		bool checkIfUserOnline(int);
		QByteArray getOnlineUsersList(int);
	public slots:
		void notifyOtherClients(QString&, int);
		void newConnectedClient();
		void clientDisconnected();
		void clientSendMessage(int, QString&);
	public:	
		Server();
		~Server();
};
#endif
