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
		static int ID;
		std::map<std::pair<QString, int> , ClientMaster*> m_cmPointersMap;
		QTcpServer* m_server;
	public:
		void notifyOtherClients(std::pair<QString, int>);
		void sendOnlineUsers(std::pair<QString, int>);
		bool checkIfUserOnline(QString);
		QByteArray getOnlineUsersList(int);
	public slots:
		void newConnectedClient();
		void clientDisconnected(ClientMaster*);
		void clientSendMessage(QString&);
		void initNewClient(QString&, int);
	public:	
		Server();
		~Server();
};

#endif
