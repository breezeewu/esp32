#pragma once

const char* lbstring_read_line(const char* pstr, char* line, int line_len);

const char* lbstring_read_tag(const char* pstr, const char* splt, char* tag, int tag_size);

// version compare, > return 1, == return 0, < return -1
int lbstring_version_compare(const char* pver1, const char* pver2);
