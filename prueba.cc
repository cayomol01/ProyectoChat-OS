#include <iostream>
#include <map>

int main() {
    std::map<std::string, int> myMap;
    myMap["apple"] = 2;
    myMap["b"] = 3;
    myMap["c"] = 4;
    myMap["d"] = 5;
    myMap["e"] = 6;
    myMap["f"] = 7;
    for (auto it = myMap.begin(); it != myMap.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    return 0;
}