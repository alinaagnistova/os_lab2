#ifndef OS_LAB2_EMA_SEARCH_STR_H
#define OS_LAB2_EMA_SEARCH_STR_H
#include <string>
#include <vector>

using namespace std;

void bad_char_heuristic(const string &pattern, vector<int> &bad_char);
void boyer_moor_search(const string &txt, const string &pattern);
void read_file_and_search(const string &path_to_file,const string &pattern);
void read_file_and_search_with_custom_cache(const string &path_to_file,const string &pattern);
void ema_search_str(const string &path_to_file, const string &pattern, int repeat_count);
void ema_search_str_with_cache(const string &path_to_file, const string &pattern, int repeat_count);


#endif //OS_LAB2_EMA_SEARCH_STR_H
