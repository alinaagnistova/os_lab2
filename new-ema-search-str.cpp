#include "ema-search-str.h"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include "library.h"

using namespace std;
enum {
    NoOfChars = 256
};

void bad_char_heuristic(const string &pattern, vector<int> &bad_char) {
    int const length = pattern.length();
    for (int i = 0; i < length; i++) {
        bad_char[static_cast<int>(pattern[i])] = i;
    }
}

void boyer_moor_search(const string &txt, const string &pattern) {
    int const pattern_length = pattern.length();
    int const txt_length = txt.length();
    vector<int> bad_char(NoOfChars, -1);
    bad_char_heuristic(pattern, bad_char);
    int shift = 0;
    while (shift <= txt_length - pattern_length) {
        int idx = pattern_length - 1;
        while (idx >= 0 && pattern[idx] == txt[shift + idx]) {
            idx--;
        }
        if (idx < 0) {
            cout << "Pattern occurs at shift =  " << shift << '\n';
            shift += (shift + pattern_length < txt_length) ? pattern_length : 1;
        } else {
            shift += max(1, idx - bad_char[static_cast<int>(txt[shift + idx])]);
        }

    }

}

void read_file_and_search_with_custom_cache(const string &path_to_file, const string &pattern) {
    const size_t buffer_size = 4096;
    vector<char> buffer(buffer_size);
    string prev_line;

    int fd = lab2_open(path_to_file.c_str());
    if (fd < 0) {
        cerr << "Error opening file" << '\n';
        return;
    }

    ssize_t bytes_read;

    while ((bytes_read = lab2_read(fd, buffer.data(), buffer_size)) > 0) {
        string current_data(buffer.data(), bytes_read);

        current_data = prev_line + current_data;

        size_t last_newline_pos = current_data.rfind('\n');
        if (last_newline_pos != string::npos) {
            string process_buffer = current_data.substr(0, last_newline_pos + 1);

            prev_line = current_data.substr(last_newline_pos + 1);

            boyer_moor_search(process_buffer, pattern);
        } else {
            prev_line = current_data;
        }
    }

    if (!prev_line.empty()) {
        boyer_moor_search(prev_line, pattern);
    }
    if (lab2_close(fd) < 0) {
        cerr << "Error closing file" << '\n';
    } else {
        cout << "File successfully closed" << '\n';
    }
}

void ema_search_str_with_cache(const string &path_to_file, const string &pattern, int repeat_count) {
    auto start_time = chrono::high_resolution_clock::now();
    for (int i = 0; i < repeat_count; ++i) {
        read_file_and_search_with_custom_cache(path_to_file, pattern);
    }

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> const elapsed = end_time - start_time;
    cout << "Total execution time for " << repeat_count << " repetitions: " << elapsed.count() << " seconds\n";
}

int main() {
    string test_file = "C:/Users/agnis/CLionProjects/os-lab2/example.txt";
    string search_pattern = "apple";
    int repeat_count = 100;
    cout << "Starting search...\n";
    ema_search_str_with_cache(test_file, search_pattern, repeat_count);

    return 0;
}