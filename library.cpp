#include "library.h"
#include <iostream>
#include <windows.h>
#include <sys/stat.h>
#include <unordered_map>
#include <queue>
#include <cstdio>

using namespace std;
constexpr size_t SECTOR_SIZE = 512;
unordered_map<int, off_t> file_offsets;

struct CachePage {
    size_t offset;    //  Смещение данных
    size_t size;      // Размер страницы
    char *data;       // Данные страницы
    size_t frequency; // Частота использования
    bool dirty;       // Флаг модификации
};

class LFUCache {
public:
    explicit LFUCache(size_t capacity) : capacity_(capacity) {}

    ssize_t read(int fd, void *buf, size_t count, size_t offset);

    ssize_t write(int fd, const void *buf, size_t count, size_t offset);

    int sync(int fd);

private:
    size_t capacity_;
    unordered_map<size_t, CachePage> hashTable_;
    priority_queue<
            tuple<size_t, size_t>,
            vector<tuple<size_t, size_t>>,
            greater<>
    > timeQueue_;

    void evict(int fd);
};

LFUCache block_cache(32 * 1024 * 1024); // 32 MiB

ssize_t LFUCache::read(int fd, void *buf, size_t count, size_t offset) {
    offset &= ~(SECTOR_SIZE - 1); // Выравниваем смещение вниз
    count = (count + SECTOR_SIZE - 1) & ~(SECTOR_SIZE - 1); // Выравниваем смещение вверх
    if (hashTable_.find(offset) != hashTable_.end()) {
        CachePage &page = hashTable_[offset];
        cerr << "Cache hit for offset: " << offset << "\n";
        memcpy(buf, page.data, count);
        page.frequency++;
        timeQueue_.emplace(page.frequency, offset);

        return count;
    }
    if (lab2_lseek(fd, offset, SEEK_SET) < 0) {
        cerr << "Error seeking before read\n";
        return -1;
    }

    char *temp_buf = new char[count];
    DWORD bytes_read;
    auto hFile = (HANDLE)_get_osfhandle(fd);
    if (!ReadFile(hFile, temp_buf, count, &bytes_read, nullptr)) {
        delete[] temp_buf;
        cerr << "Error reading file: " << GetLastError() << '\n';
        return -1;
    }

    CachePage new_page = {offset, bytes_read, temp_buf, 1, false};
    if (hashTable_.size() >= capacity_) {
        evict(fd);
    }
    hashTable_[offset] = new_page;
    timeQueue_.emplace(1, offset);

    memcpy(buf, temp_buf, bytes_read);

    return bytes_read;
}

ssize_t LFUCache::write(int fd, const void *buf, size_t count, size_t offset) {
    offset &= ~(SECTOR_SIZE - 1); // Выравниваем смещение вниз
    count = (count + SECTOR_SIZE - 1) & ~(SECTOR_SIZE - 1); // Выравниваем смещение вверх

    if (lab2_lseek(fd, offset, SEEK_SET) < 0) {
        cerr << "Error seeking before write\n";
        return -1;
    }

    if (hashTable_.find(offset) != hashTable_.end()) {
        CachePage &page = hashTable_[offset];
        cerr << "Cache hit for offset: " << offset << "\n";
        memcpy(page.data, buf, count);
        page.dirty = true;
        page.frequency++;
        timeQueue_.emplace(page.frequency, offset);

        return count;
    }

    char *new_data = new char[count];
    memcpy(new_data, buf, count);
    CachePage new_page = {offset, count, new_data, 1, true};
    if (hashTable_.size() >= capacity_) {
        evict(fd);
    }
    hashTable_[offset] = new_page;
    timeQueue_.emplace(1, offset);

    return count;
}

int LFUCache::sync(int fd) {
    auto handle = (HANDLE)_get_osfhandle(fd);
    for (auto &entry : hashTable_) {
        CachePage &page = entry.second;
        if (page.dirty) {
            if (lab2_lseek(fd, page.offset, SEEK_SET) < 0) {
                cerr << "Error seeking before sync\n";
                return -1;
            }
            DWORD written;
            if (!WriteFile(handle, page.data, static_cast<DWORD>(page.size), &written, nullptr) || written != page.size) {
                cerr << "Error syncing page: " << GetLastError() << '\n';
                return -1;
            }
            page.dirty = false;
        }
    }
    return 0;
}

void LFUCache::evict(int fd) {
    if (timeQueue_.empty()) return;

    auto [freq, offset] = timeQueue_.top();
    timeQueue_.pop();

    auto it = hashTable_.find(offset);
    if (it != hashTable_.end()) {
        if (it->second.dirty) {
            cerr << "Evicted page is dirty, syncing first.\n";
            sync(fd);
        }
        delete[] it->second.data;
        hashTable_.erase(it);
    }
}

int lab2_open(const char *path) {
    HANDLE hFile = CreateFile(
            path,
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
            nullptr
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        cerr << "Error opening file: " << GetLastError() << '\n';
        return -1;
    }
    int fd = _open_osfhandle((intptr_t)hFile, 0);
    file_offsets[fd] = 0;
    return fd;
}

int lab2_close(int fd) {
    auto hFile = (HANDLE)_get_osfhandle(fd);
    if (hFile == INVALID_HANDLE_VALUE) {
        cerr << "Invalid file descriptor\n";
        return -1;
    }
    if (block_cache.sync(fd) < 0) {
        cerr << "Error syncing data to disk\n";
        CloseHandle(hFile);
        return -1;
    }
    if (!CloseHandle(hFile)) {
        cerr << "Error closing file: " << GetLastError() << '\n';
        return -1;
    }

    file_offsets.erase(fd);
    return 0;
}

ssize_t lab2_read(int fd, void *buf, size_t count) {
    off_t current_offset = file_offsets[fd];
    ssize_t result = block_cache.read(fd, buf, count, current_offset);
    if (result < 0) {
        cerr << "Error reading from cache\n";
        return -1;
    }
    file_offsets[fd] += result;
    return result;
}

ssize_t lab2_write(int fd, const void *buf, size_t count) {
    off_t current_offset = file_offsets[fd];
    ssize_t result = block_cache.write(fd, buf, count, current_offset);
    if (result < 0) {
        cerr << "Error writing to cache\n";
        return -1;
    }
    file_offsets[fd] += result;
    if (lab2_fsync(fd) < 0) {
        cerr << "Error syncing file after write\n";
        return -1;
    }
    return result;
}

off_t lab2_lseek(int fd, off_t offset, int whence) {
    auto hFile = (HANDLE)_get_osfhandle(fd);
    if (hFile == INVALID_HANDLE_VALUE) {
        cerr << "Invalid file descriptor\n";
        return -1;
    }

    LARGE_INTEGER liOffset, newPointer;
    liOffset.QuadPart = offset;
    DWORD moveMethod;

    switch (whence) {
        case SEEK_SET: moveMethod = FILE_BEGIN; break;
        case SEEK_CUR: moveMethod = FILE_CURRENT; break;
        case SEEK_END: {
            LARGE_INTEGER fileSize;
            if (!GetFileSizeEx(hFile, &fileSize)) {
                cerr << "Error retrieving file size: " << GetLastError() << '\n';
                return -1;
            }
            liOffset.QuadPart = fileSize.QuadPart + offset;
            if (liOffset.QuadPart < 0) {
                cerr << "Error: Negative file pointer position\n";
                return -1;
            }
            moveMethod = FILE_BEGIN;
            break;
        }
        default:
            cerr << "Invalid whence value\n";
            return -1;
    }
    liOffset.QuadPart &= ~(SECTOR_SIZE - 1);

    if (!SetFilePointerEx(hFile, liOffset, &newPointer, moveMethod)) {
        cerr << "Error setting file pointer: " << GetLastError() << '\n';
        return -1;
    }

    file_offsets[fd] = static_cast<off_t>(newPointer.QuadPart);
    return file_offsets[fd];
}

int lab2_fsync(int fd) {
    return block_cache.sync(fd);
}
