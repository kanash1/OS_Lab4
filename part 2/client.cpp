#include <windows.h>
#undef max
#include "menu.h"

void WINAPI Callback(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {
	std::cout << "Message is received\n";
}

class Client {
public:
	Client() : is_connected(false), pipe(nullptr) {
		event = CreateEventA(nullptr, false, false, nullptr);
		if (event == INVALID_HANDLE_VALUE)
			throw(GetLastError());
	}
	void Connect() {
		if (!is_connected) {
			pipe = CreateFileA(
				"\\\\.\\pipe\\lab4_pipename",
				GENERIC_READ,
				0,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_OVERLAPPED,
				NULL
			);
			if (pipe != INVALID_HANDLE_VALUE) {
				is_connected = true;
				std::cout << "Connected\n";
			}
			else
				std::cout << "Connection failed: error " << GetLastError() << '\n';
		}
		else
			std::cout << "Client was already connected\n";
	}
	void ReadMessage() {
		OVERLAPPED over_read = OVERLAPPED();
		char message[buffer_size] = {'\0'};
		over_read.hEvent = event;
		if (ReadFileEx(pipe, message, 512, &over_read, Callback)) {
			SleepEx(INFINITE, true);
			std::cout << "Message: " << message << '\n';
		}
		else
			std::cout << "Reading failed: error " << GetLastError() << '\n';
	}
	void Disconnect() {
		is_connected = !CloseHandle(pipe);
		if (!is_connected) {
			std::cout << "Disonnected\n";
			pipe = nullptr;
		}
		else
			std::cout << "Disconnection failed: error " << GetLastError() << '\n';
	}
	~Client() {
		if (pipe != INVALID_HANDLE_VALUE && pipe != nullptr)
			CloseHandle(pipe);
		if (event != INVALID_HANDLE_VALUE && event != nullptr)
			CloseHandle(event);
	}
private:
	static const size_t buffer_size = 512;
	bool is_connected;
	HANDLE event;
	HANDLE pipe;
};

int main() {
	try {
		Client client;
		Menu menu(
			"CLIENT",
			"Exit",
			std::vector<Item>({
				Item("Connect to pipe", [&client]() { client.Connect(); }),
				Item("Read message", [&client]() { client.ReadMessage(); }),
				Item("Disconnect from pipe", [&client]() { client.Disconnect(); })
			})
		);
		menu_process(menu);
	} catch (DWORD& error) {
		std::cout << "Event creation failed: error " << error << '\n';
	}
}