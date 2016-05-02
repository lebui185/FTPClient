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

	void ListDirectory(std::ostream& os);
	void GetFile(std::ostream& os, std::string path);
	int GetMode() const;
	void SetMode(int mode);

private:
	std::string SendCommand(const std::string& command);
	std::string ReceiveFromCommandChannel();
	void CreateDataChannel(std::ostream& os);
	void StartDataChannel(std::ostream& os);
	std::string FormalizePort(uint16_t port);
	void FormalizeIP(std::string& ip);
	IPEndPoint ParsePassiveResponse(const std::string& msg);
};

#endif /* FTPCLIENT_H_ */
