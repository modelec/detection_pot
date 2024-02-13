//
// Created by breizhhardware on 2/13/24.
//


#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

int main(){
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    char buffer[1024] = {0};
    std::cin >> buffer;
    send(clientSocket, buffer, strlen(buffer), 0);
    close(clientSocket);
    return 0;
}