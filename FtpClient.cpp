/*
 * FtpClient.cpp
 *
 *  Created on: Apr 25, 2016
 *      Author: le
 */

#include <sstream>
#include <fstream>
#include <queue>
#include <stack>
#include <sys/stat.h>
#include <vector>
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

void FtpClient::SetActiveMode()
{
	_mode = 0;
}

void FtpClient::SetPassiveMode()
{
	_mode = 1;
}

int FtpClient::Connect(const std::string& host, int port)
{
	int bytesRead;
	string msg;
	char buf[BUFFER_SIZE];

	IPEndPoint serverEP(host, port);
	_commandSocket.SetProperties(AF_INET, SOCK_STREAM, 0);
	_commandSocket.Connect(serverEP);

	bytesRead = _commandSocket.Receive(buf, 0, sizeof(buf), 0);
	msg.assign(buf, bytesRead);

	return 1;
}

int FtpClient::Login(const std::string& username, const std::string& password)
{
	string response;
	string command;

	command = "USER " + username + "\r\n";
	response = SendCommand(command);

	command = "PASS " + password + "\r\n";
	response = SendCommand(command);

	return this->GetResponseCode(response);
}

int FtpClient::ListDirectory(ostream& os)
{
	string response;

	this->CreateDataChannel();
	response = SendCommand("MLSD\r\n");

	if (_mode == 0) // active
	{
		_dataSocket = _dataListener.Accept();
	}

	cout << response;
	this->ReceiveData(os);

	response = this->ReceiveFromCommandChannel();
	cout << response;

	return this->GetResponseCode(response);
}

int FtpClient::ChangeDirectory(const std::string &remotePath)
{
	string response;
	string command = "CWD " + remotePath + "\r\n";

	response = SendCommand(command);
	cout << response;

	return this->GetResponseCode(response);
}

int FtpClient::PrintDirectory(std::ostream& os)
{
	string response;
	string command = "PWD\r\n";

	response = SendCommand(command);
	os << response;

	return this->GetResponseCode(response);
}

int FtpClient::GetDirectory(std::string remotePath, const std::string& localPath)
{
	int result = this->ChangeDirectory(remotePath);

	if (result == 250)
	{
		string absoluteDirectory;
		string topDirectory;

		if (remotePath[0] != '/')
		{
			ostringstream oss;
			this->PrintDirectory(oss);
			string temp = oss.str();

			size_t pos1 = temp.find("\"");
			size_t pos2 = temp.find("\"", pos1 + 1);
			absoluteDirectory = temp.substr(pos1 + 1, pos2 - pos1 - 1);
		}
		else
		{
			absoluteDirectory = remotePath;
		}

		size_t pos = absoluteDirectory.rfind("/");
		if (pos != string::npos)
		{
			topDirectory = absoluteDirectory.substr(pos + 1, absoluteDirectory.length() - pos - 1);
		}

		queue<Item> itemsQueue;
		itemsQueue.push(Item(true, absoluteDirectory, localPath + "/" + topDirectory));

		while (!itemsQueue.empty())
		{
			Item currentItem = itemsQueue.front();
			itemsQueue.pop();

			if (currentItem.IsDirectory())
			{
				// Get directory info
				this->ChangeDirectory(currentItem.GetRemotePath());

				stringstream ss;
				this->ListDirectory(ss);
				vector<string> tokens;
				StringHelper::Split(ss.str(), tokens, "\r\n");

				// push directory item to queue
				for(auto &token : tokens)
				{
					Item item(token, currentItem.GetRemotePath(), currentItem.GetLocalPath());
					itemsQueue.push(item);
				}

				// Create directory
				struct stat st = {0};
				if (stat(currentItem.GetLocalPath().c_str(), &st) == -1)
				{
					mkdir(currentItem.GetLocalPath().c_str(), 0700);
				}
			}
			else // file
			{
				this->GetFile(currentItem.GetRemotePath(), currentItem.GetLocalPath());
			}
		}
	}

	return result;
}

int FtpClient::PutDirectory(const std::string& remotePath, const std::string& localPath)
{
	return 1;
}

int FtpClient::DeleteEmptyDirectory(const std::string& remotePath)
{
	string command = "RMD " + remotePath + "\r\n";
	string response = SendCommand(command);
	cout << response;

	return this->GetResponseCode(response);
}

int FtpClient::DeleteNonEmptyDirectory(const std::string& remotePath)
{
	int result = this->ChangeDirectory(remotePath);

	if (result == 250)
	{
		string absoluteDirectory;

		if (remotePath[0] != '/')
		{
			ostringstream oss;
			this->PrintDirectory(oss);
			string temp = oss.str();

			size_t pos1 = temp.find("\"");
			size_t pos2 = temp.find("\"", pos1 + 1);
			absoluteDirectory = temp.substr(pos1 + 1, pos2 - pos1 - 1);
		}
		else
		{
			absoluteDirectory = remotePath;
		}

		Item item(true, absoluteDirectory);
		result = this->DeleteNonEmptyDirectoryRecursive(item);
	}

	return result;
}

int FtpClient::DeleteNonEmptyDirectoryRecursive(Item item)
{
	if (item.IsDirectory())
	{
		this->ChangeDirectory(item.GetRemotePath());
		ostringstream oss;
		this->ListDirectory(oss);
		vector<string> tokens;
		StringHelper::Split(oss.str(), tokens, "\r\n");

		for(auto &token : tokens)
		{
			Item subItem(token, item.GetRemotePath(), "");
			this->DeleteNonEmptyDirectoryRecursive(subItem);
		}

		this->DeleteEmptyDirectory(item.GetRemotePath());
		return 1;
	}
	else
	{
		this->DeleteFile(item.GetRemotePath());
		return 1;
	}
	return 0;
}

int FtpClient::GetFile(const std::string& remotePath, std::ostream& os)
{
	string response;
	string command = "RETR " + remotePath + "\r\n";

	this->CreateDataChannel();
	response = SendCommand(command);

	if (_mode == 0) // active
	{
		_dataSocket = _dataListener.Accept();
	}

	cout << response;
	this->ReceiveData(os);

	response = this->ReceiveFromCommandChannel();
	cout << response;

	return this->GetResponseCode(response);
}

int FtpClient::GetFile(const std::string& remotePath, const std::string& localPath)
{
	int result;
	ofstream ofs(localPath);

	if (ofs.is_open())
	{
		result = this->GetFile(remotePath, ofs);
		ofs.close();
	}
	else
	{
		result = 0;
	}

	return result;
}

int FtpClient::PutFile(const std::string& remotePath, std::istream& is)
{
	string response;
	string command = "STOR " + remotePath + "\r\n";

	this->CreateDataChannel();
	response = SendCommand(command);
	cout << response;

	if (_mode == 0) // active
	{
		_dataSocket = _dataListener.Accept();
	}

	this->SendData(is);

	response = this->ReceiveFromCommandChannel();
	cout << response;

	return this->GetResponseCode(response);
}

int FtpClient::PutFile(const std::string& remotePath, const std::string& localPath)
{
	int result;
	ifstream ifs(localPath);

	if (ifs.is_open())
	{
		result = this->PutFile(remotePath, ifs);
		ifs.close();
	}
	else
	{
		result = 0;
	}

	return result;
}

int FtpClient::DeleteFile(const std::string& remotePath)
{
	string response;
	string command = "DELE " + remotePath + "\r\n";

	response = SendCommand(command);
	cout << response;

	return this->GetResponseCode(response);
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

int FtpClient::CreateDataChannel()
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
		string response = SendCommand(portCommand);
	}
	else if (_mode == 1)// passive
	{
		string response = this->SendCommand("PASV\r\n");
		IPEndPoint remoteDataEP = this->ParsePassiveResponse(response);

		_dataSocket.SetProperties(AF_INET, SOCK_STREAM, 0);
		_dataSocket.Connect(remoteDataEP);
	}

	return 1;
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

IPEndPoint FtpClient::ParsePassiveResponse(const std::string& response)
{
	size_t pos1 = response.find_last_of("(");
	size_t pos2 = response.find_last_of(")");
	string endPointStr = response.substr(pos1 + 1, pos2 - pos1 - 1);

	pos1 = endPointStr.find_last_of(",");
	pos2 = endPointStr.find_last_of(",", pos1 - 1);

	string ipStr = endPointStr.substr(0, pos2);
	StringHelper::ReplaceAll(ipStr, ",", ".");
	string portStr1 = endPointStr.substr(pos2 + 1, pos1 - pos2 - 1);
	string portStr2 = endPointStr.substr(pos1 + 1, endPointStr.length() - pos1);

	int port = stoi(portStr1) * 256 + stoi(portStr2);

	return IPEndPoint(ipStr, port);
}

int FtpClient::GetResponseCode(const std::string& response)
{
	int responseCode;
	istringstream(response) >> responseCode;

	return responseCode;
}

FtpClient::Item::Item()
{

}

FtpClient::Item::Item(bool isDirectory, const std::string &remotePath)
{
	_isDirectory = isDirectory;
	_remotePath = remotePath;
}

FtpClient::Item::Item(bool isDirectory, const std::string &remotePath, const std::string &localPath)
{
	_isDirectory = isDirectory;
	_remotePath = remotePath;
	_localPath = localPath;
}

FtpClient::Item::Item(const std::string &rawToken,
		const std::string &remoteParent,
		const std::string &localParent)
{
	vector<string> tokens;
	StringHelper::Split(rawToken, tokens, ";");
	string path;
	if (tokens[0].find("dir") != string::npos)
	{
		_isDirectory = true;
		path = tokens[2].substr(1, tokens[2].length() - 1);

	}
	else // file
	{
		_isDirectory = false;
		path = tokens[3].substr(1, tokens[3].length() - 1);
	}

	_localPath = localParent + "/" + path;
	_remotePath = remoteParent + "/" + path;
}

void FtpClient::Item::SetIsDirectory(bool value)
{
	_isDirectory = value;
}

bool FtpClient::Item::IsDirectory()
{
	return _isDirectory;
}

void FtpClient::Item::SetLocalPath(const std::string &value)
{
	_localPath = value;
}

std::string FtpClient::Item::GetLocalPath()
{
	return _localPath;
}

void FtpClient::Item::SetRemotePath(const std::string &value)
{
	_remotePath = value;
}

std::string FtpClient::Item::GetRemotePath()
{
	return _remotePath;
}
