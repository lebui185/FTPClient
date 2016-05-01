#ifndef IPENDPOINT_H_
#define IPENDPOINT_H_

#include <string>
#include <sys/socket.h>

class IPEndPoint
{
friend class Socket;
private:
	/// member
	sockaddr_storage _cEndPoint;
public:
	// constructor
	IPEndPoint();
	IPEndPoint(unsigned long int address, uint16_t port);
	IPEndPoint(const std::string &textAddress, uint16_t port);

	// public method
	std::string GetTextAddress();
	int GetAddressFamily();
	uint16_t GetPort();

private:
	// private method
	socklen_t GetAddressLength();
};

#endif /* IPENDPOINT_H_ */
