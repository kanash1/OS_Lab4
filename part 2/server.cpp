#include <windows.h>
#include <iostream>
#undef max
#include "menu.h"

class Server {
public:
	Server() : is_connected(false), pipe(INVALID_HANDLE_VALUE), event(INVALID_HANDLE_VALUE) {
		event = CreateEventA(nullptr, false, false, nullptr);
		pipe = CreateNamedPipeA(
			"\\\\.\\pipe\\lab4_pipename",
			PIPE_ACCESS_OUTBOUND,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			buffer_size,
			buffer_size,
			time_out,
			nullptr
		);
		if (event == INVALID_HANDLE_VALUE || pipe == INVALID_HANDLE_VALUE)
			throw(GetLastError());
	}
	void Connect() {
		if (!is_connected) {
			OVERLAPPED over_pipe = OVERLAPPED();
			over_pipe.hEvent = event;
			is_connected = ConnectNamedPipe(pipe, &over_pipe);
			WaitForSingleObject(event, INFINITE);
			if (is_connected)
				std::cout << "Connected\n";
			else
				std::cout << "Connection failed: error " << GetLastError() << '\n';
		}
		else 
			std::cout << "Server was already connected\n";
	}
	void WriteMessage() {
		OVERLAPPED over_write = OVERLAPPED();
		std::string message;
		std::cout << "Enter message:\n";
		std::cin >> message;
		over_write.hEvent = event;
		if (message.size() <= buffer_size) {
			if (WriteFile(pipe, message.data(), message.size(), nullptr, &over_write))
				std::cout << "Message was written sucessfully\n";
			else
				std::cout << "Writing failed: error " << GetLastError() << '\n';
		}
		else
			std::cout << "Writing failed: too big message\n";
	}
	void Disconnect() {
		is_connected = !DisconnectNamedPipe(pipe);
		if (!is_connected)
			std::cout << "Disonnected\n";
		else
			std::cout << "Disconnection failed: error " << GetLastError() << '\n';
	}
	~Server() {
		if (pipe != INVALID_HANDLE_VALUE)
			CloseHandle(pipe);
		if (event != INVALID_HANDLE_VALUE)
			CloseHandle(event);
	}
private:
	static const size_t buffer_size = 512;
	static const size_t time_out = 0;
	bool is_connected;
	HANDLE event;
	HANDLE pipe;
};

int main() {
	try {
		Server server;
		Menu menu(
			"SERVER",
			"Exit",
			std::vector<Item>({
				Item("Connect to pipe", [&server]() { server.Connect(); }),
				Item("Write message",[&server]() { server.WriteMessage(); }),
				Item("Disconect from pipe",[&server]() { server.Disconnect(); })
			})
		);
		menu_process(menu);
	} catch (DWORD& error) {
		std::cout << "Pipe or event creation failed: error " << error << '\n';
	}
}
