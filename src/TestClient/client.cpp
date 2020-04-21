#include "../tpc.h"


int main(){
    tpc::Requester r("127.0.0.1", 0);
    r.connectTo("127.0.0.1", 10000);
    tpc::Client_TCRequestPacket p;
    p.payload = "Some test data";
    p.payload_size = sizeof("Some test data");
    auto [buffer, len] = p.toBuffer();
    r.commit(buffer.c_str(), len);

    //std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(1));
    tpc::Client_TCRequestPacket p2;
    p2.payload = "Another test with more data!";
    p2.payload_size = p2.payload.size();
    auto [buffer2, len2] = p2.toBuffer();
    r.commit(buffer2.c_str(), len2);

    tpc::Client_TCRequestPacket p3;
    p3.payload = "Some test datadatadata";
    p3.payload_size = p3.payload.size();
    auto [buffer3, len3] = p3.toBuffer();
    r.commit(buffer3.c_str(), len3);

    std::cin.get();
}