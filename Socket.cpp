#include "Socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

#include <iostream>
#include <errno.h>

// constructor

Socket::Socket()
{

}

Socket::Socket(int addressFamily, int type, int protocol)
{
	_type = type;
	_protocol = protocol;
	_cSocket = socket(addressFamily, type, protocol);
}

Socket::~Socket()
{
    close(_cSocket);
}

void Socket::SetProperties(int addressFamily, int type, int protocol)
{
	_type = type;
	_protocol = protocol;
	_cSocket = socket(addressFamily, type, protocol);
}


Socket& Socket::operator=(const Socket &other)
{
	this->_cSocket = other._cSocket;
	this->_protocol = other._protocol;
	this->_type = other._type;

	return *this;
}

// Property

IPEndPoint Socket::GetLocalEndPoint()
{
	IPEndPoint localEP;
	socklen_t addrlen = localEP.GetAddressLength();

	getsockname(_cSocket, (sockaddr*)&(localEP._cEndPoint), &addrlen);

	return localEP;
}

IPEndPoint Socket::GetRemoteEndPoint()
{
	IPEndPoint remoteEP;
	socklen_t addrlen = remoteEP.GetAddressLength();

	getpeername(_cSocket, (sockaddr*)&remoteEP._cEndPoint, &addrlen);

	return remoteEP;
}

int Socket::GetAddressFamily()
{
//	sockaddr_storage addr;
//	socklen_t addrlen;
//	getsockname(_cSocket, (sockaddr*)&addr, &addrlen);
//
//	return addr.ss_family;
    return AF_INET; // hard coded
}

int Socket::GetType()
{
	return _type;
}

int Socket::GetProtocol()
{
	return _protocol;
}

// method

void Socket::Bind(IPEndPoint &ipEndPoint)
{
	int status = bind(_cSocket, (sockaddr*)&(ipEndPoint._cEndPoint), ipEndPoint.GetAddressLength());

	if (status == -1)
	{
		std::cout << strerror(errno) << std::endl;
		std::cerr << "Error bind()" << std::endl;
		Close();
	}
}

void Socket::Connect(IPEndPoint &ipEndPoint)
{
	sockaddr_storage *p = &(ipEndPoint._cEndPoint);
	int status = connect(_cSocket, (sockaddr*)p, ipEndPoint.GetAddressLength());

	if (status == -1)
	{
		std::cout << strerror(errno) << std::endl;
		std::cerr << "Error connect()" << std::endl;
	}
}

void Socket::Connect(const std::string &host, int port)
{
	std::string sPort = std::to_string(port);
	
	struct addrinfo hints;
	struct addrinfo *res, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = GetAddressFamily();
	hints.ai_socktype = GetType();
	hints.ai_protocol = GetProtocol();
	hints.ai_flags = AI_PASSIVE;

	int status = getaddrinfo(host.c_str(), sPort.c_str(), &hints, &res);

	if (status == 0)
	{
		IPEndPoint ipEndPoint;
		std::memcpy(&(ipEndPoint._cEndPoint), res->ai_addr, sizeof(*(res->ai_addr)));

		Connect(ipEndPoint);
		
	}
	else
	{
		std::cout << gai_strerror(status) << std::endl;
		// throw exception
	}

	freeaddrinfo(res);
}


void Socket::Listen(int backlog)
{
	listen(_cSocket, backlog);
}

Socket Socket::Accept()
{
	Socket incommingSocket;
	int incomingCSocket = accept(_cSocket, NULL, NULL);

	if (incomingCSocket == -1)
	{
		std::cerr << "Error accept()" << std::endl;
	}
	else
	{
		incommingSocket._cSocket = incomingCSocket;
	}
	

	return incommingSocket;
}


ssize_t Socket::Send(const char *buf, int offset, size_t size, int flags)
{
	buf += offset;
	return send(_cSocket, buf, size, flags);
}

ssize_t Socket::SendTo(const char *buf, int offset, size_t size, int flags, const IPEndPoint &remoteEndPoint)
{
	return 0;
}


ssize_t Socket::Receive(char *buf, int offset, size_t size, int flags)
{
	buf += offset;
	return recv(_cSocket, buf, size, flags);
}

ssize_t Socket::ReceiveFrom(char *buf, int offset, size_t size, int flags, IPEndPoint &remoteEndPoint)
{
	return 0;
}

void Socket::Close()
{
	close(_cSocket);
}

void Socket::Shutdown(int how)
{
	shutdown(_cSocket, how);
}

void Socket::SetSocketOption(int level, int optname, int optVal)
{
	setsockopt(_cSocket, level, optname, &optVal, sizeof(optVal));
}
