#include <iostream>
#include "bin/Dynamic_Functions.h"

using namespace std;

__attribute__((visibility("default")))
int32_t hash32(char* content, size_t SizeOfTheContent);
int32_t hash32(char* content, size_t SizeOfTheContent) {
    int32_t output = 2246822519u;
    int32_t addition = 0;
    int32_t temp;
    SizeOfTheContent = SizeOfTheContent & ~0x3;
    for (size_t i = 0; i < SizeOfTheContent; i += 4) {
        temp = *reinterpret_cast<int32_t*>(content + i);
        output ^= __rotl(temp, (i % 31));
        output += temp; 
        output ^= output >> 16;
        output *= 3266489917;
        output ^= output >> 13;
        output *= 2654435761;
        output ^= __rotl(temp, (i + 15 % 31));
        output *= 668265263;
    }
    output += addition;
    return output;
}