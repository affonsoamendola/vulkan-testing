#pragma once

#include <vector>
#include <fstream>
#include <iostream>
#include <string>

//Dumps all bytes from a file into a vector.
static std::vector<char> read_file(const std::string& filename) 
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) 
    {
        throw std::runtime_error("Failed to open file!");
    }

    size_t file_size = (size_t) file.tellg();
    std::vector<char> buffer(file_size);
    
    file.seekg(0);
    file.read(buffer.data(), file_size);

    file.close();

    return buffer;
}

static float radians(float degree)
{
    return(degree * M_PI / 180.0f);
}