#include <windows.h>
#include <vector>
#include <string>
#include <iostream>

HANDLE startProcess(const std::string&, const std::string&);

int main() {
	const size_t page_count = 3 + 0 + 7 + 1 + 3;
	const size_t page_size = 4096;
	const size_t half_page_count = page_count / 2;
	const size_t file_size = page_size * page_count;
	std::vector<HANDLE> processes(page_count, nullptr);
	std::vector<HANDLE> written_page_semaphores(page_count, nullptr);
	std::vector<HANDLE> read_page_semaphores(page_count, nullptr);
	HANDLE race_mutex = nullptr;
	HANDLE mapped_file = nullptr;
	HANDLE file_mapping = nullptr;

	std::cout << "Initialization...\n";

	mapped_file = CreateFileA("mapped_file.txt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, 0, nullptr);
	file_mapping = CreateFileMappingA(mapped_file, nullptr, PAGE_READWRITE, 0, file_size, "file_mapping");
	for (size_t i = 0; i < page_count; ++i) {
		written_page_semaphores[i] = CreateSemaphoreA(nullptr, 0, 1, ("written_page_semaphores" + std::to_string(i)).c_str());
		read_page_semaphores[i] = CreateSemaphoreA(nullptr, 1, 1, ("read_page_semaphores" + std::to_string(i)).c_str());
	}
	race_mutex = CreateMutexA(nullptr, false, "race_mutex");

	for (size_t i = 0; i < half_page_count; ++i) {
		processes[i] = startProcess("C:\\Users\\Jesus\\source\\repos\\OS_lab4_1_1\\Debug\\OS_lab4_1_1.exe",
			"C:\\Users\\Jesus\\Desktop\\OS_lab4\\Write\\log" + std::to_string(i + 1) + ".txt");

		processes[i + half_page_count] = startProcess("C:\\Users\\Jesus\\source\\repos\\OS_lab4_1_2\\Debug\\OS_lab4_1_2.exe",
			"C:\\Users\\Jesus\\Desktop\\OS_lab4\\Read\\log" + std::to_string(i + 1) + ".txt");
	}

	std::cout << "Waiting for processes to end...\n";
	WaitForMultipleObjects(processes.size(), processes.data(), true, INFINITE);

	for (const auto& process : processes)
		CloseHandle(process);
	CloseHandle(file_mapping);
	CloseHandle(mapped_file);
	CloseHandle(race_mutex);
	for (size_t i = 0; i < page_count; ++i) {
		CloseHandle(read_page_semaphores[i]);
		CloseHandle(written_page_semaphores[i]);
	}
	
	std::cout << "Application has finished its work.\n";
	system("pause");
}

HANDLE startProcess(const std::string& exe_file_path, const std::string& log_file_path) {
	PROCESS_INFORMATION pi;
	STARTUPINFOA si;
	SECURITY_ATTRIBUTES sa = { sizeof(sa), nullptr, true };
	HANDLE log_file = CreateFileA(log_file_path.data(), GENERIC_WRITE, FILE_SHARE_WRITE,
		&sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));

	si.cb = sizeof(si);
	si.hStdOutput = log_file;
	si.hStdInput = nullptr;
	si.hStdError = nullptr;
	si.dwFlags = STARTF_USESTDHANDLES;

	if (CreateProcessA(exe_file_path.data(), nullptr, nullptr, nullptr, true, 0, nullptr, nullptr, &si, &pi) != 0)
		return pi.hProcess;

	return nullptr;
}