#include "ema-search-str.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>

using namespace std;


//ema-search-str поиск подстроки в тексте во внешней памяти
//По алгоритму Бойера-Мура
enum {
    NoOfChars = 256
};//кол-во возможных символов (256 для ASCII)

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

//Добавление буфера для выполнения примечания для ema-
void read_file_and_search(const string &path_to_file, const string &pattern) {
    const auto buffer_size = static_cast<const size_t>(32 * 1024 * 1024); // 32 MiB
    string buffer;
    string prev_line;
    ifstream file(path_to_file);
    if (!file) {
        cerr << "Error opening file" << '\n';
        return;
    }
    while (file) {
        string line;
        while (buffer.size() < buffer_size && getline(file, line)) {
            buffer += prev_line + line + '\n';
            prev_line = "";
        }
        if (buffer.size() >= buffer_size || !file) {
            boyer_moor_search(buffer, pattern);
            buffer.clear();
        }
        if (!line.empty()) {
            prev_line = line;
        }
    }
    if (!buffer.empty()) {
        boyer_moor_search(buffer, pattern);
    }
    file.close();

}

void ema_search_str(const string &path_to_file, const string &pattern, int repeat_count) {
    auto start_time = chrono::high_resolution_clock::now();
    for (int i = 0; i < repeat_count; ++i) {
        read_file_and_search(path_to_file, pattern);
    }

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> const elapsed = end_time - start_time;
    cout << "Total execution time for " << repeat_count << " repetitions: " << elapsed.count() << " seconds\n";
}