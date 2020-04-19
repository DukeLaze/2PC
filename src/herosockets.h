/*
    Author: Lasse Seivaag
*/


#pragma once

#ifdef _WIN32 /*If compiling for Windows platform */

#define _WIN32_WINNT 0x0601  /*Windows 7, allows us to use functions such as inet_pton */
#define NTDDI_VERSION 0x0601 /*Same as above */
#include <winsock2.h>
#include <ws2tcpip.h>

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

typedef int SOCKET;

#endif //_WIN32
#include <vector>
#include <string.h>
#include <string>
#include <thread>
#include <iostream>
#include <functional>

namespace herosockets
{
//int inet_pton(int, const char*, void*);
//const char* inet_ntop(int, const void*, char*, socklen_t);

#pragma region Connection
class Connection {
    public:
    SOCKET client_socket;
    sockaddr_in con_info;
    Connection(SOCKET sock, sockaddr_in info) : client_socket(sock), con_info(info){}
};
#pragma endregion Connection

#pragma region Callback

class Callback{
    public:
    Callback() : obj(0), on_receive_func(0), on_connect_func(0){}

    template <typename T, void (T::*method)(char*, int, Connection)>
    static Callback create_on_receive(T* obj){
        Callback c;
        c.obj = obj;
        c.on_receive_func = &bind_on_receive<T, method>;
        return c;
    }

    template <typename T, void (T::*method)(Connection, std::string)>
    static Callback create_on_connect(T* obj){
        Callback c;
        c.obj = obj;
        c.on_connect_func = &bind_on_connect<T, method>;
        return c;
    }

    template <typename T, void (T::*method)(Connection)>
    static Callback create_on_disconnect(T* obj){
        Callback c;
        c.obj = obj;
        c.on_disconnect_func = &bind_on_disconnect<T, method>;
        return c;
    }

    void operator()(char* data, int data_len, Connection con){
        return (*on_receive_func)(obj, data, data_len, con);
    }
    void operator()(Connection con, std::string ip){
        return (*on_connect_func)(obj, con, ip);
    }
    void operator()(Connection con){
        return (*on_disconnect_func)(obj, con);
    }

    template <typename T, void (T::*method)(char*, int, Connection)>
    static void bind_on_receive(void* object, char data[], int data_len, Connection con){
        T* o = (T*)(object);
        return (o->*method)(data, data_len, con);
    }

    template <typename T, void (T::*method)(Connection, std::string)>
    static void bind_on_connect(void* object, Connection con, std::string ip){
        T* o = (T*)object;
        return (o->*method)(con, ip);
    }

    template <typename T, void (T::*method)(Connection)>
    static void bind_on_disconnect(void* object, Connection con){
        T* o = (T*)object;
        return (o->*method)(con);
    }

    typedef void (*onReceive)(void* object, char data[], int data_len, Connection con);
    typedef void (*onConnect)(void* object, Connection con, std::string ip);
    typedef void (*onDisconnect)(void* object, Connection con);
    onReceive on_receive_func;
    onConnect on_connect_func;
    onDisconnect on_disconnect_func;
    private:
    void* obj;
};
#pragma endregion Callback

#pragma region Socket
class Socket
{
public:
    virtual int start() = 0;
    virtual int quit() = 0;
    virtual int closeSocket(SOCKET) = 0;
    virtual void sendTo(char* data, int data_len, Connection con) = 0;
    Callback onReceive;
    Callback onConnect;
    Callback onDisconnect;
    static const int MAX_PACKET_SIZE = 1024;
    int domain;
    int type;
    int protocol;
    int port;
    const char* ip;
};
#pragma endregion Socket

#pragma region UDPSocket
class UDPSocket : public Socket
{
public:
    UDPSocket(short port, const char *ip, int domain, int protocol)
        : m_port(port), m_ip(ip), m_domain(domain), m_protocol(protocol)
    {
    }

    int start()
    {

#ifdef _WIN32
        WSADATA wsa_data;
        int status = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        if (status != 0)
        {
            return status;
        }
#endif
        m_socket = socket(m_domain, SOCK_DGRAM, m_protocol);
        if (m_socket == -1 || m_socket == (unsigned long long)(~0))
        {
            return -1;
        }
        m_hint.sin_family = m_domain;
        m_hint.sin_port = htons(m_port);
        inet_pton(m_domain, m_ip, &m_hint.sin_addr);
        if (bind(m_socket, (sockaddr *)&m_hint, sizeof(m_hint)) != 0)
        {
            return -1;
        }
        m_receiveThread = std::thread(&UDPSocket::receiveFrom, this);
        return 0;
    }

    int quit()
    {
        quitting = true;
        closeSocket(m_socket);
#ifdef _WIN32
        return WSACleanup();
#else
        return 0;
#endif
    }

    int closeSocket(SOCKET socket_id)
    {
        int s = 0;
#ifdef _WIN32
        s = shutdown(socket_id, SD_BOTH);
        if (s == 0)
        {
            s = closesocket(socket_id);
        }
#else
        s = shutdown(socket_id, SHUT_RDWR);
        if (s == 0)
        {
            s = close(socket_id);
        }
#endif
        return s;
    }

    void sendTo(char data[], int data_len, Connection con_info)
    {
        int con_info_size = sizeof(con_info.con_info);
        int res = sendto(m_socket, data, data_len, 0, (sockaddr *)&con_info.con_info, con_info_size);
        if (res < 0)
        {
            std::cout << errno << std::endl;
            std::cout << WSAGetLastError() << std::endl;
        }
    }

private:
    bool quitting = false;
    sockaddr_in m_hint;
    int m_domain;
    int m_protocol;
    short m_port;
    SOCKET m_socket;
    const char *m_ip;
    std::thread m_receiveThread;

    void receiveFrom()
    {
        while (!quitting)
        {
            sockaddr_in client;
            char data[MAX_PACKET_SIZE];
            int client_len = sizeof(client);
            int received_data_len;
            received_data_len = recvfrom(m_socket, data, MAX_PACKET_SIZE, 0, (sockaddr *)&client, &client_len);
            onReceive(data, received_data_len, Connection(m_socket, client));
        }
    }
};

#pragma endregion UDPSocket

#pragma region TCPServer

struct TCPServer : public Socket
{
    bool quitting = false;
    std::vector<Connection> clients;
    int client_count = 0;
    int socket_id;
    sockaddr_in hint;
    std::thread listening_thread;

    TCPServer(){
        this->domain = AF_INET;
        this->type = SOCK_STREAM;
        this->protocol = 0;
        this->port = 0;
        this->ip = "127.0.0.1";
    }
    TCPServer(int domain, const char* ip, short port) {
        this->domain = domain;
        this->type = SOCK_STREAM;
        this->protocol = 0;
        this->port = port;
        this->ip = ip;
    }


    int start()
    {
#ifdef _WIN32
        WSADATA wsa_data;
        int status = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        if (status != 0)
        {
            return status;
        }
#endif
        socket_id = socket(domain, type, protocol);
        if (socket_id == -1 || socket_id == (unsigned long long)(~0))
        {
            return -2;
        }
        hint.sin_family = domain;
        hint.sin_port = htons(port);
        inet_pton(domain, ip, &hint.sin_addr);
        if (bind(socket_id, (sockaddr *)&hint, sizeof(hint)) != 0)
        {
            return -3;
        }
        if (listen(socket_id, SOMAXCONN) != 0)
        {
            return -4;
        }
        listening_thread = std::thread(&TCPServer::listenLoop, this);
        auto receive_thread = std::thread(&TCPServer::receiveLoop, this);
        receive_thread.detach();
        return 0;
    }

    void sendTo(char* data, int data_len, Connection con){
        int status = send(con.client_socket, data, data_len, 0);
        if(status == -1){
            std::cout << "sendTo failed." << std::endl;
            exit(100);
        }
    }

    void listenLoop()
    {
        while (!quitting)
        {
            sockaddr_in client;
            socklen_t client_size = sizeof(client);
            char host[NI_MAXHOST];
            char service[NI_MAXSERV];
            int client_socket = accept(socket_id, (sockaddr *)&client, &client_size);
            if (client_socket == -1)
            {
                int error;
#ifdef _WIN32
                error = WSAGetLastError();
#else
                error = errno;
#endif
                std::cout << "Client failed to connect, error: " << error << std::endl;
            }
            int status = getnameinfo((sockaddr *)&client, client_size, host, NI_MAXHOST, service, sizeof(service), 0);
            if (status)
            {
                std::cout << host << " connected on " << service << std::endl;
            }
            else
            {
                inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
                std::cout << host << " connected on " << ntohs(client.sin_port) << std::endl;
            }
            u_long arg = 1;
            ioctlsocket(client_socket, FIONBIO, &arg);
            Connection con(client_socket, client);
            clients.push_back(con);
            onConnect(con, std::string(host));
        }
    }

    void receiveLoop()
    {
        while (!quitting)
        {
            for (int i = 0; i < clients.size(); i++)
            {
#ifdef _WIN32
                char buf[4096];
                int received = recv(clients[i].client_socket, buf, 4096, 0);
                if (received > 0)
                {
                    buf[received] = '\0';
                    onReceive(buf, received, clients[i]);
                }
                else if(WSAGetLastError() == WSAEWOULDBLOCK){}
                else{
                    onDisconnect(clients[i]);
                    clients.erase(clients.begin()+i);
                }
#else
                char buf[1024];
                int received = recv(clients[i], buf, 1024, MSG_DONTWAIT);
                if (received > 0)
                {
                    buf[received] = '\0';
                    onReceive(buf, clients[i]);
                }
#endif          
            }
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(1));
        }
    }

    int quit()
    {
        quitting = true;
        closeSocket(socket_id);
#ifdef _WIN32
        return WSACleanup();
#else
        return 0;
#endif
    }

    int closeSocket(SOCKET socket_id)
    {
        int s = 0;
#ifdef _WIN32
        s = shutdown(socket_id, SD_BOTH);
        if (s == 0)
        {
            s = closesocket(socket_id);
        }
#else
        s = shutdown(socket_id, SHUT_RDWR);
        if (s == 0)
        {
            s = close(socket_id);
        }
#endif
        return s;
    }
};

#pragma endregion TCPServer

#pragma region TCPClient

struct TCPClient : public Socket
{
    bool quitting = false;
    Connection connection = Connection(socket_id, hint);
    SOCKET socket_id;
    sockaddr_in hint;
    std::thread receive_thread;

    TCPClient(){
        this->connection = Connection(socket_id, hint);
        this->domain = AF_INET;
        this->type = SOCK_STREAM;
        this->protocol = 0;
        this->port = 0;
        this->ip = "127.0.0.1";
    }
    TCPClient(int domain, const char* ip, short port) {
        this->connection = Connection(socket_id, hint);
        this->domain = domain;
        this->type = SOCK_STREAM;
        this->protocol = 0;
        this->port = port;
        this->ip = ip;
    }

    int start()
    {
#ifdef _WIN32
        WSADATA wsa_data;
        int status = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        if (status != 0)
        {
            return status;
        }
#endif
        socket_id = socket(domain, type, protocol);
        if (socket_id == -1 || socket_id == (unsigned long long)(~0))
        {
            return -2;
        }
        hint.sin_family = domain;
        hint.sin_port = htons(port);
        inet_pton(domain, ip, &hint.sin_addr);
        if (bind(socket_id, (sockaddr *)&hint, sizeof(hint)) != 0)
        {
            return -3;
        }
        return 0;
    }

    void sendTo(char* data, int data_len, Connection con){
        int status = send(con.client_socket, data, data_len, 0);
        if(status == -1){
            std::cout << "sendTo failed." << std::endl;
            exit(100);
        }
    }

    int connectTo(const char *ip, short port)
    {
        sockaddr_in s_hint;
        s_hint.sin_family = hint.sin_family;
        s_hint.sin_port = htons(port);
        inet_pton(hint.sin_family, ip, &s_hint.sin_addr);
        int res = connect(socket_id, (sockaddr *)&s_hint, sizeof(s_hint));
        if(res == 0){
            connection = Connection(socket_id, s_hint);
            receive_thread = std::thread(&TCPClient::receiveLoop, this);
        }
        return res;
    }

    void receiveLoop()
    {
        while (!quitting)
        {
#ifdef _WIN32
            char buf[1024];
            u_long arg = 1;
            int received = recv(socket_id, buf, 1024, 0);
            if (received > 0)
            {
                buf[received] = '\0';
                onReceive(buf, received, connection);
            }
#else
            char buf[1024];
            int received = recv(socket_id, buf, 1024, MSG_DONTWAIT);
            if (received > 0)
            {
                buf[received] = '\0';
                onReceive(buf, socket_id);
            }
#endif
        }
    }

    int quit()
    {
        quitting = true;
        closeSocket(socket_id);
#ifdef _WIN32
        return WSACleanup();
#else
        return 0;
#endif
    }

    int closeSocket(SOCKET socket_id)
    {
        int s = 0;
#ifdef _WIN32
        s = shutdown(socket_id, SD_BOTH);
        if (s == 0)
        {
            s = closesocket(socket_id);
        }
#else
        s = shutdown(socket_id, SHUT_RDWR);
        if (s == 0)
        {
            s = close(socket_id);
        }
#endif
        return s;
    }
};
#pragma endregion TCPClient

} // namespace herosockets