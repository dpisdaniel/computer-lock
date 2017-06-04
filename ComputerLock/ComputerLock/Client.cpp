#include "stdafx.h"
#include "Client.h"
#include "CommonTools.h"

int SocketWrapper::nofSockets_ = 0;

void SocketWrapper::Start() {
	if (!nofSockets_) {
		WSADATA info;
		if (WSAStartup(MAKEWORD(2, 0), &info)) {
			throw "Could not start WSA";
		}
	}
	++nofSockets_;
}

void SocketWrapper::End() {
	WSACleanup();
}

SocketWrapper::SocketWrapper() : sock_(0) {
	Start();
	// UDP: use SOCK_DGRAM instead of SOCK_STREAM
	sock_ = socket(AF_INET, SOCK_STREAM, 0);

	if (sock_ == INVALID_SOCKET) {
		throw "INVALID_SOCKET";
	}

	refCounter_ = new int(1);
}

SocketWrapper::SocketWrapper(SOCKET s) : sock_(s) {
	Start();
	refCounter_ = new int(1);
};

void SocketWrapper::RecreateSocket() {
	Close();
	sock_ = socket(AF_INET, SOCK_STREAM, 0);

	if (sock_ == INVALID_SOCKET)
		throw "INVALID_SOCKET";
	// NOTE: we already have a ref for this socekt, we are just reconnecting it

}

SocketWrapper::~SocketWrapper() {
	if (!--(*refCounter_)) {
		Close();
		delete refCounter_;
	}

	--nofSockets_;
	if (!nofSockets_) End();
}

SocketWrapper::SocketWrapper(const SocketWrapper& o) {
	refCounter_ = o.refCounter_;
	(*refCounter_)++;
	sock_ = o.sock_;

	nofSockets_++;
}

SocketWrapper& SocketWrapper::operator=(SocketWrapper& o) {
	(*o.refCounter_)++;

	refCounter_ = o.refCounter_;
	sock_ = o.sock_;

	nofSockets_++;

	return *this;
}

void SocketWrapper::Close() {
	closesocket(sock_);
}

string SocketWrapper::ReceiveBytes() {
	string ret;
	char buf[1024];

	while (1) {
		u_long arg = 0;
		if (ioctlsocket(sock_, FIONREAD, &arg) != 0)
			break;

		if (arg == 0)
			break;

		if (arg > 1024) arg = 1024;

		int rv = recv(sock_, buf, arg, 0);
		if (rv <= 0) break;

		string t;

		t.assign(buf, rv);
		ret += t;
	}

	return ret;
}

string SocketWrapper::ReceiveLine() {
	string ret;
	while (1) {
		char r;

		switch (recv(sock_, &r, 1, 0)) {
		case 0: // not connected anymore;
				// ... but last line sent
				// might not end in \n,
				// so return ret anyway.
			return ret;
		case -1:
			return "";
			//      if (errno == EAGAIN) {
			//        return ret;
			//      } else {
			//      // not connected anymore
			//      return "";
			//      }
		}

		ret += r;
		if (r == '\n')  return ret;
	}
}

void SocketWrapper::SendLine(string s) {
	s += '\n';
	send(sock_, s.c_str(), s.length(), 0);
}

int SocketWrapper::SendBytes(const string& s) {
	int result = send(sock_, s.c_str(), s.length(), 0);
	return result;
}

SocketClient::SocketClient(const char* ip, int port) : SocketWrapper() {
	strcpy_s(this->ip_, MAX_IP_LENGTH, ip);
	this->port_ = port;
	string error;
	addrinfo *result = NULL;
	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	int retVal = getaddrinfo((PCSTR)ip, to_string(port).c_str(), &hints, &result);
	if (retVal != 0) {
		printf("getaddrinfo failed with error: %d\n", retVal);
		WSACleanup();
		throw "getaddrinfo failed";
	}
	sockaddr_in* addr = (sockaddr_in*)result->ai_addr;
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	memset(&(addr->sin_zero), 0, 8);

	if (::connect(sock_, (sockaddr *)addr, sizeof(sockaddr))) {
		int WSAError = WSAGetLastError();
		cout << "Failed to connect with error " << WSAError << endl;
	}
}

void SocketClient::ReConnectSocket() {
	string error;
	addrinfo *result = NULL;
	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	int retVal = getaddrinfo((PCSTR)this->ip_, to_string(this->port_).c_str(), &hints, &result);
	if (retVal != 0) {
		printf("getaddrinfo failed with error: %d\n", retVal);
		WSACleanup();
		throw "getaddrinfo failed";
	}
	sockaddr_in* addr = (sockaddr_in*)result->ai_addr;
	addr->sin_family = AF_INET;
	addr->sin_port = htons(this->port_);
	memset(&(addr->sin_zero), 0, 8);

	if (::connect(sock_, (sockaddr *)addr, sizeof(sockaddr))) {
		int WSAError = WSAGetLastError();
		cout << "Failed to connect with error " << WSAError << endl;
	}
}

UpdateLoop::UpdateLoop(int interval, char* ip, int port) : SocketClient(ip, port) {
	this->interval_ = interval;
}

UpdateLoop::UpdateLoop(int interval) : SocketClient(SERVER_IP, PORT) {
	this->interval_ = interval;
}

void UpdateLoop::StartLoop() {
	clientThread_ = thread(&UpdateLoop::LoopForever, this);
	clientThread_.detach();
}

void UpdateLoop::LoopForever() {
	std::chrono::seconds wait_interval(this->interval_);
	while (true) { //TODO: If server restarts the socket needs to be refreshed because this will get stuck in socket error 10054
		int result = SendBytes(SETTINGS_REQUEST_MESSAGE(RetrieveToken()));
		if (result == SOCKET_ERROR) {
			cout << "time to retry" << endl;
			this->Close();
			this->RecreateSocket();
			this->ReConnectSocket();
			std::this_thread::sleep_for(wait_interval); // Does the waiting anyways
			continue; // will move to the next cycle so the request can be sent again
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
		string response = ReceiveBytes();
		printdbg(response.c_str());
		ParseSettingsResponse(response);
		std::this_thread::sleep_for(wait_interval);
	}
}

void UpdateLoop::ParseSettingsResponse(const string& response) {
	vector<string> results = common::split(response, MESSAGE_PARAMETER_DELIMITER);
	if (results.size() == 0)
		return;
	string length = results[SETTINGS_RESPONSE_LENGTH_ELEMENT];
	string processes = results[SETTINGS_PROCESSES_ELEMENT];
	common::RewriteFile(PROCESS_SETTINGS, processes);
	string filePaths = results[SETTINGS_FILE_PATHS_ELEMENT];
	common::RewriteFile(FILE_PATHS_SETTINGS, filePaths);
	string fileExtensions = results[SETTINGS_FILE_EXTS_ELEMENT];
	common::RewriteFile(FILE_EXT_SETTINGS, fileExtensions);
}

NotificationUpdate::NotificationUpdate(string notification) : SocketClient(SERVER_IP, PORT) {
	this->notification_ = notification;
	string notificationRequest = GetNotificationUpdateMessage(this->notification_.c_str());
	SendBytes(notificationRequest);
}

NotificationUpdate::NotificationUpdate(string notification, char* ip, int port) : SocketClient(ip, port) {
	this->notification_ = notification;
	string notificationRequest = GetNotificationUpdateMessage(this->notification_.c_str());
	SendBytes(notificationRequest);
}

string NotificationUpdate::GetNotificationUpdateMessage(const char* notification) {
	std::string notificationMessage (to_string((strlen(notification) + OPCODE_AND_DELIMITER_LENGTH + TOKEN_AND_DELIMITER_LENGTH)) + MESSAGE_PARAMETER_DELIMITER + RetrieveToken() + MESSAGE_PARAMETER_DELIMITER + SEND_NOTIFICATION);
	notificationMessage += notification;
	return notificationMessage;
	/*char* returnMessage = new char[notificationMessage.length() + 1];
	strcpy_s(returnMessage, notificationMessage.length() + 1, notificationMessage.c_str());
	return returnMessage;*/
}

string RetrieveToken() {
	fstream infile;
	infile.open(ADMIN_SETTINGS, ios::in);
	string fileContent("");
	infile >> fileContent;
	int position = fileContent.find('=', 0);
	string token = fileContent.substr(position + 1, TOKEN_SIZE);
	return token;
}