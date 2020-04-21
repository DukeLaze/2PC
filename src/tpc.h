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

/*!
    \file tpc.h
    \brief Duo header library for Two Phase Commit.

    Depends on the SHL herosockets.h.
*/

namespace tpc
{

/*
    Log verbosity settings
    COMPILE_TYPE
    RELEASE = 0
    DEBUG = 1
    TEST = 2
*/

#define VERBOSITY 2
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
std::mutex stdo_mutex;
#define TEST_LOG(x)        \
    tpc::stdo_mutex.lock();     \
    std::cout              \
        << x << std::endl; \
    tpc::stdo_mutex.unlock();
#else
#define TEST_LOG(x)
#endif

/*!
    A constant string defining the beginning of a packet sent with our protocol.
*/
const std::string PROTOCOL_ID = "2PC";
/*!
    A constant string defining the end of a packet sent with our protocol.
*/
const std::string PACKAGE_END = "CP2";

/*!
    A value that describes a kind of network connection (client)
*/
enum ClientType : byte
{
    PARTICIPANT,
    REQUESTER,
    UNKNOWN
};

#pragma region Packets
/*
    Due to time constraints and no 3rd party library, we'll just write all fields as strings and convert.
*/

/*!
    Protocol packet identifiers using the format <Sender_Receiver(Action/Stage)>
*/
enum PacketType : byte
{
    Client_TCRequest,
    TC_ClientRequestAck,
    TC_ClientRequestAnswer,
    TC_ParticipantVote,
    TC_ParticipantCommit,
    Participant_TCAdd,
    Participant_TCVote,
    Participant_TCCommit,
    Participant_TCAck
};

/*!
    \brief A packet sent by a client to Transaction Coordinator containing some payload that the client wishes to commit.
*/
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

/*!
    \brief A packet sent by a Transaction Coordinator to client containing the result of the commit for a transaction.
    A response value of 0 means failure, and a response value of 1 means success.
*/
struct TC_ClientRequestAnswerPacket
{
    static const PacketType packet_id = PacketType::TC_ClientRequestAnswer;
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

/*!
    \brief A packet sent by a Transaction Coordinator to client in response to a Client_TCRequestPacket to confirm the TC is working on the commit
    Contains the transaction id.
*/
struct TC_ClientRequestAckPacket
{
    static const PacketType packet_id = PacketType::TC_ClientRequestAck;
    int transaction_id;

    std::tuple<std::string, int> toBuffer()
    {
        std::ostringstream ss;
        ss << packet_id << ' ';
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
        out.transaction_id = std::stoi(fields[1]);
        return out;
    }
};

/*!
    \brief A packet sent by a Transaction Coordinator to participants signifying a new transaction is started
    Contains the transaction id and payload.
*/
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

/*!
    \brief A packet sent by a Transaction Coordinator to participants with the result of the voting phase for a transaction.
    Contains the transaction id and a response (1 or 0 signifying commit or rollback respectively).
*/
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

/*!
    \brief A (tag) packet sent by a participant to Transaction Coordinator telling the TC to change the ClientState to Participant
*/
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

/*!
    \brief A packet sent by a participant to Transaction Coordinator with the results of the vote.
    Contains the transaction_id and a response (1 or 0 signifying yes or no).
*/
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

/*!
    \brief A packet sent by a participant to Transaction Coordinator with the result of its commit phase
    Contains transaction_id and response (1 or 0 signifying commited or rollbacked).
*/
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

/*!
    \brief A packet sent by a participant to Transaction Coordinator in the event of a rollback in the commit stage
    In the event that the commit stage succeeded we will instead send a Participant_TCCommitPacket with a value of 1.
*/
struct Participant_TCAckPacket
{
    static const PacketType packet_id = PacketType::Participant_TCAck;
    int transaction_id;

    std::tuple<std::string, int> toBuffer()
    {
        std::ostringstream ss;
        ss << packet_id << ' ';
        ss << transaction_id << ' ';
        return {ss.str(), ss.str().size()};
    }

    static Participant_TCAckPacket fromBuffer(std::string data, int data_length)
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
        Participant_TCAckPacket out;
        out.transaction_id = std::stoi(fields[1]);
        return out;
    }
};

#pragma endregion Packets
/*!
    \brief Client represents a network connection and acts as a wrapper for herosockets::Connection.
    con contains a SOCKET file descriptor and sockaddr_in connection info.
    buffer accumulates the data streamed by TCP.
    type is used to identify what kind of client we are.
*/
struct Client
{
    herosockets::Connection con;
    std::string buffer = "";
    ClientType type = ClientType::UNKNOWN;

    /*!
        \brief Constructor for Client
        \param herosockets::Connection con is used to set the member con.
    */
    Client(herosockets::Connection con) : con(con)
    {
    }
};

/*!
    \brief Transaction is used to track a client commit request from start to finish.
    Tracks the number of successes from participants at each stage.
    Has a runtime unique id.
    \param requester asd
*/
struct Transaction
{
    int id;
    Client requester;
    std::vector<Client> participants;
    Client_TCRequestPacket packet;
    int commitCounter = 0;
    int voteCounter = 0;
    int ackCounter = 0;

    Transaction(Client requester, std::vector<Client> participants, Client_TCRequestPacket packet)
        : requester(requester), participants(participants), packet(packet)
    {
        static int counter = 0;
        static std::mutex counter_mutex;
        counter_mutex.lock();
        id = ++counter;
        counter_mutex.unlock();
    }
};

/*!
    \brief TC is used as a transaction coordinator
*/
struct TC
{
    private: 
    std::mutex connections_mutex;
    std::vector<Client> connections;
    std::mutex transactions_mutex;
    std::vector<Transaction> transactions;
    std::atomic<bool> quit;
    public:
    herosockets::TCPServer listening_socket;
    TC(std::string ip, short port)
    {
        quit = false;
        listening_socket = herosockets::TCPServer(AF_INET, ip.c_str(), port);
        int status = listening_socket.start();
        if (status != 0)
        {
            LOG("Error starting TcpServer");
            exit(status);
        }
        herosockets::Callback c = herosockets::Callback::create_on_receive<TC, &TC::onReceive>(this);
        c.on_connect_func = &c.bind_on_connect<TC, &TC::onConnect>;
        c.on_disconnect_func = &c.bind_on_disconnect<TC, &TC::onDisconnect>;
        listening_socket.onReceive = c;
        listening_socket.onConnect = c;
        listening_socket.onDisconnect = c;
        TEST_LOG("Transaction Coordinator started.");
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

    /*!
        \brief Function to parse the data received by the TC.
        \param data Data to parse.
        \param con Connection over which data was sent.
    */
   private: 
    void parseReceivedData(std::string data, herosockets::Connection &con)
    {
        /*
            Les første byte av data,
            sjekk mot PacketType (switch?)
            håndter pakken (f.ks. opprett ny transaksjon om ClientRequest)
        */
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
        DEBUG_LOG("Fields length: " << fields.size());
        /*
            Should have more and better sanity checks, but some are better than none.
       */
        if (fields.size() < 2 || fields[0] != PROTOCOL_ID)
        {
            TEST_LOG("Potentially malicious package received, disconnecting client");
            removeClient(findClient(con));
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
            
            TEST_LOG("Started new transaction with id: " << t.id);
            auto [data, len] = vote.toBuffer();
            for (auto &c : t.participants)
            {
                sendTo(data.c_str(), len, c.con);
                TEST_LOG("Sending " << data << " to participant " << c.con.client_socket);
            }

            TC_ClientRequestAckPacket ack;
            ack.transaction_id = t.id;
            auto [buffer2, len2] = ack.toBuffer();
            sendTo(buffer2.c_str(), len2, t.requester.con);
            break;
        }
        case PacketType::Participant_TCAdd:
        {
            Client &c = findClient(con);
            c.type = PARTICIPANT;
            TEST_LOG("New Participant connected.");
            break;
        }
        case PacketType::Participant_TCCommit:
        {
            Participant_TCCommitPacket p = Participant_TCCommitPacket::fromBuffer(stripped_data, stripped_data.size());
            Transaction& t = findTransaction(p.transaction_id);
            if (strcmp(p.response.c_str(), "1") != 0)
            {
                DEBUG_LOG("a commit failed");
                TC_ParticipantCommitPacket rollback;
                rollback.response = '0';
                rollback.transaction_id = t.id;
                auto [buffer, len] = rollback.toBuffer();
                TEST_LOG("A participant has failed the commit step, aborting transaction with id: " << p.transaction_id);
                for (auto &part : t.participants)
                {
                    sendTo(buffer.c_str(), len, part.con);
                }
            }
            else
            {
                t.commitCounter++;
                DEBUG_LOG("commitCounter: " << t.commitCounter);
                DEBUG_LOG("t.participants.size(): " << t.participants.size());
                TEST_LOG("A participant has successfully completed the commit step, successes: " << t.commitCounter << " out of: " << t.participants.size() << " for transaction: " << p.transaction_id);
                if (t.commitCounter == t.participants.size())
                {
                    DEBUG_LOG("all commits are done, writing result to client");
                    TC_ClientRequestAnswerPacket answer;
                    answer.response = '1';
                    answer.transaction_id = t.id;
                    TEST_LOG("All participants have successfully committed. Completed transaction with id: " << p.transaction_id);
                    auto [buffer, len] = answer.toBuffer();
                    sendTo(buffer.c_str(), len, t.requester.con);
                }
            }
            break;
        }
        case PacketType::Participant_TCVote:
        {
            Participant_TCVotePacket p = Participant_TCVotePacket::fromBuffer(stripped_data, stripped_data.size());
            Transaction& t = findTransaction(p.transaction_id);
            if (strcmp(p.response.c_str(), "1") != 0)
            {
                DEBUG_LOG("someone voted no...");
                TC_ParticipantCommitPacket rollback;
                rollback.response = '0';
                rollback.transaction_id = t.id;
                TEST_LOG("A participant has voted no. Aborting transaction: " << p.transaction_id);
                auto [buffer, len] = rollback.toBuffer();
                for (auto &part : t.participants)
                {
                    DEBUG_LOG("Sending to participant");
                    sendTo(buffer.c_str(), len, part.con);
                }
            }
            else
            {
                t.voteCounter++;
                TEST_LOG("A participant has voted yes. Yesses: " << t.voteCounter << " out of: " << t.participants.size() << " for transaction: " << t.id);
                if (t.voteCounter == t.participants.size())
                {
                    DEBUG_LOG("all votes are in, moving on to commit");
                    TC_ParticipantCommitPacket commit;
                    commit.response = '1';
                    commit.transaction_id = t.id;
                    TEST_LOG("All participants have voted yes. Moving on to commit stage.");
                    auto [buffer, len] = commit.toBuffer();
                    for (auto &part : t.participants)
                    {
                        sendTo(buffer.c_str(), len, part.con);
                    }
                }
            }
            break;
        }

        case PacketType::Participant_TCAck:
        {
            Participant_TCAckPacket packet = Participant_TCAckPacket::fromBuffer(stripped_data, stripped_data.size());
            Transaction& t = findTransaction(packet.transaction_id);
            t.ackCounter++;
            DEBUG_LOG("commitCounter: " << t.commitCounter);
            DEBUG_LOG("t.participants.size(): " << t.participants.size());
            if (t.ackCounter == t.participants.size())
            {
                DEBUG_LOG("all rollbacks are done, writing result to client");
                TC_ClientRequestAnswerPacket answer;
                answer.response = '0';
                answer.transaction_id = t.id;
                TEST_LOG("Transaction: " << t.id << " completed. All participants have rolled back.");
                auto [buffer, len] = answer.toBuffer();
                sendTo(buffer.c_str(), len, t.requester.con);
            }
            break;
        }

        case PacketType::TC_ClientRequestAck:
        {
            //TC_ClientRequestAckPacket p = TC_ClientRequestAckPacket::fromBuffer(stripped_data, stripped_data.size());;
            TEST_LOG("TC recieved a package from another TC");
            break;
        }
        case PacketType::TC_ClientRequestAnswer:
        {
            //TC_ClientRequestAnswerPacket p = TC_ClientRequestAnswerPacket::fromBuffer(stripped_data, stripped_data.size());
            TEST_LOG("TC recieved a package from another TC");
            break;
        }
        case PacketType::TC_ParticipantCommit:
        {
            //TC_ParticipantCommitPacket p = TC_ParticipantCommitPacket::fromBuffer(stripped_data, stripped_data.size());
            TEST_LOG("TC recieved a package from another TC");
            break;
        }
        case PacketType::TC_ParticipantVote:
        {
            //TC_ParticipantVotePacket p = TC_ParticipantVotePacket::fromBuffer(stripped_data, stripped_data.size());
            TEST_LOG("TC recieved a package from another TC");
            break;
        }
        default:
            TEST_LOG("Received Packet didn't match any specified PacketType.");
            break;
        }
    }

    /*!
        \brief Function to create a client instance and push to connections array when a client connects.
        \param con Connection to client. 
        \param ip Ip address of client.
    */
    void onConnect(herosockets::Connection con, std::string ip)
    {
        TEST_LOG("New connection from: " << ip << ":" << ntohs(con.con_info.sin_port));
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
    /*!
        \brief Function to safely disconnect from a given connection.
        \param con Connection to disconnect from.
    */
    void onDisconnect(herosockets::Connection con)
    {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &con.con_info.sin_addr, ip, INET_ADDRSTRLEN);
        TEST_LOG("Client disconnected: " << ip << ":" << ntohs(con.con_info.sin_port));
        connections_mutex.lock();
        auto disconnected = std::find_if(connections.begin(), connections.end(), [&](const Client &e) {
            return e.con.client_socket == con.client_socket;
        });
        removeClient(findClient(con));
        connections_mutex.unlock();
    }
    /*!
        \brief Function to send data over a connection.
        \param data Data to be sent.
        \param data_len Length of data.
    */
    void sendTo(const char *data, int data_len, herosockets::Connection con)
    {
        std::string with_protocol = PROTOCOL_ID + ' ' + data + ' ' + PACKAGE_END;
        listening_socket.sendTo(with_protocol.c_str(), with_protocol.size(), con);
    }
    /*!
        \brief Returns the transaction object which corresponds to the transaction id.
        \param transaction_id id of transaction you want returned.
    */
    Transaction& findTransaction(int transaction_id)
    {
        transactions_mutex.lock();
        auto transaction = std::find_if(transactions.begin(), transactions.end(), [&](const Transaction &t) {
            return t.id == transaction_id;
        });
        transactions_mutex.unlock();
        return *transaction;
    }
    /*!
        \brief Returns the client object which corresponds to the connection.
        \param con Connection of client you want returned.
    */
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
    /*!
        \brief Returns vector of all connected participants.
    */
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
    /*!
        \brief Function to remove a client.
        \param client Client obj to remove.
    */
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

/*!
    \brief Class for the participant part of the 2PC proccess
 
    A Participant class with functionality for voting, commiting, rollbacking and communicating.
    The class also provides logs that are updated with each attempt to commit to keep track of changes and to allow for rollback.
    An instance of the class is supposed to be run on the server-side. In the 2PC process it plays the same role as a database managing system like MySQL would.

*/
struct Participant
{
    /*!
        TCPClient
    */
    herosockets::TCPClient socket;
    std::string buffer;
    /*!
        Provides a way for the consumers of our library to prepare for committing the data.
    */
    bool (*voteCallback)(int id, std::string payload);

    /*!
        Provides a way for the consumers of our library to safely commit the data.
    */
    bool (*commitCallback)(int id);
    
    std::mutex log_mutex;

    /*!
        \param ip The ip of the socket.
        \param port the port of the socket.
        \brief Constructor for the Participant class.

        Creates a socket with given ip and port and initializes the logs for the system.
    */
    Participant(std::string ip, short port)
    {
        socket = herosockets::TCPClient(AF_INET, ip.c_str(), port);
        herosockets::Callback c = herosockets::Callback::create_on_receive<Participant, &Participant::onReceive>(this);
        socket.onReceive = c;
        //Initialize logs

        if (mkdir("./logs") == -1)
        {
            if (errno == EEXIST)
            {
                TEST_LOG("Directory 'logs' already exists");
            }
            else
            {
                TEST_LOG("Cannot create directory folder. It does not already exist." << strerror(errno));
            }
        }
        log_mutex.lock();
        std::ofstream log;
        log.open("./logs/log.txt", std::ios_base::app | std::ios_base::out);
        DEBUG_LOG("open " << log.is_open() << " good " << log.good());
        if (log.is_open() && log.good())
        {
        }
        else
        {
            DEBUG_LOG("Something went wrong when opening log.txt\n");
        }
        log.close();

        std::ofstream revert_log;
        revert_log.open("./logs/revertlog.txt", std::ios_base::app | std::ios_base::out);
        if (revert_log.is_open() && revert_log.good())
        {
        }
        else
        {
            DEBUG_LOG("Something went wrong when opening revertlog.txt\n");
        }
        revert_log.close();
        log_mutex.unlock();
        socket.start();
    }

    /*!
        \brief A function for what to do when receiving data.
        \param data Data that was sent.
        \param con The connection over which the data was sent.
        \param dat_len Length of the data that was sent.

        This function provides different functionality depending on the type of package that was sent.
        It also checks each packet for valid format and info to ensure we only execute on messages from valid clients.

    */
    void onReceive(const char *data, int dat_len, herosockets::Connection con)
    {
        buffer += data;

        while (1)
        {
            auto protocol_id_index = buffer.find(PROTOCOL_ID, 0);
            if (protocol_id_index == std::string::npos)
            {
                break;
            }
            auto package_end_index = buffer.find(PACKAGE_END, 0);
            if (package_end_index == std::string::npos)
            {
                break;
            }
            std::string one_packet = buffer.substr(protocol_id_index, package_end_index - protocol_id_index);
            buffer = buffer.substr(package_end_index + PACKAGE_END.size(), buffer.size() - package_end_index + PACKAGE_END.size() - 1);
            DEBUG_LOG("One packet: " << one_packet);
            DEBUG_LOG("Client_buffer: " << buffer);
            parseReceivedData(one_packet, con);
        }
    }
    private:
    void parseReceivedData(std::string data, herosockets::Connection &con){
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
            TEST_LOG("Potentially malicious package received, killing self.");
            exit(1337);
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
            TEST_LOG("Received datapacket: " << data << " of type TC_ParticipantCommit with response: " << packet.response);
            if (packet.response.compare("1") == 0)
            {
                Participant_TCCommitPacket response;
                if (onCommit(packet))
                {
                    response.response = "1";

                    log_mutex.lock();
                    std::string readln;
                    std::ifstream log("./logs/log.txt");
                    std::ofstream revertlog("./logs/revertlog.txt");
                    if (revertlog.is_open() && log.is_open())
                    {
                        while (getline(log, readln))
                        {
                            revertlog << readln << std::endl;
                        }
                        log.close();
                    }
                    else
                    {
                        TEST_LOG("Could not read backup file. Something was either wrong with the file-reading or there was no backup log.");
                    }
                    revertlog.close();
                    log_mutex.unlock();
                    TEST_LOG("Data committed")
                }
                else
                {
                    TEST_LOG("Something went wrong when committing, rolling back");
                    response.response = "0";
                }
                response.transaction_id = packet.transaction_id;
                auto [data, len] = response.toBuffer();
                DEBUG_LOG("Sending data: " << data << ", from participant after commit");
                sendTo(data.c_str(), len, socket.connection);
                TEST_LOG("Ack package sent");
            }
            else
            {
                Participant_TCAckPacket response;
                response.transaction_id = packet.transaction_id;
                auto [data, len] = response.toBuffer();
                sendTo(data.c_str(), len, socket.connection);
                onRollback(response.transaction_id);
                TEST_LOG("Rollback complete");
            }
            break;
        }
        case PacketType::TC_ParticipantVote:
        {
            TEST_LOG("Received datapacket: " << data << " of type TC_ParticipantVote");
            TC_ParticipantVotePacket p = TC_ParticipantVotePacket::fromBuffer(stripped_data, stripped_data.size());
            Participant_TCVotePacket response;
            if (onVote(p))
            {
                response.response = "1";
            }
            else
            {
                response.response = "0";
                onRollback(p.transaction_id);
            }
            response.transaction_id = p.transaction_id;
            auto [data, len] = response.toBuffer();
            sendTo(data.c_str(), len, socket.connection);
            TEST_LOG("Vote is sent to TC: " << response.response);
            
            break;
        }
        case PacketType::Client_TCRequest:
        case PacketType::TC_ClientRequestAck:
        case PacketType::TC_ClientRequestAnswer:
        case PacketType::Participant_TCAdd:
        case PacketType::Participant_TCVote:
        case PacketType::Participant_TCCommit:
        default:
        {
            TEST_LOG("Wrong packettype recieved in participant");
            break;
        }
        }
    }
    public:
    /*!
        \brief Function for rolling back to previous state
        Replaces the contents of the log file with the contents of the revertlog file. 
    */
    void onRollback(int transaction_id)
    {
        static int times_called = 0;
        times_called++;
        //DEBUG_LOG("FINDME!: " << times_called);
        TEST_LOG("Reverting current log to revertlog");
        log_mutex.lock();
        std::ifstream log;

        log.open("./logs/log.txt");
        std::string line;
        /*
        log << "Transaction with id " << packet.transaction_id << std::endl;
        log << packet.payload << std::endl;
        log << "---END OF TRANSACTION---\n";
        */
        if (log.is_open())
        {
            int indexOfTransactionToBeReverted = -1;
            std::string fileContent = "";
            while (std::getline(log, line))
            {
                fileContent += line + "\n";
            }
            size_t found = fileContent.find("Transaction with id " + transaction_id);
            if (found != std::string::npos)
            {
                indexOfTransactionToBeReverted = found;
                DEBUG_LOG("Found new index " << indexOfTransactionToBeReverted);
            }
            std::string endString = "---END OF TRANSACTION---\n";
            int endOfTransactionToBeReverted = fileContent.find(endString, indexOfTransactionToBeReverted) + endString.size();


            /*             DEBUG_LOG("oldFileContent: " << fileContent); */
            //DEBUG_LOG("substr from 0 to " << indexOfTransactionToBeReverted << " gives: " << fileContent.substr(0, indexOfTransactionToBeReverted));
            std::string newFileContent = fileContent.substr(0, indexOfTransactionToBeReverted) +""+ fileContent.substr(endOfTransactionToBeReverted, ((fileContent.size()-1) - (endOfTransactionToBeReverted)));
            
            /* std::string first = fileContent.substr(0, indexOfTransactionToBeReverted - 2);
            std::string second = fileContent.substr(endOfTransactionToBeReverted, (fileContent.length() - 1) - (endOfTransactionToBeReverted - 1));
            std::string combinedmagic = first + second; */


            /*             DEBUG_LOG("index ofTransactionToBeReverted: " << indexOfTransactionToBeReverted);
            DEBUG_LOG("endOfTransactionToBeReverted: " << endOfTransactionToBeReverted);
            DEBUG_LOG("newFileContent: " << newFileContent);
            DEBUG_LOG("combined " << combinedmagic);
            DEBUG_LOG("FIRST PART: " << fileContent.substr(0, indexOfTransactionToBeReverted-2));
            DEBUG_LOG("SEC PART: " << fileContent.substr(endOfTransactionToBeReverted, (fileContent.length()-1)-(endOfTransactionToBeReverted-1))); */
            log.close();
            std::ofstream overwrite_log;
            overwrite_log.open("./logs/log.txt");
            overwrite_log << newFileContent;
            overwrite_log.close();
        }
        else
        {
            TEST_LOG("Could not read backup file. Something was either wrong with the file-reading or there was no backup log.");
        }
        log_mutex.unlock();
    }

    /*!
        \brief Function for deciding whether to vote yes or no.
        \param packet The packet of data which TC wants us to commit.
        
        Updates the log with the contents of the packet, checks whether the logs are good and checks the result of voteCallback() before deciding on whether to vote yes.
    */
    bool onVote(TC_ParticipantVotePacket packet)
    {
        log_mutex.lock();
        std::fstream log("./logs/log.txt", std::ios_base::in | std::ios_base::out | std::ios_base::app | std::ios_base::ate);
        if (log.is_open())
        {
            log << "Transaction with id " << packet.transaction_id << std::endl;
            log << packet.payload << std::endl;
            log << "---END OF TRANSACTION---\n";
        }
        std::ifstream revlog("./logs/revertlog.txt");
        DEBUG_LOG("log good open, revlog, callback: " << log.good() << log.is_open() << revlog.good() << voteCallback(packet.transaction_id, packet.payload));
        if (log.good() && revlog.good() && voteCallback(packet.transaction_id, packet.payload))
        {
            DEBUG_LOG("I vote to commit");
            log.close();
            revlog.close();
            log_mutex.unlock();
            return true;
        }
        else
        {
            DEBUG_LOG("I vote to abort");
            log.close();
            revlog.close();
            log_mutex.unlock();
            
            return false;
        }
    }
    /*!
        \brief Function that returns a bool of whether the commitCallback() succeeded.
        \param packet Packet with what to commit.
    */
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
    /*!
        \brief Function for sending data through a given connection.
        \param data Data to be sent.
        \param data_len Length of data.
        \param con Connection through which data should be sent.
    */
    void sendTo(const char *data, int data_len, herosockets::Connection con)
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
            TEST_LOG("Error connecting to server.");
        }
    }

    void commit(const char *data, int data_len)
    {
        sendTo(data, data_len, socket.connection);
    }

    void onReceive(const char *data, int dat_len, herosockets::Connection con)
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
        std::string stripped_data;
        for (int i = 1; i < fields.size(); i++)
        {
            stripped_data += fields[i] + ' ';
        }
        switch ((PacketType)std::stoi(fields[1]))
        {
        case PacketType::TC_ClientRequestAck:
        {
            TC_ClientRequestAckPacket packet = TC_ClientRequestAckPacket::fromBuffer(stripped_data, stripped_data.size());
            TEST_LOG("TC has assured us it is working on transaction " << packet.transaction_id);
            break;
        }
        case PacketType::TC_ClientRequestAnswer:
        {
            TC_ClientRequestAnswerPacket packet = TC_ClientRequestAnswerPacket::fromBuffer(stripped_data, stripped_data.size());
            if (fields[2].compare("1") == 0)
            {
                TEST_LOG("Transaction " << packet.transaction_id << " has succeeded!");
            }
            else
            {
                TEST_LOG("Transaction " << packet.transaction_id << " has failed!");
            }
            break;
        }
        default:
        {
            break;
        }
        }
    }

    int quit()
    {
        return socket.quit();
    }

    void sendTo(const char *data, int data_len, herosockets::Connection con)
    {
        std::string with_protocol = PROTOCOL_ID + ' ' + data + ' ' + PACKAGE_END;
        socket.sendTo(with_protocol.c_str(), with_protocol.size(), socket.connection);
    }
};

} // namespace tpc