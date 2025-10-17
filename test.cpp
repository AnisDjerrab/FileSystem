#include <iostream>
#include <memory>
#include <cstring>
#include <random>
#include <unordered_set>
#include <chrono>
#include <vector>

using namespace std;

// ======================================================
// Fonction utilitaire : convertir un uint32_t en binaire (32 bits)
// ======================================================
string ConvertNumberInto32BitsString(int32_t number) {
    string result(32, '0');
    for (int i = 0; i < 32; i++) {
        if (number & (1 << (31 - i))) {
            result[i] = '1';
        }
    }
    return result;
}

// ======================================================
// Fonction de hash (variante de la tienne)
// ======================================================
unique_ptr<char[]> hash32(char* content, size_t SizeOfTheContent) {
    int32_t output = 2246822519u;
    int index = 0;
    int32_t addition = 0;
    int32_t XOR = 0;
    int32_t Multiplication = 3432918353u;
    int32_t temp = 4;
    for (size_t i = 0; i < SizeOfTheContent; i += 4) {
        memcpy(&temp, content + i, 4);
        Multiplication *= temp;
        addition += temp;
        XOR ^= temp;
    }
    output *= Multiplication;
    output ^= XOR;
    output += addition;
    auto output_Final = make_unique<char[]>(4);
    memcpy(output_Final.get(), &output, 4);
    return output_Final;
}

// ======================================================
// Génération d’un bloc aléatoire de 512 octets
// ======================================================
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

// ======================================================
// Flip d’un bit aléatoire dans le bloc (corrigée)
// ======================================================
void flipRandomBit(char* block, size_t size, mt19937& gen) {
    uniform_int_distribution<size_t> byteDist(0, size - 1);
    uniform_int_distribution<int> bitDist(0, 7);
    size_t byteIndex = byteDist(gen);
    int bitIndex = bitDist(gen);
    // utiliser unsigned char pour éviter les comportements indéfinis sur signed char
    reinterpret_cast<unsigned char*>(block)[byteIndex] ^= (1u << bitIndex);

    // --- Pour debug, on peut afficher l'emplacement modifié (optionnel)
    // cout << "flipRandomBit: byteIndex=" << byteIndex << " bitIndex=" << bitIndex << endl;
}


// --- convertit un hash (4 octets) en string
string toStringFromHash(unique_ptr<char[]> hashPtr) {
    uint32_t val;
    memcpy(&val, hashPtr.get(), 4);
    char buf[9];
    snprintf(buf, sizeof(buf), "%08x", val);
    return string(buf);
}


int main() {
    constexpr size_t BLOCK_SIZE = 128;
    constexpr int MAX_SECONDS = 5;  // durée max du test

    double number = 0;
    int counter = 0;

    auto block = generateRandomBlock(BLOCK_SIZE);

    auto initialCopy = make_unique<char[]>(BLOCK_SIZE);
    memcpy(initialCopy.get(), block.get(), BLOCK_SIZE);
    auto tempInitialCopy = make_unique<char[]>(BLOCK_SIZE);
    memcpy(tempInitialCopy.get(), block.get(), BLOCK_SIZE);
    string originalHash = toStringFromHash(hash32(tempInitialCopy.get(), BLOCK_SIZE));
    unordered_set<string> seenHashes;
    seenHashes.insert(originalHash);

    random_device rd;
    mt19937 gen(rd());

    auto start = chrono::steady_clock::now();
    size_t iterations = 0;

    while (true) {
        iterations++;

        auto corruptedBlock = make_unique<char[]>(BLOCK_SIZE);
        memcpy(corruptedBlock.get(), block.get(), BLOCK_SIZE);

        flipRandomBit(corruptedBlock.get(), BLOCK_SIZE, gen);
        auto tempCorruptedBlock = make_unique<char[]>(BLOCK_SIZE);
        memcpy(tempCorruptedBlock.get(), corruptedBlock.get(), BLOCK_SIZE);
        string newHash = toStringFromHash(hash32(tempCorruptedBlock.get(), BLOCK_SIZE));
        if (newHash == originalHash) {
            counter += 1;
            number += iterations;
            iterations = 0;
            if (counter >= 10) {
                cout << "Moyenne de nombre d'iterations avant collision : " << number / 10 << endl; 
                break;
            }
        }
        if (seenHashes.find(newHash) != seenHashes.end()) {
            counter += 1;
            number += iterations;
            iterations = 0;
            if (counter >= 10) {
                cout << "Moyenne de nombre d'iterations avant collision : " << number / 10 << endl; 
                break;
            }
        }
        seenHashes.insert(newHash);
    }
    return 0;
}
