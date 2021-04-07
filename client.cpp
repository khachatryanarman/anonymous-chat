#include <iostream>
#include "client.h"
#include <QTcpSocket>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton> 
#include <QFormLayout>
#include <QString>
#include <QByteArray>
#include <QShortcut>
#include <QMessageBox>
#include <QComboBox> 	
#include <QRect>
#include <QApplication>
#include <QDesktopWidget>
#include <QLabel>
#include <QIcon>
#include <assert.h>

void Client::changeDialogBox(int index)
{
	if (-1 == index) {
		return;
	}
	assert(0 != m_userToWriteBox);
	assert(0 != m_txt);
	m_userToWriteBox->setItemIcon(index, QIcon());
	m_txt->clear();
	int id = m_userToWriteBox->itemData(index, Qt::UserRole).toInt();
    std::map<int, QString>::iterator it = m_dialogsMap.find(id);
	if (it != m_dialogsMap.end()) {	
		m_txt->setText(it->second);	
	}
}

void Client::logInSlot()
{	
	assert(0 != m_logInField);
	assert(0 != m_socket);
	assert(0 != m_logInWindow);	
	assert(0 != m_window);
	assert(0 != m_logInLayout);
    static bool alreadyAdded = false;
	if (!m_logInField->text().isEmpty() && !(m_logInField->text().contains('^'))) {
		m_name = m_logInField->text();
		if (m_socket->state() == QTcpSocket::ConnectedState) {
			m_logInWindow->hide();
			m_window->setWindowTitle(m_name);
			successfullyConnected();
		} else if (!alreadyAdded) {
			QLabel* label = new QLabel();
			label->setText("you are'nt connected yet");
			m_logInLayout->addWidget(label);
			alreadyAdded = true; 
		}
	} else {
		qDebug()<<"not allowed special symbols in username";
	}
}

void Client::connectToSocketError(QAbstractSocket::SocketError)
{
	assert(0 != m_socket);
	m_socket->close();
	m_socket->connectToHost(QHostAddress::LocalHost, 3333);
}

void Client::connectionLost()
{
	assert(0 != m_window);
	assert(0 != m_msgBox);
	m_window->hide();
	m_msgBox->show();	
}

void Client::acknowledgementFromServer(QString& data)
{
}

void Client::getOnlineUsers()
{
	m_window->hide();
	assert(0 != m_socket);
	qDebug()<<"request for online users";
	QString online = "5^";
	QByteArray data = online.toLocal8Bit();
	m_socket->write(data);
}

void Client::readFromTextBox()
{
	assert(0 != m_lineEdit);
	assert(0 != m_txt);
	assert(0 != m_userToWriteBox);
	
	if (!m_lineEdit->text().isEmpty()) {
		m_txt->setTextColor(QColor(185,0,0));
		QString msgToSend = m_lineEdit->text();
		if (m_userToWriteBox->currentIndex() != -1) {
			m_txt->append("You: "+msgToSend);
		} else {
			m_txt->setTextColor(QColor(192, 192, 192));
			m_txtForLogs->append("all users are offline");
		}
		if (m_userToWriteBox->currentText() != "") {
			int index = m_userToWriteBox->currentIndex();	
			int ID = m_userToWriteBox->itemData(index, Qt::UserRole).toInt();	
			QString receiver = m_userToWriteBox->itemText(index);
			sendMessage(msgToSend,ID,receiver);
			m_lineEdit->clear();
		} else {
			qDebug()<<"cant send the msg";
		}
	}
}

void Client::showOnlineUsers()
{	
	assert(0 != m_userToWriteBox);
	if (!m_onlineUsers.empty()) {
         for (std::map<int, QString>::iterator it = m_onlineUsers.begin(); it != m_onlineUsers.end(); ++it) {
			m_userToWriteBox->addItem(QIcon(),it->second, it->first);
		}
	}
}

void Client::successfullyConnected()
{
	assert(0 != m_window);
	assert(0 != m_socket);
	QString sendName = "3^";
	sendName.append(m_name);
	QByteArray data = sendName.toLocal8Bit();
    m_socket->write(data);
	m_window->show();
}

void Client::mySocketRead()
{	
	assert(0 != m_socket);
    QByteArray array = m_socket->readAll();
	QString data = QString(array);
	parseString(data);	
	qDebug()<<"raw data from socket: "+data;
}

void Client::parseAddOnlineUser(QString& str)
{
	QString name = str.section('^', 1, 1);
	QString id = str.section('^', 2, 2);
	QString dialogstr;
	m_dialogsMap.insert(std::make_pair(id.toInt(), dialogstr));
	addOnlineUser(name, id);	
}

void Client::parseRemoveOnlineUser(QString& str)
{	
	assert(0 != m_txt);
	QString name = str.section('^', 1, 1);
	QString id = str.section('^', 2, 2);
	std::map<int, QString>::iterator it = m_dialogsMap.find(id.toInt());
	if (it != m_dialogsMap.end()) {
		m_dialogsMap.erase(it);
	}

	removeOnlineUser(id);
	m_txtForLogs->append(name + " left the chat");
}

void Client::parseReceiveMessage(QString& str)
{
	int count = str.count('^');
	QString ID = str.section('^', 2, 2);
	QString name = str.section('^', 1, 1);
	QString message = str.section('^', 5, count);
	receiveMessage(name, ID.toInt(), message);	
}

void Client::parseGetOnlineUsers(QString& str)
{
	int i = 1;
	int count = str.count('^') - 1;
	while (i < count)	{
		QString username = str.section('^', i, i);
		QString ID = str.section('^', i+1, i+1);
	    m_onlineUsers.insert(std::make_pair(ID.toInt(), username));
		QString dialogstr;
	    m_dialogsMap.insert(std::make_pair(ID.toInt(), dialogstr));	
	
		i += 2;
	}
	showOnlineUsers();
}

void Client::parseString(QString str_data)
{
	QString command = str_data.section('^', 0, 0);
	int commandID = command.toInt();
	switch (commandID) {
   		case 0: parseAddOnlineUser(str_data);
    		break;
  		case 1:	parseRemoveOnlineUser(str_data);
    		break;
		case 3: acknowledgementFromServer(str_data);
			break;
		case 4:	parseReceiveMessage(str_data);	
			break;		
		case 5: parseGetOnlineUsers(str_data);
			break;
	}
}

void Client::addOnlineUser(QString name, QString ID)
{
	assert(0 != m_userToWriteBox);
	assert(0 != m_txt);
	int id = ID.toInt();
	m_onlineUsers.insert(std::make_pair(id, name));
    m_userToWriteBox->addItem(QIcon(), name, id);
	m_txt->setTextColor(QColor(192, 192, 192));
	m_txtForLogs->append(name + " has entered the chat");
}

void Client::setOrChangeUsername()
{
	assert(0 != m_socket);
	QString str = "3^";
	str.append(m_name);
	QByteArray data = str.toLocal8Bit();
	m_socket->write(data);
}

void Client::removeOnlineUser(QString id)
{
	assert(0 != m_userToWriteBox);
	int ID = id.toInt();
	std::map<int, QString>::iterator it = m_onlineUsers.find(ID);
	if (it != m_onlineUsers.end()) {
		m_onlineUsers.erase(it);
	}
	int indexOf = m_userToWriteBox->findData(ID, Qt::UserRole);
	if (-1 != indexOf) {
		m_userToWriteBox->removeItem(indexOf);
	} else {
		qDebug()<<"can't delete field from comboBox";
	}
}

void Client::receiveMessage(QString name, int id, QString receiveMessage)
{
	assert(0 != m_txt);
	assert(0 != m_userToWriteBox);
	m_txt->setTextColor(QColor(0,0,185));
	if( id == m_userToWriteBox->itemData(m_userToWriteBox->currentIndex(), Qt::UserRole).toInt()  && name == m_userToWriteBox->currentText()) {
		m_txt->append(name + ": " + receiveMessage);
	} else {
		m_txtForLogs->setTextColor(QColor(0,120,0));
		m_txtForLogs->append("new message from " + name);
		m_txtForLogs->setTextColor(QColor(192,192,192));
	
		int index = m_userToWriteBox->findData(id);	
		qDebug()<<"set item icon notif";	
		m_userToWriteBox->setItemIcon(index, QIcon("notification")); 
	}
    std::map<int, QString>::iterator it = m_dialogsMap.find(id);
	it->second.append(name + ": " + receiveMessage + "\n");
}

void Client::sendMessage(QString msg, int ID, QString username)
{
	assert(0 != m_socket);
	QString strId = QString::number(ID);
	QString str = "4^" + m_name + '^' + username + '^' + strId + '^' + msg;
    std::map<int, QString>::iterator it = m_dialogsMap.find(strId.toInt());
	if (it != m_dialogsMap.end()) {
		QString str = "You: "+msg+"\n";
		it->second.append(str);
	}
	QByteArray data = str.toLocal8Bit();
	m_socket->write(data);
}

void Client::initOfObjects()
{
	m_logInWindow = new QWidget;
    m_logInField = new QLineEdit;
	m_logInButton = new QPushButton("Log in"); 
	m_logInLayout = new QFormLayout(m_logInWindow);
	m_window = new QWidget;
	m_lineEdit = new QLineEdit;
	m_sendButton = new QPushButton("send");
	m_layout = new QFormLayout(m_window);
	m_msgBox = new QMessageBox;
	m_userToWriteBox = new QComboBox;
	m_txt = new QTextEdit; 
	m_socket = new QTcpSocket;
	m_txtForLogs = new QTextEdit;
}

void Client::connectSocketToHost()
{
	assert(0 != m_socket);
	m_socket->connectToHost(QHostAddress::LocalHost, 3333);
}

void Client::layoutCreator()
{
	assert(0 != m_logInLayout);
	assert(0 != m_logInField);
	assert(0 != m_logInWindow);
	assert(0 != m_txt);
	assert(0 != m_layout);
	assert(0 != m_msgBox);	
	assert(0 != m_txtForLogs);
	assert(0 != m_layout);
	assert(0 != m_lineEdit);
	assert(0 != m_sendButton);
	assert(0 != m_userToWriteBox);

	m_logInLayout->addWidget(m_logInField);
	m_logInLayout->addWidget(m_logInButton);
	m_logInWindow->setWindowTitle("Log in");
	m_logInWindow->show();	
	m_txt->setReadOnly(true);
	m_txtForLogs->setReadOnly(true);
	m_txtForLogs->setMaximumHeight(75);
	m_txtForLogs->setTextColor(QColor(192,192,192));
	m_layout->addWidget(m_txt);
	m_layout->addWidget(m_txtForLogs);
	m_layout->addWidget(m_userToWriteBox);
	m_layout->addWidget(m_lineEdit);
	m_layout->addWidget(m_sendButton);
    m_msgBox->setText("Connection Lost");
	QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int x = (screenGeometry.width() - m_window->width()) / 2;
    int y = (screenGeometry.height() - m_window->height()) / 2;
   	m_window->move(x, y);
	m_logInWindow->move(x,y);
	m_lineEdit->setFocusPolicy(Qt::StrongFocus);
	m_lineEdit->setFocus();	
}

void Client::signalSlotConnections()
{	
	assert(0 != m_userToWriteBox);	
	assert(0 != m_logInField);
	assert(0 != m_logInButton);
	assert(0 != m_socket);
	assert(0 != m_lineEdit);
	assert(0 != m_sendButton);
	
	QObject::connect(m_userToWriteBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeDialogBox(int)));
	QObject::connect(m_logInField, SIGNAL(returnPressed()), this, SLOT(logInSlot()));
	QObject::connect(m_logInButton, SIGNAL(clicked()), this, SLOT(logInSlot()));
	QObject::connect(m_socket, SIGNAL(disconnected()), this, SLOT(connectionLost()));
	QObject::connect(m_socket, SIGNAL(readyRead()), this, SLOT(mySocketRead()));
	QObject::connect(m_lineEdit, SIGNAL(returnPressed()), this, SLOT(readFromTextBox()));
	QObject::connect(m_sendButton, SIGNAL(clicked()), this, SLOT(readFromTextBox()));
	QObject::connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectToSocketError(QAbstractSocket::SocketError)));
}

Client::Client()
	: QObject(),
	m_logInWindow(0), 
	m_logInField(0),
	m_logInButton(0),
	m_logInLayout(0),
	m_window(0),
	m_lineEdit(0),
	m_sendButton(0),
	m_layout(0),
	m_msgBox(0),
	m_userToWriteBox(0),
	m_txt(),
	m_socket(0),
	m_txtForLogs(0)
{
	initOfObjects();
	layoutCreator();
	signalSlotConnections();
	connectSocketToHost();
}

Client::~Client()
{
	
}
