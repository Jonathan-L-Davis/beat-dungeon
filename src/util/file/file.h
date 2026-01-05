#ifndef FILE_H
#define FILE_H

#include <string>
#include <vector>

std::vector<uint8_t> load_file(std::string file);

bool load_file(std::string file, const std::vector<uint8_t>& data);

#endif//FILE_H
