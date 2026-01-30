#ifndef FILE_H
#define FILE_H

#include <string>
#include <vector>

std::vector<uint8_t> load_file(std::string file);

bool load_file(std::string file,  std::vector<uint8_t>& data);
bool save_file(std::string file_path, const std::vector<uint8_t>& data);

void append_to_buffer(std::vector<uint8_t>& file, uint8_t x);
void append_to_buffer(std::vector<uint8_t>& file, uint16_t x);
void append_to_buffer(std::vector<uint8_t>& file, uint32_t x);
void append_to_buffer(std::vector<uint8_t>& file, uint64_t x);

void read_from_buffer(std::vector<uint8_t> file, int& i, uint8_t& x);
void read_from_buffer(std::vector<uint8_t> file, int& i, uint16_t& x);
void read_from_buffer(std::vector<uint8_t> file, int& i, uint32_t& x);
void read_from_buffer(std::vector<uint8_t> file, int& i, uint64_t& x);

#endif//FILE_H
