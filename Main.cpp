#include <iostream>
#include <fstream>
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

	string localPath = "Mouse love rice.mp3";
	string remotePath = "\"/Asian/Lao Shu Ai Da Mi.mp3\"";
	ofstream ofs(localPath);

	FtpClient ftpClient;

	ftpClient.Connect(serverIP, SERVER_PORT, username, password);
	ftpClient.ListDirectory(cout);
	ftpClient.GetFile(ofs, remotePath);

	ofs.close();

	return 0;
}
