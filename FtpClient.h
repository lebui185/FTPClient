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
public:
	FtpClient();
	~FtpClient();

	void Connect(const std::string& host, int port = 21);
	void Connect(const std::string& host, int port,
			const std::string& username, const std::string& password);
	void Login(const std::string& username, const std::string& password);

	bool ListDirectory(std::ostream& os);
	bool ChangeDirectory(const std::string &remotePath);
	bool GetDirectory(const std::string &remotePath, const std::string &localPath);
	bool PutDirectory(const std::string &remotePath, const std::string &localPath);
	bool DeleteEmptyDirectory(const std::string &remotePath);
	bool DeleteNonEmptyDirectory(const std::string &remotePath);

	bool GetFile(const std::string &remotePath, std::ostream& os);
	bool GetFile(const std::string &remotePath, const std::string &localPath);
	bool PutFile(const std::string &remotePath, std::istream& is);
	bool PutFile(const std::string &remotePath, const std::string &localPath);
	bool DeleteFile(const std::string& remotePath);

	int GetMode() const;
	void SetMode(int mode);

private:
	std::string SendCommand(const std::string& command);
	std::string ReceiveFromCommandChannel();
	void CreateDataChannel();
	void ReceiveData(std::ostream& os);
	void SendData(std::istream& is);
	std::string FormalizePort(uint16_t port);
	void FormalizeIP(std::string& ip);
	IPEndPoint ParsePassiveResponse(const std::string& msg);
};

#endif /* FTPCLIENT_H_ */
