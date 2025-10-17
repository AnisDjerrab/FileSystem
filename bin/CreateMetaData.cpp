/* Theses functions are essentiel in creating the genera metadata and empty table (indexation table, inodes table, local table) 
 * It also creates a set of generic files to show the user the capabilities of this fs. 
*/


#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <vector>
#include <bitset>
#include <dlfcn.h>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <iomanip>
#include <chrono>
#include <memory>
#include "Dynamic_Functions.h"


using namespace std;

static int SizeOfTheBlock = 512;

struct WriteFile {
    unique_ptr<char[]> Path;
    unique_ptr<char[]> content;
    unique_ptr<char[]> type;
    unique_ptr<char[]> name; 
    unique_ptr<char[]> date;
    char permissions;
    string PathOfTheData;
};

__attribute__((visibility("default")))
void ReadData() __asm__("ReadData");
void ReadData() {
    fstream Read("partitions/Data.bin", ios::in);
    auto buffer = make_unique<char[]>(32);
    while (Read.read(buffer.get(), 32)) {
        bitset<2058> bits(256);
        cout << bits.to_string() << endl;
    }
    Read.close();
}
__attribute__((visibility("default")))
void CreateGeneralMetaData(bool UserJournalisation, string Path) __asm__("CreateGeneralMetaData");
void CreateGeneralMetaData(bool UserJournalisation, string Path) {
    fstream Write(Path, ios::out | ios::binary | ios::trunc);
    auto now = chrono::system_clock::now();
    time_t current_time = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss << std::put_time(std::localtime(&current_time), "%Y%m%d%H%M%S");
    string time_str = ss.str();
    int64_t intDate = stoll(time_str);
    unique_ptr<char[]> identifier = Dynamic_Functions::ConvertNumberInto32BitsString(1234567890);
    vector<pair<unique_ptr<char[]>, const int>> baseVariables;
    baseVariables.emplace_back(move(identifier), 4);                                                               //general identifier 
    baseVariables.emplace_back(move(Dynamic_Functions::ConvertNumberInto64BitsString(intDate)), 8);                //creation date
    baseVariables.emplace_back(move(Dynamic_Functions::ConvertNumberInto64BitsString(-1001)), 8);                  //identifier number (files and folders)
    baseVariables.emplace_back(move(Dynamic_Functions::ConvertNumberInto64BitsString(0)), 8);                      //number of files
    baseVariables.emplace_back(move(Dynamic_Functions::ConvertNumberInto64BitsString(0)), 8);                      //number of folders
    baseVariables.emplace_back(move(Dynamic_Functions::ConvertNumberInto32BitsString()), 4);                       //unique identifier
    baseVariables.emplace_back(move(Dynamic_Functions::ConvertNumberInto64BitsString(0)), 8);                      //size of the data (bits)
    baseVariables.emplace_back(move(Dynamic_Functions::ConvertNumberInto64BitsString(intDate)), 8);                //last mount date
    baseVariables.emplace_back(move(Dynamic_Functions::ConvertNumberInto32BitsString(0)), 4);                      //name of the disk 1 (placeholder)
    baseVariables.emplace_back(move(Dynamic_Functions::ConvertNumberInto32BitsString(0)), 4);                      //name of the disk 2 (placeholder)
    baseVariables.emplace_back(move(Dynamic_Functions::ConvertNumberInto32BitsString(0)), 4);                      //name of the disk 3 (placeholder)
    baseVariables.emplace_back(move(Dynamic_Functions::ConvertNumberInto32BitsString(0)), 4);                      //name of the disk 4 (placeholder)
    baseVariables.emplace_back(move(Dynamic_Functions::ConvertNumberInto16BitsString(100)), 2);                    //version 
    bitset<4096> bits(0);
    auto intermediaire = make_unique<char[]>(SizeOfTheBlock);
    for (int i = 0; i < SizeOfTheBlock ; i++) {
        intermediaire[i] = static_cast<char>((bits >> (i*8)).to_ulong() & 0xFF);
    }
    Write.seekp(0, ios::beg);
    Write.write(intermediaire.get(), SizeOfTheBlock);
    int number = 0;
    for (const auto& pair : baseVariables) {
        Write.seekp(number, ios::beg);
        Write.write(pair.first.get(), pair.second);
        number+=pair.second;
    }
    Write.close();
    if (UserJournalisation) {
    	fstream LogJournal("bin/LogJournal.log", ios::out | ios::in | ios::app);
        LogJournal << "the General metadata were succesfully created!\n";
    	LogJournal.close();
    }} 
__attribute__((visibility("default")))
void CreateIndexationTable(bool UserJournalisation, string Path) __asm__("CreateIndexationTable");
void CreateIndexationTable(bool UserJournalisation, string Path) {
    fstream Write(Path, ios::out | ios::binary | ios::in); 
    Write.seekp(SizeOfTheBlock , ios::beg);
    unique_ptr<char[]> firstMarker = Dynamic_Functions::ConvertNumberInto32BitsString(-1);
    Write.write(firstMarker.get(), 4);
    bitset<4032> DefaultBits1(0);
    auto Intermediaire1 = make_unique<char[]>(SizeOfTheBlock  - 8);
    for (int i = 0; i < SizeOfTheBlock  - 8; i++) {
        Intermediaire1[i] = static_cast<char>((DefaultBits1 >> (i*8)).to_ulong() & 0xFF);
    }
    Write.seekp(260, ios::beg);
    Write.write(Intermediaire1.get(), SizeOfTheBlock  - 8);
    Write.seekp(508, ios::beg);
    Write.write(firstMarker.get(), 4);
    Write.close();
    if (UserJournalisation) {
    	fstream LogJournal("bin/LogJournal.log", ios::out | ios::in | ios::app);
    	LogJournal << "the Indexation table was succesfully created!\n";
    	LogJournal.close();
    }
}
__attribute__((visibility("default")))
void GenerateFirstFile(string PathOfTheFS) __asm__("GenerateFirstFile");
void GenerateFirstFile(string PathOfTheFS) {
    unique_ptr<char[]> output = Dynamic_Functions::ConvertStringIntoUTX8(U"Thank's for downloading this project!\n\nThis is a complete file system robust and polyvalent pretty similar tu Brtfs or ZFS. it includes several types of file (eg: .txt, .md, .img) and folders.\n\nIt also incudes advanced options of reparation an restauration and a java-swing interface.\n\nIt was coded in Java and C++, and works both on Windows and Linux.\n\n@AnisDjerrab, 2025, Algeria, Lisence GPL-3.0");
    auto now = chrono::system_clock::now();
    time_t current_time = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss << std::put_time(std::localtime(&current_time), "%Y%m%d%H%M%S");
    string time_str = ss.str();
    int64_t IntDate = stoll(time_str);
    unique_ptr<char[]> FilePath = Dynamic_Functions::ConvertStringIntoUTX8(U"/");
    unique_ptr<WriteFile> W = Dynamic_Functions::CreateStruct(move(FilePath), move(output), Dynamic_Functions::ConvertNumberInto16BitsString(1), Dynamic_Functions::ConvertStringIntoUTX8(U"README"), Dynamic_Functions::ConvertNumberInto64BitsString(IntDate), PathOfTheFS, Dynamic_Functions::ConvertNumberInto8BitsString(1));
    unordered_map<int, int> temp1;
    unordered_map<int, int> temp2 = {{256, 508}};
    int temp3 = 512;
    Dynamic_Functions::CreateFile(W.get(), "/README.utx8", temp1, temp3, temp2);
    Dynamic_Functions::DestroyStruct(move(W));
}

