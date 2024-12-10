#include <fstream>
#include <iostream>
#include <string>
#include <random>

using namespace std;

void generate_test_file(const string &filename, size_t file_size, size_t line_length, const string &pattern) {
    ofstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Error: Unable to create file: " << filename << '\n';
        return;
    }

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<char> dis('a', 'z');

    size_t written_size = 0;
    while (written_size < file_size) {
        string line;

        for (size_t i = 0; i < line_length; ++i) {
            line += dis(gen);
        }

        if (gen() % 5 == 0) {
            size_t insert_pos = gen() % line_length;
            line.replace(insert_pos, pattern.size(), pattern);
        }

        line += '\n';
        file.write(line.c_str(), line.size());
        written_size += line.size();
    }

    file.close();
    cout << "Test file created: " << filename << '\n';
}

int main() {
    string filename = "C:/Users/agnis/CLionProjects/os-lab2/testfile2.txt";
    size_t file_size = 100 * 1024 * 1024;
    size_t line_length = 100;
    string pattern = "searchme";

    generate_test_file(filename, file_size, line_length, pattern);
    return 0;
}
