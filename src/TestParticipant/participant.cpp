#include "../tpc.h"
#include <fstream>
/*
    File/Folder "Database" service
*/

void reset(){
    remove("./db/Another/test.dbfil");
    remove("./db/Some/test.dbfil");
    rmdir("./db/Another");
    rmdir("./db/Some");
    rmdir("./db");

}

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
    std::string filename = "./db/"+data[2]+"/"+data[3]+".dbfil";
    DEBUG_LOG(filename);
    std::fstream row;
    //check if row exists, not allowed to overwrite or duplicate
    std::fstream check(filename, std::ios_base::in);
    if(check.is_open()){
        TEST_LOG("Tried to overwrite row, returning false.");
        return false;
    }
    row.open(filename, std::ios_base::out);
    if (!row.is_open())
    {
        DEBUG_LOG("create_row errorcode: " << strerror(errno));
        DEBUG_LOG("Unable to create row");
        return false;
    }
    for (int i = 2; i < data.size(); i++)
    {
        row << data[i] + " ";
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
    std::fstream log("./db/transaction.log", std::ios_base::in | std::ios_base::out | std::ios_base::app);
    if (!log.is_open())
    {
        DEBUG_LOG("Error opening transaction log");
        return false;
    }

    std::vector<std::string> transactions;
    std::string transaction;
    while(std::getline(log, transaction)){
        if(log.eof()){
            break;
        }
        transactions.push_back(transaction+"\n");
    }

    int transaction_index = -1;
    TEST_LOG("Committing transaction with ID: " << id);
    std::vector<std::string> fields;
    for(int i = 0; i < transactions.size(); i++){
        std::istringstream ss(transactions[i]);
        fields = {};
        std::string line;
        while (std::getline(ss, line, ' '))
        {
            if (ss.eof())
            {
                break;
            }
            fields.push_back(line);
        }
        if(std::stoi(fields[0]) == id){
            TEST_LOG("Found transaction at index: " << i);
            transaction_index = i;
            break;
        }
    }

    if(transaction_index == -1){
        TEST_LOG("Size of fields in onCommit_Bacllback: " << fields.size());
        TEST_LOG("Unable to find the transaction in transactions.log");
        return false;
    }
    std::vector<std::string> original;
    for(auto& s : fields){
        original.push_back(s+" ");
    }
    bool success = create_table(fields[2]);
    bool success2 = create_row(fields);
    return success && success2;
}

int main()
{
    reset();
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
    p.sendTo(buffer.c_str(), len, p.socket.connection);
    std::cin.get();
}