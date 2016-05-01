#include <iostream>
#include <string>
#include "FtpClient.h"
using namespace std;

//cd, dir, ls, ldc, get, put, detele

int main()
{
	string serverIP = "192.168.56.2";
	const uint16_t SERVER_PORT = 21;
	string username = "lebui";
	string password = "123";

	FtpClient ftpClient;
	ftpClient.Connect(serverIP, SERVER_PORT, username, password);
	ftpClient.ListDirectory(cout);

	return 0;
}
