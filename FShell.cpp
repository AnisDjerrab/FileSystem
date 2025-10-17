#include <iostream>
#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include <termios.h>
#include <unistd.h>
#include <csignal>
#include "bin/Dynamic_Functions.h"


using namespace std;

struct Settings {
    unordered_map<string, string> variables = {};
    unordered_map<string, vector<string>> lists = {};
    unordered_map<string, unordered_map<string, string>> maps = {};
};
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

static struct termios origin_term;

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &origin_term);
    termios term = origin_term;
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 1; 
    term.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}
void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &origin_term);
}


void interrupt(int signal) {
    Dynamic_Functions::cleanupDynamiqueFunctions();
    disableRawMode();
    exit(0);
}

// This's the essential function which compilates the code into ByteCode. 
unordered_map<bool, vector<string>> parseCommand(string input) {
    bool Root = false;
    vector<string> tokens;
    bool inQuotes = false;
    string element;
    unordered_map<bool, vector<string>> output;
    for (int i = 0; i < input.size(); i++) {
        if (inQuotes) {
            if (input[i] == '"' || input[i] == '\'') {
                if (!(input.size() - 1 == i)) {
                    for (int o = i; o < input.size(); o++) {
                        if (input[i] == ' '|| input[i] || '\t' || input[i] == '\n') {
                            break;
                        }
                        i++;
                    }
                }
                inQuotes = false;
                tokens.push_back(element);
            } else {
                element += input[i];
            }
        } else {
            if (input[i] == ' ' || input[i] == '\t' || input[i] == '\n') {
                if (element.size() > 0) {
                    tokens.push_back(element);
                }
                element = "";
            } else if (input[i] == '"' || input[i] == '\'') {
                if (element.size() > 0) {
                    element += input[i];
                } else {
                    inQuotes = true;
                }
            } else if (input[i] == ';')  {
                output[Root] = tokens;
                tokens.clear();
                Root = false;
            } else {
                element += input[i];
            }
        }
    }
    if (element.size() > 0) {
        tokens.push_back(element);
        output[Root] = tokens;
    }
    return output;
}

int main() {
    enableRawMode();
    signal(SIGINT, interrupt);
    signal(SIGTERM, interrupt);
    signal(SIGQUIT, interrupt);
    Dynamic_Functions::initDynamiqueFunctions();
    unique_ptr<Settings> RetrievedSettings = Dynamic_Functions::RecoverSettings();
    cout << "Welcome to FShell version " << RetrievedSettings->variables["version"] << endl;
    cout << "Type 'help' to list avaliable commands and options." << endl;
    string currentPath = "~";
    unordered_map<string, vector<string>> fileDetailedInfos;
    unordered_map<string, unique_ptr<MetaData>> mountedPartitions;
    unordered_map<string, string> externalPathOfThePartition;
    while (true) {
        cout << currentPath << " " << ">> ";
        string line;
        char c;
        int index = 0;
        while (true) {
            c = getchar();
            if (c == '\n') {
                cout << c;
                break;
            } else if (c == '\033') {
                c = getchar();
                if (c == EOF) {
                    break;
                } else if (c == '[') {
                    c = getchar();
                    if (c == 'C') {
                        if (index < line.size()) {
                            cout << "\033[C";
                            index++;
                        }
                    } else if (c == 'D') {
                        if (index > 0) {
                            cout << "\033[D";
                            index -= 1;
                        }
                    } else if (c != 'A' && c != 'B') {
                        cout << '\033' << '[' << c;
                    }
                } else {
                    cout << '\033' << c;
                }
            } else if (c == '\t') {
                line.insert(index, "   ");
                for (int i = 0; i < index; i++) {
                    cout << "\b";
                }
                cout << line;
                for (int i = 0; i < line.size() - index - 3; i++) {
                    cout << "\b";
                }
                index += 3;
            } else if (c == '\b' || c == 127) {
                if (index > 0 && line.size() > 0) {
                    index -= 1;
                    line.erase(index, 1);
                    for (int i = 0; i < index + 1; i++) {
                        cout << "\b";
                    }
                    cout << line << " ";
                    for (int i = 0; i < line.size() - index + 1; i++) {
                        cout << "\b";
                    }
                }
            } else {
                line.insert(line.begin() + index, c);
                for (int i = 0; i < index; i++) {
                    cout << "\b";
                }
                cout << line;
                for (int i = 0; i < line.size() - index - 1; i++) {
                    cout << "\b";
                }
                index++;
            }
        }
        bool Break = false;
        unordered_map<bool, vector<string>> output = parseCommand(line);
        for (const auto& [Root, Command] : output) {
            if (Command.size() >= 1) {
                if (Command[0] == "mount") {
                    bool RW = true;
                    bool IgnoreErrors = false;
                    string Path = "";
                    bool Error = false;
                    for (int i = 1; i < Command.size(); i++) {
                        if (Command[i][0] == '#') {
                            break;
                        } else if (Command[i] == "-o" || Command[i] == "--option") {
                            i++;
                            if (!(i < Command.size())) {
                                cout << "\033[1;31mOption '" << Command[i - 1] << "' not recognized. Try '" << Command[i - 1] << " ro' or '" << Command[i-1] << " rw'\033[0m" << endl;
                                Error = true; 
                                break;
                            } else {
                                if (Command[i] == "ro" || Command[i] == "RO") {
                                    bool RW = false;
                                } else if (Command[i] != "rw" || Command[i] != "RW") {
                                    cout << "\033[1;31mOption '" << Command[i] << "' not recognized. Try '" << Command[i - 1] << " ro' or '" << Command[i-1] << " rw'\033[0m" << endl;
                                    Error = true; 
                                    break;
                                }
                            }
                        } else if (Command[i] == "--force" || Command[i] == "-f") {
                            IgnoreErrors = true;
                        } else if (Command[i] == "--import" || Command[i] == "-i") {
                            i++;
                            if (!(i < Command.size())) {
                                cout << "\033[1;31mOption '" << Command[i - 1] << "' not recognized. Try '" << Command[i - 1] << " $PathOfTheFile$'\033[0m" << endl;
                                Error = true; 
                                break;
                            } else {
                                ifstream file(Path, ios::out | ios::in);
                                if (!file) {
                                    Error = true;
                                } 
                                file.close();    
                            }
                        } else if (Command[i] != "--default" && Path.size() == 0) {
                            if (find(RetrievedSettings->lists["internal_partitions"].begin(), RetrievedSettings->lists["internal_partitions"].end(), Command[i]) != RetrievedSettings->lists["internal_partitions"].end()) {
                                filesystem::path file("partitions/" + Command[i]);
                                if (filesystem::exists(file)) {
                                    Path = Command[i];
                                } else {
                                    cout << "\033[1;31mFile 'partitions/" << Command[i] << "' doesn't exist.\033[0m" << endl;
                                    Error = true;
                                    break;
                                }
                            } else if (find(RetrievedSettings->lists["external_partitions"].begin(), RetrievedSettings->lists["external_partitions"].end(), Command[i]) != RetrievedSettings->lists["external_partitions"].end()) {                      
                                filesystem::path file(Command[i]);
                                if (filesystem::exists(file)) {
                                    Path = Command[i];
                                } else {
                                    cout << "\033[1;31mFile '" << Command[i] << "' doesn't exist.\033[0m" << endl; // adapt mount command to accomodiat the inability to mount already mouned functions
                                    Error = true;
                                    break;
                                }
                            } else {
                                cout << "\033[1;31mFile '" << Command[i] << "' doesn't exist.\033[0m" << endl;
                                Error = true;
                                break;
                            }
                        } else {
                            cout << "\033[1;31mUnknown option '" << Command[i] << "'.\033[0m" << endl;
                        }
                    }
                    for (const auto& pair : externalPathOfThePartition) {
                        if (pair.second == Path) {
                            Error = true;
                            cout << "\033[1;31mCannot mount already mounted partition '" << Path << "'.\033[0m" << endl;
                            break;
                        }
                    }
                    if (!Error) {
                        if (Path != "") {
                            string letter = "";
                            while (true) {
                                if (letter.size() == 0) {
                                    letter += char(65);
                                } else {
                                    bool AddAnotherCharacter = true;
                                    for (int i = 0; i < letter.size(); i++) {
                                        if (letter[i] < 90) {
                                            letter[i] = letter[i] + 1;
                                            AddAnotherCharacter = false;
                                        }
                                    }
                                    if (AddAnotherCharacter) {
                                        for (int i = 0; i < letter.size(); i++) {
                                            letter[i] = char(65);
                                        }
                                        letter += char(65);
                                    }
                                }
                                if (mountedPartitions.find(letter + ":") == mountedPartitions.end()) {
                                    break;
                                }
                            }  
                            mountedPartitions[letter + ":"] = Dynamic_Functions::MappTheMetaData(Path, RW, IgnoreErrors);
                            externalPathOfThePartition[letter + ":"] = Path;
                            cout << "\033[1;32mThe partition '" << Path << "' was succesfully mounted at drive letter '" << letter << ":/'.\033[0m" << endl;
                        } else {
                            cout << "\033[1;31mPath not specified.\033[0m" << endl;
                        }
                    }
                } else if (Command[0] == "exit") {
                    if (Command.size() == 1) {
                        Break = true;
                        break;
                    } else {
                        if (!(Command[1][0] == '#')) {
                            cout << "\033[1;31mInvalid option '" << Command[1] << "'.\033[0m" << endl;
                        } else {
                            Break = true;
                            break;
                        }
                    }
                } else if (Command[0] == "echo") {
                    string echo = "";
                    int beginning = 3;
                    for (int i = 0; i < line.size(); i++) {
                        if (line[i] == '>') {
                            break;
                        }
                        if (line[i] == ' ' && beginning == 2) {
                            beginning = 1;
                        } else if (line[i] == 'e' && beginning == 3) {
                            beginning = 2;
                        } else if (line[i] != ' ' && beginning == 1) {
                            beginning = 0;
                            echo += line[i];
                        } else if (beginning == 0) {
                            echo += line[i];
                        }
                    }
                    cout << echo << endl;
                } else if (Command[0] == "pwd") {
                    if (Command.size() == 1) {
                        cout << currentPath << endl;
                    } else {
                        if (!(Command[1][0] == '#')) {
                            cout << "\033[1;31mInvalid option '" << Command[1] << "'.\033[0m" << endl;
                        } else {
                            cout << currentPath << endl;
                        }
                    }
                } else if (Command[0] == "help") {
                    if (Command.size() == 1) {
                        cout << "FShell version " << RetrievedSettings->variables["version"] << " , the terminal emulator for CLI FS utilites.\nCommands :\n" << endl;
                    } else {
                        if (!(Command[1][0] == '#')) {
                            cout << "\033[1;31mInvalid option '" << Command[1] << "'.\033[0m" << endl;
                        }
                    }
                } else if (Command[0] == "partctl") {
                    string Path = "";
                    bool external = false;
                    bool create = false;
                    bool format = false;
                    bool delete_ = false;
                    bool Error = false;
                    for (int i = 1; i < Command.size(); i++) {
                        if (Command[i] == "--create" || Command[i] == "-c") {
                            create = true;
                        } else if (Command[i] == "--format" || Command[i] == "-F") {
                            format = true;
                        } else if (Command[i] == "--external" || Command[i] == "-e") {
                            external = true;
                        } else if (Command[i] == "--delete" || Command[i] == "-d") {
                            delete_ = true;
                        } else if (Path.size() == 0) {
                            Path = Command[i];
                        } else {
                            cout << "\033[1;31mUnknown option '" << Command[i] << "'.\033[0m" << endl;
                        }
                    }
                    if (external) {
                        if (format || delete_) {
                            filesystem::path file(Path);
                            if (!(filesystem::exists(file))) {
                                Error = true;
                                cout << "\033[1;31mInvalid path '" << Path << "'.\033[0m" << endl;
                            }
                        } else if (create) {
                            ifstream file(Path, ios::out | ios::trunc);
                            if (!file) {
                                Error = true;
                            } 
                            file.close();                    }
                    }
                    if (Path.size() > 0 && (external == true || create == true || format == true || delete_ == true) && Error == false) {
                        if (create) {
                            if (find(RetrievedSettings->lists["internal_partitions"].begin(), RetrievedSettings->lists["internal_partitions"].end(), Path) != RetrievedSettings->lists["internal_partitions"].end() || find(RetrievedSettings->lists["external_partitions"].begin(), RetrievedSettings->lists["external_partitions"].end(), Path) != RetrievedSettings->lists["external_partitions"].end()) {
                                cout << "\033[1;31mPartition '" << Path << "' already exists.\033[0m" << endl;
                            } else {
                                if (external) {
                                    Dynamic_Functions::CreateGeneralMetaData(Path);
                                    Dynamic_Functions::CreateIndexationTable(Path);
                                    Dynamic_Functions::GenerateFirstFile(Path);
                                    RetrievedSettings->lists["external_partitions"].push_back(Path);
                                } else {
                                    Dynamic_Functions::CreateGeneralMetaData("partitions/" + Path);
                                    Dynamic_Functions::CreateIndexationTable("partitions/" + Path);
                                    Dynamic_Functions::GenerateFirstFile("partitions/" + Path);
                                    RetrievedSettings->lists["internal_partitions"].push_back(Path);
                                }
                                Dynamic_Functions::WriteConfigs(RetrievedSettings.get());
                                string letter = "";
                                while (true) {
                                    if (letter.size() == 0) {
                                        letter += char(65);
                                    } else {
                                        bool AddAnotherCharacter = true;
                                        for (int i = 0; i < letter.size(); i++) {
                                            if (letter[i] < 90) {
                                                letter[i] = letter[i] + 1;
                                                AddAnotherCharacter = false;
                                            }
                                        }
                                        if (AddAnotherCharacter) {
                                            for (int i = 0; i < letter.size(); i++) {
                                                letter[i] = char(65);
                                            }
                                            letter += char(65);
                                        }
                                    }
                                    if (mountedPartitions.find(letter + ":") == mountedPartitions.end()) {
                                        break;
                                    }

                                }    
                                if (external) {
                                    mountedPartitions[letter + ":"] = Dynamic_Functions::MappTheMetaData(Path, true, true);
                                } else {
                                    mountedPartitions[letter + ":"] = Dynamic_Functions::MappTheMetaData("partitions/" + Path, true, true);
                                }
                                externalPathOfThePartition[letter + ":"] = Path;
                                cout << "\033[1;32mThe partition '" << Path << "' was succesfully created and mounted at drive letter '" << letter << ":/'.\033[0m" << endl;
                            }
                        } else if (format) {
                            bool Continue = true;
                            for (const auto& pair : externalPathOfThePartition) {
                                if (pair.second == Path) {
                                    Continue = false;
                                }
                            }
                            if (Continue) {
                                bool modified = false;
                                if (external) {
                                    Dynamic_Functions::CreateGeneralMetaData(Path);
                                    Dynamic_Functions::CreateIndexationTable(Path);
                                    Dynamic_Functions::GenerateFirstFile(Path);
                                    RetrievedSettings->lists["external_partitions"].push_back(Path);
                                    modified = true;
                                } else {
                                    if (find(RetrievedSettings->lists["internal_partitions"].begin(), RetrievedSettings->lists["internal_partitions"].end(), Path) == RetrievedSettings->lists["partitions"].end()) {
                                        Error = true;
                                    } else {
                                        Dynamic_Functions::CreateGeneralMetaData("partitions/" + Path);
                                        Dynamic_Functions::CreateIndexationTable("partitions/" + Path);
                                        Dynamic_Functions::GenerateFirstFile("partitions/" + Path);
                                    }
                                }
                                if (modified) {
                                    Dynamic_Functions::WriteConfigs(RetrievedSettings.get());
                                }
                                if (!Error) {
                                    string letter = "";
                                    while (true) {
                                        if (letter.size() == 0) {
                                            letter += char(65);
                                        } else {
                                            bool AddAnotherCharacter = true;
                                            for (int i = 0; i < letter.size(); i++) {
                                                if (letter[i] < 90) {
                                                    letter[i] = letter[i] + 1;
                                                    AddAnotherCharacter = false;
                                                }
                                            }
                                            if (AddAnotherCharacter) {
                                                for (int i = 0; i < letter.size(); i++) {
                                                    letter[i] = char(65);
                                                }
                                                letter += char(65);
                                            }
                                        }
                                        if (mountedPartitions.find(letter + ":") == mountedPartitions.end()) {
                                            break;
                                        }
                                    }    
                                    mountedPartitions[letter + ":"] = Dynamic_Functions::MappTheMetaData(Path, true, true);
                                    externalPathOfThePartition[letter + ":"] = Path;
                                    cout << "\033[1;32mThe partition '" << Path << "' was succesfully formatted and mounted at drive letter '" << letter << ":/'.\033[0m" << endl;
                                }
                            } else {
                                cout << "\033[1;31mPartition '" << Path << "' is mounted.\033[0m" << endl;
                            }
                        } else if (delete_) {
                            bool Continue = true;
                            for (const auto& pair : externalPathOfThePartition) {
                                if (pair.second == Path) {
                                    Continue = false;
                                }
                            }
                            if (Continue) {
                                if (external) {
                                    if (find(RetrievedSettings->lists["external_partitions"].begin(), RetrievedSettings->lists["external_partitions"].end(), Path) != RetrievedSettings->lists["external_partitions"].end()) { 
                                        RetrievedSettings->lists["external_partitions"].erase(remove(RetrievedSettings->lists["external_partitions"].begin(), RetrievedSettings->lists["external_partitions"].end(), Path), RetrievedSettings->lists["external_partitions"].end());
                                        Dynamic_Functions::WriteConfigs(RetrievedSettings.get());
                                    }
                                    filesystem::path file(Path);
                                    if (filesystem::exists(file)) {
                                        filesystem::remove(file);
                                        cout << "\033[1;32mThe partition '" << Path << "' was succesfully deleted.\033[0m" << endl;
                                    }
                                } else {
                                    if (find(RetrievedSettings->lists["internal_partitions"].begin(), RetrievedSettings->lists["internal_partitions"].end(), Path) != RetrievedSettings->lists["internal_partitions"].end()) { 
                                        RetrievedSettings->lists["internal_partitions"].erase(remove(RetrievedSettings->lists["internal_partitions"].begin(), RetrievedSettings->lists["internal_partitions"].end(), Path), RetrievedSettings->lists["internal_partitions"].end());
                                        filesystem::path file("partitions/" + Path);
                                        Dynamic_Functions::WriteConfigs(RetrievedSettings.get());
                                        if (filesystem::exists(file)) {
                                            filesystem::remove(file);
                                            cout << "\033[1;32mThe partition '" << Path << "' was succesfully deleted.\033[0m" << endl;
                                        }
                                    } else {
                                        cout << "\033[1;31mfile '" << Path << "' doesn't exist in the configuration file. Try options '--external'.\033[0m" << endl;
                                    }
                                }
                            }
                        } else {
                            cout << "\033[1;31mPartition '" << Path << "' is mounted.\033[0m" << endl;
                        }
                    } else if (Error == false) {
                        cout << "\033[1;31mNo option specified.\033[0m" << endl;
                    }
                } else if (Command[0] == "umount") {
                    string input;
                    if (Command.size() > 2) {
                        cout << "\033[1;31mUnknown option '" << Command[2] << "'.\033[0m" << endl;
                    } else {
                        if (find(RetrievedSettings->lists["internal_partitions"].begin(), RetrievedSettings->lists["internal_partitions"].end(), Command[1]) != RetrievedSettings->lists["internal_partitions"].end()) {
                            input = "partitions/" + Command[1];
                        } else {
                            input = Command[1];
                        }
                        string key;
                        for (const auto& pair : externalPathOfThePartition) {
                            if (pair.second == input) {
                                key = pair.first;
                                break;
                            } else if (pair.first == input) {
                                key = pair.first;
                                break;
                            }
                        }
                        for (const auto& pair : fileDetailedInfos) {
                            if (pair.first.substr(0, key.size()) == key) {
                                fileDetailedInfos.erase(pair.first);
                            }
                        }
                        if (key.size() > 0) {
                            externalPathOfThePartition.erase(key);
                            mountedPartitions.erase(key);
                            if (currentPath.substr(0, key.size()) == key) {
                                currentPath = "~";
                            }
                            cout << "\033[1;32mThe partition '" << input << "' was succesfully unmounted.\033[0m" << endl;
                        } else {
                            cout << "\033[1;31mThe partition '" << input << "' is not mounted.\033[0m" << endl;
                        }
                    }
                } else {
                    cout << "\033[1;31mCommand '" << Command[0] << "' not recognized.\033[0m" << endl;
                }
            }
            if (fileDetailedInfos.size() > 100) {
                for (const auto& pair : fileDetailedInfos) {
                    if (pair.first != currentPath) {
                        fileDetailedInfos.erase(pair.first);
                    }
                    if (fileDetailedInfos.size() <= 100) {
                        break;
                    }
                }
            }
        }
        if (Break) {
            break;
        }
    }
    Dynamic_Functions::cleanupDynamiqueFunctions();
    disableRawMode();
    return 0;
}
