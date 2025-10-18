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


string ConvertNumberInto32BitsString(int64_t number) {
    string result(32, '0');
    for (int i = 0; i < 32; i++) {
        if (number & (1 << (31 - i))) {
            result[i] = '1';
        }
    }
    return result;
}


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
    constexpr size_t BLOCK_SIZE = 2048;

    int number = 0;

    auto block = generateRandomBlock(BLOCK_SIZE);

    int64_t originalHash = hash64(block.get(), BLOCK_SIZE);

    unordered_map<int64_t, char*> seenHashesAndBlocks;
    seenHashesAndBlocks[originalHash] = block.get();

    random_device rd;
    mt19937 gen(rd());

    auto start = chrono::steady_clock::now();
    size_t iterations = 0;

    while (true) {
        number++;
        iterations++;
        auto corruptedBlock = generateRandomBlock(BLOCK_SIZE);
        int64_t newHash = hash64(corruptedBlock.get(), BLOCK_SIZE);
        if (seenHashesAndBlocks.find(newHash) != seenHashesAndBlocks.end()) { 
            while (seenHashesAndBlocks[newHash] != corruptedBlock.get()) {
                corruptedBlock = generateRandomBlock(BLOCK_SIZE);
                newHash = hash64(corruptedBlock.get(), BLOCK_SIZE);
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
