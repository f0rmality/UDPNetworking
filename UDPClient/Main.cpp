#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <thread>
#include <chrono>

#include <deque>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

#define NUM_PACKAGES 250

SOCKET createUDPSocket (string server, int port);
void sendMessages (int numberOfMessages, SOCKET toSend);
void listenToMessages (SOCKET toListen);
string encryptDecrypt (string toEncrypt);
void shouldStopListening ();

/** --- GLOBAL VARIABLES */
//startup winsock
bool stopListening = false;
WSADATA g_data;
WORD g_version = MAKEWORD (2, 2);
sockaddr_in g_server;

void main(int argc, char* argv[]) 
{
	SOCKET out = createUDPSocket ("127.0.0.1", 54000);

	using namespace std::chrono;

	std::thread stopListeningThread (shouldStopListening);
	std::thread listenerThread(std::bind(listenToMessages,out));
	std::thread senderThread (std::bind(sendMessages, NUM_PACKAGES, out));

	stopListeningThread.join ();
	senderThread.join ();
	listenerThread.join ();

	std::cout << "threads closed " << std::endl;
	
	//close the socket
	closesocket(out);

	//cleanup winsock
	WSACleanup();
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


deque<std::chrono::steady_clock::time_point> starts;

void sendMessages (int numberOfMessages, SOCKET toSend)
{
	for (int i = 0; i < numberOfMessages; ++i)
	{
		string s ("");
		s += std::to_string(numberOfMessages) + " ";
		s += std::to_string(i) + " ";

		time_t rawtime;
		struct tm timeinfo;
		char buffer[40];

		/*time (&rawtime);
		localtime_s (&timeinfo, &rawtime);
		timeinfo.tm_sec / 1000;

		strftime (buffer, sizeof (buffer), "%I:%M:%S", &timeinfo);
		std::string str (buffer);
		s += str;*/

		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now ();
		starts.push_back (start);
		
		// add above here any more necessary information (like timer)

		string encrypted = encryptDecrypt (s);

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
	while (!stopListening)
	{
		ZeroMemory (buf, 1024);

		//wait for message
		int bytesIn = recvfrom (toListen, buf, 1024, 0, (sockaddr*)&g_server, &serverLength);
		if (bytesIn == SOCKET_ERROR)
		{
			cout << "Error receiving from client" << WSAGetLastError () << endl;
			continue;
		}

		auto start = starts.front ();
		starts.pop_front ();
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now ();
		//std::chrono::duration_cast<std::chrono::microseconds>(end - start).count ();

		cout << "Message received from server: " << buf << " " << 
			std::chrono::duration_cast<std::chrono::microseconds>(end - start).count () << "us" << endl;
	}
}

void shouldStopListening ()
{
	getchar ();
	stopListening = true;
}

string encryptDecrypt (string toEncrypt)
{
	char key = 'K'; //Any char will work
	string output = toEncrypt;

	for (int i = 0; i < toEncrypt.size (); i++)
		output[i] = toEncrypt[i] ^ key;

	return output;
}