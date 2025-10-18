#include <iostream>
#include <memory>
#include <cstring>
#include <random>
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <vector>
#include <string>

using namespace std;


string ConvertNumberInto32BitsString(__int128_t number) {
    string result(32, '0');
    for (int i = 0; i < 32; i++) {
        if (number & (1 << (31 - i))) {
            result[i] = '1';
        }
    }
    return result;
}


__int128_t hash128(char* content, size_t SizeOfTheContent) {
    __int128_t output = 340282366920938463463374607431768211297;
    __int128_t temp;
    SizeOfTheContent = SizeOfTheContent & ~0xF;
    for (size_t i = 0; i < SizeOfTheContent; i += 16) {
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
