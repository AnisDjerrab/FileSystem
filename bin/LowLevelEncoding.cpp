#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <bitset>
#include <cstring>
#include <cstdint>

using namespace std;

__attribute__((visibility("default")))
char ConvertNumberInto8BitsString(int8_t number) __asm__("ConvertNumberInto8BitsString");
char ConvertNumberInto8BitsString(int8_t number) {
    char output;
    output = static_cast<char>(number & 0xFF);
    return output;
}

__attribute__((visibility("default")))
unique_ptr<char[]> ConvertNumberInto16BitsString(int16_t number) __asm__("ConvertNumberInto16BitsString");
unique_ptr<char[]> ConvertNumberInto16BitsString(int16_t number) {
    auto output = make_unique<char[]>(2); 
    number = htobe16(number);
    memcpy(output.get(), &number, 2);
    return output;
}

__attribute__((visibility("default")))
unique_ptr<char[]> ConvertNumberInto32BitsString(int32_t number) __asm__("ConvertNumberInto32BitsString");
unique_ptr<char[]> ConvertNumberInto32BitsString(int32_t number) {
    auto output = make_unique<char[]>(4); 
    number = htobe32(number);
    memcpy(output.get(), &number, 4);
    return output;
}

__attribute__((visibility("default")))
unique_ptr<char[]> ConvertNumberInto64BitsString(int64_t number) __asm__("ConvertNumberInto64BitsString");
unique_ptr<char[]> ConvertNumberInto64BitsString(int64_t number) {
    auto output = make_unique<char[]>(8); 
    number = htobe64(number);
    memcpy(output.get(), &number, 8);
    return output;
} 

/* UTX8 stands for Ultra Text 8-bits.
 * This is a custom text encoding used in this FS, which support a large variety of special characters.  
 * It uses a 8-bits encoding, and is set to be the main text encoding system in this project.
 */

__attribute__((visibility("default")))
unique_ptr<char[]> ConvertStringIntoUTX8(u32string str) __asm__("ConvertStringIntoUTX8") ;
unique_ptr<char[]> ConvertStringIntoUTX8(u32string str) {
    auto output = make_unique<char[]>(str.size() + 1);
    static bool initialized = false;
    static unordered_map<char32_t, char> cases = {};
    static char placeholder = 1;
    if (!initialized) {
        initialized = true;
        u32string chars = U"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890/*-+.:;,!?()_=[]-~&#\"'\\{}^$²%§@|éèà \\n\\t\\r&`çêîôû£°€¢¥«»„“’µ¶·…†‡³±×÷¿¡âäåãõëïöüùñýÿčćđšžęłńśźżďěňřťůőűċġħżāēģīķļņūŵŷðþÂÄÅÃÕËÏÖÜÙÑÝŸČĆĐŠŽĘŁŃŚŹŻĎĚŇŘŤŮŐŰĊĠĦŻĀĒĢĪĶĻŅŪŴŶÐÞ–—‚‘©®™•ª₣₤₧₨₪⁴⁵≤≥≠≈√∞∑∫абвαβ";
        int value = 2;
        for (char c : chars) {
            cases[c] = value;
            value += 1;
        }
    }
    for (int i = 0; i < str.size(); i++) {
        bool found = false;
        if (cases.find(str[i]) != cases.end()) {
            output[i] = cases[str[i]];
        } else {
            output[i] = placeholder;
        }
    }
    output[str.size()] = '\0';
    return output;
}
