#include <iostream>
#include "project.pb.h"
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdbool>
#include <map>
#include <list>
#include <pthread.h>
#include <cstdint>

/* 
    Declaramos todas las variables globales que estaran utilizando los threads creados en el servidor.
    Users               contiene el usuario y la ip de cada conexion.
    Users State         contiene el usuario y el su estado actual.
    Users Sockets       contiene el usuario y el socket de cada conexion. Cuando la conexion finaliza 
                        se elimina ese valor del mapa.
 */
std::map<std::string, std::string> users;
std::map<std::string, int> users_state;
std::map<std::string, int> users_sockets;


/* 
    Clien Handler

    Esta es la funcion encargada de controlar las peticiones realizadas por el cliente en cada conexion. 
    Se ha declarado de tal manera que pueda ser utilizada por los threads y reciba el socket de la
    conexion. Con nuestra programacion defensiva, obligamos al usuario que la primer accion que realice
    sea la opcion 1, registrarse. Pues para realizar peticiones, aunque no todas las opciones requieren
    que haya un usuario registrado, no seria logico que se acepten.

 */
void *client_handler(void* user_socket) {

    // Instanciamos la variable que vamos a utilizar para almacenar el nombre del usuario conectado.
    std::string current_user = ""; 
    // Convertimos el socket a un formato manejable por el sistema
    int socket = *((int *) user_socket);
    // Declaramos una variable que nos permita saber si el servidor debe seguir escuchando a esta
    // conexion en especifico.
    bool user_while_flag = true;

    // El while nos permite estar al pendiente de cualquier peticion por parte de la conexion a la que
    // se refiere.
    while(user_while_flag){

        // Decidimos manejar un buffer de 1024 bytes.
        char buffer[1024] = {0};

        // Escribimos la informacion recibida a traves del socket en el buffer.
        int valread = recv(socket, buffer, 1024, 0);
        
        // Si la cantidad de bytes recibidos a traves del socket es mayor a 0 significa que se recibio
        // informacion por lo que procedemos a manejar la peticion.
        if (valread > 0) {

            // Convertimos a string el mensaje recibido a traves de la peticion en el socket.
            std::string request_str(buffer, valread);

            // Instanciamos la variable, basada en el protocolo, que recibira la informacion enviada por
            // el cliente a traves del socket.
            chat::UserRequest request;
            
            // Almacenamos la peticion del usuario en la variable request.
            if (!request.ParseFromString(request_str)) {
                std::cerr << "Error al deserializar la solicitud" << std::endl;
            }

            // Instanciamos las variables que vamos a estar utilizando en todas las opciones.
            std::int32_t option = request.option();
            std::string user_name;
            bool user_flag = false;

            // Instanciamos las variables que vamos a estar utilizando en la opcion 1.
            chat::UserRegister newUser;
            std::string ip;

            // Instanciamos las variables que vamos a estar utilizando en la opcion 2.
            bool info_user_type;
            chat::UserInfoRequest user_info_request;
            chat::AllConnectedUsers connected_users;
            chat::UserInfo user_info;

            // Instanciamos las variables que vamos a estar utilizando en la opcion 3.
            chat::ChangeStatus change_status;
            std::int32_t new_status;

            // Instanciamos las variables que vamos a estar utilizando en la opcion 4.
            bool m_type;
            std::string recipient;
            std::string sender;
            chat::newMessage mensaje;
            std::string message_string;

            // Instanciamos las variables que vamos a estar utilizando para la response del servidor.
            std::string response_str;
            chat::ServerResponse server_response;
            std::string message_response;

            /* 
                Opcion 1, de acuerdo con el protocolo, esta opcion se utiliza para manejar el registro de usuarios.
            */
            if (option == 1) {

                // Extraemos las variables que vienen en la request del usuario.
                newUser = request.newuser();
                user_name = newUser.username();
                ip = newUser.ip();

                // Buscamos dentro de nuestras variables si el usuario ya esta registrado o activo.
                for (auto it = users.begin(); it != users.end(); ++it)
                {
                    // Si el usuario se encuentra registrado o activo, se le notifica al cliente un error.
                    if (it->first == user_name && users_state[user_name] != 3)
                    {
                        message_response = "\n- El usuario ya esta registrado o esta activo";
                        server_response.set_option(1);
                        server_response.set_code(400);
                        server_response.set_allocated_servermessage(&message_response);
                        user_flag = true;

                        if (!server_response.SerializeToString(&response_str)) {
                            std::cerr << "Failed to serialize message." << std::endl;
                            
                        }

                        send(socket, response_str.c_str(), response_str.length(), 0);
                        std::cout << "\n- El usuario que se ha enviado ya esta registrado o activo: " << it->first << std::endl;
                    }
                }

                // Si no se encuentra el usuario registrado o el usuario esta inactivo, 
                // se crea o activa el usuario y se notifica al cliente.
                if (!user_flag || ( user_flag &&  users_state[user_name] == 3)){
                    users[user_name] = ip;
                    users_state[user_name] = 1;
                    users_sockets[user_name] = socket;
                    current_user = user_name;
                    message_response = "\n- Usuario registrado o activado exitosamente";
                    server_response.set_option(1);
                    server_response.set_code(200);
                    server_response.set_allocated_servermessage(&message_response);
                    user_flag = true;

                    if (!server_response.SerializeToString(&response_str)) {
                        std::cerr << "Failed to serialize message." << std::endl;
                        
                    }

                    send(socket, response_str.c_str(), response_str.length(), 0);
                    std::cout << "\n- El usuario se ha registrado exitosamente: " << user_name << std::endl;
                }
            }

            // En caso no se haya registrado el usuario y se este intentando hacer otra
            // peticion, se notifica al cliente el error.
            if (current_user == "") {
                message_response = "\n- No hay usuario conectado";
                server_response.set_option(1);
                server_response.set_code(400);
                server_response.set_allocated_servermessage(&message_response);

                if (!server_response.SerializeToString(&response_str)) {
                    std::cerr << "Failed to serialize message." << std::endl;
                    
                }

                send(socket, response_str.c_str(), response_str.length(), 0);
                std::cout << "\n- No hay usuario asociado a la conexion "<< std::endl;
            }

            /* 
                Opcion 2,  de acuerdo con el protocolo, esta opcion se utiliza para retornar la informacion
                de uno o varios usuarios.
            */
            if (option == 2 && current_user != "") {

                // Extraemos las variables que vienen en la request del usuario.
                user_info_request = request.inforequest();
                info_user_type = user_info_request.type_request();

                // Dependiendo de la opcion enviada por el cliente se selecciona. En el protocolo se define que pasa
                // cuando es true o false.
                if (info_user_type) {

                    // Se reune toda la informacion de los usuarios en la estructura de datos.
                    for (auto it = users.begin(); it != users.end(); ++it)
                    {
                        user_info.set_username(it->first);
                        user_info.set_ip(it->second);
                        user_info.set_status(users_state[it->first]);
                        connected_users.add_connectedusers()->CopyFrom(user_info);
                        user_info.Clear();
                    }

                    // Se envia la iformacion al cliente.
                    server_response.set_option(2);
                    server_response.set_code(200);
                    message_response = "\n- Se han enviado todos los usuarios";
                    server_response.set_allocated_servermessage(&message_response);
                    server_response.set_allocated_connectedusers(&connected_users);

                    if (!server_response.SerializeToString(&response_str)) {
                        std::cerr << "Failed to serialize message." << std::endl;
                        
                    }

                    send(socket, response_str.c_str(), response_str.length(), 0);
                    std::cout << "\n- Se han enviado todos los usuarios al cliente" << std::endl;

                } 
                else {
                    // En esta opcion el cliente envia el usuario del que desea informacion
                    user_name = user_info_request.user();

                    // Se busca la informacion del cliente
                    for (auto it = users.begin(); it != users.end(); ++it)
                    {
                        if (it->first == user_name){
                            user_info.set_username(it->first);
                            user_info.set_ip(it->second);
                            user_info.set_status(users_state[it->first]);
                            user_flag = true;
                        }

                    }

                    // Se envia la respuesta al usuario.
                    if (user_flag) {
                        server_response.set_option(2);
                        server_response.set_code(200);
                        message_response = "\n- Se ha enviado el usuario";
                        server_response.set_allocated_servermessage(&message_response);
                        server_response.set_allocated_userinforesponse(&user_info);

                        if (!server_response.SerializeToString(&response_str)) {
                            std::cerr << "\n- Failed to serialize message." << std::endl;
                            
                        }

                        send(socket, response_str.c_str(), response_str.length(), 0);
                        user_info.Clear();
                        std::cout << "\n- Se han enviado el usuario solicitado: " << user_name << std::endl;
                    }
                    // En caso no exista el usuario solicitado, se le notifica al cliente.
                    else {
                        server_response.set_option(2);
                        server_response.set_code(400);
                        message_response = "El usuario solicitado no existe";
                        server_response.set_allocated_servermessage(&message_response);

                        if (!server_response.SerializeToString(&response_str)) {
                            std::cerr << "Failed to serialize message." << std::endl;
                            
                        }

                        send(socket, response_str.c_str(), response_str.length(), 0);
                        std::cout << "\n- El usuario solicitado por el cliente no existe: " << user_name << std::endl;
                    }
                        
                }
            }

            if (option == 3 && current_user != "") {

                

                change_status = request.status();
                user_name = change_status.username();
                new_status = change_status.newstatus();

                for (auto it = users.begin(); it != users.end(); ++it)
                    {
                        if (it->first == user_name){
                            user_flag = true;
                        }

                    }
                    if (new_status < 1 || new_status > 3){
                        server_response.set_option(3);
                        server_response.set_code(400);
                        message_response = "\n- El status enviado es incorrecto";
                        server_response.set_allocated_servermessage(&message_response);

                        if (!server_response.SerializeToString(&response_str)) {
                            std::cerr << "Failed to serialize message." << std::endl;
                            
                        }

                        send(socket, response_str.c_str(), response_str.length(), 0);
                        std::cout << "\n- El status enviado es incorrecto" << std::endl;
                    }
                    else if (user_flag) {
                        users_state[user_name] = new_status;
                        server_response.set_option(3);
                        server_response.set_code(200);
                        message_response = "\n- Se ha cambiado el status exitosamente";
                        server_response.set_allocated_servermessage(&message_response);

                        if (!server_response.SerializeToString(&response_str)) {
                            std::cerr << "Failed to serialize message." << std::endl;
                            
                        }

                        send(socket, response_str.c_str(), response_str.length(), 0);
                        user_info.Clear();
                        std::cout << "\n- Se han enviado el usuario solicitado: " << user_name << std::endl;
                        std::cout << "\n- Nuevo status: " << new_status << std::endl;
                    }
                    else {
                        server_response.set_option(3);
                        server_response.set_code(400);
                        message_response = "El usuario solicitado no existe";
                        server_response.set_allocated_servermessage(&message_response);

                        if (!server_response.SerializeToString(&response_str)) {
                            std::cerr << "Failed to serialize message." << std::endl;
                            
                        }

                        send(socket, response_str.c_str(), response_str.length(), 0);
                        std::cout << "\n- El usuario solicitado por el cliente no existe: " << user_name << std::endl;
                    }

            }

            if (option == 4){
                //Enviar mensajes

                user_flag = false;


                mensaje = request.message();
                m_type = mensaje.message_type();
                message_string = mensaje.message();
                sender = mensaje.sender();

                
                //Si el mensaje no es para todos sino que privado
                if (m_type == false){
                    recipient = mensaje.recipient();
                    for (auto it = users.begin(); it != users.end(); ++it){
                        if (it->first == recipient){
                            user_flag = true;
                        }

                    }
                    //Si el usuario si existe y también está activo
                    if(user_flag && users_state[recipient]==1){
                        server_response.set_option(4);
                        server_response.set_code(200);
                        message_response = "\n- " + sender + " --> "+ message_string + " --> "+ recipient;
                        server_response.set_allocated_servermessage(&message_response);
                        server_response.set_allocated_message(&mensaje);
                        if (!server_response.SerializeToString(&response_str)) {
                            std::cerr << "Failed to serialize message." << std::endl;
                            
                        }
                        send(users_sockets[recipient], response_str.c_str(), response_str.length(), 0);
                        std::cout << "\n- "<< sender << " --> "<< message_string << " --> " << recipient << std::endl;
                        send(socket, response_str.c_str(), response_str.length(), 0);
                        user_info.Clear();
                    }

                    //Si el usuario no existe o está inactivo
                    else {
                        server_response.set_option(4);
                        server_response.set_code(400);
                        message_response = "\n- El usuario solicitado no existe o está inactivo: "  + recipient;
                        server_response.set_allocated_servermessage(&message_response);

                        if (!server_response.SerializeToString(&response_str)) {
                            std::cerr << "Failed to serialize message." << std::endl;
                            
                        }

                        send(socket, response_str.c_str(), response_str.length(), 0);
                        std::cout << "\n- El usuario solicitado por el cliente no existe o está inactivo: " << recipient << std::endl;
                    }

                }
                //Para mandar a todos los usuarios conectados
                else if (m_type){
                    server_response.set_option(4);
                    server_response.set_code(200);
                    message_response = "\n- " + sender + " ha enviado el mensaje a todos los usuarios conectados";
                    server_response.set_allocated_servermessage(&message_response);
                    server_response.set_allocated_message(&mensaje);
                    if (!server_response.SerializeToString(&response_str)) {
                        std::cerr << "Failed to serialize message." << std::endl;
                        
                    }

                    //Se realiza un for loop para todos los usuarios que estén conectados
                    for (auto it = users_sockets.begin(); it != users_sockets.end(); ++it) {
                        if(users_state[it->first]==1){
                            if(users_sockets[it->first] != socket){
                                send(users_sockets[it->first], response_str.c_str(), response_str.length(), 0);
                                std::cout << "\n- "<< sender << " --> "<< message_string << " --> " << it->first << std::endl;
                            }

                            
                        }
                    }
                    
                    //Mandar respuesta al que mandó el mensaje
                    send(socket, response_str.c_str(), response_str.length(), 0);
                    std::cout << "\n- " << sender << " ha enviado a todos los usuarios conectados "<< std::endl;
                    user_info.Clear();
                
                }

                //Si no se encuentra la opción que se mandó
                else {
                    server_response.set_option(4);
                    server_response.set_code(400);
                    message_response = "\n- La opción marcada para mensaje no existe";
                    server_response.set_allocated_servermessage(&message_response);

                    if (!server_response.SerializeToString(&response_str)) {
                        std::cerr << "Failed to serialize message." << std::endl;
                        
                    }

                    send(socket, response_str.c_str(), response_str.length(), 0);
                    std::cout << "\n- La opción marcada para mensaje no existe: " << std::endl;
                }
            }

            server_response.release_servermessage();
            server_response.release_connectedusers();
            server_response.release_userinforesponse();
            server_response.release_message();
            request.Clear();
            request.clear_inforequest();
            request.clear_message();
            request.clear_newuser();
            request.clear_option();
            request.clear_status();
            memset(buffer, 0, sizeof(buffer));

        } else if (valread == 0){
            users_state[current_user] = 3;
            std::cout << "El usuario: " << current_user << " se ha desconectado";
            user_while_flag = 0;
            close(socket);
            pthread_exit(NULL);
        } else {
            users_state[current_user] = 3;
            printf("Fatal Error: -1\n");
            user_while_flag = 0;
            pthread_exit(NULL);
        }

    }

    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // Crear socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Error al crear socket" << std::endl;
        return 1;
    }

    // Configurar la dirección del servidor
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Asignar dirección al socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Error al asignar dirección" << std::endl;
        
    }

    // Escuchar conexiones entrantes
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Error al escuchar conexiones entrantes" << std::endl;
        
    }

    std::cout << "Servidor iniciado en el puerto 8080..." << std::endl;



    while (true) {
        // Aceptar una conexión entrante
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Error al aceptar conexión entrante" << std::endl;
            continue;
        }

        std::cout << ".\n- Nueva conexión aceptada" << std::endl;

        pthread_t id;
        pthread_create(&id, NULL, client_handler, (void *) &new_socket);
    }

    return 0;
}
