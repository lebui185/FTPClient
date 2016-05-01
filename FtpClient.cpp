/*
 * FtpClient.cpp
 *
 *  Created on: Apr 25, 2016
 *      Author: le
 */

#include <sstream>
#include "FtpClient.h"
using namespace std;

FtpClient::FtpClient()
{
	// TODO Auto-generated constructor stub

}

FtpClient::~FtpClient()
{
	_dataSocket.Close();
	_dataListener.Close();
	_commandSocket.Close();
}

void FtpClient::Connect(const std::string& host, int port)
{
	int bytesRead;
	string msg;
	char buf[BUFFER_SIZE];

	IPEndPoint serverEP(host, port);
	_commandSocket.SetProperties(AF_INET, SOCK_STREAM, 0);
	_commandSocket.Connect(serverEP);

	bytesRead = _commandSocket.Receive(buf, 0, sizeof(buf), 0);
	msg.assign(buf, bytesRead);
}

void FtpClient::Connect(const std::string& host, int port,
		const std::string& username, const std::string& password)
{
	this->Connect(host, port);
	this->Login(username, password);
}

void FtpClient::Login(const std::string& username, const std::string& password)
{
	string receiveMsg;
	string command;

	// username
	command = "USER " + username + "\r\n";
	receiveMsg = SendCommand(command);

	// password
	command = "PASS " + password + "\r\n";
	receiveMsg = SendCommand(command);
}

void FtpClient::ListDirectory(ostream& os)
{
	string receiveMsg;
	string command = "LIST\r\n";
	int bytesRead;
	char buf[BUFFER_SIZE];

	this->PrepareDataChannel();
	_commandSocket.Send(command.c_str(), 0, command.length(), 0);

	Socket temp = _dataListener.Accept(); // xài tạm chỗ này, không gán trực tiếp được
	_dataSocket = temp; // xài tạm chỗ này, không gán trực tiếp được

	this->ReceiveData(os);
	bytesRead = _commandSocket.Receive(buf, 0, sizeof(buf), 0);
	receiveMsg.assign(buf, bytesRead);
}

std::string FtpClient::SendCommand(const std::string& command)
{
	int bytesRead;
	char buf[BUFFER_SIZE];

	_commandSocket.Send(command.c_str(), 0, command.length(), 0);
	bytesRead = _commandSocket.Receive(buf, 0, sizeof(buf), 0);

	return string(buf, bytesRead);
}

void FtpClient::PrepareDataChannel()
{
	IPEndPoint localDataEP(_commandSocket.GetLocalEndPoint().GetTextAddress(), 0);
	_dataListener.SetProperties(AF_INET, SOCK_STREAM, 0);
	_dataListener.Bind(localDataEP);
	_dataListener.Listen(1);

	string portStr = this->FormalizePort(_dataListener.GetLocalEndPoint().GetPort());
	string ipStr = localDataEP.GetTextAddress();
	this->FormalizeIP(ipStr);

	string portCommand = "PORT " + ipStr + "," + portStr + "\r\n";
	string receiveMsg = SendCommand(portCommand);
}

std::string FtpClient::FormalizePort(uint16_t port)
{
	int secondNum = port & 255;
	int firstNum = (port & 65280) >> 8;
	ostringstream ss;
	ss << firstNum << "," << secondNum;
	return ss.str();
}

void FtpClient::FormalizeIP(std::string& ip)
{
	size_t pos = 0;
	string search = ".";
	string replacement = ",";

	while ((pos = ip.find(search, pos)) != std::string::npos)
	{
		ip.replace(pos, search.length(), replacement);
		pos += replacement.length();
	}
}

void FtpClient::ReceiveData(std::ostream& os)
{
	int bytesRead;
	char buf[BUFFER_SIZE];

	bytesRead = _dataSocket.Receive(buf, 0, sizeof(buf), 0);
	os.write(buf, bytesRead);
	_dataSocket.Shutdown(SHUT_RDWR);
	_dataSocket.Close();
}
