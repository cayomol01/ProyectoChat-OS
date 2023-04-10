#include <iostream>
#include "project.pb.h"

#include <pthread.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdbool>
#include <map>
#include <list>

std::map<std::string, std::string> users;
std::map<std::string, int> users_state;
std::map<std::string, int> users_sockets;
//Recibe 
void client_handler(int *user_socket){
    std::string current_user = "";

    bool user_flag = true;

    while(user_flag){
        char buffer[1024] = {0};
        int valread = recv(*user_socket, buffer, 1024, 0);
        
        if (valread > 0) {
            std::string request_str(buffer, valread);
            chat::UserRequest request;
            
            if (!request.ParseFromString(request_str)) {
                std::cerr << "Error al deserializar la solicitud" << std::endl;
            }


            std::int32_t option = request.option();
            chat::UserRegister newUser;
            chat::ChangeStatus status;


            // Case 1 Variables
            std::string user_name;
            std::string ip;

            bool user_flag = false;
            

            //Case 2 Variables

            bool info_user_type;
            chat::UserInfoRequest user_info_request;
            chat::AllConnectedUsers connected_users;
            chat::UserInfo user_info;

            // Case 3 Variables

            chat::ChangeStatus change_status;
            std::int32_t new_status;

            // Case 4 variables
            bool m_type;

            std::string recipient;
            std::string sender;
            chat::newMessage mensaje;
            std::string message_string;

            


            std::string response_str;
            chat::ServerResponse server_response;
            std::string message_response;

            if (option == 1) {
                newUser = request.newuser();
                user_name = newUser.username();
                ip = newUser.ip();

                for (auto it = users.begin(); it != users.end(); ++it)
                {
                    if (it->first == user_name && users_state[user_name] != 3)
                    {
                        message_response = "\n- El usuario ya esta registrado o esta activo";
                        server_response.set_option(1);
                        server_response.set_code(400);
                        server_response.set_allocated_servermessage(&message_response);
                        user_flag = true;

                        if (!server_response.SerializeToString(&response_str)) {
                            std::cerr << "Failed to serialize message." << std::endl;
                            return 1;
                        }

                        send(new_socket, response_str.c_str(), response_str.length(), 0);
                        std::cout << "\n- El usuario que se ha enviado ya esta registrado o activo: " << it->first << std::endl;
                    }
                }

                if (!user_flag || ( user_flag &&  users_state[user_name] == 3)){
                    users[user_name] = ip;
                    users_state[user_name] = 1;
                    users_sockets[user_name] = new_socket;
                    message_response = "\n- Usuario registrado o activado exitosamente";
                    server_response.set_option(1);
                    server_response.set_code(200);
                    server_response.set_allocated_servermessage(&message_response);
                    user_flag = true;

                    if (!server_response.SerializeToString(&response_str)) {
                        std::cerr << "Failed to serialize message." << std::endl;
                        return 1;
                    }

                    send(new_socket, response_str.c_str(), response_str.length(), 0);
                    std::cout << "\n- El usuario se ha registrado exitosamente: " << user_name << std::endl;
                }
            }

            //El socket no cambia, es decir que no se cierra y por lo tanto se puede mandar a ese socket

            //El usuario 
            if (option == 2) {
                user_info_request = request.inforequest();
                info_user_type = user_info_request.type_request();
                if (info_user_type) {

                    for (auto it = users.begin(); it != users.end(); ++it)
                    {
                        user_info.set_username(it->first);
                        user_info.set_ip(it->second);
                        user_info.set_status(users_state[it->first]);
                        connected_users.add_connectedusers()->CopyFrom(user_info);
                        user_info.Clear();

                    }

                    server_response.set_option(2);
                    server_response.set_code(200);
                    message_response = "\n- Se han enviado todos los usuarios";
                    server_response.set_allocated_servermessage(&message_response);
                    server_response.set_allocated_connectedusers(&connected_users);

                    if (!server_response.SerializeToString(&response_str)) {
                        std::cerr << "Failed to serialize message." << std::endl;
                        return 1;
                    }

                    send(new_socket, response_str.c_str(), response_str.length(), 0);
                    std::cout << "\n- Se han enviado todos los usuarios al cliente" << std::endl;

                } 
                else {
                    user_name = user_info_request.user();
                    for (auto it = users.begin(); it != users.end(); ++it)
                    {
                        if (it->first == user_name){
                            user_info.set_username(it->first);
                            user_info.set_ip(it->second);
                            user_info.set_status(users_state[it->first]);
                            user_flag = true;
                        }

                    }

                    if (user_flag) {
                        server_response.set_option(2);
                        server_response.set_code(200);
                        message_response = "\n- Se ha enviado el usuario";
                        server_response.set_allocated_servermessage(&message_response);
                        server_response.set_allocated_userinforesponse(&user_info);

                        if (!server_response.SerializeToString(&response_str)) {
                            std::cerr << "\n- Failed to serialize message." << std::endl;
                            return 1;
                        }

                        send(new_socket, response_str.c_str(), response_str.length(), 0);
                        user_info.Clear();
                        std::cout << "\n- Se han enviado el usuario solicitado: " << user_name << std::endl;
                    }
                    else {
                        server_response.set_option(2);
                        server_response.set_code(400);
                        message_response = "El usuario solicitado no existe";
                        server_response.set_allocated_servermessage(&message_response);

                        if (!server_response.SerializeToString(&response_str)) {
                            std::cerr << "Failed to serialize message." << std::endl;
                            return 1;
                        }

                        send(new_socket, response_str.c_str(), response_str.length(), 0);
                        std::cout << "\n- El usuario solicitado por el cliente no existe: " << user_name << std::endl;
                    }
                        
                }
            }

            if (option == 3) {

                

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
                            return 1;
                        }

                        send(new_socket, response_str.c_str(), response_str.length(), 0);
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
                            return 1;
                        }

                        send(new_socket, response_str.c_str(), response_str.length(), 0);
                        user_info.Clear();
                        std::cout << "\n- Se han enviado el usuario solicitado: " << user_name << std::endl;
                    }
                    else {
                        server_response.set_option(3);
                        server_response.set_code(400);
                        message_response = "El usuario solicitado no existe";
                        server_response.set_allocated_servermessage(&message_response);

                        if (!server_response.SerializeToString(&response_str)) {
                            std::cerr << "Failed to serialize message." << std::endl;
                            return 1;
                        }

                        send(new_socket, response_str.c_str(), response_str.length(), 0);
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
                            return 1;
                        }
                        send(users_sockets[recipient], response_str.c_str(), response_str.length(), 0);
                        std::cout << "\n- "<< sender << " --> "<< message_string << " --> " << recipient << std::endl;
                        send(new_socket, response_str.c_str(), response_str.length(), 0);
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
                            return 1;
                        }

                        send(new_socket, response_str.c_str(), response_str.length(), 0);
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
                        return 1;
                    }

                    //Se realiza un for loop para todos los usuarios que estén conectados
                    for (auto it = users_sockets.begin(); it != users_sockets.end(); ++it) {
                        if(users_state[it->first]==1){
                            send(users_sockets[it->first], response_str.c_str(), response_str.length(), 0);
                            std::cout << "\n- "<< sender << " --> "<< message_string << " --> " << it->first << std::endl;
                            
                        }
                    }
                    
                    //Mandar respuesta al que mandó el mensaje
                    send(new_socket, response_str.c_str(), response_str.length(), 0);
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
                        return 1;
                    }

                    send(new_socket, response_str.c_str(), response_str.length(), 0);
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
            users_state[*user] = 3;
            std::cout << "El usuario: " << *user << " se ha desconectado";
            user_flag = 0;
        } else {
            printf("Fatal Error: -1\n");
            user_flag = 0;
        }

    }
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
        return 1;
    }

    // Escuchar conexiones entrantes
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Error al escuchar conexiones entrantes" << std::endl;
        return 1;
    }

    std::cout << "Servidor iniciado en el puerto 8080..." << std::endl;



    while (true) {
        // Aceptar una conexión entrante
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Error al aceptar conexión entrante" << std::endl;
            continue;
        }

        std::cout << ".\n- Nueva conexión aceptada" << std::endl;

        // Leer mensaje enviado por el cliente
        int valread = recv(new_socket, buffer, 1024, 0);

        std::cout << valread << std::endl;

        std::string request_str(buffer, valread);
        chat::UserRequest request;
        if (!request.ParseFromString(request_str)) {
            std::cerr << "Error al deserializar la solicitud" << std::endl;
        }


        std::int32_t option = request.option();
        chat::UserRegister newUser;
        chat::ChangeStatus status;


        // Case 1 Variables
        std::string user_name;
        std::string ip;

        bool user_flag = false;
        

        //Case 2 Variables

        bool info_user_type;
        chat::UserInfoRequest user_info_request;
        chat::AllConnectedUsers connected_users;
        chat::UserInfo user_info;

        // Case 3 Variables

        chat::ChangeStatus change_status;
        std::int32_t new_status;

        // Case 4 variables
        bool m_type;

        std::string recipient;
        std::string sender;
        chat::newMessage mensaje;
        std::string message_string;

        


        std::string response_str;
        chat::ServerResponse server_response;
        std::string message_response;

        if (option == 1) {
            newUser = request.newuser();
            user_name = newUser.username();
            ip = newUser.ip();

            for (auto it = users.begin(); it != users.end(); ++it)
            {
                if (it->first == user_name && users_state[user_name] != 3)
                {
                    message_response = "\n- El usuario ya esta registrado o esta activo";
                    server_response.set_option(1);
                    server_response.set_code(400);
                    server_response.set_allocated_servermessage(&message_response);
                    user_flag = true;

                    if (!server_response.SerializeToString(&response_str)) {
                        std::cerr << "Failed to serialize message." << std::endl;
                        return 1;
                    }

                    send(new_socket, response_str.c_str(), response_str.length(), 0);
                    std::cout << "\n- El usuario que se ha enviado ya esta registrado o activo: " << it->first << std::endl;
                }
            }

            if (!user_flag || ( user_flag &&  users_state[user_name] == 3)){
                users[user_name] = ip;
                users_state[user_name] = 1;
                users_sockets[user_name] = new_socket;
                message_response = "\n- Usuario registrado o activado exitosamente";
                server_response.set_option(1);
                server_response.set_code(200);
                server_response.set_allocated_servermessage(&message_response);
                user_flag = true;

                if (!server_response.SerializeToString(&response_str)) {
                    std::cerr << "Failed to serialize message." << std::endl;
                    return 1;
                }

                send(new_socket, response_str.c_str(), response_str.length(), 0);
                std::cout << "\n- El usuario se ha registrado exitosamente: " << user_name << std::endl;
            }
        }

        //El socket no cambia, es decir que no se cierra y por lo tanto se puede mandar a ese socket

        //El usuario 
        if (option == 2) {
            user_info_request = request.inforequest();
            info_user_type = user_info_request.type_request();
            if (info_user_type) {

                for (auto it = users.begin(); it != users.end(); ++it)
                {
                    user_info.set_username(it->first);
                    user_info.set_ip(it->second);
                    user_info.set_status(users_state[it->first]);
                    connected_users.add_connectedusers()->CopyFrom(user_info);
                    user_info.Clear();

                }

                server_response.set_option(2);
                server_response.set_code(200);
                message_response = "\n- Se han enviado todos los usuarios";
                server_response.set_allocated_servermessage(&message_response);
                server_response.set_allocated_connectedusers(&connected_users);

                if (!server_response.SerializeToString(&response_str)) {
                    std::cerr << "Failed to serialize message." << std::endl;
                    return 1;
                }

                send(new_socket, response_str.c_str(), response_str.length(), 0);
                std::cout << "\n- Se han enviado todos los usuarios al cliente" << std::endl;

            } 
            else {
                user_name = user_info_request.user();
                for (auto it = users.begin(); it != users.end(); ++it)
                {
                    if (it->first == user_name){
                        user_info.set_username(it->first);
                        user_info.set_ip(it->second);
                        user_info.set_status(users_state[it->first]);
                        user_flag = true;
                    }

                }

                if (user_flag) {
                    server_response.set_option(2);
                    server_response.set_code(200);
                    message_response = "\n- Se ha enviado el usuario";
                    server_response.set_allocated_servermessage(&message_response);
                    server_response.set_allocated_userinforesponse(&user_info);

                    if (!server_response.SerializeToString(&response_str)) {
                        std::cerr << "\n- Failed to serialize message." << std::endl;
                        return 1;
                    }

                    send(new_socket, response_str.c_str(), response_str.length(), 0);
                    user_info.Clear();
                    std::cout << "\n- Se han enviado el usuario solicitado: " << user_name << std::endl;
                }
                else {
                    server_response.set_option(2);
                    server_response.set_code(400);
                    message_response = "El usuario solicitado no existe";
                    server_response.set_allocated_servermessage(&message_response);

                    if (!server_response.SerializeToString(&response_str)) {
                        std::cerr << "Failed to serialize message." << std::endl;
                        return 1;
                    }

                    send(new_socket, response_str.c_str(), response_str.length(), 0);
                    std::cout << "\n- El usuario solicitado por el cliente no existe: " << user_name << std::endl;
                }
                    
            }
        }

        if (option == 3) {

            

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
                        return 1;
                    }

                    send(new_socket, response_str.c_str(), response_str.length(), 0);
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
                        return 1;
                    }

                    send(new_socket, response_str.c_str(), response_str.length(), 0);
                    user_info.Clear();
                    std::cout << "\n- Se han enviado el usuario solicitado: " << user_name << std::endl;
                }
                else {
                    server_response.set_option(3);
                    server_response.set_code(400);
                    message_response = "El usuario solicitado no existe";
                    server_response.set_allocated_servermessage(&message_response);

                    if (!server_response.SerializeToString(&response_str)) {
                        std::cerr << "Failed to serialize message." << std::endl;
                        return 1;
                    }

                    send(new_socket, response_str.c_str(), response_str.length(), 0);
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
                        return 1;
                    }
                    send(users_sockets[recipient], response_str.c_str(), response_str.length(), 0);
                    std::cout << "\n- "<< sender << " --> "<< message_string << " --> " << recipient << std::endl;
                    send(new_socket, response_str.c_str(), response_str.length(), 0);
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
                        return 1;
                    }

                    send(new_socket, response_str.c_str(), response_str.length(), 0);
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
                    return 1;
                }

                //Se realiza un for loop para todos los usuarios que estén conectados
                for (auto it = users_sockets.begin(); it != users_sockets.end(); ++it) {
                    if(users_state[it->first]==1){
                        send(users_sockets[it->first], response_str.c_str(), response_str.length(), 0);
                        std::cout << "\n- "<< sender << " --> "<< message_string << " --> " << it->first << std::endl;
                        
                    }
                }
                
                //Mandar respuesta al que mandó el mensaje
                send(new_socket, response_str.c_str(), response_str.length(), 0);
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
                    return 1;
                }

                send(new_socket, response_str.c_str(), response_str.length(), 0);
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
        close(new_socket);
    }

    return 0;
}
