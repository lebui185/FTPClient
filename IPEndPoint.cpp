#include "IPEndPoint.h"

#include <iostream>
#include <cstring>
#include <arpa/inet.h>

using namespace std;

// constructor

IPEndPoint::IPEndPoint()
{
	std::memset(&_cEndPoint, 0, sizeof(_cEndPoint));
}

IPEndPoint::IPEndPoint(unsigned long int address, uint16_t port)
{
	switch (address)
	{
		case INADDR_ANY:
		{
			sockaddr_in *p = (sockaddr_in*)&_cEndPoint;

			p->sin_family = AF_INET;
			p->sin_port = htons(port);
			p->sin_addr.s_addr = htonl(INADDR_ANY);
			break;
		}
	}
}

IPEndPoint::IPEndPoint(const std::string &textAddress, uint16_t port)
{
	std::memset(&_cEndPoint, 0, sizeof(_cEndPoint));

	in_addr inAddr;
	in6_addr in6Addr;

	if (inet_pton(AF_INET, textAddress.c_str(), &inAddr) == 1)
	{
		sockaddr_in *p = (sockaddr_in*)&_cEndPoint;

		p->sin_family = AF_INET;
		p->sin_port = htons(port);
		p->sin_addr = inAddr;

		return;
	}

	if (inet_pton(AF_INET6, textAddress.c_str(), &in6Addr) == 1)
	{
		sockaddr_in6 *p = (sockaddr_in6*)&_cEndPoint;

		p->sin6_family = AF_INET6;
		p->sin6_port = htons(port);
		p->sin6_addr = in6Addr;

		return;
	}

	cout << "error" << endl;

	// throw exception
}

// method
std::string IPEndPoint::GetTextAddress()
{
	char buffer[INET6_ADDRSTRLEN];
	void *p;

	switch (_cEndPoint.ss_family)
	{
		case AF_INET:
			p = &((sockaddr_in*)&_cEndPoint)->sin_addr;
			break;

		case AF_INET6:
			p = &((sockaddr_in6*)&_cEndPoint)->sin6_addr;
			break;
	}

	inet_ntop(_cEndPoint.ss_family, p, buffer, sizeof(buffer));
	std::string IPText(buffer);

	return IPText;
}

int IPEndPoint::GetAddressFamily()
{
	return _cEndPoint.ss_family;
}

uint16_t IPEndPoint::GetPort()
{
	uint16_t port;

	switch (_cEndPoint.ss_family)
	{
		case AF_INET:
		{
			sockaddr_in *p = (sockaddr_in*)&_cEndPoint;
			port = p->sin_port;
		}
		break;

		case AF_INET6:
		{
			sockaddr_in6 *p = (sockaddr_in6*)&_cEndPoint;
			port = p->sin6_port;
		}
		break;
	}

	return ntohs(port);
}

socklen_t IPEndPoint::GetAddressLength()
{
	return sizeof(_cEndPoint);
}
