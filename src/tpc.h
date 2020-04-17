#pragma once
#include <iostream>
#include <thread>
#include "herosockets.h"
#include <vector>
#include <atomic>


/*
    Package structure:
    protocol_id 4bytes
    clienttype 1byte
    status 1 byte(prepare, abort, commit)
    payload_size 2 bytes
    payload
*/
enum ClientType{
    PARTICIPANT,
    REQUESTER,
    UNKNOWN
};

enum ClientState{
    READY,
    VOTING,
    ACK
};

struct Client{
    SOCKET socket_id;
    ClientType type;
};

struct Transaction{
    std::vector<Client> participants;
};

struct TC {
    std::vector<Client> participants;
    std::atomic<bool> quit;
    herosockets::TCPServer listening_socket;

    TC(std::string ip, short port){
        quit = false;
        listening_socket = herosockets::TCPServer(AF_INET, ip.c_str(), port);
        int status = listening_socket.start();
        if(status != 0){
            std::cout << "Error starting TcpServer" << std::endl;
            std::cout << status << std::endl;
        }
        herosockets::Callback c = herosockets::Callback::create_on_receive<TC, &TC::onReceive>(this);
        c.on_connect_func = &c.bind_on_connect<TC, &TC::onConnect>;
        c.on_disconnect_func = &c.bind_on_disconnect<TC, &TC::onDisconnect>;
        listening_socket.onReceive = c;
        listening_socket.onConnect = c;
        listening_socket.onDisconnect = c;
        
    }
    void onReceive(char* data, int dat_len, herosockets::Connection con){
        std::cout << data << std::endl;
    }
    void onConnect(herosockets::Connection con, std::string ip){
        std::cout << "New conenction from: " << ip << ":" << ntohs(con.con_info.sin_port) << std::endl;
    }
    void onDisconnect(herosockets::Connection con){
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &con.con_info.sin_addr, ip, INET_ADDRSTRLEN);
        std::cout << "Client disconnected: " << ip << ":" << ntohs(con.con_info.sin_port) << std::endl;
    }
};