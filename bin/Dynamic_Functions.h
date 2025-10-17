#ifndef  DYNAMIC_FUNCTIONS_H
#define DYNAMIC_FUNCTIONS_H 

#include <dlfcn.h>
#include <iostream>
#include <memory>
#include <vector>
#include <fstream>
#include <unordered_map>

using namespace std;

typedef struct WriteFile WriteFile;
typedef struct Settings Settings;
typedef struct MetaData MetaData;
typedef struct WriteDirectory WriteDirectory;

namespace Dynamic_Functions {
    extern int64_t (*Convert64BitsStringIntoNumber)(const char*);
    extern int32_t (*Convert32BitsStringIntoNumber)(const char*);
    extern int16_t (*Convert16BitsStringIntoNumber)(const char*);
    extern u32string (*ConvertUTX8IntoString)(const char*);
    extern void (*WriteInJournal)(unordered_map<int, int>&, unordered_map<int, int>&, int&, fstream&, unordered_map<int, int>&, vector<int>&, const char*, int);
    extern vector<vector<string>> (*listFiles)(vector<int64_t> , string, bool);
    extern string (*ConvertDate)(string);
    extern string (*ConvertU32StringIntoString)(u32string);
    extern char (*ConvertNumberInto8BitsString)(int8_t);
    extern unique_ptr<char[]> (*ConvertNumberInto16BitsString)(int16_t);
    extern unique_ptr<char[]> (*ConvertNumberInto32BitsString)(int32_t);
    extern unique_ptr<char[]> (*ConvertNumberInto64BitsString)(int64_t);
    extern unique_ptr<char[]> (*ConvertStringIntoUTX8)(u32string);
    extern vector<int> (*listFreeBlocks)(bool, bool, unordered_map<int, int>&, unordered_map<int, int>&, unordered_map<int, int>&, vector<int>&, int&, fstream&, int);
    extern unique_ptr<WriteFile> (*CreateStruct)(unique_ptr<char[]>, unique_ptr<char[]>, unique_ptr<char[]>, unique_ptr<char[]>, unique_ptr<char[]>, string, char);
    extern void (*CreateFile)(bool, bool, WriteFile*, const char*, unordered_map<int, int>&, int&, unordered_map<int, int>&);
    extern void (*DestroyStruct)(unique_ptr<WriteFile> );
    extern unique_ptr<WriteDirectory> (*CreateObject)(unique_ptr<char[]>, unique_ptr<char[]>, unique_ptr<char[]>, string, char);
    extern void (*CreateDirectory)(bool, bool, WriteDirectory*, const char*, unordered_map<int, int>&, int&, unordered_map<int, int>&);
    extern void (*DestroyObject)(unique_ptr<WriteDirectory> );
    extern unique_ptr<Settings> (*RecoverSettings)();
    extern void (*WriteConfigs)(Settings*);
    extern void (*DestroySettings)(unique_ptr<Settings>);
    extern unique_ptr<MetaData> (*MappTheMetaData)(string, bool, bool);
    extern void (*ReadData)();
    extern void (*CreateGeneralMetaData)(string);
    extern void (*CreateIndexationTable)(string);
    extern void (*GenerateFirstFile)(string);
    extern void initDynamiqueFunctions();
    extern void cleanupDynamiqueFunctions();
}

#endif