#include <iostream>
#include <map>
#include <pthread.h>



void client_handler(void *socket){
    int sockfd = *(int*)socket; // cast void pointer to int pointer and dereference
    std::cout << "Socket file descriptor: " << sockfd << std::endl;
}

int main() {
    std::map<std::string, int> myMap;
    myMap["apple"] = 2;
    myMap["b"] = 3;
    myMap["c"] = 4;
    myMap["d"] = 5;
    myMap["e"] = 6;
    myMap["f"] = 7;

    myMap["apple"] = 3;
    for (auto it = myMap.begin(); it != myMap.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }

    std::string a = "a";
    std::string b = "b";
    std::string c = "c";

    std::cout << a + b + c << std::endl;

    return 0;

    
}

