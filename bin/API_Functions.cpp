/* This Code is responsible of parsing the configuration file that comes with the file system.
 * It contains various informations neede to run the file explorer, including the .dll or .so and .jar responsible for various tasks
 * All the error are logged into the LogJournal.log log file. if something doesn't work correctly, take a look at it.
 * it uses a simplified syntax semi-json semi-yaml, where identation is nor required but {} and [] are. it's not recursive.
 * the syntax looks like this : 
 *  variable:valueOfTheVariable
 *  #comment
 *  variable  :valueOfTheVariable;, # it works! spaces, comments, and various punctuation signs are ignored
 *  list=[
 *      element,
 *      element2
 *  ]
 *  map={
 *      key:value,
 *      secondKey:value
 *  }
*/




#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <memory>

using namespace std;

struct Settings {
    unordered_map<string, string> variables = {};
    unordered_map<string, vector<string>> lists = {};
    unordered_map<string, unordered_map<string, string>> maps = {};
};

__attribute__((visibility("default")))
unique_ptr<Settings> RecoverSettings() __asm__("RecoverSettings");
unique_ptr<Settings> RecoverSettings() {
    auto S = make_unique<Settings>();
    fstream ConfigReader("bin/Configs.cfg", ios::in);
    fstream LogJournalWriter("bin/LogJournal.log", ios::out);
    if (!ConfigReader.is_open()) {
        LogJournalWriter << "error 0014: no valid Configuration file found. \n";
        S.reset();
        return nullptr;
    } else {
        LogJournalWriter << "the Configuration file was succesfully opened! \n";
    }
    string line = "";
    bool isInList = false;
    bool isInMap = false;
    unordered_map<string, string> temp_variables = {};
    unordered_map<string, vector<string>> temp_lists = {};
    unordered_map<string, unordered_map<string, string>> temp_maps = {};
    int counter = 0;
    string tempNameOfTheList = "";
    string tempNameOfTheMap = "";
    while(getline(ConfigReader, line)) {
        counter += 1;
        bool pass = true;
        if (line == "") {
            pass = false;
        } else {
            for (int i = 0; i < line.size(); i++) {
                if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n' && line[i] != '#') {
                    break;
                } else if (line[i] == '#') {
                    pass = false;
                }
            }
        }
        if (pass) {
            if (isInList == false && isInMap == false) {
                bool Continue = true;
                bool IsInListTemp = false;
                bool IsInMapTemp = false;
                for (int i = 0; i < line.size(); i++) {
                    if (line[i] == '=') {
                        Continue = false;
                    } 
                }
                if (Continue) {
                    vector<string> result = {};
                    string variable = "";
                    for (int i = 0; i < line.size(); i++) {
                        if (line[i] == ':') {
                            result.push_back(variable);
                            variable = "";
                        } else if (line[i] == '#') {
                            break;
                        } else if (line[i] != '\n' && line[i] != ' ' && line[i] != '\t' && line[i] != ';' && line[i] != ',') {
                            variable += line[i];
                        } 
                    }
                    result.push_back(variable);
                    if (result.size() > 2) {
                        LogJournalWriter << "error 0001: line Containing 3 or more entries in the configuration file: " << to_string(counter);                            
                        LogJournalWriter << "\n";

                    } else if (result.size() < 2) { 
                        LogJournalWriter << "error 0012: line Containing 1 or less entries in the configuration file: " << to_string(counter);
                        LogJournalWriter << "\n";
                    } else {
                        temp_variables[result[0]] = result[1];
                    }
                } else {
                    string lineTemp = line;
                    bool IsDone = false;
                    for (int i = 0; i < line.size(); i++) {
                        if (line[i] == '[') {
                            IsInListTemp = true;
                            IsDone = true;
                        } else if (line[i] == '{') {
                            IsInMapTemp = true;
                            IsDone = true;
                        }
                    }
                    if (!IsDone) {
                        counter += 1;
                        getline(ConfigReader, line);
                        for (int i = 0; i < line.size(); i++) {
                            if (line[i] == '[') {
                                IsDone = true;
                                IsInListTemp = true;
                            } else if (line[i] == '{') {
                                IsDone = true;
                                IsInMapTemp = true;
                            } else if (line[i] == '#') {
                                break;
                            }
                        }
                    }
                    if (!IsDone) {
                        LogJournalWriter << "error 0015: config file is corrupted. \n";
                    }
                    if (IsInListTemp) {
                        string variable = "";
                        int IndexOfTheEqual = -1;
                        int IndexOfTheBracket = -1;
                        bool Stop = false;
                        bool ErrorEncountered = false;
                        for (int i = 0; i < lineTemp.size(); i++) {
                            if (!Stop) {
                                if (lineTemp[i] == '=') {
                                    Stop = true;
                                    IndexOfTheEqual = i;
                                } else if (lineTemp[i] != ' ' && lineTemp[i] != '\t') {
                                    variable += lineTemp[i];
                                }
                            }
                            if (lineTemp[i] == '[') {
                                IndexOfTheBracket = i;
                            } else if (lineTemp[i] == '=') {
                                for (int o = i + 1; o < lineTemp.size(); o++) {
                                    if (lineTemp[o] == '#') {
                                        break;
                                    } else if (lineTemp[o] != ' ' && lineTemp[o] != '\n' && lineTemp[o] != '[' && lineTemp[o] != '\t' && lineTemp[o] != '\r') {
                                        ErrorEncountered = true;
                                        break;
                                    }
                                }
                            }
                        }
                        if (ErrorEncountered) {
                            LogJournalWriter << "error 0002: invalid character after '=' at line: " << to_string(counter);
                            LogJournalWriter << "\n";
                        } else if (variable == "") {
                            LogJournalWriter << "error 0003: no valid name was given at line: " << to_string(counter);
                            LogJournalWriter << "\n";
                        } else if (IndexOfTheBracket < IndexOfTheEqual && IndexOfTheBracket != -1) {
                            LogJournalWriter << "error 0004: invalid location of the '[' at line: " << to_string(counter);
                            LogJournalWriter << "\n";
                        } else if (IndexOfTheBracket == -1 && IndexOfTheEqual == -1) {
                            LogJournalWriter << "error 0005: no '[' or '=' found at line: " + to_string(counter);
                            LogJournalWriter << "\n";
                        } else {
                            isInList = true;
                            bool foundOnlyOneCaracter = true;
                            if (variable.size() != 0) {
                                for (int y = 0; y < variable.size(); y++) {
                                    if (variable[y] != '[' && variable[y] != ' ' && variable[y] != '\t' && variable[y] != '\r' && variable[y] != '#') {
                                        foundOnlyOneCaracter = false;
                                        break;
                                    }
                                    if (variable[y] == '#') {
                                        break;
                                    }
                                }
                            }
                            if (!foundOnlyOneCaracter) {
                                tempNameOfTheList = variable;
                            }
                            temp_lists[tempNameOfTheList] = {};
                        }
                    } else if (IsInMapTemp) {
                        string variable = "";
                        int IndexOfTheEqual = -1;
                        int IndexOfTheBracket = -1;
                        bool Stop = false;
                        bool ErrorEncountered = false;
                        for (int i = 0; i < lineTemp.size(); i++) {
                            if (!Stop) {
                                if (lineTemp[i] == '=') {
                                    Stop = true;
                                    IndexOfTheEqual = i;
                                } else if (lineTemp[i] != ' ' && lineTemp[i] != '\t') {
                                    variable += lineTemp[i];
                                }
                            }
                            if (lineTemp[i] == '{') {
                                IndexOfTheBracket = i;
                            } else if (lineTemp[i] == '=') {
                                for (int o = i; o < lineTemp.size(); o++) {
                                    if (lineTemp[o] == '#') {
                                        break;
                                    } else if (lineTemp[o] != ' ' && lineTemp[o] != '\n' && lineTemp[o] != '{' && lineTemp[o] != '=' && lineTemp[o] != '\r' && lineTemp[o] != '\t') {
                                        ErrorEncountered = true;
                                        break;
                                    }
                                }
                            }
                        }
                        if (ErrorEncountered) {
                            LogJournalWriter << "error 0006: invalid character after '=' at line: " << to_string(counter);
                            LogJournalWriter << "\n";
                        } else if (variable == "") {
                            LogJournalWriter << "error 0007: no valid name was given at line: " << to_string(counter);
                            LogJournalWriter << "\n";
                        } else if (IndexOfTheBracket < IndexOfTheEqual && IndexOfTheBracket != -1) {
                            LogJournalWriter << "error 0008: invalid location of the '{' at line: " << to_string(counter);
                            LogJournalWriter << "\n";
                        } else if (IndexOfTheBracket == -1 && IndexOfTheEqual == -1) {
                            LogJournalWriter << "error 0009: no '{' or '=' found at line: " << to_string(counter);
                            LogJournalWriter << "\n";
                        } else {
                            isInMap = true;
                            bool foundOnlyOneCaracter = true;
                            if (variable.size() != 0) {
                                for (int y = 0; y < variable.size(); y++) {
                                    if (variable[y] != '{' && variable[y] != ' ' && variable[y] != '\t' && variable[y] != '\r' && variable[y] != '#') {
                                        foundOnlyOneCaracter = false;
                                        break;
                                    }
                                    if (variable[y] == '#') {
                                        break;
                                    }
                                }
                            }
                            if (!foundOnlyOneCaracter) {
                                tempNameOfTheMap = variable;
                            }
                            temp_maps[variable] = {};
                        }
                    }
                }
            } 
        } 
        if (isInList) {
            while (getline(ConfigReader, line)) {
                counter += 1;
                bool Continue = false;
                int CommentIndex = -1;
                for (int i = 0; i < line.size(); i++) {
                    if (line[i] == '#') {
                        CommentIndex = i;
                        break;
                    }
                }
                for (int i = 0; i < line.size(); i++) {
                    if (line[i] != '\t' && line[i] != '#' && line[i] != ' ' && line[i] != ']') {
                        if (CommentIndex == -1) {
                            Continue = true;
                            break;
                        } else if (i < CommentIndex) {
                            Continue = true;
                            break;
                        }
                    }
                }
                if (!Continue) {
                    tempNameOfTheList = "";
                    isInList = false;
                    break;
                }
                string variable = "";
                for (int i = 0; i < line.size(); i++) {
                    if (line[i] == '#' || line[i] == ',') {
                        break;
                    } else if (line[i] != '\n' && line[i] != ' ' && line[i] != '\t') {
                        variable += line[i];
                    } 
                }
                if (variable == "") {
                    LogJournalWriter << "error 0010: trying to access an empty element at line: " << to_string(counter);
                    LogJournalWriter << "\n";
                } else {
                    temp_lists[tempNameOfTheList].push_back(variable);
                }
            }
        } else if (isInMap) {
            while (getline(ConfigReader, line)) {
                counter += 1;
                bool Continue = false;
                int CommentIndex = -1;
                for (int i = 0; i < line.size(); i++) {
                    if (line[i] == '#') {
                        CommentIndex = i;
                        break;
                    }
                }
                for (int i = 0; i < line.size(); i++) {
                    if (line[i] != '\t' && line[i] != '#' && line[i] != ' ' && line[i] != '}') {
                        if (CommentIndex == -1) {
                            Continue = true;
                            break;
                        } else if (i < CommentIndex) {
                            Continue = true;
                            break;
                        }
                    }
                }
                if (!Continue) {
                    tempNameOfTheMap = "";
                    isInMap = false;
                    break;
                }
                vector<string> result = {};
                string variable = "";
                for (int i = 0; i < line.size(); i++) {
                    if (line[i] == ':') {
                        result.push_back(variable);
                        variable = "";
                    } else if (line[i] == '#') {
                        break;
                    } else if (line[i] != '\n' && line[i] != ' ' && line[i] != '\t' && line[i] != ',') {
                        variable += line[i];
                    } 
                }
                result.push_back(variable);
                if (result.size() > 2) {
                    LogJournalWriter << "error 0011: element in map contain more than two sub-elements at line: " << to_string(counter);
                    LogJournalWriter << "\n";
                } else if (result.size() < 2) {
                    LogJournalWriter << "error 0013: element in map contain less than two sub-elements at line: " << to_string(counter);
                    LogJournalWriter << "\n";
                } else {
                    temp_maps[tempNameOfTheMap][result[0]] = result[1];
                }
            }
        }
    }
    ConfigReader.close();
    LogJournalWriter.close();
    S->variables = temp_variables;
    S->lists = temp_lists;
    S->maps = temp_maps;
    return S;
}

__attribute__((visibility("default")))
void WriteConfigs(Settings* Settings) __asm__("WriteConfigs");
void WriteConfigs(Settings* Settings) {
    fstream configReader("bin/Configs.cfg", ios::out);
    configReader << "# Warning : This configuration file isn't supposed to be manually modified. It contains important settings and configurations that could be lost if the file is corrupted." << endl;
    for (const auto& pair : Settings->variables) {
        configReader << pair.first << " : " << pair.second << endl;
    }
    for (const auto& pair : Settings->lists) {
        configReader << pair.first << " = [" << endl;
        for (int i = 0; i < pair.second.size(); i++) {
            if (pair.second.size() - 1 == i) {
                configReader << "\t" << pair.second[i] << endl;
            } else {
                configReader << "\t" << pair.second[i] << "," << endl;
            }
        } 
        configReader << "]" << endl;
    }
    for (const auto& pair : Settings->maps) {
        configReader << pair.first << " = {" << endl;
        int number = 0;
        for (const auto& key : pair.second) {
            if (pair.second.size() - 1 == number) {
                configReader << "\t" << key.first << " : " << key.second << endl;
            } else {
                configReader << "\t" << key.first << " : " << key.second << "," << endl;
            }
            number++;
        } 
        configReader << "]" << endl;
    }
}

__attribute__((visibility("default")))
void DestroySettings(unique_ptr<Settings> Settings) __asm__("DestroySettings");
void DestroySettings(unique_ptr<Settings> Settings) {
    Settings.reset();
}


