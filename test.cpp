#include <iostream>
#include <memory>
#include <cstring>
#include <random>
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <vector>

using namespace std;


string ConvertNumberInto32BitsString(int32_t number) {
    string result(32, '0');
    for (int i = 0; i < 32; i++) {
        if (number & (1 << (31 - i))) {
            result[i] = '1';
        }
    }
    return result;
}


int32_t hash32(char* content, size_t SizeOfTheContent) {
    int32_t output = 2246822519u;
    int index = 0;
    int32_t addition = 0;
    int32_t temp = 4;
    for (size_t i = 0; i < SizeOfTheContent; i += 4) {
        memcpy(&temp, content + i, 4);
        addition += temp ^ 374761393;
        int32_t mix = temp * 2246822519 ^ (output);
        mix += (mix ^ __rotl(mix, ((i + 11) % 31))) ^ (mix >> 15 * mix >> 23);
        output += mix;
        output ^= __rotl(temp, (i % 31));
        output += temp; 
        output ^= output >> 16;
        output *= 3266489917;
        output ^= output >> 13;
        output *= 2654435761;
        output ^= output >> 16;
        output ^= __rotl(temp, (i + 15 % 31));
        output *= 668265263;
    }
    output += addition;
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


string toStringFromHash(unique_ptr<char[]> hashPtr) {
    uint32_t val;
    memcpy(&val, hashPtr.get(), 4);
    char buf[9];
    snprintf(buf, sizeof(buf), "%08x", val);
    return string(buf);
}


int main() {
    constexpr size_t BLOCK_SIZE = 128;
    constexpr int MAX_SECONDS = 5; 

    auto block = generateRandomBlock(BLOCK_SIZE);

    int32_t originalHash = hash32(block.get(), BLOCK_SIZE);
    unordered_map<int32_t, char*> seenHashesAndBlocks;
    seenHashesAndBlocks[originalHash] = block.get();

    random_device rd;
    mt19937 gen(rd());

    auto start = chrono::steady_clock::now();
    size_t iterations = 0;

    while (true) {
        iterations++;

        auto corruptedBlock = make_unique<char[]>(BLOCK_SIZE);
        memcpy(corruptedBlock.get(), block.get(), BLOCK_SIZE);
        flipRandomBit(corruptedBlock.get(), BLOCK_SIZE, gen);
        int32_t newHash = hash32(corruptedBlock.get(), BLOCK_SIZE);
        if (seenHashesAndBlocks.find(newHash) != seenHashesAndBlocks.end()) { 
            while (strcmp(seenHashesAndBlocks[newHash], corruptedBlock.get())) {
                memcpy(corruptedBlock.get(), block.get(), BLOCK_SIZE);
                flipRandomBit(corruptedBlock.get(), BLOCK_SIZE, gen);
                newHash = hash32(corruptedBlock.get(), BLOCK_SIZE);
                cout << "corrupted block" << endl;
            }
        }
        if (newHash == originalHash) {
            cout << "Nombre d'iterations avant collision : " << iterations << endl; 
            break;
        }
        if (seenHashesAndBlocks.find(newHash) != seenHashesAndBlocks.end()) {
            cout << "Nombre d'iterations avant collision : " << iterations  << endl; 
            break;
        }
        seenHashesAndBlocks[newHash] = corruptedBlock.get();
    }
    return 0;
}
