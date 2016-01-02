//
//  TCPHandler.cpp
//  FlacDemon
//
//  Created by merryclarke on 02/01/2016.
//  Copyright (c) 2016 c4software. All rights reserved.
//

#include "TCPHandler.h"

FlacDemon::TCPHandler::TCPHandler(){
    //init
    this->acceptThread = nullptr;
}
FlacDemon::TCPHandler::~TCPHandler(){
    //delete
}
void FlacDemon::TCPHandler::initialize(){
    //make lots of this configurable in settings
    struct sockaddr_in server_address;
    int port = 8995;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        std::cout << "Error opening socket for tcp" << std::endl;
        return;
    }
    bzero((char*) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    if(bind(sockfd, (const struct sockaddr * )&server_address, sizeof(server_address)) < 0){
        std::cout << "Error binding socket" << std::endl;
        //detail of error
        return;
    }
    listen(sockfd, 5);
    this->acceptThread = new std::thread(&TCPHandler::runAcceptLoop, this, sockfd);
    
    // some kind of accept thread loop?
}
void FlacDemon::TCPHandler::runAcceptLoop(int sockfd){
    struct sockaddr_in client_address;
    socklen_t client_length;
    int newsockfd;
    client_length = sizeof(client_address);
    std::cout << "waiting for tcp connection" << std::endl;
    if((newsockfd = accept(sockfd, (struct sockaddr * )&client_address, &client_length)) < 0){
        std::cout << "Error on tcp socket accept" << std::endl;
        return;
    }
    std::cout << "Socket connection accepted" << std::endl;
}