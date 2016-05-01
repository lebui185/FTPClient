#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <string>
#include <netdb.h>
#include "IPEndPoint.h"

class Socket
{
private:
	// member
	int _cSocket;
	int _type;
	int _protocol;
public:

	Socket();
	Socket(int addressFamily, int type, int protocol);
        ~Socket();
        
	void SetProperties(int addressFamily, int type, int protocol);

	// property
	IPEndPoint GetLocalEndPoint();
	IPEndPoint GetRemoteEndPoint();
	int GetProtocol();
	int GetAddressFamily();
	int GetType();

	// method
	void Bind(IPEndPoint &ipEndPoint);

	void Connect(IPEndPoint& ipEndPoint);
	void Connect(const std::string &host, int port);

	void Listen(int backlog);
	Socket Accept();

	ssize_t Send(const char *buf, int offset, size_t size, int flags);
	ssize_t SendTo(const char *buf, int offset, size_t size, int flags, const IPEndPoint &remoteEndPoint);

	ssize_t Receive(char *buf, int offset, size_t size, int flags);
	ssize_t ReceiveFrom(char *buf, int offset, size_t size, int flags, IPEndPoint &remoteEndPoint);

	void SetSocketOption(int level, int optname, int optVal);
	void Close();
	void Shutdown(int how);
};

#endif
