#include <iostream>
#include "project.pb.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

int main(int argc, char *argv[]) {

    // Crea un objeto de la clase "Message" definida en tu archivo .proto
    // y establece los valores de los campos según lo requieras
    chat:: UserRegister user_register;
    user_register.set_username("abc");
    user_register.set_ip("123.123.123.123");

    chat:: UserRequest request;
    request.set_option(1);
    request.set_allocated_newuser(&user_register);

    // Serialize message to string
    std::string request_str;
    if (!request.SerializeToString(&request_str)) {
        std::cerr << "Failed to serialize message." << std::endl;
        return 1;
    }

    // Crea un socket y establece la conexión con el servidor
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "Error creando el socket" << endl;
        return 1;
    }

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // Cambia la dirección IP con la del servidor
    server.sin_family = AF_INET;
    server.sin_port = htons(8080); // Cambia el puerto con el que estés trabajando

    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        cerr << "Error de conexión" << endl;
        return 1;
    }

    // Send message to server
    if (send(sock, request_str.c_str(), request_str.length(), 0) < 0) {
        std::cerr << "Failed to send message." << std::endl;
        return 1;
    }

    std::cout << "Message sent to server." << std::endl;

    request.release_newuser();

    char buffer[1024]; // Buffer para almacenar los datos recibidos
    int bytes_received = recv(sock, buffer, sizeof(buffer), 0); // Recibir datos del servidor

    if (bytes_received <= 0) {
        cerr << "Failed to receive message." << std::endl;
        return 1;
    }

    // Deserializar respuesta del servidor
    std::string response_str(buffer, bytes_received); // Convertir buffer a string
    chat::ServerResponse response; // Objeto para almacenar la respuesta deserializada

    if (!response.ParseFromString(response_str)) {
        cerr << "Failed to parse response." << std::endl;
        return 1;
    }

    // Acceder a los datos de la respuesta
    std::cout << "Response received from server:" << std::endl;
    std::cout << "Option: " << response.option() << std::endl;
    std::cout << "Message: " << response.servermessage() << std::endl;



    // Cierra el socket y finaliza la conexión
    close(sock);

    return 0;
}
