#pragma once
#include <iostream>
#include <thread>
#include "herosockets.h"
#include <vector>
#include <atomic>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <ios>
#include <mutex>

namespace tpc
{

/*
    Log verbosity settings
    COMPILE_TYPE
    RELEASE = 0
    DEBUG = 1
    TEST = 2
    
*/

#define VERBOSITY 1
#if VERBOSITY == 0
#define LOG(x) std::cout << x << std::endl;
#else
#define LOG(x)
#endif

#if VERBOSITY == 1
#define DEBUG_LOG(x) std::cout << x << std::endl;
#else
#define DEBUG_LOG(x)
#endif

#if VERBOSITY == 2
#define TEST_LOG(x) \
    std:            \
    .cout << x << std::endl;
#else
#define TEST_LOG(x)
#endif

const std::string PROTOCOL_ID = "RTTY";
const std::string PACKAGE_END = "YTTR";

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
    Participant_TCAdd,
    Participant_TCVote,
    Participant_TCCommit
};

struct Client_TCRequestPacket
{
    static const PacketType packet_id = PacketType::Client_TCRequest;
    static const ClientType type = ClientType::REQUESTER;
    int payload_size;
    std::string payload;

    std::tuple<std::string, int> toBuffer()
    {
        std::ostringstream ss;
        ss << packet_id << ' ';
        ss << type << ' ';
        ss << payload_size << ' ';
        ss << payload << ' ';
        std::cout << "Client_TCRequestPacket test: " << ss.str() << std::endl;
        return {ss.str(), ss.str().size()};
    }

    static Client_TCRequestPacket fromBuffer(std::string data, int data_length)
    {
        Client_TCRequestPacket out;
        std::istringstream ss(data);
        std::vector<std::string> fields;
        std::string line;
        while (std::getline(ss, line, ' '))
        {
            if (ss.eof())
            {
                break;
            }
            fields.push_back(line);
        }
        out.payload_size = std::stoi(fields[2]);
        std::string payload = "";
        for (int i = 3; i < fields.size(); i++)
        {
            payload += fields[i] + " ";
        }
        out.payload = payload.substr(0, payload.size() - 2);
        return out;
    }
};

struct TC_ClientRequestAnswerPacket
{
    static const PacketType packet_id = PacketType::TC_CLientRequestAnswer;
    std::string response;
    int transaction_id;

    std::tuple<std::string, int> toBuffer()
    {
        std::ostringstream ss;
        ss << packet_id << ' ';
        ss << response << ' ';
        ss << transaction_id << ' ';
        return {ss.str(), ss.str().size()};
    }

    static TC_ClientRequestAnswerPacket fromBuffer(std::string data, int data_length)
    {
        std::istringstream ss(data);
        std::vector<std::string> fields;
        std::string line;
        while (std::getline(ss, line, ' '))
        {
            if (ss.eof())
            {
                break;
            }
            fields.push_back(line);
        }
        TC_ClientRequestAnswerPacket out;
        out.response = fields[1];
        out.transaction_id = std::stoi(fields[2]);
        return out;
    }
};

struct TC_ClientRequestAckPacket
{
    static const PacketType packet_id = PacketType::TC_ClientRequestAck;
    std::string response;
    int transaction_id;

    std::tuple<std::string, int> toBuffer()
    {
        std::ostringstream ss;
        ss << packet_id << ' ';
        ss << response << ' ';
        ss << transaction_id << ' ';
        return {ss.str(), ss.str().size()};
    }
    static TC_ClientRequestAckPacket fromBuffer(std::string data, int data_length)
    {
        std::istringstream ss(data);
        std::vector<std::string> fields;
        std::string line;
        while (std::getline(ss, line, ' '))
        {
            if (ss.eof())
            {
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

struct TC_ParticipantVotePacket
{
    static const PacketType packet_id = PacketType::TC_ParticipantVote;
    int transaction_id;
    std::string payload;

    std::tuple<std::string, int> toBuffer()
    {
        std::ostringstream ss;
        ss << packet_id << ' ';
        ss << transaction_id << ' ';
        ss << payload << ' ';
        return {ss.str(), ss.str().size()};
    }
    static TC_ParticipantVotePacket fromBuffer(std::string data, int data_length)
    {
        std::istringstream ss(data);
        std::vector<std::string> fields;
        std::string line;
        while (std::getline(ss, line, ' '))
        {
            if (ss.eof())
            {
                break;
            }
            fields.push_back(line);
        }
        TC_ParticipantVotePacket out;
        out.transaction_id = std::stoi(fields[1]);
        std::string payload = "";
        for (int i = 2; i < fields.size(); i++)
        {
            payload += fields[i] + ' ';
        }
        out.payload = payload.substr(0, payload.size() - 2);

        return out;
    }
};

struct TC_ParticipantCommitPacket
{
    static const PacketType packet_id = PacketType::TC_ParticipantCommit;
    int transaction_id;
    std::string response;

    std::tuple<std::string, int> toBuffer()
    {
        std::ostringstream ss;
        ss << packet_id << ' ';
        ss << transaction_id << ' ';
        ss << response << ' ';
        return {ss.str(), ss.str().size()};
    }

    static TC_ParticipantCommitPacket fromBuffer(std::string data, int data_length)
    {
        std::istringstream ss(data);
        std::vector<std::string> fields;
        std::string line;
        while (std::getline(ss, line, ' '))
        {
            if (ss.eof())
            {
                break;
            }
            fields.push_back(line);
        }
        TC_ParticipantCommitPacket out;
        out.transaction_id = std::stoi(fields[1]);
        out.response = fields[2];
        return out;
    }
};

struct Participant_TCAddPacket
{
    static const PacketType packet_id = PacketType::Participant_TCAdd;

    std::tuple<std::string, int> toBuffer()
    {
        std::ostringstream ss;
        ss << packet_id << ' ';
        return {ss.str(), ss.str().size()};
    }
    static Participant_TCAddPacket fromBuffer(std::string data, int data_length)
    {
        std::istringstream ss(data);
        std::vector<std::string> fields;
        std::string line;
        while (std::getline(ss, line, ' '))
        {
            if (ss.eof())
            {
                break;
            }
            fields.push_back(line);
        }
        Participant_TCAddPacket out;
        return out;
    }
};

struct Participant_TCVotePacket
{
    static const PacketType packet_id = PacketType::Participant_TCVote;
    int transaction_id;
    std::string response;

    std::tuple<std::string, int> toBuffer()
    {
        std::ostringstream ss;
        ss << packet_id << ' ';
        ss << transaction_id << ' ';
        ss << response << ' ';
        return {ss.str(), ss.str().size()};
    }

    static Participant_TCVotePacket fromBuffer(std::string data, int data_length)
    {
        std::istringstream ss(data);
        std::vector<std::string> fields;
        std::string line;
        while (std::getline(ss, line, ' '))
        {
            if (ss.eof())
            {
                break;
            }
            fields.push_back(line);
        }
        Participant_TCVotePacket out;
        out.transaction_id = std::stoi(fields[1]);
        out.response = fields[2];
        return out;
    }
};

struct Participant_TCCommitPacket
{
    static const PacketType packet_id = PacketType::Participant_TCCommit;
    int transaction_id;
    std::string response;

    std::tuple<std::string, int> toBuffer()
    {
        std::ostringstream ss;
        ss << packet_id << ' ';
        ss << transaction_id << ' ';
        ss << response << ' ';
        return {ss.str(), ss.str().size()};
    }

    static Participant_TCCommitPacket fromBuffer(std::string data, int data_length)
    {
        std::istringstream ss(data);
        std::vector<std::string> fields;
        std::string line;
        while (std::getline(ss, line, ' '))
        {
            if (ss.eof())
            {
                break;
            }
            fields.push_back(line);
        }
        Participant_TCCommitPacket out;
        out.transaction_id = std::stoi(fields[1]);
        out.response = fields[2];
        return out;
    }
};

#pragma endregion Packets

struct Client
{
    herosockets::Connection con;
    std::string buffer = "";
    ClientType type = ClientType::UNKNOWN;
    ClientState state = ClientState::READY;

    Client(herosockets::Connection con) : con(con)
    {
    }
};

struct Transaction
{
    int id;
    Client requester;
    std::vector<Client> participants;
    Client_TCRequestPacket packet;
    int commitCounter = 0;
    int voteCounter = 0;
    Transaction(Client requester, std::vector<Client> participants, Client_TCRequestPacket packet)
        : requester(requester), participants(participants), packet(packet)
    {
        static int counter = 0;
        id = ++counter;
    }
};

struct TC
{
    std::mutex connections_mutex;
    std::vector<Client> connections;
    std::mutex transactions_mutex;
    std::vector<Transaction> transactions;
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
    void onReceive(const char *data, int data_len, herosockets::Connection con)
    {
        auto &client = findClient(con);
        client.buffer += data;

        while (1)
        {
            auto protocol_id_index = client.buffer.find(PROTOCOL_ID, 0);
            if (protocol_id_index == std::string::npos)
            {
                break;
            }
            auto package_end_index = client.buffer.find(PACKAGE_END, 0);
            if (package_end_index == std::string::npos)
            {
                break;
            }
            std::string one_packet = client.buffer.substr(protocol_id_index, package_end_index - protocol_id_index);
            client.buffer = client.buffer.substr(package_end_index + PACKAGE_END.size(), client.buffer.size() - package_end_index + PACKAGE_END.size() - 1);
            DEBUG_LOG("One packet: " << one_packet);
            DEBUG_LOG("Client_buffer: " << client.buffer);
            parseReceivedData(one_packet, con);
        }
    }

    void parseReceivedData(std::string data, herosockets::Connection &con)
    {
        /*
            Les første byte av data,
            sjekk mot PacketType (switch?)
            håndter pakken (f.ks. opprett ny transaksjon om ClientRequest)
        */
        std::istringstream ss(data);
        DEBUG_LOG("Data for stringstream");
        std::vector<std::string> fields;
        std::string line;
        while (std::getline(ss, line, ' '))
        {
            if (ss.eof())
            {
                break;
            }
            fields.push_back(line);
        }
        DEBUG_LOG("Fields length: " << fields.size());
        /*
            Should have more and better sanity checks, but some are better than none.
       */
        if (fields.size() < 2 || fields[0] != PROTOCOL_ID)
        {
            std::cout << "Potentially malicious package received, disconnecting client" << std::endl;
        }
        /*
            Rebuild data with protocol stripped
        */
        std::string stripped_data;
        for (int i = 1; i < fields.size(); i++)
        {
            stripped_data += fields[i] + ' ';
        }
        /*
        Check package type
      */
        switch ((PacketType)std::stoi(fields[1]))
        {
        case PacketType::Client_TCRequest:
        {
            DEBUG_LOG("Start of switch stripped data: " << stripped_data);
            Client_TCRequestPacket p = Client_TCRequestPacket::fromBuffer(stripped_data, stripped_data.size());
            Transaction t = {findClient(con), findParticipants(), p};
            transactions_mutex.lock();
            transactions.push_back(t);
            transactions_mutex.unlock();
            TC_ParticipantVotePacket vote;
            vote.payload = p.payload;
            vote.transaction_id = t.id;
            auto [data, len] = vote.toBuffer();
            for (auto &c : t.participants)
            {
                send(data.c_str(), len, c.con);
            }
            TEST_LOG("Started new transaction with id: " << t.id);
            TC_ClientRequestAckPacket ack;
            ack.response = '1';
            ack.transaction_id = t.id;
            auto [buffer2, len2] = ack.toBuffer();
            send(buffer2.c_str(), len2, t.requester.con);
            break;
        }
        case PacketType::Participant_TCAdd:
        {
            Client &c = findClient(con);
            c.type = PARTICIPANT;
            break;
        }
        case PacketType::Participant_TCCommit:
        {
            Participant_TCCommitPacket p = Participant_TCCommitPacket::fromBuffer(stripped_data, stripped_data.size());
            Transaction t = findTransaction(p.transaction_id);
            if (strcmp(p.response.c_str(), "1") != 0)
            {
                DEBUG_LOG("a commit failed");
                TC_ParticipantCommitPacket rollback;
                rollback.response = '0';
                rollback.transaction_id = t.id;
                auto [buffer, len] = rollback.toBuffer();
                for (auto &part : t.participants)
                {
                    send(buffer.c_str(), len, part.con);
                }
                TC_ClientRequestAnswerPacket answer;
                answer.response = '0';
                answer.transaction_id = t.id;
                auto [buffer2, len2] = answer.toBuffer();
                send(buffer2.c_str(), len2, t.requester.con);
            }
            else
            {
                t.commitCounter++;
                DEBUG_LOG("commitCounter: " << t.commitCounter);
                DEBUG_LOG("t.participants.size(): " << t.participants.size());
                if (t.commitCounter == t.participants.size())
                {
                    DEBUG_LOG("all commits are done, writing result to client");
                    TC_ClientRequestAnswerPacket answer;
                    answer.response = '1';
                    answer.transaction_id = t.id;
                    auto [buffer, len] = answer.toBuffer();
                    send(buffer.c_str(), len, t.requester.con);
                }
            }
            break;
        }
        case PacketType::Participant_TCVote:
        {
            Participant_TCVotePacket p = Participant_TCVotePacket::fromBuffer(stripped_data, stripped_data.size());
            Transaction t = findTransaction(p.transaction_id);
            if (strcmp(p.response.c_str(), "1") != 0)
            {
                DEBUG_LOG("someone voted no...");
                TC_ParticipantCommitPacket rollback;
                rollback.response = '0';
                rollback.transaction_id = t.id;
                auto [buffer, len] = rollback.toBuffer();
                for (auto &part : t.participants)
                {
                    DEBUG_LOG("Sending to participant");
                    send(buffer.c_str(), len, part.con);
                }
                TC_ClientRequestAnswerPacket answer;
                answer.response = '0';
                answer.transaction_id = t.id;
                auto [buffer2, len2] = answer.toBuffer();
                send(buffer2.c_str(), len2, t.requester.con);
            }
            else
            {
                t.voteCounter++;
                /* DEBUG_LOG("votecount " << t.voteCounter);
                DEBUG_LOG("participants.size(): " << t.participants.size()); */
                if (t.voteCounter == t.participants.size())
                {
                    DEBUG_LOG("all votes are in, moving on to commit");
                    for (auto &part : t.participants)
                    {
                        TC_ParticipantCommitPacket commit;
                        commit.response = '1';
                        commit.transaction_id = t.id;
                        auto [buffer, len] = commit.toBuffer();
                        send(buffer.c_str(), len, part.con);
                    }
                }
            }
            break;
        }
        case PacketType::TC_ClientRequestAck:
        {
            //TC_ClientRequestAckPacket p = TC_ClientRequestAckPacket::fromBuffer(stripped_data, stripped_data.size());;
            LOG("TC recieved a package from another TC");
            break;
        }
        case PacketType::TC_CLientRequestAnswer:
        {
            //TC_ClientRequestAnswerPacket p = TC_ClientRequestAnswerPacket::fromBuffer(stripped_data, stripped_data.size());
            LOG("TC recieved a package from another TC");
            break;
        }
        case PacketType::TC_ParticipantCommit:
        {
            //TC_ParticipantCommitPacket p = TC_ParticipantCommitPacket::fromBuffer(stripped_data, stripped_data.size());
            LOG("TC recieved a package from another TC");
            break;
        }
        case PacketType::TC_ParticipantVote:
        {
            //TC_ParticipantVotePacket p = TC_ParticipantVotePacket::fromBuffer(stripped_data, stripped_data.size());
            LOG("TC recieved a package from another TC");
            break;
        }
        default:
            std::cout << "Received Packet didn't match any specified PacketType." << std::endl;
            break;
        }
    }

    void onConnect(herosockets::Connection con, std::string ip)
    {
        std::cout << "New connection from: " << ip << ":" << ntohs(con.con_info.sin_port) << std::endl;
        Client c(con);
        connections_mutex.lock();
        auto client = std::find_if(connections.begin(), connections.end(), [&](const Client &e) {
            return e.con.client_socket == con.client_socket;
        });
        if (client == connections.end())
        {
            connections.push_back(c);
        }
        connections_mutex.unlock();
    }
    void onDisconnect(herosockets::Connection con)
    {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &con.con_info.sin_addr, ip, INET_ADDRSTRLEN);
        std::cout << "Client disconnected: " << ip << ":" << ntohs(con.con_info.sin_port) << std::endl;
        connections_mutex.lock();
        auto disconnected = std::find_if(connections.begin(), connections.end(), [&](const Client &e) {
            return e.con.client_socket == con.client_socket;
        });
        connections_mutex.unlock();
        Client d_client = *disconnected;

        //if (disconnected.)
    }

    void send(const char *data, int data_len, herosockets::Connection con)
    {
        std::string with_protocol = PROTOCOL_ID + ' ' + data + ' ' + PACKAGE_END;
        listening_socket.sendTo(with_protocol.c_str(), with_protocol.size(), con);
    }

    Transaction findTransaction(int transaction_id)
    {
        transactions_mutex.lock();
        auto transaction = std::find_if(transactions.begin(), transactions.end(), [&](const Transaction &t) {
            return t.id == transaction_id;
        });
        transactions_mutex.unlock();
        return *transaction;
    }

    Client &findClient(herosockets::Connection con)
    {
        connections_mutex.lock();
        auto client = std::find_if(connections.begin(), connections.end(), [&](const Client &e) {
            return e.con.client_socket == con.client_socket;
        });
        auto a = (client - connections.begin());
        if (a >= connections.size())
        {
            connections.push_back(Client(con));
            connections_mutex.unlock();
            return connections.at(a);
        }
        auto &res = connections.at(a);
        connections_mutex.unlock();
        return res;
    }

    std::vector<Client> findParticipants()
    {
        std::vector<Client> clients;
        for (auto &p : connections)
        {
            if (p.type == ClientType::PARTICIPANT)
            {
                clients.push_back(p);
            }
        }
        return clients;
    }

    void removeClient(Client client)
    {
        connections_mutex.lock();
        auto client_it = std::find_if(connections.begin(), connections.end(), [&](const Client &e) {
            return e.con.client_socket == client.con.client_socket;
        });
        listening_socket.closeSocket(client_it->con.client_socket);
        connections.erase(client_it);
        connections_mutex.unlock();
    }
};
struct Participant
{
    std::string ip;
    herosockets::TCPClient socket;

    /*
        Provides a way for the consumers of our library to prepare for committing the data.
    */
    bool (*voteCallback)(int id, std::string payload);
    /*
        Provides a way for the consumers of our library to safely commit the data.
    */
    bool (*commitCallback)(int id);

    Participant(std::string ip, short port)
    {
        socket = herosockets::TCPClient(AF_INET, ip.c_str(), port);
        this->ip = ip;
        herosockets::Callback c = herosockets::Callback::create_on_receive<Participant, &Participant::onReceive>(this);
        socket.onReceive = c;
        //Initialize logs
        if (mkdir("./logs") == -1)
        {
            if (errno == EEXIST)
            {
                std::cout << "Directory 'logs' already exists\n";
            }
            else
            {
                std::cout << "cannot create sessionnamefolder error:" << strerror(errno) << std::endl;
            }
        }
        std::ofstream log;
        log.open("./logs/log.txt");
        log << "Initial log";
        log.close();
        std::ofstream revert_log;
        revert_log.open("./logs/revertlog.txt");
        revert_log << "Initial revert";
        revert_log.close();
        socket.start();
    }

    void onReceive(const char *data, int dat_len, herosockets::Connection con)
    {
        std::cout << "Received data: \n"
                  << data << "\nWith length " << dat_len << std::endl;

        std::istringstream ss(data);
        std::vector<std::string> fields;
        std::string line;
        while (std::getline(ss, line, ' '))
        {
            if (ss.eof())
            {
                break;
            }
            fields.push_back(line);
        }
        /*
            Should have more and better sanity checks, but some are better than none.
       */
        if (fields.size() < 2 || fields[0] != PROTOCOL_ID)
        {
            std::cout << "Potentially malicious package received, disconnecting client" << std::endl;
            return;
        }
        /*
            Rebuild data with protocol stripped
        */
        std::string stripped_data;
        for (int i = 1; i < fields.size(); i++)
        {
            stripped_data += fields[i] + ' ';
        }
        /*
        Check package type
        */
        switch ((PacketType)std::stoi(fields[1]))
        {
        case PacketType::TC_ParticipantCommit:
        {
            TC_ParticipantCommitPacket packet = TC_ParticipantCommitPacket::fromBuffer(stripped_data, stripped_data.size());
            if (packet.response.compare("1") == 0)
            {
                DEBUG_LOG("We should commit the payload");

                Participant_TCCommitPacket response;
                if (onCommit(packet))
                {
                    response.response = "1";
                }
                else
                {
                    DEBUG_LOG("Something went wrong when committing, rolling back");
                    response.response = "0";
                }
                response.transaction_id = packet.transaction_id;
                auto [data, len] = response.toBuffer();
                DEBUG_LOG("Sending data: " << data << ", from participant after commit");
                send(data.c_str(), len, socket.connection);
            }
            else
            {
                Participant_TCCommitPacket response;
                DEBUG_LOG("Something went wrong, rolling back");
                response.response = "0";
                response.transaction_id = packet.transaction_id;
                auto [data, len] = response.toBuffer();
                send(data.c_str(), len, socket.connection);
                onRollback();
            }
            break;
        }
        case PacketType::TC_ParticipantVote:
        {
            TC_ParticipantVotePacket p = TC_ParticipantVotePacket::fromBuffer(stripped_data, stripped_data.size());
            DEBUG_LOG("Voting on commit");
            Participant_TCVotePacket response;
            if (onVote(p))
            {
                response.response = "1";
            }
            else
            {
                response.response = "0";
            }
            response.transaction_id = p.transaction_id;
            auto [data, len] = response.toBuffer();
            DEBUG_LOG("EQRFKQEPIFKQE: " << data);
            send(data.c_str(), len, socket.connection);
            break;
        }
        case PacketType::Client_TCRequest:
        case PacketType::TC_ClientRequestAck:
        case PacketType::TC_CLientRequestAnswer:
        case PacketType::Participant_TCAdd:
        case PacketType::Participant_TCVote:
        case PacketType::Participant_TCCommit:
        default:
        {
            DEBUG_LOG("Wrong packettype recieved in participant");
            break;
        }
        }
        /*
        std::ofstream revert_log;
        revert_log.open("./logs/revertlog.txt");
        std::string line;
        std::ifstream previous_log("./logs/log.txt");
        if (previous_log.is_open())
        {
            while (getline(previous_log, line))
            {
                revert_log << line << std::endl;
            }
            previous_log.close();
        }
        else
        {
            std::cout << "Could not read previous log. Something was either wrong with the file-reading or there was no previous log.";
        }
        revert_log.close();
        std::ofstream write_ahead_log;
        write_ahead_log.open("./logs/log.txt");
        write_ahead_log << "Message recieved from " << ip << std::endl << data << std::endl;
        write_ahead_log.close();
        std::ifstream updated_log("./logs/log.txt");
        std::ifstream updated_revert_log("./logs/revertlog.txt");
        if(updated_log.good() && updated_revert_log.good()){return true;}
        else {return false;}
        */
    }

    void onRollback()
    {
        std::cout << "Reverting current log to revertlog";
        std::ofstream log;
        log.open("./logs/log.txt");
        std::string line;
        std::ifstream revert_log("./logs/revertlog.txt");
        if (revert_log.is_open())
        {
            while (getline(revert_log, line))
            {
                log << line << std::endl;
            }
            revert_log.close();
        }
        else
        {
            std::cout << "Could not read backup file. Something was either wrong with the file-reading or there was no backup log.";
        }
        log.close();
    }

    bool onVote(TC_ParticipantVotePacket packet)
    {
        std::cout << "Called onVote\n";
        std::ofstream log("./logs/log.txt", std::ios_base::app | std::ios_base::out);
        log << "\nTransaction with id " << packet.transaction_id << std::endl;
        log << packet.payload << std::endl;
        log << "---END OF TRANSACTION---\n";
        std::ofstream revlog("./logs/revertlog.txt");

        if (log.good() && revlog.good() && voteCallback(packet.transaction_id, packet.payload))
        {
            std::cout << "I vote to commit\n";
            return true;
        }
        else
        {
            std::cout << "I vote to abort\n";
            return false;
        }
    }

    bool onCommit(TC_ParticipantCommitPacket packet)
    {
        if (commitCallback(packet.transaction_id))
        {
            //Success
            return true;
        }
        else
        {
            //Abort
            return false;
        }
    }

    void send(const char *data, int data_len, herosockets::Connection con)
    {
        std::string with_protocol = PROTOCOL_ID + ' ' + data + ' ' + PACKAGE_END;
        socket.sendTo(with_protocol.c_str(), with_protocol.size(), socket.connection);
    }
};

struct Requester
{
    herosockets::TCPClient socket;
    const char *ip;
    Requester(const char *ip, int port)
    {
        socket = herosockets::TCPClient(AF_INET, ip, port);
        this->ip = ip;
        socket.start();

        herosockets::Callback c = herosockets::Callback::create_on_receive<Requester, &Requester::onReceive>(this);
        socket.onReceive = c;
    }

    void connectTo(const char *ip, short port)
    {
        int result_from_connect = socket.connectTo(ip, port);
        DEBUG_LOG("Result from connect: " << result_from_connect);
        if (result_from_connect != 0)
        {
            std::cout << "Error connecting to server." << std::endl;
        }
    }

    void commit(const char *data, int data_len)
    {
        send(data, data_len, socket.connection);
    }

    void onReceive(const char *data, int dat_len, herosockets::Connection con)
    {
        std::cout << data << std::endl;
    }

    int quit()
    {
        return socket.quit();
    }

    void send(const char *data, int data_len, herosockets::Connection con)
    {
        std::string with_protocol = PROTOCOL_ID + ' ' + data + ' ' + PACKAGE_END;
        socket.sendTo(with_protocol.c_str(), with_protocol.size(), socket.connection);
    }
};

} // namespace tpc