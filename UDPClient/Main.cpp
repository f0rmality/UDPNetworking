#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <thread>
#include <chrono>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

#define NUM_PACKAGES 250

SOCKET createUDPSocket (string server, int port);
void sendMessages (int numberOfMessages, SOCKET toSend);
void listenToMessages (SOCKET toListen);
string encryptDecrypt (string toEncrypt);

/** --- GLOBAL VARIABLES */
//startup winsock
WSADATA g_data;
WORD g_version = MAKEWORD (2, 2);
sockaddr_in g_server;

void main(int argc, char* argv[]) 
{
	SOCKET out = createUDPSocket ("127.0.0.1", 54000);

	std::thread listenerThread(std::bind(listenToMessages,out));

	std::thread senderThread (std::bind(sendMessages, NUM_PACKAGES, out));

	senderThread.join ();
	listenerThread.join ();
	
	//close the socket
	closesocket(out);

	//cleanup winsock
	WSACleanup();

	getchar ();
}

SOCKET createUDPSocket (string serverIP, int port)
{
	int wsOk = WSAStartup (g_version, &g_data);

	if (wsOk != 0)
	{
		cout << "Can't start winsock!" << wsOk;
		return -1;
	}

	//create a hint structure for the server

	int serverLength = sizeof (g_server);
	char buf[1024];

	g_server.sin_family = AF_INET;
	g_server.sin_port = htons (port);

	inet_pton (AF_INET, serverIP.c_str(), &g_server.sin_addr);

	//socket creation
	SOCKET out = socket (AF_INET, SOCK_DGRAM, 0);

	return out;
}

void sendMessages (int numberOfMessages, SOCKET toSend)
{
	for (int i = 0; i < numberOfMessages; ++i)
	{
		string s ("");
		s += std::to_string(numberOfMessages);
		s += std::to_string(i);
		// s += std::chrono::now()
		// add above here any more necessary information (like timer)

		string encrypted = encryptDecrypt (s);
		cout << " e " << s << " " << encrypted << endl;

		int sendOK = sendto (toSend, encrypted.c_str (), encrypted.size () + 1, 0, (sockaddr*)&g_server, sizeof (g_server));

		if (sendOK == SOCKET_ERROR)
		{
			cout << "Houston, we have a problem " << WSAGetLastError () << " at package number " << i << endl;
		}
	}
}

void listenToMessages (SOCKET toListen)
{
	char buf[1024];
	int serverLength = sizeof (g_server);

	//client needs to listen for messages or will close socket immediately
	while (true)
	{
		ZeroMemory (buf, 1024);

		//wait for message
		int bytesIn = recvfrom (toListen, buf, 1024, 0, (sockaddr*)&g_server, &serverLength);
		if (bytesIn == SOCKET_ERROR)
		{
			cout << "Error receiving from client" << WSAGetLastError () << endl;
			continue;
		}

		cout << "Message received from server: " << buf << endl;
	}
}

string encryptDecrypt (string toEncrypt)
{
	char key = 'K'; //Any char will work
	string output = toEncrypt;

	for (int i = 0; i < toEncrypt.size (); i++)
		output[i] = toEncrypt[i] ^ key;

	return output;
}