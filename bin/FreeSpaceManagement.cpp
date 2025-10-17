/* This is the set of fuction that provide utilities in defferent areas of the program to : 
 * 1) list free Blocks and provide their indexes 
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <dlfcn.h>
#include <memory>
#include <bitset>
#include <cstring>
#include "Dynamic_Functions.h"

using namespace std;

static int SizeOfTheBlock = 512;

__attribute__((visibility("default")))
vector<int> listFreeBlocks(bool Journalisation, bool UserJournalisation, unordered_map<int, int>& freeBlocksHeaders, unordered_map<int, int>& freeSegments, unordered_map<int, int>& IndexationSegments, vector<int> Indexes, int& SizeOfTheFile, fstream& Write, int NumberOfBlocks) __asm__("listFreeBlocks");
vector<int> listFreeBlocks(bool Journalisation, bool UserJournalisation, unordered_map<int, int>& freeBlocksHeaders, unordered_map<int, int>& freeSegments, unordered_map<int, int>& IndexationSegments, vector<int> Indexes, int& SizeOfTheFile, fstream& Write, int NumberOfBlocks) {
    fstream LogJournal("bin/LogJournal.log", ios::in | ios::out);
    vector<int> Blocks;
    vector<int> elementsToRemoveFromMap;
    for (const auto& [key, value] : freeBlocksHeaders) {
        vector<int> previousHeadBlocks;
        int currentBlock = key;
        while (true) {
            auto type = make_unique<char[]>(4);
            Write.seekg(currentBlock, ios::beg);
            Write.read(type.get(), 4);
            int Converted_type = Dynamic_Functions::Convert32BitsStringIntoNumber(type.get());
            if (Converted_type == -2) {
                auto number = make_unique<char[]>(4);
                Write.seekg(currentBlock + 4, ios::beg);
                Write.read(number.get(), 4);
                int Converted_number = Dynamic_Functions::Convert32BitsStringIntoNumber(number.get()); 
                if (Converted_number == 0) {
                    Blocks.push_back(currentBlock);
                    if (Blocks.size() >= NumberOfBlocks) {
                        break;
                    }
                    if (previousHeadBlocks.empty()) {
                        bitset<128> bits(0);
                        auto temp = make_unique<char[]>(16);
                        for (int i = 0; i < 16; i++) {
                            temp[i] = static_cast<char>((bits >> (i*8)).to_ulong() & 0xFF);
                        }
                        Write.seekp(key - 8, ios::beg);
                        Write.write(temp.get(), ios::beg);
                        elementsToRemoveFromMap.push_back(key);
                        break;
                    } else {
                        currentBlock = previousHeadBlocks[previousHeadBlocks.size() - 1];
                        previousHeadBlocks.pop_back();
                        bool Break = false;
                        while (true) {
                            Write.seekg(currentBlock + 4, ios::beg);
                            Write.read(number.get(), 4);
                            int resultat = Dynamic_Functions::Convert64BitsStringIntoNumber(number.get()) - 1;
                            if (resultat <= 0) {
                                currentBlock = previousHeadBlocks[previousHeadBlocks.size() - 1];
                                Blocks.push_back(currentBlock);
                                if (Blocks.size() >= NumberOfBlocks) {
                                    break;
                                }
                                previousHeadBlocks.pop_back();
                            } else {
                                Write.seekp(currentBlock + 4, ios::beg);
                                Write.write(Dynamic_Functions::ConvertNumberInto64BitsString(resultat).get(), 8);
                                break;
                            }
                            if (previousHeadBlocks.empty()) {
                                Break = true;
                                break;
                            }
                        }
                        if (Break) {
                            break;
                        }
                    }
                } else if (Converted_number <= 63) {
                    auto index = make_unique<char[]>(8);
                    Write.seekg(key + Converted_number*8, ios::beg);
                    Write.read(index.get(), 8);
                    int64_t Converted_index = Dynamic_Functions::Convert64BitsStringIntoNumber(index.get());
                    if (Converted_index != 0 && Converted_index % SizeOfTheBlock == 0) {
                        previousHeadBlocks.push_back(currentBlock);
                        currentBlock = Converted_index;
                    } else {
                        if (UserJournalisation) {
                        	LogJournal << "warning 0002: Some free blocks cannot be used. This is not a critical error, but this may waste performance." << endl;
                        }
                    }
                } else {
                    if (UserJournalisation) {
                    	LogJournal << "warning 0003: Some free blocks headers are corrupted. This is not a critical error, but some free blocks may not be used." << endl;
                    }
                }
            } else if (Converted_type == -3) {
                auto number = make_unique<char[]>(8);
                Write.seekg(currentBlock + 4, ios::beg);
                Write.read(number.get(), 8);
                int Converted_number = Dynamic_Functions::Convert64BitsStringIntoNumber(number.get());
                if (Converted_number == 0) {
                    Blocks.push_back(currentBlock);
                    if (Blocks.size() >= NumberOfBlocks) {
                        break;
                    }
                    if (previousHeadBlocks.empty()) {
                        bitset<128> bits(0);
                        auto temp = make_unique<char[]>(16);
                        for (int i = 0; i < 16; i++) {
                            temp[i] = static_cast<char>((bits >> (i*8)).to_ulong() & 0xFF);
                        }
                        Write.seekp(key - 8, ios::beg);
                        Write.write(temp.get(), ios::beg);
                        elementsToRemoveFromMap.push_back(key);
                        break;
                    } else {
                        currentBlock = previousHeadBlocks[previousHeadBlocks.size() - 1];
                        previousHeadBlocks.pop_back();
                        bool Break = false;
                        while (true) {
                            Write.seekg(currentBlock + 4, ios::beg);
                            Write.read(number.get(), 4);
                            int resultat = Dynamic_Functions::Convert64BitsStringIntoNumber(number.get()) - 1;
                            if (resultat <= 0) {
                                currentBlock = previousHeadBlocks[previousHeadBlocks.size() - 1];
                                Blocks.push_back(currentBlock);
                                if (Blocks.size() >= NumberOfBlocks) {
                                    break;
                                }
                                previousHeadBlocks.pop_back();
                            } else {
                                Write.seekp(currentBlock + 4, ios::beg);
                                Write.write(Dynamic_Functions::ConvertNumberInto64BitsString(resultat).get(), 8);
                                break;
                            }
                            if (previousHeadBlocks.empty()) {
                                Break = true;
                                break;
                            }
                        }
                        if (Break) {
                            break;
                        }
                    }
                    if (Blocks.size() >= NumberOfBlocks) {
                        break;
                    }
                } else if (Converted_number <= 63) {
                    int BlocksAllocated = 0;
                    bool Break = false;
                    for (int i = 1; i <= Converted_number; i++) {
                        auto index = make_unique<char[]>(8);
                        Write.seekg(currentBlock + i*8, ios::beg);
                        Write.read(index.get(), 8);
                        int64_t Converted_index = Dynamic_Functions::Convert64BitsStringIntoNumber(index.get());
                        if (Converted_index % SizeOfTheBlock == 0 && Converted_index > SizeOfTheBlock) {
                            BlocksAllocated += 1;
                            Blocks.push_back(Converted_index);
                            if (Blocks.size() >= NumberOfBlocks) {
                                Break = true;
                                break;
                            }
                        } else {
                            BlocksAllocated += 1;
                            if (UserJournalisation) {
                            	LogJournal << "warning 0004: Some free blocks cannot be used. This is not a critical error." << endl;
                            }
                        }
                    }
                    if (Converted_number - BlocksAllocated > 0) {
                        Write.seekp(currentBlock + 4, ios::beg);
                        Write.write(Dynamic_Functions::ConvertNumberInto32BitsString(Converted_number - BlocksAllocated).get(), 4);
                    } else {
                        Blocks.push_back(currentBlock);
                        if (Blocks.size() >= NumberOfBlocks) {
                            break;
                        }
                        if (previousHeadBlocks.empty()) {
                            bitset<128> bits(0);
                            auto temp = make_unique<char[]>(16);
                            for (int i = 0; i < 16; i++) {
                                temp[i] = static_cast<char>((bits >> (i*8)).to_ulong() & 0xFF);
                            }
                            Write.seekp(key - 8, ios::beg);
                            Write.write(temp.get(), ios::beg);
                            elementsToRemoveFromMap.push_back(key);
                            break;
                        } else {
                            currentBlock = previousHeadBlocks[previousHeadBlocks.size() - 1];
                            previousHeadBlocks.pop_back();
                            bool Break = false;
                            while (true) {
                                Write.seekg(currentBlock + 4, ios::beg);
                                Write.read(number.get(), 4);
                                int resultat = Dynamic_Functions::Convert64BitsStringIntoNumber(number.get()) - 1;
                                if (resultat <= 0) {
                                    currentBlock = previousHeadBlocks[previousHeadBlocks.size() - 1];
                                    Blocks.push_back(currentBlock);
                                    if (Blocks.size() >= NumberOfBlocks) {
                                        break;
                                    }
                                    previousHeadBlocks.pop_back();
                                } else {
                                    Write.seekp(currentBlock + 4, ios::beg);
                                    Write.write(Dynamic_Functions::ConvertNumberInto64BitsString(resultat).get(), 8);
                                    break;
                                }
                                if (previousHeadBlocks.empty()) {
                                    Break = true;
                                    break;
                                }
                            }
                            if (Break) {
                                break;
                            }
                        }
                    }
                    if (Break) {
                        break;
                    }
                }
            } else {
                if (UserJournalisation) {
                	LogJournal << "warning 0004: One header block was completely lost. This is not a critical error, but it can severely affect performances and free space management in the fs." << endl;
                }
            }
        }
        if (Blocks.size() >= NumberOfBlocks) {
            break;
        }
    }
    while (Blocks.size() < NumberOfBlocks) {
        Blocks.push_back(SizeOfTheFile);
        SizeOfTheFile += SizeOfTheBlock;
    }
    for (int i = 0; i < elementsToRemoveFromMap.size(); i++) {
        freeBlocksHeaders.erase(elementsToRemoveFromMap[i]);
    }
    if (Journalisation) {
        auto journalisation = make_unique<char[]>(Blocks.size() * 10);
        for (int i = 0; i < Blocks.size(); i++) {
            memcpy(Dynamic_Functions::ConvertNumberInto64BitsString(Blocks[i]).get(), journalisation.get() + i * 10, 8);
            memcpy(Dynamic_Functions::ConvertNumberInto16BitsString(-1).get(), journalisation.get() + i * 10 + 8, 2);
        }
        Dynamic_Functions::WriteInJournal(freeSegments, freeBlocksHeaders, SizeOfTheFile, Write, IndexationSegments, Indexes, journalisation.get(), Blocks.size() * 10);
    }
    return Blocks;
}

__attribute__((visibility("default")))
void MarkBlocksAsFree(vector<int> indexOfTheBlocks, bool newHeader, unordered_map<int, int>& freeBlocksHeaders, unordered_map<int, int>& freeSegments, unordered_map<int, int>& IndexationSegments, fstream& Write, int& SizeOfTheFile) __asm__("MarkBlocksAsFree");
void MarkBlocksAsFree(vector<int> indexOfTheBlocks, bool newHeader, unordered_map<int, int>& freeBlocksHeaders, unordered_map<int, int>& freeSegments, unordered_map<int, int>& IndexationSegments, fstream& Write, int& SizeOfTheFile) {
    bool CreateNewHeader = false;
    if (indexOfTheBlocks.size() > 1000) {
        CreateNewHeader = true;
    }
    if (CreateNewHeader || newHeader) {
        int levels = 0;
        int number = indexOfTheBlocks.size();
        while (number /= 63 > 63) {
            levels += 1;
        }
        int DisponibleSize = 0;
        for (const auto& pair : IndexationSegments) {
            DisponibleSize += pair.second - pair.first;
        }
        if (DisponibleSize < 8) {
            return;
        } else if (DisponibleSize < 64) {
            vector<int> temp;
            int IndexToWrite = Dynamic_Functions::listFreeBlocks(false, false, freeBlocksHeaders, freeSegments, IndexationSegments, temp, SizeOfTheFile, Write, 1)[0];
            auto buffer = Dynamic_Functions::ConvertNumberInto64BitsString(IndexToWrite);
            for (const auto& pair : IndexationSegments) {
                if (pair.second - pair.first >= 16) {
                    Write.seekp(pair.second - 16, ios::beg);
                    Write.write(Dynamic_Functions::ConvertNumberInto64BitsString(0).get(), 8);
                    Write.seekp(pair.second - 8, ios::beg);
                    Write.write(buffer.get(), 8);
                    if (pair.second - 16 == pair.first) {
                        IndexationSegments.erase(pair.first);
                    } else {
                        IndexationSegments[pair.first] = pair.second - 8;
                    }
                    IndexationSegments[IndexToWrite + 4] = IndexToWrite + SizeOfTheBlock - 8;
                    break;
                }
            }
        }
        int number = 0;
        int index = 0;
        int IndexToWrite = 0;
        auto firstMarker = Dynamic_Functions::ConvertNumberInto32BitsString(-2);
        auto secondMarker = Dynamic_Functions::ConvertNumberInto32BitsString(-3);
        for (int i = 0; i < levels - 1; i++) {
            number += i^63;
            for (size_t o = 0; o < i^63; o++) {
                IndexToWrite = indexOfTheBlocks[index];
                Write.seekp(IndexToWrite, ios::beg);
                index++;
                if (i == levels - 2) {
                    Write.write(secondMarker.get(), 4);
                } else {
                    Write.write(firstMarker.get(), 4);
                }
                if (indexOfTheBlocks.size() - number >= 63) {
                    if (i == levels - 2) {
                        number += 63;
                    }
                    auto buffer = make_unique<char[]>(SizeOfTheBlock - 8);
                    for (int y = 0; y < 63; y++) {
                        memcpy(Dynamic_Functions::ConvertNumberInto64BitsString(indexOfTheBlocks[index]).get(), buffer.get() + y*8, 8);
                        index++;
                    }
                    IndexToWrite += 4;
                    Write.seekp(IndexToWrite, ios::beg);
                    Write.write(Dynamic_Functions::ConvertNumberInto32BitsString(63).get(), 4);
                    IndexToWrite += 4;
                    Write.seekp(IndexToWrite);
                    Write.write(buffer.get(), SizeOfTheBlock - 8);
                } else {
                    int RemainingSize = indexOfTheBlocks.size() - number;
                    if (i == levels - 2) {
                        number += RemainingSize;
                    }
                    if (RemainingSize == 0) {
                        IndexToWrite += 4;
                        Write.seekp(IndexToWrite, ios::beg);
                        Write.write(Dynamic_Functions::ConvertNumberInto32BitsString(0).get(), 4);
                    } else {
                        auto buffer = make_unique<char[]>(RemainingSize * 8);
                        for (size_t y = 0; y < RemainingSize; y++) {
                            memcpy(Dynamic_Functions::ConvertNumberInto64BitsString(indexOfTheBlocks[index]).get(), buffer.get() + y*8, 8);
                            index++;
                        }
                        IndexToWrite += 4;
                        Write.seekp(IndexToWrite, ios::beg);
                        Write.write(Dynamic_Functions::ConvertNumberInto32BitsString(RemainingSize).get(), 4);
                        IndexToWrite += SizeOfTheBlock - RemainingSize * 8 - 4;
                        Write.seekp(IndexToWrite, ios::beg);
                        Write.write(buffer.get(), SizeOfTheBlock - 8);
                    }
                }
            }
        }
        for (const auto& pair : IndexationSegments) {
            if (pair.second - pair.first >= 16) {
                Write.seekp(pair.second - 16, ios::beg);
                Write.write(Dynamic_Functions::ConvertNumberInto64BitsString(-250).get(), 8);
                Write.seekp(pair.second - 8, ios::beg);
                Write.write(Dynamic_Functions::ConvertNumberInto64BitsString(indexOfTheBlocks[0]).get(), 8);
            }
        }
    } else {
        
    }
}
