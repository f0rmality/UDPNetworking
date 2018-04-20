#include <iostream>
#include <string>
#include <map>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

string encryptDecrypt (string toEncrypt)
{
	char key = 'K'; //Any char will work
	string output = toEncrypt;

	for (int i = 0; i < toEncrypt.size (); i++)
		output[i] = toEncrypt[i] ^ key;

	return output;
}

struct ClientMessage
{

};

std::map<int, ClientMessage> clients;

void main() {

	//startup winsock
	WSADATA data;
	WORD version = MAKEWORD(2, 2);

	int wsOk = WSAStartup(version, &data);

	if (wsOk != 0) {
		cout << "Can't start winsock!" << wsOk;
		return;
	}

	//bind socket to to ip address and port
	SOCKET in = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in serverHint;
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(54000); //convert from little to big endian

	if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
		cout << "Can't find socket!" << WSAGetLastError() << endl;
		return;
	}

	sockaddr_in client;
	int clientLength = sizeof(client);
	ZeroMemory(&client, clientLength);

	char buf[1024];


	//string encrypted = encryptDecrypt ("this is a test");
	//cout << "Encrypted:" << encrypted << "\n";

	//string decrypted = encryptDecrypt (encrypted);
	//cout << "Decrypted:" << decrypted << "\n";

	static int i = 0;
	
	//enter a loop
	while (true) {

		ZeroMemory(buf, 1024);

		//wait for message
		int bytesIn = recvfrom(in, buf, 1024, 0, (sockaddr*)&client, &clientLength);
		if (bytesIn == SOCKET_ERROR) {
			cout << "Error receiving from client" << WSAGetLastError() << endl;
			continue;
		}

		//display message and client info
		char clientIP[256];
		ZeroMemory(clientIP, 256);

		inet_ntop(AF_INET, &client.sin_addr, clientIP, 256);
		sendto (in, buf, 2, 0, (sockaddr*)&client, sizeof (client));
		cout << "Message received from: " << clientIP << " " << buf << " " << ++i << endl;
	}

	//do we sort the messages by ping afterwards???

	//close socket
	closesocket(in);

	//shutdown winsock
	WSACleanup();

}