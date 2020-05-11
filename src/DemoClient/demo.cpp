#include "../tpc.h"


int main(){
    tpc::Requester r("127.0.0.1", 0);
    r.connectTo("127.0.0.1", 10000);

    std::string in = "";
    while(true){
        std::getline(std::cin,in);
        if(in == "exit")
        {
            break;
        }
        tpc::Client_TCRequestPacket p;
        p.payload = in;
        p.payload_size = in.length();
        auto[buffer, len] = p.toBuffer();
        r.commit(buffer.c_str(), len);
    }

    std::cin.get();
}