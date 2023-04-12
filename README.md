# Proyecto Chat - Sistemas Operativos


## Descripción
El objetivo de este proyecto es implementar un cliente y servidor para soportar un chat. Este chat tiene las siguientes características:
- Registro de usuarios
- Cambiar estado de un usuario
- Mensajes a todos los usuarios
- Mensajes privados
- Listar usuarios conectados
- Obtener información de un usuario específico

## Compilar
- Para compilar el servidor usamos el siguiente comando `g++ server.cc project.pb.cc -o server -lprotobuf -pthread`
- Para compilar el cliente usamos el siguiente comando `g++ client.cc project.pb.cc -o client -lprotobuf -pthread`

## Ejecutar
- Servidor: `./server`
- Cliente: `./client <userName> <IP> <PORT>`

## Comando
- Para el cliente el comando de ayuda es `/help`
