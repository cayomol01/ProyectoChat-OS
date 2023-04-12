/*
Universidad del Valle de Guatemala
Sistemas Operativos
Proyecto 01 - Chat
*/
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <vector>
#include "project.pb.h"
#include <pthread.h>

// Namespaces
using namespace std;
using namespace chat;

void showHelp() {
    cout << "Available commands:" << endl;
    cout << "/help: Show available commands" << endl;
    cout << "/list: Show connected users" << endl;\
    cout << "/info: Show an user info" << endl;
    cout << "/b: Send message to all connected users" << endl;
    cout << "/p: Send message to a specific user" << endl;
    cout << "/me: Show your name and status" << endl;
    cout << "/status: Change your status. 1 = ACTIVE; 2 = BUSY; 3 = INACTIVE" << endl;
    cout << "/exit: Close connection with server" << endl;
}

string getStatusName(int status) {
    if (status == 1)
    {
        return "ACTIVE";
    } else if(status == 3) {
        return "INACTIVE";
    } else if (status == 2) {
        return "BUSY";
    }
    
}

void showUserInfo(UserInfo userInfo) {
    cout << "Username: " << userInfo.username() << endl;
    cout << "IP: " << userInfo.ip() << endl;
    cout << "Status: " << getStatusName(userInfo.status()) << endl;
}

void *receiveResponse(void* socket) {
    while (true) {
        // Listen for any response
        char buffer[1024] = {0};
        int bytesReceived = recv(*(int*)socket, buffer, 1024, 0);
        if(bytesReceived == -1) {
            cout << "Error: Could not receive message from server" << endl;
            pthread_exit(NULL);
        }
        // If the bytes received are 0 exit the thread
        if(bytesReceived == 0) {
            cout << "Info: Connection closed by server" << endl;
            pthread_exit(NULL);
        }
        // Deserialize response
        ServerResponse serverResponse;
        serverResponse.ParseFromString(buffer);
        if (serverResponse.code() == 200) {
            int option = serverResponse.option();
            if(option == 1) {
                // User was registered successfully
                cout << "Info: Registered user" << endl;
            } else if (option == 2) {
                // List of users
                if(serverResponse.has_connectedusers()) {
                    // Display users
                    cout << "Users: " << serverResponse.connectedusers().connectedusers_size() << endl;
                    for(int i = 0; i < serverResponse.connectedusers().connectedusers_size(); i++) {
                        UserInfo userInfo = serverResponse.connectedusers().connectedusers(i);
                        showUserInfo(userInfo);
                    }
                } else {
                    // Display single user
                    UserInfo userInfo = serverResponse.userinforesponse();
                    showUserInfo(userInfo);
                }
            } else if (option == 3) {
                // Status change
                cout << "Info: Status changed" << endl;
            } else if (option == 4) {
                // Message received
                if(serverResponse.has_message()) {
                    newMessage message = serverResponse.message();
                    cout << "Message from " << message.sender() << ": " << message.message() << endl;
                }
            }
            cout << endl;
        } else {
            // Show error message
            cout << "Error: " << "Error receiving server response." << endl;
        }
        // Clean buffer
        memset(buffer, 0, 1024);
    }
}




int main(int argc, char *argv[])
{
    // 1. Receive arguments
    if(argc != 4) {
        cout << "Error: Invalid number of arguments" << endl;
        return 1;
    }
    string clientName = argv[1];
    string serverIP = argv[2];
    string serverPort = argv[3];
    cout << "Creating client with the following parameters:" << endl;
    cout << "Client name: " << clientName << endl;
    cout << "Server IP: " << serverIP << endl;
    cout << "Server port: " << stoi(serverPort) << endl;

    // 2. Create socket client
    int socketClient = socket(AF_INET, SOCK_STREAM, 0);
    if(socketClient == -1) {
        cerr << "Error: Could not create socket" << endl;
        return 1;
    }
    struct sockaddr_in serverAddress;
    serverAddress.sin_addr.s_addr = inet_addr(serverIP.c_str());
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(stoi(serverPort));
    if(connect(socketClient, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1) {
        cerr << "Error: Could not connect to server" << endl;
        return 1;
    }

    // 3. Register user
    UserRequest registerRequest;
    UserRegister registerUser;
    registerUser.set_username(clientName);
    // Get client IP
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(serverAddress.sin_addr), clientIP, INET_ADDRSTRLEN);
    registerUser.set_ip(clientIP);
    registerRequest.set_option(1);
    registerRequest.set_allocated_newuser(&registerUser);
    // Serialize request to send to server
    string serializedRequest;
    registerRequest.SerializeToString(&serializedRequest);
    // Send request to server
    int sendResult = send(socketClient, serializedRequest.c_str(), serializedRequest.size(), 0);
    if(sendResult == -1) {
        cerr << "Error: Could not register user" << endl;
        return 1;
    }
    registerRequest.clear_option();
    registerRequest.release_newuser();
    pthread_t id;
    pthread_create(&id, NULL, receiveResponse, (void *)&socketClient);
    // 4. Get user input
    string userInput;
    while(true) {
        cout << "> ";
        getline(cin, userInput);

        if(userInput == "/exit") {
            break;  // Exit loop
        } else if (userInput == "/help") {
            showHelp(); // Show help
        } else if (userInput == "/list") {
            // Show connected users
            UserRequest listRequest;
            listRequest.set_option(2);
            UserInfoRequest userInfoRequest;
            userInfoRequest.set_type_request(true);
            listRequest.set_allocated_inforequest(&userInfoRequest);
            string serializedRequest;
            listRequest.SerializeToString(&serializedRequest);
            // Send request to server
            int sendResult = send(socketClient, serializedRequest.c_str(), serializedRequest.size(), 0);
            if(sendResult == -1) {
                cerr << "Error: Could not send message to server" << endl;
                return 1;
            }
            listRequest.clear_option();
            listRequest.release_inforequest();
        } else if (userInput == "/status") {
            cout << "New status: ";
            string newStatusInput;
            getline(cin, newStatusInput);
            UserRequest changeStatusRequest;
            changeStatusRequest.set_option(3);
            ChangeStatus changeStatus;
            changeStatus.set_newstatus(stoi(newStatusInput));
            changeStatus.set_username(clientName);
            changeStatusRequest.set_allocated_status(&changeStatus);
            // Send request to server
            string serializedRequest;
            changeStatusRequest.SerializeToString(&serializedRequest);
            int sendResult = send(socketClient, serializedRequest.c_str(), serializedRequest.size(), 0);
            if(sendResult == -1) {
                cerr << "Error: Could not send message to server" << endl;
                return 1;
            }
            changeStatusRequest.clear_option();
            changeStatusRequest.release_status();
        } else if (userInput == "/me") {
            UserRequest meRequest;
            meRequest.set_option(2);
            UserInfoRequest meInfoRequest;
            meInfoRequest.set_type_request(false);
            meInfoRequest.set_user(clientName);
            meRequest.set_allocated_inforequest(&meInfoRequest);
            // Serialize request
            string serializedRequest;
            meRequest.SerializeToString(&serializedRequest);
            // Send request to server
            int sendResult = send(socketClient, serializedRequest.c_str(), serializedRequest.size(), 0);
            if(sendResult == -1) {
                cerr << "Error: Could not send message to server" << endl;
                return 1;
            }
            meRequest.clear_option();
            meRequest.release_inforequest();
        } else if (userInput == "/info") {
            // Same as /me but ask which user name
            cout << "User name: ";
            string userName;
            getline(cin, userName);
            UserRequest infoRequest;
            infoRequest.set_option(2);
            UserInfoRequest infoRequestInfo;
            infoRequestInfo.set_type_request(false);
            infoRequestInfo.set_user(userName);
            infoRequest.set_allocated_inforequest(&infoRequestInfo);
            // Serialize request
            string serializedRequest;
            infoRequest.SerializeToString(&serializedRequest);
            // Send request to server
            int sendResult = send(socketClient, serializedRequest.c_str(), serializedRequest.size(), 0);
            if(sendResult == -1) {
                cerr << "Error: Could not send message to server" << endl;
                return 1;
            }
            infoRequest.clear_option();
            infoRequest.release_inforequest();
        } else if (userInput == "/b") {
            // Send a broadcast message
            cout << "Message: ";
            string message;
            getline(cin, message);
            UserRequest broadcastRequest;
            broadcastRequest.set_option(4);
            newMessage broadcastMessage;
            broadcastMessage.set_message(message);
            broadcastMessage.set_sender(clientName);
            broadcastMessage.set_message_type(true);
            broadcastRequest.set_allocated_message(&broadcastMessage);
            // Serialize request
            string serializedRequest;
            broadcastRequest.SerializeToString(&serializedRequest);
            // Send request to server
            int sendResult = send(socketClient, serializedRequest.c_str(), serializedRequest.size(), 0);
            if(sendResult == -1) {
                cerr << "Error: Could not send message to server" << endl;
                return 1;
            }
            broadcastRequest.clear_option();
            broadcastRequest.release_message();
            
        } else if (userInput == "/p") {
            // Send a private message,
            cout << "To: ";
            string to;
            getline(cin, to);
            cout << "Message: ";
            string message;
            getline(cin, message);
            UserRequest privateMessageRequest;
            newMessage privateMessage;
            privateMessage.set_message_type(false);
            privateMessage.set_message(message);
            privateMessage.set_sender(clientName);
            privateMessage.set_recipient(to);
            
            privateMessageRequest.set_option(4);
            privateMessageRequest.set_allocated_message(&privateMessage);
            // Send request to the server
            // Serialize request
            string serializedRequest;
            privateMessageRequest.SerializeToString(&serializedRequest);
            // Send request to server
            int sendResult = send(socketClient, serializedRequest.c_str(), serializedRequest.size(), 0);
            if(sendResult == -1) {
                cerr << "Error: Could not send message to server" << endl;
                return 1;
            }
            privateMessageRequest.clear_option();
            privateMessageRequest.release_message();
        }
    } 
    // wait for thread to finish
    pthread_join(id, NULL);
    close(socketClient);
    return 0;
}
