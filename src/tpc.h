#pragma once
#include <iostream>
#include <thread>
#include "herosockets.h"
#include <vector>
#include <atomic>
#include <fstream>
#include <algorithm>
#include <sstream>

namespace tpc
{
const std::string PROTOCOL_ID = "RTTY";

enum ClientType : byte
{
    PARTICIPANT,
    REQUESTER,
    UNKNOWN
};

enum ClientState : byte
{
    READY,
    VOTING,
    ACK
};

#pragma region Packets
/*
    Due to time constraints and no 3rd party library, we'll just write all fields as strings and convert.
*/

/*
    From_ToType
*/
enum PacketType : byte 
{
    Client_TCRequest,
    TC_ClientRequestAck,
    TC_CLientRequestAnswer,
    TC_ParticipantVote,
    TC_ParticipantCommit,
    Participant_TCVote,
    Participant_TCCommit
};

struct Client_TCRequestPacket{
    static const PacketType packet_id = PacketType::Client_TCRequest;
    static const ClientType type = ClientType::REQUESTER;
    int payload_size;
    std::string payload;

    std::tuple<std::string, int> toBuffer(){
        std::ostringstream ss;
        ss << packet_id << ' ';
        ss << type << ' ';
        ss << payload_size << ' ';
        ss << payload << ' ';
        std::cout << "Client_TCRequestPacket test: " << ss.str() << std::endl;
        return {ss.str(), ss.str().size()};
    }

    static Client_TCRequestPacket fromBuffer(std::string data, int data_length){
        Client_TCRequestPacket out;
        std::istringstream ss(data);
        std::vector<std::string> fields;
        std::string line;
        while(std::getline(ss, line, ' ')){
            if(ss.eof()){
                break;
            }
            fields.push_back(line);
        }
        out.payload_size = std::stoi(fields[2]);
        std::string payload = "";
        for(int i = 3; i < fields.size(); i++){
            payload.append(fields[i] + " ");
            std::cout << fields[i] << std::endl;
        }
        std::cout << payload << std::endl;
        std::cout << payload.c_str() << std::endl;
        out.payload = payload.c_str();
        return out;
    }
};

struct TC_ClientRequestAnswerPacket{
    static const PacketType packet_id = PacketType::TC_CLientRequestAnswer;
    std::string response;

    std::tuple<std::string, int> toBuffer(){
        std::ostringstream ss;
        ss << packet_id << ' ';
        ss << response << ' ';
        return {ss.str(), ss.str().size()};
    }

    TC_ClientRequestAnswerPacket fromBuffer(std::string data, int data_length){
        std::istringstream ss(data);
        std::vector<std::string> fields;
        std::string line;
        while(std::getline(ss, line, ' ')){
            if(ss.eof()){
                break;
            }
            fields.push_back(line);
        }
        TC_ClientRequestAnswerPacket out;
        out.response = fields[1];
        return out;
    }
};

struct TC_ClientRequestAckPacket{
    static const PacketType packet_id = PacketType::TC_ClientRequestAck;
    std::string response;
    int transaction_id;
    
    std::tuple<std::string, int> toBuffer(){
        std::ostringstream ss;
        ss << packet_id << ' ';
        ss << response << ' ';
        ss << transaction_id << ' ';
        return {ss.str(), ss.str().size()};
    }
    TC_ClientRequestAckPacket fromBuffer(std::string data, int data_length){
        std::istringstream ss(data);
        std::vector<std::string> fields;
        std::string line;
        while(std::getline(ss, line, ' ')){
            if(ss.eof()){
                break;
            }
            fields.push_back(line);
        }
        TC_ClientRequestAckPacket out;
        out.response = fields[1];
        out.transaction_id = std::stoi(fields[2]);
        return out;
    }
};

struct TC_ParticipantVotePacket{
    static const PacketType packet_id = PacketType::TC_ParticipantVote;
    std::string response;
    std::string payload;

    std::tuple<std::string, int> toBuffer(){
        std::ostringstream ss;
        ss << packet_id << ' ';
        ss << response << ' ';
        ss << payload << ' ';
        return {ss.str(), ss.str().size()};
    }
    TC_ParticipantVotePacket fromBuffer(std::string data, int data_length){
        std::istringstream ss(data);
        std::vector<std::string> fields;
        std::string line;
        while(std::getline(ss, line, ' ')){
            if(ss.eof()){
                break;
            }
            fields.push_back(line);
        }
        TC_ParticipantVotePacket out;
        out.response = fields[1];
        out.payload = std::stoi(fields[2]);
        return out;
    }
};

struct TC_ParticipantCommitPacket{
    static const PacketType packet_id = PacketType::TC_ParticipantCommit;
    std::string response;
    
    std::tuple<std::string, int> toBuffer(){
        std::ostringstream ss;
        ss << packet_id << ' ';
        ss << response << ' ';
        return {ss.str(), ss.str().size()};
    }

    TC_ParticipantCommitPacket fromBuffer(std::string data, int data_length){
        std::istringstream ss(data);
        std::vector<std::string> fields;
        std::string line;
        while(std::getline(ss, line, ' ')){
            if(ss.eof()){
                break;
            }
            fields.push_back(line);
        }
        TC_ParticipantCommitPacket out;
        out.response = fields[1];
        return out;
    }
};

struct Participant_TCVotePacket{
    static const PacketType packet_id = PacketType::Participant_TCVote;
    std::string response;

    std::tuple<std::string, int> toBuffer(){
        std::ostringstream ss;
        ss << packet_id << ' ';
        ss << response << ' ';
        return {ss.str(), ss.str().size()};
    }

    Participant_TCVotePacket fromBuffer(std::string data, int data_length){
        std::istringstream ss(data);
        std::vector<std::string> fields;
        std::string line;
        while(std::getline(ss, line, ' ')){
            if(ss.eof()){
                break;
            }
            fields.push_back(line);
        }
        Participant_TCVotePacket out;
        out.response = fields[1];
        return out;
    }
};

struct Participant_TCCommitPacket{
    static const PacketType packet_id = PacketType::Participant_TCCommit;
    std::string response;

    std::tuple<std::string, int> toBuffer(){
        std::ostringstream ss;
        ss << packet_id << ' ';
        ss << response << ' ';
        return {ss.str(), ss.str().size()};
    }

    Participant_TCCommitPacket fromBuffer(std::string data, int data_length){
        std::istringstream ss(data);
        std::vector<std::string> fields;
        std::string line;
        while(std::getline(ss, line, ' ')){
            if(ss.eof()){
                break;
            }
            fields.push_back(line);
        }
        Participant_TCCommitPacket out;
        out.response = fields[1];
        return out;
    }
};

#pragma endregion Packets

struct Client
{
    SOCKET socket_id;
    ClientType type;
    ClientState state;
};

struct Transaction
{
    int id = 0;
    std::vector<Client> participants;
    Transaction(int id, std::vector<Client> participants) : id(id), participants(participants)
    {
    }
};

struct TC
{
    std::vector<Client> participants;
    std::atomic<bool> quit;
    herosockets::TCPServer listening_socket;

    TC(std::string ip, short port)
    {
        quit = false;
        listening_socket = herosockets::TCPServer(AF_INET, ip.c_str(), port);
        int status = listening_socket.start();
        if (status != 0)
        {
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
    void onReceive(char *data, int dat_len, herosockets::Connection con)
    {
        std::cout << data << std::endl;
        /*
            Les første byte av data,
            sjekk mot PacketType (switch?)
            håndter pakken (f.ks. opprett ny transaksjon om ClientRequest)
        */
    }
    void onConnect(herosockets::Connection con, std::string ip)
    {
        std::cout << "New connection from: " << ip << ":" << ntohs(con.con_info.sin_port) << std::endl;
    }
    void onDisconnect(herosockets::Connection con)
    {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &con.con_info.sin_addr, ip, INET_ADDRSTRLEN);
        std::cout << "Client disconnected: " << ip << ":" << ntohs(con.con_info.sin_port) << std::endl;
        auto disconnected = std::find_if(participants.begin(), participants.end(), [&](const Client &e) {
            return e.socket_id == con.client_socket;
        });
        Client d_client = *disconnected;
        
        //if (disconnected.)
        
    }
};
struct Participant {
    std::string ip;
    herosockets::TCPClient socket;
    Participant(std::string ip, short port){
        //socket = herosockets::TCPClient(AF_INET, SOCK_STREAM, 0, ip.c_str(), port);
        this->ip = ip;

    }
    
    //herosockets::Connection con
    void onRecieve(char* data, int dat_len){
        std::cout << "Recieved data: \n" << data << "\nWith length " << dat_len << std::endl;
        std::ofstream revert_log;
        revert_log.open("./logs/revertlog.txt");
        std::string line;
        std::ifstream previous_file ("./logs/log.txt");
        if(previous_file.is_open()){
            while(getline (previous_file,line)){
                revert_log << line << std::endl;
            }
            previous_file.close();
        }
        else{
            std::cout <<"Could not read previous file. Something was either wrong with the file-reading or there was no previous log.";
        }
        revert_log.close();
        std::ofstream write_ahead_log;
        write_ahead_log.open("./logs/log.txt");
        write_ahead_log << "Message recieved from " << ip << std::endl << data << std::endl;
        write_ahead_log.close();
    }
    void onRollback(){
        std::cout << "Reverting current log to revertlog";
        std::ofstream log;
        log.open("./logs/log.txt");
        std::string line;
        std::ifstream revert_log ("./logs/revertlog.txt");
        if(revert_log.is_open()){
            while(getline (revert_log,line)){
                log << line << std::endl;
            }
            revert_log.close();
        }
        else{
            std::cout <<"Could not read backup file. Something was either wrong with the file-reading or there was no backup log.";
        }
        log.close();
        

    }
};

struct Requester {
    herosockets::TCPClient socket;
    const char* ip;
    Requester(const char* ip, int port)
    {
        socket = herosockets::TCPClient(AF_INET, ip, port);
        this->ip = ip;
        socket.start();
    }
    
    void commit(char* data, int data_len, char* server_ip, int port)
    {
        if(socket.connectTo(ip, port) == 0)
        {
            socket.sendTo(data, data_len, socket.connection);
        }
        else std::cout << "Error connecting to server." << std::endl;

        herosockets::Callback c = herosockets::Callback::create_on_receive<Requester, &Requester::onReceive>(this);
        socket.onReceive = c;
    }

    void onReceive(char *data, int dat_len, herosockets::Connection con)
    {
        std::cout << data << std::endl;
    }

    int quit()
    {
        return socket.quit();
    }
};

} // namespace tpc