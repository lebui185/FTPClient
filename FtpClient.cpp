/*
 * FtpClient.cpp
 *
 *  Created on: Apr 25, 2016
 *      Author: le
 */

#include <sstream>
#include <fstream>
#include "StringHelper.h"
#include "FtpClient.h"
using namespace std;

FtpClient::FtpClient()
{
	_mode = 1; // passive
}

FtpClient::~FtpClient()
{
	_dataSocket.Close();
	_dataListener.Close();
	_commandSocket.Close();
}

int FtpClient::GetMode() const
{
	return _mode;
}

void FtpClient::SetMode(int mode)
{
	_mode = mode;
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

	command = "USER " + username + "\r\n";
	receiveMsg = SendCommand(command);

	command = "PASS " + password + "\r\n";
	receiveMsg = SendCommand(command);
}

bool FtpClient::ListDirectory(ostream& os)
{
	string receiveMsg;

	this->CreateDataChannel();
	receiveMsg = SendCommand("LIST\r\n");

	if (_mode == 0) // active
	{
		_dataSocket = _dataListener.Accept();
	}

	cout << receiveMsg;
	this->ReceiveData(os);

	receiveMsg = this->ReceiveFromCommandChannel();
	cout << receiveMsg;

	return true;
}

bool FtpClient::ChangeDirectory(const std::string &remotePath)
{
	string receiveMsg;
	string command = "CWD " + remotePath + "\r\n";

	receiveMsg = SendCommand(command);
	cout << receiveMsg;

	return true;
}

bool FtpClient::GetDirectory(const std::string& remotePath,
		const std::string& localPath)
{
	return true;
}

bool FtpClient::PutDirectory(const std::string& remotePath,
		const std::string& localPath)
{
	return true;
}

bool FtpClient::DeleteEmptyDirectory(const std::string& remotePath)
{
	string receiveMsg;
	string command = "RMD " + remotePath + "\r\n";

	receiveMsg = SendCommand(command);
	cout << receiveMsg;

	return true;
}

bool FtpClient::DeleteNonEmptyDirectory(const std::string& remotePath)
{
	return true;
}

bool FtpClient::GetFile(const std::string& remotePath, std::ostream& os)
{
	string receiveMsg;
	string command = "RETR " + remotePath + "\r\n";

	this->CreateDataChannel();
	receiveMsg = SendCommand(command);

	if (_mode == 0) // active
	{
		_dataSocket = _dataListener.Accept();
	}

	cout << receiveMsg;
	this->ReceiveData(os);

	receiveMsg = this->ReceiveFromCommandChannel();
	cout << receiveMsg;

	return true;
}

bool FtpClient::GetFile(const std::string& remotePath,
		const std::string& localPath)
{
	bool result;
	ofstream ofs(localPath);

	if (ofs.is_open())
	{
		result = this->GetFile(remotePath, ofs);
		ofs.close();
	}
	else
	{
		result = false;
	}

	return result;
}

bool FtpClient::PutFile(const std::string& remotePath, std::istream& is)
{
	string receiveMsg;
	string command = "STOR " + remotePath + "\r\n";

	this->CreateDataChannel();
	receiveMsg = SendCommand(command);
	cout << receiveMsg;

	if (_mode == 0) // active
	{
		_dataSocket = _dataListener.Accept();
	}

	this->SendData(is);

	receiveMsg = this->ReceiveFromCommandChannel();
	cout << receiveMsg;

	return true;
}

bool FtpClient::PutFile(const std::string& remotePath,
		const std::string& localPath)
{
	bool result;
	ifstream ifs(localPath);

	if (ifs.is_open())
	{
		result = this->PutFile(remotePath, ifs);
		ifs.close();
	}
	else
	{
		result = false;
	}

	return result;
}

bool FtpClient::DeleteFile(const std::string& remotePath)
{
	string receiveMsg;
	string command = "DELE " + remotePath + "\r\n";

	receiveMsg = SendCommand(command);
	cout << receiveMsg;

	return true;
}

std::string FtpClient::SendCommand(const std::string& command)
{
	int bytesRead;
	char buf[BUFFER_SIZE];

	_commandSocket.Send(command.c_str(), 0, command.length(), 0);
	bytesRead = _commandSocket.Receive(buf, 0, sizeof(buf), 0);

	return string(buf, bytesRead);
}

std::string FtpClient::ReceiveFromCommandChannel()
{
	int bytesRead;
	char buf[BUFFER_SIZE];
	bytesRead = _commandSocket.Receive(buf, 0, sizeof(buf), 0);
	return string(buf, bytesRead);
}

void FtpClient::CreateDataChannel()
{
	if (_mode == 0) // active
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
	else if (_mode == 1)// passive
	{
		string receiveMsg = this->SendCommand("PASV\r\n");
		IPEndPoint remoteDataEP = this->ParsePassiveResponse(receiveMsg);

		_dataSocket.SetProperties(AF_INET, SOCK_STREAM, 0);
		_dataSocket.Connect(remoteDataEP);
	}
}

void FtpClient::ReceiveData(std::ostream& os)
{
	int bytesRead;
	char buf[BUFFER_SIZE];

	do {
		bytesRead = _dataSocket.Receive(buf, 0, sizeof(buf), 0);
		if (bytesRead > 0)
		{
			os.write(buf, bytesRead);
		}
		else
		{
			break;
		}
	} while (true);

	_dataSocket.Shutdown(SHUT_RDWR);
	_dataSocket.Close();
}

void FtpClient::SendData(std::istream& is)
{
	char buf[BUFFER_SIZE];

	do {
		is.read(buf, BUFFER_SIZE);
		if (is.gcount() > 0)
		{
			_dataSocket.Send(buf, 0, is.gcount(), 0);
		}
		else
		{
			break;
		}
	} while (true);

	_dataSocket.Shutdown(SHUT_RDWR);
	_dataSocket.Close();
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
	StringHelper::ReplaceAll(ip, ".", ",");
}

IPEndPoint FtpClient::ParsePassiveResponse(const string& receiveMsg)
{
	size_t pos1 = receiveMsg.find_last_of("(");
	size_t pos2 = receiveMsg.find_last_of(")");
	string endPointStr = receiveMsg.substr(pos1 + 1, pos2 - pos1 - 1);

	pos1 = endPointStr.find_last_of(",");
	pos2 = endPointStr.find_last_of(",", pos1 - 1);

	string ipStr = endPointStr.substr(0, pos2);
	StringHelper::ReplaceAll(ipStr, ",", ".");
	string portStr1 = endPointStr.substr(pos2 + 1, pos1 - pos2 - 1);
	string portStr2 = endPointStr.substr(pos1 + 1, endPointStr.length() - pos1);

	int port = stoi(portStr1) * 256 + stoi(portStr2);

	return IPEndPoint(ipStr, port);
}
