#include <QObject>
#include <QDataStream>
#include <QHostAddress>
#include <QLabel>
#include <QTextEdit>
#include <map>
#include <utility>    
	
class QLineEdit;
class QPushButton;
class QFormLayout;
class QWidget;
class QTcpSocket;
class QMessageBox;
class QShortcut;
class QComboBox;

class Client : public QObject
{
	Q_OBJECT
	private:
	    std::map<int, QString> m_onlineUsers;
		std::map<int, QString> m_dialogsMap;

		QString m_name;		
		QWidget* m_logInWindow;
		QLineEdit* m_logInField;
		QPushButton* m_logInButton;
		QFormLayout* m_logInLayout;
		QWidget* m_window;
		QLineEdit* m_lineEdit;
		QPushButton* m_sendButton;
		QFormLayout* m_layout;
		QComboBox* m_userToWriteBox;
		QTextEdit* m_txt;
		QTcpSocket*	m_socket;
		QTextEdit* m_txtForLogs;
		QLabel* m_label;	
	public:
		void parseReceiveMessage(QString&);	
		void parseRemoveOnlineUser(QString&);	
		void parseAddOnlineUser(QString&);
		void parseGetOnlineUsers(QString&);
		void accessFromServer(QString&);
		void successfullyConnected();
		void parseString(QString);
		void addOnlineUser(QString, QString);
		void removeOnlineUser(QString);
		void receiveMessage(QString, int, QString);
		void setOrChangeUsername();
		void sendMessage(QString, int, QString);
		void showOnlineUsers();
		void initOfObjects();
		void signalSlotConnections();
		void layoutCreator();
		void connectSocketToHost();
	public slots:
		void connectToSocketError(QAbstractSocket::SocketError);
		void changeDialogBox(int);
		void logInSlot();
		void getOnlineUsers();	
		void mySocketRead();
		void readFromTextBox();
		void connectionLost();
		void socketWasConnected();
	public:
		Client();
		~Client();			
};
