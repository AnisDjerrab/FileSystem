/* This function is the essential function that mapps the filesystem.
*/

#include <iostream>
#include <unordered_map>
#include <vector>
#include <memory>
#include <dlfcn.h>
#include <fstream>
#include <cstring>
#include <string>
#include <filesystem>
#include <ctime>
#include "Dynamic_Functions.h"

using namespace std;

const int SizeOfTheBlock = 256;

struct MetaData {
    unordered_map<string, unordered_map<int64_t, int>> files;
    unordered_map<int, int> freeBlocksHeaders;
    unordered_map<int, int> disponibleSpotsInTheIndexationBlocks;
    unordered_map<int64_t, vector<int64_t>> FilesInWD;
    int SizeOfTheData;
    int64_t CreationDate;
    int64_t LastMountDate;
    int64_t NumberOfFiles;
    int64_t NumberOfFolders;
    int64_t identifierNumber;
    string ErrorCode;
};

__attribute__((visibility("default")))
unique_ptr<MetaData> MappTheMetaData(bool UserJournalisation, string PathOfTheData, bool ReadWrite, bool IgnoreErrors) __asm__("MappTheMetaData");
unique_ptr<MetaData> MappTheMetaData(bool UserJournalisation, string PathOfTheData, bool ReadWrite, bool IgnoreErrors) {
    fstream Write(PathOfTheData, ios::in | ios::out | ios::binary);
    fstream LogJournal;
    if (UserJournalisation) {
    	fstream LogJournal("bin/LogJournal.log", ios::out | ios::in | ios::app);
    }
    auto M = make_unique<MetaData>();
    M->ErrorCode = "";
    bool interrupt = false;
    auto IndexationBlocks = make_unique<unordered_map<const char*, int>>();
    if (UserJournalisation) {
    	LogJournal << "Mounting FileSystem : " << PathOfTheData << endl;
    }
    auto FirstBlock = make_unique<char[]>(SizeOfTheBlock);
    Write.seekg(SizeOfTheBlock);
    Write.read(FirstBlock.get(), SizeOfTheBlock);
    auto Blocks = make_unique<unordered_map<int64_t, int>>();
    IndexationBlocks->emplace(FirstBlock.get(), 248);
    bool CreateNewIndexationBlock = false;
    Dynamic_Functions::ReadData();
    for (const auto& [key, value] : *IndexationBlocks) {
        auto marker = make_unique<char[]>(4);
        memcpy(marker.get(), key, 4);
        if (Dynamic_Functions::Convert64BitsStringIntoNumber(marker.get()) != -1) {
            if (UserJournalisation) {
            	LogJournal << "error 0014: one of the indexation blocks is corrupted. This is a critical error, cannot proceed with mount." << endl;
            }
            if (!IgnoreErrors) {
                interrupt = true;
                break;
            }
        }
        memcpy(marker.get(), key + 124, 4);
        if (Dynamic_Functions::Convert64BitsStringIntoNumber(marker.get()) != -1) {
            if (UserJournalisation) {
            	LogJournal << "error 0015: one of the indexation blocks is corrupted. This is a critical error, cannot proceed with mount." << endl;
            }
            if (!IgnoreErrors) {
                interrupt = true;
                break;
            }
        }
        bool Break = false;
        for (int o = 4; o < 124; o+=8) {
            auto buffer = make_unique<char[]>(8);
            memcpy(buffer.get(), key + o, 8);
            int64_t result = Dynamic_Functions::Convert64BitsStringIntoNumber(buffer.get());
            if (result >= SizeOfTheBlock && result % SizeOfTheBlock == 0) {
                auto identifier = make_unique<char[]>(8);
                Write.seekg(result / 8, ios::beg);
                Write.read(identifier.get(), 8);
                (*Blocks)[Dynamic_Functions::Convert64BitsStringIntoNumber(identifier.get())] = result;
            } else if (result == 0) {
                int SizeOfTheFreeBlock = 64;
                while (true) {
                    o+=8;
                    if (o >= 124) {
                        break;
                    } 
                    memcpy(buffer.get(), key + o, 8);
                    int64_t number = Dynamic_Functions::Convert64BitsStringIntoNumber(buffer.get());
                    if (number == 0) {
                        SizeOfTheFreeBlock += 64;
                    } else {
                        o-=4;
                        break;
                    }
                }
                M->disponibleSpotsInTheIndexationBlocks[value + o*8] = SizeOfTheFreeBlock;
            } else if (result == -450) {
                o+=8;
                memcpy(buffer.get(), key + o, 8);
                int64_t number = Dynamic_Functions::Convert64BitsStringIntoNumber(buffer.get());
                if (number >= 1024 && number % SizeOfTheBlock == 0) {
                    auto temp = make_unique<char[]>(SizeOfTheBlock);
                    Write.seekg(number / 8, ios::beg),
                    Write.read(temp.get(), SizeOfTheBlock);
                    IndexationBlocks->emplace(temp.get(), number);
                } else {
                    if (UserJournalisation) {
                    	LogJournal << "error 0016: one of the indexation blocks is corrupted. This is a critical error, cannot proceed with mount." << endl;
                    }
                    if (!IgnoreErrors) {
                        interrupt = true;
                        break;
                    }
                }
            } else if (result == -250) {
                o+=8;
                memcpy(buffer.get(), key + o, 8);
                int64_t number = Dynamic_Functions::Convert64BitsStringIntoNumber(buffer.get());
                M->freeBlocksHeaders[value + o*8] = number;
            } else {
                if (UserJournalisation) {
                	LogJournal << "warning 0001: one of the indexation blocks contains invalid data. Non critical error." << endl;
                }
            }
        }
        if (Break) {
            break;
        }
    }
    if (interrupt) {
        if (UserJournalisation) {
        	LogJournal.close();
        }        Write.close();
        M->ErrorCode = "One of the indexation blocks is corrupted. cannot proceed with mount.";
        return M;
    }
    for (const auto& [key, value] : *Blocks) {
        u32string path = U"";
        auto temp = make_unique<char[]>(121);
        Write.seekg(value / 8 + 44, ios::beg);
        Write.read(temp.get(), 86);
        temp[86] = '\0';
        path += Dynamic_Functions::ConvertUTX8IntoString(temp.get());
        if (path.size() / 4 < 86) {
            M->files[Dynamic_Functions::ConvertU32StringIntoString(path)][key] = value;
        } else {
            int64_t indexAchieved = value;
            auto ptr = make_unique<char[]>(8);
            Write.seekg(indexAchieved / 8, ios::beg);
            Write.read(ptr.get(), 8);
            indexAchieved = Dynamic_Functions::Convert64BitsStringIntoNumber(ptr.get());
            bool ProprelyEnded = false;
            while (true) {
                Write.seekg(indexAchieved / 8 + 8, ios::beg);
                Write.read(temp.get(), SizeOfTheBlock - 8);
                temp[SizeOfTheBlock - 8] = '\0';
                u32string str = Dynamic_Functions::ConvertUTX8IntoString(temp.get());
                path += str;
                if (str.size() / 4 < SizeOfTheBlock - 8) {
                    M->files[Dynamic_Functions::ConvertU32StringIntoString(path)][key] = value;
                    ProprelyEnded = true;
                    break;
                } 
                Write.seekg(indexAchieved / 8, ios::beg);
                Write.read(ptr.get(), 8); 
                indexAchieved = Dynamic_Functions::Convert64BitsStringIntoNumber(ptr.get());
            }
            if (!ProprelyEnded) {
                if (UserJournalisation) {
                	LogJournal << "warning 0003: some data in the inode Blocks is corrupted. Non critical error." << endl;
                }
                M->files[Dynamic_Functions::ConvertU32StringIntoString(path)][key] = value;
            }
        }
    }
    M->SizeOfTheData = filesystem::file_size(PathOfTheData);
    Blocks.reset();
    if (ReadWrite) {
        auto now = chrono::system_clock::now();
        time_t current_time = chrono::system_clock::to_time_t(now);
        stringstream ss;
        ss << std::put_time(std::localtime(&current_time), "%Y%m%d%H%M%S");
        string time_str = ss.str();
        int64_t IntDate = stoll(time_str);
        Write.seekp(48, ios::beg);
        Write.write(Dynamic_Functions::ConvertNumberInto64BitsString(IntDate).get(), 8);
    }
    auto Creation = make_unique<char[]>(8);
    Write.seekg(4, ios::beg);
    Write.read(Creation.get(), 8);
    M->CreationDate = Dynamic_Functions::Convert64BitsStringIntoNumber(Creation.get());
    if (ReadWrite) {
        auto now = chrono::system_clock::now();
        time_t current_time = chrono::system_clock::to_time_t(now);
        stringstream ss;
        ss << std::put_time(std::localtime(&current_time), "%Y%m%d%H%M%S");
        string time_str = ss.str();
        int64_t IntDate = stoll(time_str);
        M->LastMountDate = IntDate;
        Write.seekp(48, ios::beg);
        Write.write(Dynamic_Functions::ConvertNumberInto64BitsString(IntDate).get(), ios::beg);
    } else {
        auto Mount = make_unique<char[]>(8);
        Write.seekg(48, ios::beg);
        Write.read(Mount.get(), 8);
        M->LastMountDate = Dynamic_Functions::Convert64BitsStringIntoNumber(Mount.get());
    }
    auto Files = make_unique<char[]>(8);
    Write.seekg(20, ios::beg);
    Write.read(Files.get(), 8);
    M->NumberOfFiles = Dynamic_Functions::Convert64BitsStringIntoNumber(Files.get());
    auto Folders = make_unique<char[]>(8);
    Write.seekg(28, ios::beg);
    Write.read(Folders.get(), 8);
    M->NumberOfFolders = Dynamic_Functions::Convert64BitsStringIntoNumber(Folders.get());
    auto identifier = make_unique<char[]>(8);
    Write.seekg(12, ios::beg);
    Write.read(identifier.get(), 8);
    M->identifierNumber = Dynamic_Functions::Convert64BitsStringIntoNumber(identifier.get());
    return M;
}

__attribute__((visibility("default")))
vector<vector<string>> listFiles(vector<int64_t> fileHeaders, string Path, bool IgnoreErrors) __asm__("listFiles");
vector<vector<string>> listFiles(vector<int64_t> fileHeaders, string Path, bool IgnoreErrors) {
    fstream Write(Path, ios::in | ios::out | ios::binary);
    bool error = false;
    vector<vector<string>> output;
    for (int i = 0; i < fileHeaders.size(); i++) {
        int indexToRead = fileHeaders[i];
        auto index = make_unique<char[]>(8);
        Write.seekg(indexToRead, ios::beg);
        Write.read(index.get(), 8);
        int64_t converted_index = Dynamic_Functions::Convert64BitsStringIntoNumber(index.get());
        indexToRead += 8;
        vector<string> fileInformations;
        auto type = make_unique<char[]>(2);
        Write.seekg(indexToRead, ios::beg);
        Write.read(type.get(), 2);
        int16_t converted_type = Dynamic_Functions::Convert16BitsStringIntoNumber(type.get());
        fileInformations.push_back(to_string(converted_type));
        auto SizeBlocks = make_unique<char[]>(8);
        indexToRead += 2;
        Write.seekg(indexToRead, ios::beg);
        Write.read(SizeBlocks.get(), 8);
        int64_t converted_SizeBlocks = Dynamic_Functions::Convert64BitsStringIntoNumber(SizeBlocks.get());
        fileInformations.push_back(to_string(converted_SizeBlocks * SizeOfTheBlock));
        indexToRead += 8;
        if (converted_type > 0) {
            auto SizeContent = make_unique<char[]>(8);
            Write.seekg(indexToRead, ios::beg);
            Write.read(SizeContent.get(), 8);
            int64_t converted_SizeContent = Dynamic_Functions::Convert64BitsStringIntoNumber(SizeContent.get());
            fileInformations.push_back(to_string(converted_SizeContent));
        }
        indexToRead += 8;       
        string formatted_Date;
        auto creationDate = make_unique<char[]>(8);
        Write.seekg(indexToRead, ios::beg);
        Write.read(creationDate.get(), 8);
        int64_t converted_creationDate = Dynamic_Functions::Convert64BitsStringIntoNumber(creationDate.get());
        formatted_Date = Dynamic_Functions::ConvertDate(to_string(converted_creationDate));
        fileInformations.push_back(formatted_Date);
        indexToRead += 8;
        auto lastModificationDate = make_unique<char[]>(8);
        Write.seekg(indexToRead, ios::beg);
        Write.read(lastModificationDate.get(), 8);
        int64_t converted_lastModificationDate = Dynamic_Functions::Convert64BitsStringIntoNumber(lastModificationDate.get());
        formatted_Date = Dynamic_Functions::ConvertDate(to_string(converted_lastModificationDate));
        fileInformations.push_back(formatted_Date);
        indexToRead += 8;
        auto buffer = make_unique<char[]>(SizeOfTheBlock - 8);
        Write.seekg(indexToRead, ios::beg);
        Write.read(buffer.get(), SizeOfTheBlock - indexToRead);
        u32string path = Dynamic_Functions::ConvertUTX8IntoString(buffer.get());
        if (!(path.size() < SizeOfTheBlock - indexToRead)) {
            while (true) {
                indexToRead = converted_index;
                Write.seekg(indexToRead, ios::beg);
                Write.read(index.get(), 8);
                converted_index = Dynamic_Functions::Convert64BitsStringIntoNumber(index.get());
                indexToRead += 8;
                Write.seekg(indexToRead);
                Write.read(buffer.get(), SizeOfTheBlock - 8);
                u32string temp = Dynamic_Functions::ConvertUTX8IntoString(buffer.get());
                path += temp;
                if (temp.size() < SizeOfTheBlock - 8) {
                    indexToRead += temp.size();
                    break;
                }               
            }
        }
        if (path.size() == 0) {
            if (!IgnoreErrors) {
                error = true;
                break;
            }
        } else {
            fileInformations.push_back(Dynamic_Functions::ConvertU32StringIntoString(path));
        }
        vector<int> headerBlocks;
        auto temp = make_unique<char[]>(8);
        int size = 0;
        int number = 0;
        while (true) {
            if (size > 0) {
                memcpy(buffer.get(), temp.get(), size);
            }
            indexToRead += 8;
            int intIndex = SizeOfTheBlock - (indexToRead % SizeOfTheBlock);
            Write.seekg(indexToRead, ios::beg);
            Write.read(buffer.get() + size, intIndex);
            size = 0;
            bool Break;
            for (size_t i = 0; i < intIndex; i+=8) {
                if (i + 8 < intIndex % 8) {
                    size = intIndex % 8;
                    memcpy(buffer.get() + i, temp.get(), size);
                    break;
                } else {
                    memcpy(buffer.get() + i, temp.get(), 8);
                    int64_t converted_temp = Dynamic_Functions::Convert64BitsStringIntoNumber(temp.get());
                    if (number > 0) {
                        if (converted_temp > 0) {
                            headerBlocks.push_back(converted_temp);
                            number++;
                        }
                        if (number == 3) {
                            number = 0;
                        }
                    } else if (number < 0) {
                        if (converted_temp == -700 || converted_temp == -600) {
                            break;
                        } else if (converted_temp == -800) {
                            headerBlocks.push_back(converted_temp);
                            number++;
                        } else if (converted_temp != -900 && converted_temp > 0) {
                            headerBlocks.push_back(converted_temp);
                        } else if (converted_temp == 0) {
                            Break = true;
                        }
                    } 
                }
            }
            if (Break) {
                break;
            }
            indexToRead = converted_index;
            Write.seekg(indexToRead, ios::beg);
            Write.read(index.get(), 8);
            converted_index = Dynamic_Functions::Convert64BitsStringIntoNumber(index.get());
            indexToRead += 8;
        }
        bool firstBlock;
        u32string name;
        auto storage = make_unique<char[]>(SizeOfTheBlock);
        for (int i = 0; i < headerBlocks.size(); i++) {
            if (headerBlocks[i] == -800) {
                if (headerBlocks.size() >= i + 2) {
                    for (int o = headerBlocks[i + 1]; o < headerBlocks[i + 2]; o += SizeOfTheBlock) {
                        if (firstBlock) {
                            Write.seekg(o + 8 + 19, ios::beg);
                            Write.read(storage.get(), SizeOfTheBlock - 8 - 19); 
                            memset(storage.get(), 0, SizeOfTheBlock - 8 - 19);
                            u32string temp = Dynamic_Functions::ConvertUTX8IntoString(storage.get());
                            name += temp;
                            if (temp.size() < SizeOfTheBlock - 8 - 19) {
                                break;
                            }
                        } else if (o == headerBlocks[i + 1]) {
                            Write.seekg(o + 8, ios::beg);
                            Write.read(storage.get(), SizeOfTheBlock - 8); 
                            memset(storage.get(), 0, SizeOfTheBlock - 8);
                            u32string temp = Dynamic_Functions::ConvertUTX8IntoString(storage.get());
                            name += temp;
                            if (temp.size() < SizeOfTheBlock - 8) {
                                break;
                            }
                        } else {
                            Write.seekg(o, ios::beg);
                            Write.read(storage.get(), SizeOfTheBlock);
                            u32string temp = Dynamic_Functions::ConvertUTX8IntoString(storage.get());
                            name += temp;
                            if (temp.size() < SizeOfTheBlock) {
                                break;
                            }
                        }
                    }
                    i += 2;
                }
            } else if (headerBlocks[i] > 0) {
                if (firstBlock) {
                    Write.seekg(headerBlocks[i] + 8 + 19, ios::beg);
                    Write.read(storage.get(), SizeOfTheBlock - 8 - 19); // finish this and after that rewrite the function split in FShell.cpp
                    memset(storage.get(), 0, SizeOfTheBlock - 8 - 19);
                    u32string temp = Dynamic_Functions::ConvertUTX8IntoString(storage.get());
                    name += temp;
                    if (temp.size() < SizeOfTheBlock - 8 - 19) {
                        break;
                    }
                } else {
                    Write.seekg(headerBlocks[i] + 8, ios::beg);
                    Write.read(storage.get(), SizeOfTheBlock - 8); // finish this and after that rewrite the function split in FShell.cpp
                    memset(storage.get(), 0, SizeOfTheBlock - 8);
                    u32string temp = Dynamic_Functions::ConvertUTX8IntoString(storage.get());
                    name += temp;
                    if (temp.size() < SizeOfTheBlock - 8) {
                        break;
                    }
                }
            }
        }
        fileInformations.push_back(Dynamic_Functions::ConvertU32StringIntoString(name));
        output.push_back(fileInformations);
    }
    if (error) {
        return {{"Some errors occured when listing files. The partition may be corrupted"}};
    } 
    return output;
}