#include "../tpc.h"

/*
    File/Folder "Database" service
*/

void setup()
{
    if (mkdir("./db") != 0)
    {
        DEBUG_LOG("Error: " << errno);
        if (errno != EEXIST)
        {
            DEBUG_LOG("Error creating db");
            exit(101);
        }
    }
    std::fstream log("./db/transaction.log", std::ios_base::in | std::ios_base::out | std::ios_base::app);
    log.close();
}

bool create_table(std::string name)
{
    std::string folder_name = "./db/" + name;
    if (mkdir(folder_name.c_str()) != 0)
    {
        DEBUG_LOG("Error: " << errno);
        if (errno != EEXIST)
        {
            DEBUG_LOG("Error creating table");
            return false;
        }
    }
    return true;
}

bool create_row(std::vector<std::string> &data)
{
    std::fstream row(data[0] + "/" + data[1]);
    if (!row.is_open())
    {
        DEBUG_LOG("Unable to create row");
        return false;
    }
    for (int i = 2; i < data.size(); i++)
    {
        row << data[i];
    }
    return true;
}

bool onVote_callback(int id, std::string payload)
{
    DEBUG_LOG("Received commit packet: <id: " << id << "> <payload: " << payload << ">");
    std::fstream log("./db/transaction.log", std::ios_base::in | std::ios_base::out | std::ios_base::app | std::ios_base::ate);
    log << id << " " << payload.size() << " " << payload << '\n';
    log.close();
    return true;
}

bool onCommit_callback(int id)
{
    DEBUG_LOG("Received commit packet, trying to commit transaction with id: " << id);
    return false;
    std::fstream log("./db/transaction.log", std::ios_base::in | std::ios_base::out | std::ios_base::app);
    if (!log.is_open())
    {
        DEBUG_LOG("Error opening transaction log");
        return false;
    }

    return true;
}

int main()
{
    setup();
    tpc::Participant p("127.0.0.1", 0);
    p.voteCallback = onVote_callback;
    p.commitCallback = onCommit_callback;
    if (p.socket.connectTo("127.0.0.1", 10000) != 0)
    {
        DEBUG_LOG("ERROR CONNECTING TO TC");
        exit(100);
    }
    tpc::Participant_TCAddPacket addme;
    auto [buffer, len] = addme.toBuffer();
    p.send(buffer.c_str(), len, p.socket.connection);
    std::cin.get();
}