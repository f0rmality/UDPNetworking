#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <chrono>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

#define NUM_PACKAGES 250

string encryptDecrypt (string toEncrypt)
{
	char key = 'K'; //Any char will work
	string output = toEncrypt;

	for (int i = 0; i < toEncrypt.size (); i++)
		output[i] = toEncrypt[i] ^ key;

	return output;
}


void main(int argc, char* argv[]) {

	//startup winsock
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	


	int wsOk = WSAStartup(version, &data);

	if (wsOk != 0) {
		cout << "Can't start winsock!" << wsOk;
		return;
	}

	//create a hint structure for the server
	sockaddr_in server;
	int serverLength = sizeof(server);
	char buf[1024];

	server.sin_family = AF_INET;
	server.sin_port = htons(54000);

	inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

	//socket creation
	SOCKET out = socket(AF_INET, SOCK_DGRAM, 0);

	//write out to the socket
	// string s(argv[1]);
	

	for (int i = 0; i < NUM_PACKAGES; ++i)
	{
		string s ("");
		s += NUM_PACKAGES + " ";
		s += i;
		// s += std::chrono::now()
		// add above here any more necessary information (like timer)

		int sendOK = sendto (out, s.c_str (), s.size () + 1, 0, (sockaddr*)&server, sizeof (server));

		if (sendOK == SOCKET_ERROR)
		{
			cout << "Houston, we have a problem " << WSAGetLastError () << " at package number " << i << endl;
		}
	}

	//client needs to listen for messages or will close socket immediately
	while (true) {

		ZeroMemory(buf, 1024);

		//wait for message
		int bytesIn = recvfrom(out, buf, 1024, 0, (sockaddr*)&server, &serverLength);
		if (bytesIn == SOCKET_ERROR) {
			cout << "Error receiving from client" << WSAGetLastError() << endl;
			continue;
		}

		cout << "Message received from server: " << " " << buf << " " << endl;
	}

	//close the socket
	closesocket(out);

	//cleanup winsock
	WSACleanup();

	getchar ();
}