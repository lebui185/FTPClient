#ifndef FTPCLIENT_H_
#define FTPCLIENT_H_

#include <iostream>
#include <string>
#include "Socket.h"

class FtpClient
{
private:
	Socket _commandSocket;
	Socket _dataSocket;
	Socket _dataListener;
	static const size_t BUFFER_SIZE = 1024;
	int _mode;

private:
	class Item
	{
	private:
		bool _isDirectory;
		std::string _localPath;
		std::string _remotePath;
	public:
		Item();
		Item(bool isDirectory, const std::string &remotePath);
		Item(bool isDirectory, const std::string &remotePath, const std::string &localPath);
		Item(const std::string &rawToken,
				const std::string &remoteParent,
				const std::string &localParent);

		void SetIsDirectory(bool value);
		bool IsDirectory();
		void SetLocalPath(const std::string &value);
		std::string GetLocalPath();

		void SetRemotePath(const std::string &value);
		std::string GetRemotePath();
	};

public:
	FtpClient();
	~FtpClient();

	int Connect(const std::string& host, int port = 21);
	int Login(const std::string& username, const std::string& password);

	int ListDirectory(std::ostream& os);
	int ChangeDirectory(const std::string &remotePath);
	int PrintDirectory(std::ostream& os);
	int GetDirectory(std::string remotePath, const std::string &localPath);
	int PutDirectory(const std::string &remotePath, const std::string &localPath);
	int DeleteEmptyDirectory(const std::string &remotePath);
	int DeleteNonEmptyDirectory(const std::string &remotePath);

	int GetFile(const std::string &remotePath, std::ostream& os);
	int GetFile(const std::string &remotePath, const std::string &localPath);
	int PutFile(const std::string &remotePath, std::istream& is);
	int PutFile(const std::string &remotePath, const std::string &localPath);
	int DeleteFile(const std::string& remotePath);

	void SetActiveMode();
	void SetPassiveMode();

private:
	std::string SendCommand(const std::string& command);
	std::string ReceiveFromCommandChannel();
	int CreateDataChannel();
	void ReceiveData(std::ostream& os);
	void SendData(std::istream& is);
	std::string FormalizePort(uint16_t port);
	void FormalizeIP(std::string& ip);
	IPEndPoint ParsePassiveResponse(const std::string& msg);
	int GetResponseCode(const std::string &response);
	int DeleteNonEmptyDirectoryRecursive(Item item);
};

#endif /* FTPCLIENT_H_ */
