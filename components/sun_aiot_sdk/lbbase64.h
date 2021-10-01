#pragma once

int lbbase64_decode(const char* in_str, unsigned char* out, int out_size);

char* lbbase64_encode(char* out, int out_size, const unsigned char* in, int in_size);