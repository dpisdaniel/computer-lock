#pragma once

#pragma comment(lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#define WIN32_LEAN_AND_MEAN
#define DEFAULT_BUFLEN 512
#define PORT 5151
#define SERVER_IP "192.168.1.144"
#define USE_STRICT_FORMAT TRUE

#define PROTOCOL_CONSTANTS
#define MESSAGE_PARAMETER_DELIMITER ':'
#define OPCODE_AND_DELIMITER_LENGTH 2
#define REQUEST_SETTINGS "1" + MESSAGE_PARAMETER_DELIMITER
#define SEND_NOTIFICATION "2" + MESSAGE_PARAMETER_DELIMITER //TODO: If sending a notification fails, write it to a file and have the settings update thread update the server with the notifications from that file
#define PING "3" + MESSAGE_PARAMETER_DELIMITER // PING REQUEST FOR FUTURE USE
#define TOKEN_SIZE 20
#define TOKEN_AND_DELIMITER_LENGTH 21
#undef PROTOCOL_CONSTANTS
#define SETTINGS_REQUEST_MESSAGE(token) to_string(token.length() + 1 + OPCODE_AND_DELIMITER_LENGTH) + MESSAGE_PARAMETER_DELIMITER + token + MESSAGE_PARAMETER_DELIMITER + "1" + MESSAGE_PARAMETER_DELIMITER
#define SERVER_PING_REQUEST "2" + MESSAGE_PARAMETER_DELIMITER + "3" + MESSAGE_PARAMETER_DELIMITER

// Settings response structure
#define SETTINGS_RESPONSE_LENGTH_ELEMENT 0
#define SETTINGS_PROCESSES_ELEMENT 1
#define SETTINGS_FILE_PATHS_ELEMENT 2 
#define SETTINGS_FILE_EXTS_ELEMENT 3

#define MAX_IP_LENGTH 16

#define DEBUG
#ifdef DEBUG
#define printdbg(msg) cout << msg << endl
#else
#define printdbg(msg)
#endif

class SocketWrapper {
public:
	virtual ~SocketWrapper();
	SocketWrapper(const SocketWrapper&);
	SocketWrapper& operator=(SocketWrapper&);

	string ReceiveLine();
	string ReceiveBytes();

	void Close();

	// The parameter of SendLine is not a const reference
	// because SendLine modifes the std::string passed.
	void SendLine(string);

	// The parameter of SendBytes is a const reference
	// because SendBytes does not modify the std::string passed 
	// (in contrast to SendLine).
	int SendBytes(const string&);

protected:
	SocketWrapper(SOCKET s);
	SocketWrapper();
	// Recreates sock_ after closing it. This is part of the reconnection strategy in case the server goes down.
	void RecreateSocket();
	SOCKET sock_;
	int* refCounter_;

private:
	// Starts up WSA
	static void Start();
	// Cleans up WSA
	static void End();
	static int  nofSockets_;
};

class SocketClient : public SocketWrapper {
public:
	SocketClient(const char* ip, int port);
protected:
	thread clientThread_;
	// This function reconnects a socket after a call to SockerWrapper::RecreateSocket. This is part of the reconnection strategy in case the server goes down.
	void ReConnectSocket();
	char ip_[MAX_IP_LENGTH];
	int port_;
};

class UpdateLoop : public SocketClient {
/*
	This class wraps the SocketClient class and allows the user to ask for settings changes once in a given time interval.
*/
public:
	/*Starts a socket connection to the server that will ask for settings changes every interval seconds.
	the interval is in seconds. IP is the ipv4 address of the server and the port is the server's port
	*/
	UpdateLoop(int interval, char* ip, int port);
	UpdateLoop(int interval);
	void StartLoop();
private:
	// Parses the response containing the settings and adds the settings to the settings file
	void ParseSettingsResponse(const string& response);
	void LoopForever();
	int interval_; // The interval between each settings update request
};

class NotificationUpdate : public SocketClient {
/*
	This class wraps the SocketClient class and allows the user to send a notification packet to the server.
*/
public:
	NotificationUpdate(string notification);
	NotificationUpdate(string notification, char* ip, int port);
private:
	/*
	Builds the final notification update message.
	The notification message has the following structure:
	[MESSAGE_LENGTH_AFTER_LENGTH_PART];3;[NOTIFICATION]
	*/
	string GetNotificationUpdateMessage(const char* notification);
	string notification_;
};

//Retrieves the token that was inserted by the admin
string RetrieveToken();