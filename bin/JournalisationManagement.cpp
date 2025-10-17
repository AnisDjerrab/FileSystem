#include <iostream>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <cstring>
#include <memory>
#include "bin/Dynamic_Functions.h"

const static int SizeOfTheBlock = 256;

using namespace std;

__attribute__((visibility("default")))
void WriteInJournal(unordered_map<int, int>& freeSegments, unordered_map<int, int>& freeBlocks, int& SizeOfTheFile, fstream& Write, unordered_map<int, int>& IndexationSegments, vector<int>& indexes, unique_ptr<char[]> content, int len) __asm__("WriteInJournal");
void WriteInJournal(unordered_map<int, int>& freeSegments, unordered_map<int, int>& freeBlocks, int& SizeOfTheFile, fstream& Write, unordered_map<int, int>& IndexationSegments, vector<int>& indexes, unique_ptr<char[]> content, int len) {
    int Size = 0;
    for (const auto& pair : freeSegments) {
        Size += pair.second - pair.first;
    }
    if (!(Size > len)) {
        int necessarySize = (len - Size) / (SizeOfTheBlock - 8);
        int SizeOfTheIndexationBlocks = 0;
        for (const auto& pair : IndexationSegments) {
            SizeOfTheIndexationBlocks += pair.second - pair.first;
        }
        vector<int> indexOfTheNewIndexationBlocksCreated = Dynamic_Functions::listFreeBlocks(false, false, freeBlocks, freeSegments, IndexationSegments, indexes, SizeOfTheFile, Write, (necessarySize * 16 + 64) / (SizeOfTheBlock - 8));
        for (const auto& indexOfTheNewIndexationBlockCreated : indexOfTheNewIndexationBlocksCreated) {
            bitset<960> bits(0);
            auto buffer = make_unique<char[]>(SizeOfTheBlock - 8);
            for (int i = 0; i < SizeOfTheBlock - 8; i++) {
                buffer[i] = static_cast<char>((bits >> (i*8)).to_ulong() & 0xFF);
            }
            Write.seekp(indexOfTheNewIndexationBlockCreated , ios::beg);
            Write.write(Dynamic_Functions::ConvertNumberInto32BitsString(-2).get(), 8);
            Write.seekp(indexOfTheNewIndexationBlockCreated + 4, ios::beg);
            Write.write(buffer.get(), SizeOfTheBlock - 8);
            Write.seekp(indexOfTheNewIndexationBlockCreated + SizeOfTheBlock - 4, ios::beg);
            Write.write(Dynamic_Functions::ConvertNumberInto32BitsString(-2).get(), 8);
            for (const auto& pair : IndexationSegments) {
                if (pair.first - pair.second >= 16) {
                    auto buffer = make_unique<char[]>(16);
                    memcpy(buffer.get(), Dynamic_Functions::ConvertNumberInto64BitsString(-450).get(), 8);
                    memcpy(buffer.get() + 8, Dynamic_Functions::ConvertNumberInto64BitsString(indexOfTheNewIndexationBlockCreated).get(), 8);
                    Write.seekp(pair.second - 16, ios::beg);
                    Write.write(buffer.get(), 16);
                    IndexationSegments[pair.first] = pair.second - 16;
                }
            }
        }
        vector<int> Blocks = Dynamic_Functions::listFreeBlocks(false, false, freeBlocks, freeSegments, IndexationSegments, indexes, SizeOfTheFile, Write, necessarySize);
        for (int i = 0; i < Blocks.size(); i++) {
            freeSegments[Blocks[i] + 4] = SizeOfTheBlock - 8;
            for (const auto& pair : IndexationSegments) {
                if (pair.second - pair.first >= 16) {
                    Write.seekp(pair.second - 16);
                    Write.write(Dynamic_Functions::ConvertNumberInto64BitsString(-5).get(), 8);
                    Write.seekp(pair.second - 8);
                    Write.write(Dynamic_Functions::ConvertNumberInto64BitsString(Blocks[i]).get(), 8);
                    if (pair.first == pair.second - 16) {
                        IndexationSegments.erase(pair.first);
                        break;
                    } else {
                        IndexationSegments[pair.first] = pair.second - 16;
                    }
                }
            }
            Write.seekp(Blocks[i], ios::beg);
            Write.write(Dynamic_Functions::ConvertNumberInto32BitsString(-5).get(), 4);
            Write.seekp(Blocks[i] + SizeOfTheBlock - 4, ios::beg);
            Write.write(Dynamic_Functions::ConvertNumberInto32BitsString(-5).get(), 4);
        }
    }
    int indexInTheContent = 0;
    for (const auto& pair : freeSegments) {
        if (indexInTheContent + pair.second - pair.first >= len) {
            auto buffer = make_unique<char[]>(pair.second - pair.first);
            memcpy(content.get() + indexInTheContent, buffer.get(), pair.second - pair.first);
            indexInTheContent += pair.second - pair.first;
            Write.seekp(pair.first, ios::beg);
            Write.write(content.get(), pair.second - pair.first);
            freeSegments.erase(pair.first);
        } else {
            auto buffer = make_unique<char[]>(len - indexInTheContent);
            memcpy(content.get() + indexInTheContent, buffer.get(), len - indexInTheContent);
            Write.seekp(pair.second - (len - indexInTheContent), ios::beg);
            Write.write(content.get(), (len - indexInTheContent));
            freeSegments[pair.first] = pair.second - (len - indexInTheContent);
            indexInTheContent = len;
        }
        if (indexInTheContent >= len) {
            break;
        }
    }
}
