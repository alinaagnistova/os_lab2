#include "library.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
using namespace std;

void test_lab2_functions() {
    const char *filename = "C:/Users/agnis/CLionProjects/os-lab2/testfile.txt";
    const size_t buffer_size = 512;
    vector<char> buffer(buffer_size, 'A');

    cout << "--------------------Testing lab2_open---------------------" << '\n';
    int fd = lab2_open(filename);
    if (fd < 0) {
        cerr << "lab2_open failed" << strerror(errno) << '\n';
        return;
    }
    cout << "lab2_open passed.\n";

    cout << "--------------------Testing lab2_write---------------------" << '\n';
    ssize_t written = lab2_write(fd, buffer.data(), buffer_size);
    if (written < 0) {
        cerr << "lab2_write failed.\n";
        lab2_close(fd);
        return;
    }
    cout << "lab2_write passed. Bytes written: " << written << '\n';

    cout << "--------------------Testing lab2_read---------------------" << '\n';
    vector<char> read_buffer(buffer_size, '\0');
    ssize_t read_bytes = lab2_read(fd, read_buffer.data(), buffer_size);
    if (read_bytes < 0) {
        cerr << "lab2_read failed.\n";
        lab2_close(fd);
        return;
    }
    cout << "lab2_read passed. Bytes read: " << read_bytes << '\n';

    cout << "--------------------Checking data integrity---------------------" << '\n';
    cout << "Written data: ";
    for (size_t i = 0; i < buffer_size; ++i) {
        cout << hex << (int)(unsigned char)buffer[i] << " ";
    }
    cout << "\n";

    cout << "Read data: ";
    for (size_t i = 0; i < buffer_size; ++i) {
        cout << hex << (int)(unsigned char)read_buffer[i] << " ";
    }
    cout << "\n";

    if (memcmp(buffer.data(), read_buffer.data(), buffer_size) == 0) {
        cout << "Data integrity check passed.\n";
    } else {
        cerr << "Data integrity check failed.\n";
    }
    cout << "--------------------Testing lab2_lseek---------------------" << '\n';
    off_t offset = 1024;
    int whence = SEEK_SET;
    cout << "Testing lab2_lseek (SEEK_SET) with offset: " << offset << ", whence: " << whence << '\n';
    off_t result = lab2_lseek(fd, offset, whence);
    if (result < 0) {
        cerr << "lab2_lseek (SEEK_SET) failed with offset: " << offset << ", whence: " << whence << '\n';
        lab2_close(fd);
        return;
    }
    cout << "lab2_lseek passed (SEEK_SET). New offset: " << result << '\n';

    offset = 512;
    whence = SEEK_CUR;
    cout << "Testing lab2_lseek (SEEK_CUR) with offset: " << offset << ", whence: " << whence << '\n';
    result = lab2_lseek(fd, offset, whence);
    if (result < 0) {
        cerr << "lab2_lseek (SEEK_CUR) failed with offset: " << offset << ", whence: " << whence << '\n';
        lab2_close(fd);
        return;
    }
    cout << "lab2_lseek (SEEK_CUR) passed. New offset: " << result << '\n';
    offset = -1512;
    whence = SEEK_END;
    cout << "Testing lab2_lseek (SEEK_END) with offset: " << offset << ", whence: " << whence << '\n';
    result = lab2_lseek(fd, offset, whence);
    if (result < 0) {
        cerr << "lab2_lseek (SEEK_END) failed with offset: " << offset << ", whence: " << whence << '\n';
        lab2_close(fd);
        return;
    }
    cout << "lab2_lseek (SEEK_END) passed. New offset: " << result << '\n';

    cout << "--------------------Testing lab2_fsync---------------------" << '\n';
    if (lab2_fsync(fd) == 0) {
        cout << "lab2_fsync passed.\n";
    } else {
        cerr << "lab2_fsync failed.\n";
    }

    cout << "--------------------Testing lab2_close---------------------" << '\n';
    if (lab2_close(fd) == 0) {
        cout << "lab2_close passed.\n";
    } else {
        cerr << "lab2_close failed.\n";
    }
}

int main() {
    cout << "Starting tests...\n";
    test_lab2_functions();
    cout << "Tests completed.\n";
    return 0;
}
