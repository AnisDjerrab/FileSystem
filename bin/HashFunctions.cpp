#include <iostream>

using namespace std;

__attribute__((visibility("default")))
int32_t hash32(char* content, size_t SizeOfTheContent);
int32_t hash32(char* content, size_t SizeOfTheContent) {
    int32_t output = 2246822519u;
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
    return output;
}
__attribute__((visibility("default")))
int64_t hash64(char* content, size_t SizeOfTheContent);
int64_t hash64(char* content, size_t SizeOfTheContent) {
    int64_t output = 8895235923790041094;
    int64_t temp;
    SizeOfTheContent = SizeOfTheContent & ~0x7;
    for (size_t i = 0; i < SizeOfTheContent; i += 8) {
        temp = *reinterpret_cast<int64_t*>(content + i);
        output ^= __rotl(temp, (i % 31));
        output += temp; 
        output ^= output >> 44;
        output *= 9223372036854775783;
        output ^= output >> 33;
        output *= 9223372036854775657;
        output ^= __rotl(temp, (i + 30 % 63));
        output *= 2870177450012600261;
    }
    return output;
}
