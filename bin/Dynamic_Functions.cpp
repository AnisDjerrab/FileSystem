#include "Dynamic_Functions.h"
#include <iostream>

using namespace std;

static void* API_Functions = nullptr;
static void* CreateMetaData = nullptr;
static void* FreeSpaceManagement = nullptr;
static void* GenerateFiles = nullptr;
static void* GenerateDirectories = nullptr;
static void* LowLevelDecoding = nullptr;
static void* LowLevelEncoding = nullptr;
static void* MountFS = nullptr;
static void* JournalisatonManagement = nullptr;

namespace Dynamic_Functions {
    void (*WriteInJournal)(unordered_map<int, int>&, unordered_map<int, int>&, int&, fstream&, unordered_map<int, int>&, vector<int>&, const char*, int);
    int64_t (*Convert64BitsStringIntoNumber)(const char*) = nullptr;
    int32_t (*Convert32BitsStringIntoNumber)(const char*) = nullptr;
    int16_t (*Convert16BitsStringIntoNumber)(const char*) = nullptr;
    string (*ConvertDate)(string);    
    vector<vector<string>> (*listFiles)(vector<int64_t> , string, bool);
    string (*ConvertU32StringIntoString)(u32string);
    u32string (*ConvertUTX8IntoString)(const char*) = nullptr;
    char (*ConvertNumberInto8BitsString)(int8_t) = nullptr;
    unique_ptr<char[]> (*ConvertNumberInto16BitsString)(int16_t) = nullptr;
    unique_ptr<char[]> (*ConvertNumberInto32BitsString)(int32_t) = nullptr;
    unique_ptr<char[]> (*ConvertNumberInto64BitsString)(int64_t) = nullptr;
    unique_ptr<char[]> (*ConvertStringIntoUTX8)(u32string) = nullptr;
    vector<int> (*listFreeBlocks)(bool, bool, unordered_map<int, int>&, unordered_map<int, int>&, unordered_map<int, int>&, vector<int>&, int&, fstream&, int) = nullptr;
    unique_ptr<WriteFile> (*CreateStruct)(unique_ptr<char[]>, unique_ptr<char[]>, unique_ptr<char[]>, unique_ptr<char[]>, unique_ptr<char[]>, string, char) = nullptr;
    void (*CreateFile)(bool, bool, WriteFile*, const char*,  unordered_map<int, int>&, int&, unordered_map<int, int>&) = nullptr;
    void (*DestroyStruct)(unique_ptr<WriteFile> ) = nullptr;
    unique_ptr<Settings> (*RecoverSettings)() = nullptr;
    void (*DestroySettings)(unique_ptr<Settings>) = nullptr;
    unique_ptr<MetaData> (*MappTheMetaData)(string, bool, bool) = nullptr;
    void (*ReadData)() = nullptr;
    void (*CreateGeneralMetaData)(string) = nullptr;
    void (*CreateIndexationTable)(string) = nullptr;
    void (*GenerateFirstFile)(string) = nullptr;
    void (*WriteConfigs)(Settings*) = nullptr;
    unique_ptr<WriteDirectory> (*CreateObject)(unique_ptr<char[]>, unique_ptr<char[]>, unique_ptr<char[]>, string, char) = nullptr;
    void (*CreateDirectory)(bool, bool, WriteDirectory*, const char*, unordered_map<int, int>&, int&, unordered_map<int, int>&) = nullptr;
    void (*DestroyObject)(unique_ptr<WriteDirectory> ) = nullptr;

    void initDynamiqueFunctions() {
        if (JournalisatonManagement != nullptr && API_Functions != nullptr && CreateMetaData != nullptr && FreeSpaceManagement != nullptr && GenerateFiles != nullptr && LowLevelDecoding != nullptr && LowLevelEncoding != nullptr && MountFS != nullptr && GenerateDirectories != nullptr) {
            return;            
        } else {
            if (API_Functions == nullptr) {
                API_Functions = dlopen("bin/API_Functions.so", RTLD_LAZY);
                RecoverSettings = reinterpret_cast<unique_ptr<Settings> (*)()>(dlsym(API_Functions, "RecoverSettings"));
                WriteConfigs = reinterpret_cast<void (*)(Settings*)>(dlsym(API_Functions, "WriteConfigs"));
                DestroySettings = reinterpret_cast<void (*)(unique_ptr<Settings>)>(dlsym(API_Functions, "DestroySettings"));
            }
            if (CreateMetaData == nullptr) {
                CreateMetaData = dlopen("bin/CreateMetaData.so", RTLD_LAZY);
                ReadData = reinterpret_cast<void (*)()>(dlsym(CreateMetaData, "ReadData"));
                CreateGeneralMetaData = reinterpret_cast<void (*)(string)>(dlsym(CreateMetaData, "CreateGeneralMetaData"));
                CreateIndexationTable = reinterpret_cast<void (*)(string)>(dlsym(CreateMetaData, "CreateIndexationTable"));
                GenerateFirstFile = reinterpret_cast<void (*)(string)>(dlsym(CreateMetaData, "GenerateFirstFile"));
            }
            if (FreeSpaceManagement == nullptr) {
                FreeSpaceManagement = dlopen("bin/FreeSpaceManagement.so", RTLD_LAZY);
                listFreeBlocks = reinterpret_cast<vector<int> (*)(bool, bool, unordered_map<int, int>&, unordered_map<int, int>&, unordered_map<int, int>&, vector<int>&, int&, fstream&, int)>(dlsym(FreeSpaceManagement, "listFreeBlocks"));
            }
            if (GenerateFiles == nullptr) {
                GenerateFiles = dlopen("bin/GenerateFiles.so", RTLD_LAZY);
                CreateStruct = reinterpret_cast<unique_ptr<WriteFile> (*)(unique_ptr<char[]>, unique_ptr<char[]>, unique_ptr<char[]>, unique_ptr<char[]>, unique_ptr<char[]>, string, char)>(dlsym(GenerateFiles, "CreateStruct"));
                CreateFile = reinterpret_cast<void (*)(bool, bool, WriteFile*, const char*, unordered_map<int, int>&, int&, unordered_map<int, int>&)>(dlsym(GenerateFiles, "CreateFile"));
                DestroyStruct = reinterpret_cast<void (*)(unique_ptr<WriteFile>)>(dlsym(GenerateFiles, "DestroyStruct"));
            }
            if (LowLevelDecoding == nullptr) {
                LowLevelDecoding = dlopen("bin/LowLevelDecoding.so", RTLD_LAZY);
                Convert64BitsStringIntoNumber = reinterpret_cast<int64_t (*)(const char*)>(dlsym(LowLevelDecoding, "Convert64BitsStringIntoNumber"));
                Convert32BitsStringIntoNumber = reinterpret_cast<int32_t (*)(const char*)>(dlsym(LowLevelDecoding, "Convert32BitsStringIntoNumber"));
                Convert16BitsStringIntoNumber = reinterpret_cast<int16_t (*)(const char*)>(dlsym(LowLevelDecoding, "Convert16BitsStringIntoNumber"));
                ConvertUTX8IntoString = reinterpret_cast<u32string (*)(const char*)>(dlsym(LowLevelDecoding, "ConvertUTX8IntoString"));   
                ConvertDate = reinterpret_cast<string (*)(string)>(dlsym(LowLevelDecoding, "ConvertDate"));
                ConvertU32StringIntoString = reinterpret_cast<string (*)(u32string)>(dlsym(LowLevelDecoding, "ConvertU32StringIntoString"));             
            }
            if (LowLevelEncoding == nullptr) {
                LowLevelEncoding = dlopen("bin/LowLevelEncoding.so", RTLD_LAZY);
                ConvertNumberInto8BitsString = reinterpret_cast<char (*)(int8_t)>(dlsym(LowLevelEncoding, "ConvertNumberInto8BitsString"));
                ConvertNumberInto16BitsString = reinterpret_cast<unique_ptr<char[]> (*)(int16_t)>(dlsym(LowLevelEncoding, "ConvertNumberInto16BitsString"));
                ConvertNumberInto32BitsString = reinterpret_cast<unique_ptr<char[]> (*)(int32_t)>(dlsym(LowLevelEncoding, "ConvertNumberInto32BitsString"));
                ConvertNumberInto64BitsString = reinterpret_cast<unique_ptr<char[]> (*)(int64_t)>(dlsym(LowLevelEncoding, "ConvertNumberInto64BitsString"));
                ConvertStringIntoUTX8 = reinterpret_cast<unique_ptr<char[]> (*)(u32string)>(dlsym(LowLevelEncoding, "ConvertStringIntoUTX8"));    
            }    
            if (MountFS == nullptr) {
                MountFS = dlopen("bin/MountFS.so", RTLD_LAZY);
                MappTheMetaData = reinterpret_cast<unique_ptr<MetaData> (*)(string, bool, bool)>(dlsym(MountFS, "MappTheMetaData"));
                listFiles = reinterpret_cast<vector<vector<string>> (*)(vector<int64_t>, string, bool)>(dlsym(MountFS, "listFiles"));
            }  
            if (GenerateDirectories == nullptr) {
                GenerateDirectories = dlopen("bin/GenerateFiles.so", RTLD_LAZY);
                CreateObject = reinterpret_cast<unique_ptr<WriteDirectory> (*)(unique_ptr<char[]>, unique_ptr<char[]>, unique_ptr<char[]>, string, char)>(dlsym(GenerateDirectories, "CreateObject"));
                CreateDirectory = reinterpret_cast<void (*)(bool, bool, WriteDirectory*, const char*,  unordered_map<int, int>&, int&, unordered_map<int, int>&)>(dlsym(GenerateDirectories, "CreateDirectory"));
                DestroyObject = reinterpret_cast<void (*)(unique_ptr<WriteDirectory>)>(dlsym(GenerateDirectories, "DestroyObject"));
            }
            if (JournalisatonManagement == nullptr) {
                JournalisatonManagement = dlopen("bin/JournalisationManagement.so", RTLD_LAZY);
                WriteInJournal = reinterpret_cast<void (*)(unordered_map<int, int>&, unordered_map<int, int>&, int&, fstream&, unordered_map<int, int>&, vector<int>&, const char*, int)>(dlsym(JournalisatonManagement, "WriteInJournal"));
            }
        }
    }

    void cleanupDynamiqueFunctions() {
        dlclose(API_Functions);
        dlclose(CreateMetaData);
        dlclose(FreeSpaceManagement);
        dlclose(GenerateFiles);
        dlclose(LowLevelDecoding);
        dlclose(LowLevelEncoding);
        dlclose(GenerateDirectories);
        dlclose(MountFS);
        dlclose(JournalisatonManagement);
        JournalisatonManagement = nullptr;
        WriteInJournal = nullptr;
        MountFS = nullptr;
        GenerateDirectories = nullptr;
        API_Functions = nullptr;
        CreateMetaData = nullptr;
        FreeSpaceManagement = nullptr;
        GenerateFiles = nullptr;
        LowLevelDecoding = nullptr;
        LowLevelEncoding = nullptr;
        Convert64BitsStringIntoNumber = nullptr;
        Convert32BitsStringIntoNumber = nullptr;
        ConvertUTX8IntoString = nullptr;
        ConvertNumberInto8BitsString = nullptr;
        ConvertNumberInto16BitsString = nullptr;
        ConvertNumberInto32BitsString = nullptr;
        ConvertNumberInto64BitsString = nullptr;
        ConvertStringIntoUTX8 = nullptr;
        listFreeBlocks = nullptr;
        CreateStruct = nullptr;
        CreateFile = nullptr;
        DestroyStruct = nullptr;
        RecoverSettings = nullptr;
        DestroySettings = nullptr;
        MappTheMetaData = nullptr;
        ReadData = nullptr;
        CreateGeneralMetaData = nullptr;
        CreateIndexationTable = nullptr;
        GenerateFirstFile = nullptr;
        WriteConfigs = nullptr;
        CreateDirectory = nullptr;
        CreateObject = nullptr;
        DestroyObject = nullptr;
        Convert16BitsStringIntoNumber = nullptr;
        ConvertDate = nullptr;
        listFiles = nullptr;
        ConvertU32StringIntoString = nullptr;
    }
}