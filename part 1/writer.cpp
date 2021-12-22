#include <windows.h>
#include <chrono>
#include <ctime>
#include <string>
#include <vector>

class Logger {
public:
    Logger() : process_start_time(std::chrono::system_clock::now()) {}
    void logging(const std::string&, const HANDLE&);
private:
    std::chrono::system_clock::time_point process_start_time;
};

int main() {
    Logger logger;
    srand(static_cast<unsigned int>(time(nullptr)));

    const size_t operation_count = 3;
    const size_t page_count = 3 + 0 + 7 + 1 + 3;
    const size_t page_size = 4096;
    const size_t file_size = page_size * page_count;
    std::vector<HANDLE> written_page_semaphores(page_count, nullptr);
    std::vector<HANDLE> read_page_semaphores(page_count, nullptr);
    HANDLE std_out = nullptr;
    HANDLE race_mutex = nullptr;
    HANDLE file_mapping = nullptr;
    void* map_view = nullptr;

    std_out = GetStdHandle(STD_OUTPUT_HANDLE);
    file_mapping = OpenFileMappingA(FILE_MAP_WRITE, false, "file_mapping");
    map_view = MapViewOfFile(file_mapping, FILE_MAP_ALL_ACCESS, 0, 0, file_size);
    for (size_t i = 0; i < page_count; ++i) {
        written_page_semaphores[i] = OpenSemaphoreA(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, false, ("written_page_semaphores" + std::to_string(i)).c_str());
        read_page_semaphores[i] = OpenSemaphoreA(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, false, ("read_page_semaphores" + std::to_string(i)).c_str());
    }
    race_mutex = OpenMutexA(MUTEX_ALL_ACCESS, false, "race_mutex");
    VirtualLock(map_view, file_size);

    if (file_mapping != nullptr) {
        for (size_t i = 0; i < operation_count; ++i) {
            size_t page = WaitForMultipleObjects(read_page_semaphores.size(), read_page_semaphores.data(), false, INFINITE);
            logger.logging("OBTAIN READ_PAGE_SEMAPHORE FOR PAGE " + std::to_string(page), std_out);

            WaitForSingleObject(race_mutex, INFINITE);
            logger.logging("OBTAIN RACE_MUTEX", std_out);

            logger.logging("START WRITING", std_out);
            SleepEx(rand() % 1000 + 500, false);
            logger.logging("END WRITING", std_out);

            if (ReleaseMutex(race_mutex))
                logger.logging("RELEASE RACE_MUTEX", std_out);
            else
                logger.logging("CODE " + std::to_string(GetLastError()), std_out);

            if (ReleaseSemaphore(written_page_semaphores[page], 1, nullptr))
                logger.logging("RELEASE WRITTEN_PAGE_SEMAPHORE FOR PAGE " + std::to_string(page), std_out);
            else
                logger.logging("CODE " + std::to_string(GetLastError()), std_out);
        }
    }
    else
        logger.logging("MAPPING WRITE FAIL", std_out);

    VirtualUnlock(map_view, file_size);
}

void Logger::logging(const std::string& message, const HANDLE& out) {
    auto time = std::chrono::system_clock::now();
    std::string log = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(time - process_start_time).count()) + " " + message + "\n";
    WriteFile(out, log.data(), log.size(), nullptr, nullptr);
}