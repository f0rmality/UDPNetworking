#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <thread>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

std::string encryptDecrypt (std::string toEncrypt)
{
	char key = 'K'; //Any char will work
	std::string output = toEncrypt;

	for (int i = 0; i < toEncrypt.size (); i++)
		output[i] = toEncrypt[i] ^ key;

	return output;
}

std::vector<std::string> split (const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream (s);
	while (std::getline (tokenStream, token, delimiter))
	{
		tokens.push_back (token);
	}
	return tokens;
}

struct ClientMessage
{
	int totalMessages = 0;
	int messagesReceived = 0;
};

std::map<char*, ClientMessage> clients;

static bool close = false;
void shouldClose ()
{
	getchar ();
	close = true;
}

void main() {
	std::thread stopListeningThread (shouldClose);
	//startup winsock
	WSADATA data;
	WORD version = MAKEWORD(2, 2);

	int wsOk = WSAStartup(version, &data);

	if (wsOk != 0) {
		std::cout << "Can't start winsock!" << wsOk;
		return;
	}

	//bind socket to to ip address and port
	SOCKET in = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in serverHint;
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(54000); //convert from little to big endian

	if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
		std::cout << "Can't find socket!" << WSAGetLastError() << std::endl;
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
	
	//enter a loop
	while (!close) {

		ZeroMemory(buf, 1024);

		//wait for message
		int bytesIn = recvfrom(in, buf, 1024, 0, (sockaddr*)&client, &clientLength);
		if (bytesIn == SOCKET_ERROR) {
			std::cout << "Error receiving from client" << WSAGetLastError() << std::endl;
			continue;
		}

		std::string decryptedMessage = encryptDecrypt (buf);

		std::vector<std::string> messages = split (decryptedMessage, ' ');

		//display message and client info
		char clientIP[256];
		ZeroMemory(clientIP, 256);

		inet_ntop(AF_INET, &client.sin_addr, clientIP, 256);

		clients[clientIP].totalMessages = stoi (messages[0]);
		++clients[clientIP].messagesReceived;

		sendto (in, decryptedMessage.c_str(), decryptedMessage.length() + 1, 0, (sockaddr*)&client, sizeof (client));
		std::cout << "Message received from: " << clientIP << " " << //endl;
			 " " << (float)clients[clientIP].messagesReceived << 
			 " " << (float)clients[clientIP].totalMessages << std::endl;
	}

	//do we sort the messages by ping afterwards???

	std::cout << "out!" << std::endl;

	stopListeningThread.join ();

	//close socket
	closesocket(in);

	//shutdown winsock
	WSACleanup();

}