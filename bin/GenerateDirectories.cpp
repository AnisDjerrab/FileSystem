/* This code is responsible of Creating Directories In the file system
 * The creation of files is done in 5 essential steps : creation of it's header (name, type...), of it's local inode table (that contains informations about the Blocks's index), And Finally adding it to The Inode & Indexation tables
 * It uses a modular aproach with generique 256 bytes Blocks
 * And it may have a reserved 64-bits space at it's beginning
 * to enable massive redunducy and repairability, it contains duplicated data (espetienlly identifier) 
 * it also massively user bitset to write in a binary file.
*/

#include <iostream>
#include <bitset>
#include <dlfcn.h>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <fstream>
#include <memory>
#include "Dynamic_Functions.h"

using namespace std;

const static int SizeOfTheBlock = 256;

struct WriteDirectory {
    unique_ptr<char[]> Path;
    unique_ptr<char[]> name; 
    unique_ptr<char[]> date;
    char permissions; 
    string PathOfTheData;
};

__attribute__((visibility("default")))
unique_ptr<WriteDirectory> CreateObject(unique_ptr<char[]> p, unique_ptr<char[]> n, unique_ptr<char[]> d, string PathOfTheFS, char permissions) __asm__("CreateObject");
unique_ptr<WriteDirectory> CreateObject(unique_ptr<char[]> p, unique_ptr<char[]> n, unique_ptr<char[]> d, string PathOfTheFS, char permissions) {
    auto W = make_unique<WriteDirectory>();
    W->Path = move(p);
    W->name = move(n);
    W->date = move(d);
    W->permissions = permissions;
    W->PathOfTheData = move(PathOfTheFS);
    return W;
}

__attribute__((visibility("default")))
void CreateDirectory(bool UserJournalisation, WriteDirectory* W, const char* RealNameOfTheFile, unordered_map<int, int>& freeBlocks, int& SizeOfTheFile, unordered_map<int, int>& disponibleSpotsInTheIndexationTable) __asm__("CreateDirectory");
void CreateDirectory(bool UserJournalisation, bool Journalisation, WriteDirectory* W, const char* RealNameOfTheFile, unordered_map<int, int>& freeBlocks, int& SizeOfTheFile, unordered_map<int, int>& disponibleSpotsInTheIndexationTable) {
    fstream LogJournalWriter;
    if (UserJournalisation) {
    	fstream LogJournal("bin/LogJournal.log", ios::out | ios::in | ios::app);
    	LogJournalWriter << "Beginning creating of the file : " << RealNameOfTheFile << endl;
    }
    fstream Write(W->PathOfTheData, ios::out | ios::in | ios::binary);
    vector<int> IndexInTheData;
    unordered_map<int, int> freeSegments;
    vector<int> indexes;
    int UsedFreeBlocks = 0;
    int BlocksToAllocate = 0;
    auto identifier = make_unique<char[]> (8);
    Write.seekg(12, ios::beg);
    Write.read(identifier.get(), 8);
    //creation of The Header Block:
    auto IndexesOfTheBlocksOfTheHeader = make_unique<vector<int>>();
    auto lengthOfTheHeader = make_unique<int>(strlen(W->name.get())*8 + 8 + 8 + 8 + 2);
    auto IndexesOfTheIdentifiersOfTheHeader = make_unique<vector<int>>(); 
    for (size_t i = 0; ;i++) {
        if (i == 0) {
            *lengthOfTheHeader += 8;
            BlocksToAllocate += 1;
        } else {
            BlocksToAllocate += 1;
        }
        if (*lengthOfTheHeader < BlocksToAllocate * SizeOfTheBlock) {
            break;
        }
    }
    auto temp = make_unique<vector<int>>(Dynamic_Functions::listFreeBlocks(Journalisation, UserJournalisation, freeBlocks, freeSegments, disponibleSpotsInTheIndexationTable, indexes, SizeOfTheFile, Write, BlocksToAllocate));
    IndexesOfTheBlocksOfTheHeader->insert(IndexesOfTheBlocksOfTheHeader->end(), temp->begin(), temp->end());
    temp.reset();
    for (int i = 0; i < IndexesOfTheBlocksOfTheHeader->size(); i++) {
        if (i == 0) {
            IndexesOfTheIdentifiersOfTheHeader->push_back(i);
        } else {
            if (IndexesOfTheBlocksOfTheHeader->at(i - 1) + SizeOfTheBlock != IndexesOfTheBlocksOfTheHeader->at(i)) {
                IndexesOfTheIdentifiersOfTheHeader->push_back(i);
                *lengthOfTheHeader += 8;
            }
        }
        if (*lengthOfTheHeader > IndexesOfTheBlocksOfTheHeader->size() * SizeOfTheBlock) {
            IndexesOfTheBlocksOfTheHeader->push_back(Dynamic_Functions::listFreeBlocks(Journalisation, UserJournalisation, freeBlocks, freeSegments, disponibleSpotsInTheIndexationTable, indexes, SizeOfTheFile, Write, 1)[0]);
        }
    }
    BlocksToAllocate = 0;
    auto contentOfTheHeader = make_unique<string>();
    contentOfTheHeader->append(Dynamic_Functions::ConvertNumberInto16BitsString(-1).get(), 2);
    contentOfTheHeader->append(W->date.get());
    contentOfTheHeader->append(W->date.get());
    contentOfTheHeader->append(W->permissions, 1);
    contentOfTheHeader->append(W->name.get());
    int IndexAchieved = 0;
    int IndexInTheContent = 0;
    while (true) {
        bool exit = false;
        bool found = false;
        for (size_t o = 0; o < IndexesOfTheIdentifiersOfTheHeader->size(); o++) {
            if (IndexAchieved == IndexesOfTheIdentifiersOfTheHeader->at(o)) {
                found = true;
                break;
            }
        }
        auto content = make_unique<char[]>(SizeOfTheBlock);
        if (found) {
            if (contentOfTheHeader->size() - IndexInTheContent < SizeOfTheBlock - 8) {
                memcpy(content.get(), identifier.get(), 8);
                memcpy(content.get() + 8, (contentOfTheHeader->substr(IndexInTheContent, contentOfTheHeader->size())).c_str(), contentOfTheHeader->size() - IndexInTheContent);
                exit = true;
            } else {
                memcpy(content.get(), identifier.get(), 8);
                memcpy(content.get() + 8, (contentOfTheHeader->substr(IndexInTheContent, IndexInTheContent + SizeOfTheBlock - 8)).c_str(), SizeOfTheBlock - 8);
                IndexInTheContent += SizeOfTheBlock - 8;
            }
        } else {
            if (contentOfTheHeader->size() - IndexInTheContent < SizeOfTheBlock) {
                memcpy(content.get(), (contentOfTheHeader->substr(IndexInTheContent, contentOfTheHeader->size())).c_str(), contentOfTheHeader->size() - IndexInTheContent);
                exit = true;
            } else {
                memcpy(content.get(), (contentOfTheHeader->substr(IndexInTheContent, IndexInTheContent + SizeOfTheBlock)).c_str(), SizeOfTheBlock/8);
                IndexInTheContent += SizeOfTheBlock;
            }
        }
        Write.seekp(IndexesOfTheBlocksOfTheHeader->at(IndexAchieved), ios::beg);
        Write.write(content.get(), SizeOfTheBlock);
        content.reset();
        IndexAchieved++;
        if (exit) {
            break;
        }
    }
    contentOfTheHeader.reset();
    if (UserJournalisation) {
    	LogJournalWriter << "Succesfully created the header of the file : " << RealNameOfTheFile << endl;
    }
    //Creation Of The Local Indexation Table that contains crucial Information about the Files
    auto IndexesOfTheBlocksOfTheLocalInodeTable = make_unique<vector<int>>();
    auto IndexesOfTheIdentifiersOfTheLocalInodeTable = make_unique<vector<int>>(); 
    auto MappageOfAllTheClustersAndInodes = make_unique<vector<unique_ptr<char[]>>>();
    MappageOfAllTheClustersAndInodes->push_back(Dynamic_Functions::ConvertNumberInto64BitsString(-900));
    for (size_t i = 0; i < UsedFreeBlocks - 1; i++) {
        auto FirstValue = Dynamic_Functions::ConvertNumberInto64BitsString(IndexInTheData[i]);
        while(IndexInTheData[i] + SizeOfTheBlock == IndexInTheData[i + 1]) {
            if (!(i + 2 < UsedFreeBlocks - 1)) {
                break;
            }
            i++;
        }
        auto secondValue = Dynamic_Functions::ConvertNumberInto64BitsString(IndexInTheData[i + 1]);
        if (FirstValue != secondValue) {
            MappageOfAllTheClustersAndInodes->push_back(Dynamic_Functions::ConvertNumberInto64BitsString(-800));
            MappageOfAllTheClustersAndInodes->push_back(move(FirstValue));
            MappageOfAllTheClustersAndInodes->push_back(move(secondValue));
        } else {
            MappageOfAllTheClustersAndInodes->push_back(move(FirstValue));
        }
    }
    int lengthOfTheLocalInodeTable = MappageOfAllTheClustersAndInodes->size()*8;
    for (size_t i = 0; ; i++) {
        if (i == 0) {
            lengthOfTheLocalInodeTable += 8;
            BlocksToAllocate += 1;
        } else {
            BlocksToAllocate += 1;
        }
        if (lengthOfTheLocalInodeTable < BlocksToAllocate * SizeOfTheBlock) {
            break;
        }
    }
    temp = make_unique<vector<int>>(Dynamic_Functions::listFreeBlocks(Journalisation, UserJournalisation, freeBlocks, freeSegments, disponibleSpotsInTheIndexationTable, indexes, SizeOfTheFile, Write, BlocksToAllocate));
    IndexesOfTheBlocksOfTheLocalInodeTable->insert(IndexesOfTheBlocksOfTheLocalInodeTable->end(), temp->begin(), temp->end());
    temp.reset();
    for (int i = 0; i < IndexesOfTheBlocksOfTheLocalInodeTable->size(); i++) {
        if (i == 0) {
            IndexesOfTheIdentifiersOfTheLocalInodeTable->push_back(i);
        } else {
            if (IndexesOfTheBlocksOfTheLocalInodeTable->at(i - 1) + SizeOfTheBlock != IndexesOfTheBlocksOfTheLocalInodeTable->at(i)) {
                IndexesOfTheIdentifiersOfTheLocalInodeTable->push_back(i);
                lengthOfTheLocalInodeTable += 8;
            }
        }
        if (lengthOfTheLocalInodeTable > IndexesOfTheBlocksOfTheLocalInodeTable->size() * SizeOfTheBlock) {
            IndexesOfTheBlocksOfTheLocalInodeTable->push_back(Dynamic_Functions::listFreeBlocks(Journalisation, UserJournalisation, freeBlocks, freeSegments, disponibleSpotsInTheIndexationTable, indexes, SizeOfTheFile, Write, 1)[0]);
        }
    }
    IndexAchieved = 0;
    IndexInTheContent = 0;
    while (true) {
        bool exit = false;
        auto content = make_unique<char[]>(SizeOfTheBlock);
        int index = -1;
        bool found = false;
        for (size_t o = 0; o < IndexesOfTheIdentifiersOfTheLocalInodeTable->size(); o++) {
            if (IndexAchieved == IndexesOfTheIdentifiersOfTheLocalInodeTable->at(o)) {
                index = 0;
                found = true;
                break;
            }
        }
        if (found) {
            if ((MappageOfAllTheClustersAndInodes->size() - IndexInTheContent)*8 < SizeOfTheBlock - 8) {
                memcpy(content.get(), identifier.get(), 8);
                for (size_t i = IndexInTheContent; i < MappageOfAllTheClustersAndInodes->size(); i++) {
                    memcpy(content.get() + 8 + (i - IndexInTheContent)*8, MappageOfAllTheClustersAndInodes->at(i).get(), 8);
                }
                exit = true;
            } else {
                memcpy(content.get(), identifier.get(), 8);
                for (size_t i = IndexInTheContent; i < SizeOfTheBlock/8 - 1; i++) {
                    memcpy(content.get() + 8 + (i - IndexInTheContent)*8, MappageOfAllTheClustersAndInodes->at(i).get(), 8);
                    IndexInTheContent++;
                }
            }
        } else {
            if ((MappageOfAllTheClustersAndInodes->size() - IndexInTheContent)*8 < SizeOfTheBlock - 8) {
                for (size_t i = IndexInTheContent; i < MappageOfAllTheClustersAndInodes->size(); i++) {
                    memcpy(content.get() + (i - IndexInTheContent)*8, MappageOfAllTheClustersAndInodes->at(i).get(), 8);
                }
                exit = true;
            } else {
                for (size_t i = IndexInTheContent; i < SizeOfTheBlock/8; i++) {
                    memcpy(content.get() + (i - IndexInTheContent)*8, MappageOfAllTheClustersAndInodes->at(i).get(), 8);
                    IndexInTheContent++;
                }
            }
        }
        Write.seekp(IndexesOfTheBlocksOfTheLocalInodeTable->at(IndexAchieved), ios::beg);
        Write.write(content.get(), SizeOfTheBlock);
        IndexAchieved++;
        if (exit) {
            break;
        }
    }
    UsedFreeBlocks += IndexesOfTheBlocksOfTheLocalInodeTable->size();
    IndexInTheData.insert(IndexInTheData.begin(), IndexesOfTheBlocksOfTheLocalInodeTable->begin(), IndexesOfTheBlocksOfTheLocalInodeTable->end());
    IndexesOfTheBlocksOfTheLocalInodeTable.reset();
    IndexesOfTheIdentifiersOfTheLocalInodeTable.reset();
    if (UserJournalisation) {
    	LogJournalWriter << "the local inodes table of the file : " << RealNameOfTheFile << " was succesfully created." << endl;
    }
    //adding the program to The Inode Table
    auto ContentOfTheInodeBlock = make_unique<string>();
    ContentOfTheInodeBlock->append(Dynamic_Functions::ConvertNumberInto16BitsString(-1).get(), 2);
    ContentOfTheInodeBlock->append(Dynamic_Functions::ConvertNumberInto64BitsString(0).get(), 8); //size of the file
    ContentOfTheInodeBlock->append(Dynamic_Functions::ConvertNumberInto64BitsString(0).get(), 8);
    ContentOfTheInodeBlock->append(W->date.get(), 8);
    ContentOfTheInodeBlock->append(W->date.get(), 8);
    ContentOfTheInodeBlock->append(W->Path.get());
    for (size_t i = 0; i < MappageOfAllTheClustersAndInodes->size(); i++) {
        ContentOfTheInodeBlock->append(MappageOfAllTheClustersAndInodes->at(i).get(), 8);
    }
    MappageOfAllTheClustersAndInodes.reset();
    auto IndexOfTheBlocksOfTheMainInodeTable = make_unique<vector<int>>();
    while (ContentOfTheInodeBlock->size() < IndexOfTheBlocksOfTheMainInodeTable->size() * (SizeOfTheBlock - 8)) {
        BlocksToAllocate += 1;
    }
    temp = make_unique<vector<int>>(Dynamic_Functions::listFreeBlocks(Journalisation, UserJournalisation, freeBlocks, freeSegments, disponibleSpotsInTheIndexationTable, indexes, SizeOfTheFile, Write, BlocksToAllocate));
    IndexOfTheBlocksOfTheMainInodeTable->insert(IndexOfTheBlocksOfTheMainInodeTable->end(), temp->begin(), temp->end());
    temp.reset();
    int IndexOfTheFileVariable = IndexOfTheBlocksOfTheMainInodeTable->at(0) + 2;
    BlocksToAllocate = 0;
    IndexAchieved = 0;
    IndexInTheContent = 0;
    while (true) {
        bool exit = false;
        auto content = make_unique<char[]>(SizeOfTheBlock);
        if (IndexAchieved + 1 == IndexOfTheBlocksOfTheMainInodeTable->size()) {
            memcpy(content.get(), Dynamic_Functions::ConvertNumberInto64BitsString(IndexOfTheBlocksOfTheMainInodeTable->at(0)).get(), 8);
        } else {
            memcpy(content.get(), Dynamic_Functions::ConvertNumberInto64BitsString(IndexOfTheBlocksOfTheMainInodeTable->at(0)).get(), 8);
        }
        if (ContentOfTheInodeBlock->size() - IndexInTheContent < SizeOfTheBlock - 8) {
            memcpy(content.get() + 8, (ContentOfTheInodeBlock->substr(IndexInTheContent, ContentOfTheInodeBlock->size())).c_str(), ContentOfTheInodeBlock->size() - IndexInTheContent);
            exit = true;
        } else {
            memcpy(content.get() + 8, (ContentOfTheInodeBlock->substr(IndexInTheContent, IndexInTheContent + SizeOfTheBlock - 64)).c_str(), SizeOfTheBlock - 8); 
        }
        IndexInTheContent += SizeOfTheBlock - 8;
        Write.seekp(IndexOfTheBlocksOfTheMainInodeTable->at(IndexAchieved), ios::beg);
        Write.write(content.get(), SizeOfTheBlock);
        IndexAchieved++;
        if (exit) {
            break;
        }
    }
    ContentOfTheInodeBlock.reset();
    UsedFreeBlocks += IndexOfTheBlocksOfTheMainInodeTable->size();
    int IndexOfTheHeader = IndexOfTheBlocksOfTheMainInodeTable->size();
    IndexInTheData.insert(IndexInTheData.begin(), IndexOfTheBlocksOfTheMainInodeTable->begin(), IndexOfTheBlocksOfTheMainInodeTable->end());
    int totalSizeOfTheFile = UsedFreeBlocks + IndexOfTheBlocksOfTheMainInodeTable->size();
    IndexOfTheBlocksOfTheMainInodeTable.reset();
    if (UserJournalisation) {
    	LogJournalWriter << "The file : " << RealNameOfTheFile << " was succesfully added to the global inode table." << endl;
    }
    //Adding The Inode Blocks to the Indexation Table
    int SizeOfTheIndexationBlocks = 0;
    for (const auto& pair : disponibleSpotsInTheIndexationTable) {
        SizeOfTheIndexationBlocks += pair.second - pair.first;
    }
    int indexOfTheNewIndexationBlockCreated;
    if (64 > SizeOfTheIndexationBlocks) {
        indexOfTheNewIndexationBlockCreated = Dynamic_Functions::listFreeBlocks(Journalisation, UserJournalisation, freeBlocks, freeSegments, disponibleSpotsInTheIndexationTable, indexes, SizeOfTheFile, Write, 1)[0];
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
        for (const auto& pair : disponibleSpotsInTheIndexationTable) {
            if (pair.first - pair.second >= 16) {
                auto buffer = make_unique<char[]>(16);
                memcpy(buffer.get(), Dynamic_Functions::ConvertNumberInto64BitsString(-450).get(), 8);
                memcpy(buffer.get() + 8, Dynamic_Functions::ConvertNumberInto64BitsString(indexOfTheNewIndexationBlockCreated).get(), 8);
                Write.seekp(pair.second - 16, ios::beg);
                Write.write(buffer.get(), 16);
                disponibleSpotsInTheIndexationTable[pair.first] = pair.second - 16;
            }
        }
    } 
    for (const auto& pair : disponibleSpotsInTheIndexationTable) {
        if (pair.first - pair.second >= 8) {
            Write.seekp(pair.second - 8, ios::beg);
            Write.write(Dynamic_Functions::ConvertNumberInto64BitsString(IndexOfTheHeader).get(), 8);
            if (pair.first == pair.second - 8) {
                disponibleSpotsInTheIndexationTable.erase(pair.second);
            } else {
                disponibleSpotsInTheIndexationTable[pair.first] = pair.second - 8;
            }
            break;
        }
    }
    Write.seekp(IndexOfTheFileVariable, ios::beg);
    Write.write(Dynamic_Functions::ConvertNumberInto64BitsString(totalSizeOfTheFile * SizeOfTheBlock).get(), 8);
    auto NumberOfFiles = make_unique<char[]>(8);
    Write.seekg(20, ios::beg);
    Write.read(NumberOfFiles.get(), 8);
    int64_t number = Dynamic_Functions::Convert64BitsStringIntoNumber(NumberOfFiles.get());
    Write.seekp(20, ios::beg);
    Write.write(Dynamic_Functions::ConvertNumberInto64BitsString(number + 1).get(), 8);
    int64_t IdentifierNumber = Dynamic_Functions::Convert64BitsStringIntoNumber(identifier.get());
    Write.seekp(8, ios::beg);
    Write.write(Dynamic_Functions::ConvertNumberInto64BitsString(IdentifierNumber - 1).get(), 8);
    if (UserJournalisation) {
    	LogJournalWriter << "the indexation table was succesfully updated & the file was succesfully created." << endl;
        LogJournalWriter.close();
    }
    Write.close();
}

__attribute__((visibility("default")))
void DestroyObject(unique_ptr<WriteDirectory> W) __asm__("DestroyObject");
void DestroyObject(unique_ptr<WriteDirectory> W) {
    W.reset();
}
