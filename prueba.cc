#include <iostream>
#include <map>
#include <pthread.h>



void *client_handler(void* user_socket) {
    std::string current_user = "";
    int socket = *((int *) user_socket);

    std::cout << "aaaa " << socket << std::endl;

    return NULL;
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

    int new_socket = 1;
    pthread_t id;

    pthread_create(&id, NULL, client_handler, (void *)&new_socket);
    std::cout << "hola" << std::endl;

    return 0;


}

