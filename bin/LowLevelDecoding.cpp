#include <iostream>
#include <cstring>
#include <memory>
#include <unordered_map>

using namespace std;

__attribute__((visibility("default")))
int64_t Convert64BitsStringIntoNumber(const char* input) __asm__("Convert64BitsStringIntoNumber");
int64_t Convert64BitsStringIntoNumber(const char* input) {
    int64_t output = 0;
    memcpy(&output, input, 8);
    return be64toh(output);
}
__attribute__((visibility("default")))
int32_t Convert32BitsStringIntoNumber(const char* input) __asm__("Convert32BitsStringIntoNumber");
int32_t Convert32BitsStringIntoNumber(const char* input) {
    int32_t output = 0;
    memcpy(&output, input, 4);
    return be32toh(output);
}
__attribute__((visibility("default")))
int16_t Convert16BitsStringIntoNumber(const char* input) __asm__("Convert16BitsStringIntoNumber");
int16_t Convert16BitsStringIntoNumber(const char* input) {
    int16_t output = 0;
    memcpy(&output, input, 2);
    return be16toh(output);
}


__attribute__((visibility("default")))
u32string ConvertUTX8IntoString(const char* str) __asm__("ConvertStringIntoUTX8") ;
u32string ConvertUTX8IntoString(const char* str) {
    u32string output = U"";
    static bool initialized = false;
    static unordered_map<char, char32_t> cases = {};
    if (!initialized) {
        initialized = true;
        u32string chars = U"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890/*-+.:;,!?()_=[]-~&#\"'\\{}^$²%§@|éèà \\n\\t\\r&`çêîôû£°€¢¥«»„“’µ¶·…†‡³±×÷¿¡âäåãõëïöüùñýÿčćđšžęłńśźżďěňřťůőűċġħżāēģīķļņūŵŷðþÂÄÅÃÕËÏÖÜÙÑÝŸČĆĐŠŽĘŁŃŚŹŻĎĚŇŘŤŮŐŰĊĠĦŻĀĒĢĪĶĻŅŪŴŶÐÞ–—‚‘©®™•ª₣₤₧₨₪⁴⁵≤≥≠≈√∞∑∫абвαβ";
        int value = 2;
        for (char32_t c : chars) {
            cases[value] = c;
            value += 1;
        }
    }
    for (int i = 0; i < strlen(str); i++) {
        bool found = false;
        if (cases.find(str[i]) != cases.end()) {
            output += cases[str[i]];
        } else {
            output += U"�";
        }
    }
    return output;
}


__attribute__((visibility("default")))
string ConvertDate(string input) __asm__("ConvertDate");
string ConvertDate(string input) {
    string output;
    output.reserve(19);
    output.append(input.substr(0, 4)).append("/")
        .append(input.substr(4, 6)).append("/")
        .append(input.substr(6, 8)).append(" ")
        .append(input.substr(8, 10)).append(":")
        .append(input.substr(10, 12)).append(":")
        .append(input.substr(12, 14));
    return output;
}


__attribute__((visibility("default")))
string ConvertU32StringIntoString(u32string input) __asm__("ConvertU32StringIntoString");
string ConvertU32StringIntoString(u32string input) {
    string output;
    output.reserve(input.size());
    for (char32_t c : input) {
        if (c <= 127) {
            output.push_back(static_cast<char>((c >> 24) & 255));
        } else if (c <= 2047) {
            output.push_back(static_cast<char>((c >> 24) & 255));
            output.push_back(static_cast<char>((c >> 16) & 255));
        } else if (c <= 65535) {
            output.push_back(static_cast<char>((c >> 24) & 255));
            output.push_back(static_cast<char>((c >> 16) & 255));
            output.push_back(static_cast<char>((c >> 8) & 255));
        } else {
            output.push_back(static_cast<char>((c >> 24) & 255));
            output.push_back(static_cast<char>((c >> 16) & 255));
            output.push_back(static_cast<char>((c >> 8) & 255));
            output.push_back(static_cast<char>(c & 255));
        }
    }
    return output;
}