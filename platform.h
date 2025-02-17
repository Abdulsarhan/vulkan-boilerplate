#ifndef PLATFORM_H
#define PLATFORM_H

void platform_get_window_size(uint32_t* width, uint32_t* height);

char* platform_read_file(const char* file, uint32_t* length);
#endif //PLATFORM_H