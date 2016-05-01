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

	IPEndPoint localEP("127.0.0.1", 12000);
	Socket server(AF_INET, SOCK_STREAM, 0);
	server.SetSocketOption(SOL_SOCKET, SO_REUSEADDR, 1);
	Socket client;

	server.Bind(localEP);
	server.Listen(0);
	client = server.Accept();

	cout << client.GetRemoteEndPoint().GetTextAddress() << endl;
	cout << client.GetRemoteEndPoint().GetPort() << endl;

	string message = "Hello";
	client.Send(message.c_str(), 0, message.length(), 0);

	client.Close();

//	string serverIP = "192.168.56.2";
//	const uint16_t SERVER_PORT = 21;
//	string username = "lebui";
//	string password = "123";
//
//	FtpClient ftpClient;
//	ftpClient.Connect(serverIP, SERVER_PORT, username, password);
//	ftpClient.ListDirectory(cout);

	return 0;
}
