#ifndef FTPCLIENT_H_
#define FTPCLIENT_H_

#include <iostream>
#include "Socket.h"

class FtpClient
{
private:
	Socket _commandSocket;
	Socket _dataSocket;
	Socket _dataListener;
	static const size_t BUFFER_SIZE = 1024;
public:
	FtpClient();
	~FtpClient();

	void Connect(const std::string& host, int port = 21);
	void Connect(const std::string& host, int port,
			const std::string& username, const std::string& password);
	void Login(const std::string& username, const std::string& password);

	void ListDirectory(std::ostream& os);

private:
	std::string SendCommand(const std::string& command);
	void PrepareDataChannel();
	std::string FormalizePort(uint16_t port);
	void FormalizeIP(std::string& ip);
	void ReceiveData(std::ostream& os);
};

#endif /* FTPCLIENT_H_ */
