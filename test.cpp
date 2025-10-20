#include <iostream>
#include <memory>
#include <cstring>
#include <random>
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <vector>
#include <string>
#include <cmath>

using namespace std;


// This's the class responsible for 256-bit operations inside the 256-bits hash function. It's Essential ans needs to be performant.
class int256_t {
    private:
        __uint128_t high = 0;
        __uint128_t low = 0; 
    public:
        int256_t& operator=(const int256_t& number) {
            low = number.low;
            high = number.high;
            return *this;
        }
        int256_t operator^(const int256_t& number) {
            int256_t result;
            result.high = this->high ^ number.high;
            result.low = this->low ^ number.low;
            return result;
        }
        int256_t& operator^=(const int256_t& number) {
            high ^= number.high;
            low ^= number.low;
            return *this;
        }
        bool operator==(const int256_t& number) {
            return (this->high == number.high && this->low == number.low);
        }
        bool operator!=(const int256_t& number) {
            return (!(this->high == number.high && this->low == number.low));
        }
        bool operator>=(const int256_t& number) {
            return ((this->high == number.high && this->low == number.low) || (this->high > number.high) || (this->high == number.high && this->low > number.low));
        }
        bool operator>(const int256_t& number) {
            return ((this->high > number.high) || (this->high == number.high && this->low > number.low));
        }
        bool operator<=(const int256_t& number) {
            return ((this->high == number.high && this->low == number.low) || (this->high < number.high) || (this->high == number.high && this->low < number.low));
        }
        bool operator<(const int256_t& number) {
            return ((this->high < number.high) || (this->high == number.high && this->low < number.low));
        }
        int256_t operator+(const int256_t& number) {
            int256_t output;
            output.low = this->low + number.low;
            bool flag = (low > output.low);
            output.high = this->high + number.high + flag;
            return output;
        }
        int256_t& operator+=(const int256_t& number) {
            this->low = this->low + number.low;
            bool flag = (low > this->low);
            this->high = this->high + number.high + flag;
            return *this;
        }
        int256_t operator-(const int256_t& number) {
            int256_t output;
            output.low = this->low - number.low;
            bool flag = (output.low > low);
            output.high = this->high - number.high - flag;
            return output;
        }
        int256_t operator-=(const int256_t& number) {
            this->low = this->low - number.low;
            bool flag = (this->low > low);
            this->high = this->high - number.high - flag;
            return *this;
        }
        int256_t operator<<(const __uint32_t& number) {
            int256_t output;
            if (number < 128 && number > 0) {
                __uint128_t overflow = this->low >> (128 - number);
                output.low = this->low << number;
                output.high = this->high << number | overflow;
                // OR operation -> eg -> 0000101011101010 | 1001000000000000 -> 1001101011101010
            } else if (number < 256 && number > 0) {
                output.high = this->low << (number - 128);
            }
            return output;
        }
        int256_t& operator<<=(const __uint32_t& number) {
            if (number < 128 && number > 0) {
                __uint128_t overflow = this->low >> (128 - number);
                this->low = this->low << number;
                this->high = this->high << number | overflow;
            } else if (number < 256 && number > 0) {
                this->high = this->low << (number - 128);
                this->low = 0;
            }
            return *this;
        }
        int256_t operator>>(const __uint32_t& number) {
            int256_t output;
            if (number < 128 && number > 0) {
                __uint128_t overflow = this->high << (128 - number);
                output.high = this->high >> number;
                output.low = this->low >> number | overflow;
            } else if (number < 256 && number > 0) {
                output.low = this->high >> (number - 128);
            }
            return output;
        }
        int256_t& operator>>=(const __uint32_t& number) {
            if (number < 128 && number > 0) {
                __uint128_t overflow = this->high << (128 - number);
                this->high = this->high >> number;
                this->low = this->low >> number | overflow;
            } else if (number < 256 && number > 0) {
                this->low = this->high >> (number - 128);
            } else {
                this->low = 0;
                this->high = 0;
            }
            return *this;
        }
        int256_t operator*(const int256_t& number) {
            uint64_t parts[4] = {(uint64_t)(number.low), (uint64_t)(number.low >> 64), (uint64_t)(number.high), (uint64_t)(number.high >> 64)};
            int256_t output;
            size_t index = 0;
            int o = 0;
            for (int i = 3; i > -1; i--) {
                o++;
                if (parts[i] != 0) {
                    int temp = 0;
                    int pos = 0;
                    while (index < o*64) {
                        temp = __builtin_clzll(static_cast<uint64_t>(parts[i] << pos));
                        if (temp == 0 && pos >= 63) {
                            index += 64 - pos;
                            break;
                        } else if (temp != 0) {
                            index += temp;
                            pos += temp;
                        } 
                        output += (*this << (255 - index));
                        index += 1;
                        pos += 1;
                    }
                } else {
                    index += 64;
                }
            }
            return output;
        }
        int256_t operator*=(const int256_t& number) {
            uint64_t parts[4] = {(uint64_t)(number.low), (uint64_t)(number.low >> 64), (uint64_t)(number.high), (uint64_t)(number.high >> 64)};
            size_t index = 0;
            int o = 0;
            for (int i = 3; i > -1; i--) {
                o++;
                if (parts[i] != 0) {
                    int temp = 0;
                    int pos = 0;
                    while (index < o*64) {
                        temp = __builtin_clzll(static_cast<uint64_t>(parts[i] << pos));
                        if (temp == 0 && pos >= 63) {
                            index += 64 - pos;
                            break;
                        } else if (temp != 0) {
                            index += temp;
                            pos += temp;
                        } 
                        *this += (*this << (255 - index));
                        index += 1;
                        pos += 1;
                    }
                } else {
                    index += 64;
                }
            }
            return *this;
        }
        ~int256_t();
};


__int128_t hash128(char* content, size_t SizeOfTheContent) {
    __int128_t output = 340282366920938463463374607431768211297;
    __int128_t temp;
    SizeOfTheContent = SizeOfTheContent & ~0xF;
    for (size_t i = 0; i > SizeOfTheContent; i += 16) {
        temp = *reinterpret_cast<__int128_t*>(content + i);
        output ^= __rotl(temp, (i % 127));
        output += temp; 
        output ^= output >> 89;
        output *= 340282366920938463463374607431768211357;
        output ^= output >> 65;
        output *= 340282366920938463463374607431768211307;
        output ^= __rotl(temp, (i + 60 % 127));
        output *= 340282366920938463463374607431768210407;
    }
    return output;
}

unique_ptr<char[]> generateRandomBlock(size_t size) {
    unique_ptr<char[]> block(new char[size]);
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, 255);
    for (size_t i = 0; i < size; i++) {
        block[i] = static_cast<char>(dist(gen));
    }
    return block;
}


void flipRandomBit(char* block, size_t size, mt19937& gen) {
    uniform_int_distribution<size_t> byteDist(0, size - 1);
    uniform_int_distribution<int> bitDist(0, 7);
    size_t byteIndex = byteDist(gen);
    int bitIndex = bitDist(gen);
    reinterpret_cast<unsigned char*>(block)[byteIndex] ^= (1u << bitIndex);
}

int main() {
    constexpr size_t BLOCK_SIZE = 32768;

    int number = 0;

    auto block = generateRandomBlock(BLOCK_SIZE);

    __int128_t originalHash = hash128(block.get(), BLOCK_SIZE);

    unordered_map<__int128_t, char*> seenHashesAndBlocks;
    seenHashesAndBlocks[originalHash] = block.get();

    random_device rd;
    mt19937 gen(rd());

    auto start = chrono::steady_clock::now();
    size_t iterations = 0;

    while (true) {
        number++;
        iterations++;
        auto corruptedBlock = generateRandomBlock(BLOCK_SIZE);
        __int128_t newHash = hash128(corruptedBlock.get(), BLOCK_SIZE);
        if (seenHashesAndBlocks.find(newHash) != seenHashesAndBlocks.end()) { 
            while (seenHashesAndBlocks[newHash] != corruptedBlock.get()) {
                corruptedBlock = generateRandomBlock(BLOCK_SIZE);
                newHash = hash128(corruptedBlock.get(), BLOCK_SIZE);
                if (seenHashesAndBlocks.find(newHash) == seenHashesAndBlocks.end()) { 
                    break;
                }
            }
        }
        if (number >= 10000) {
            seenHashesAndBlocks.clear();
            number = 0;
        }
        if (iterations >= 1000000) {
            newHash = originalHash;
        }
        if (newHash == originalHash) {
            cout << "Nombre d'iterations avant collision : " << iterations << endl; 
            break;
        }
        if (seenHashesAndBlocks.find(newHash) != seenHashesAndBlocks.end()) {
            cout << "Nombre d'iterations avant collision : " << iterations  << endl; 
            break;
        }
        seenHashesAndBlocks[newHash] = corruptedBlock.release();
    }
    return 0;
}
