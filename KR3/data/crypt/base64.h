#pragma once

#include <stdint.h>

size_t encode64(char * dest, size_t limit, const void * src, size_t srclen);
size_t decode64(void * dest, size_t limit, const char * src, size_t srclen);
size_t encode64(char * dest, const void * src, size_t srclen);
size_t decode64(void * dest, const char * src, size_t srclen);

size_t decode64_len(const char * src, size_t srclen);
