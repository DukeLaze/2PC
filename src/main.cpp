#include "tpc.h"

int main(){
    //tpc::Participant testp("127.0.0.1", 3000);
    //char* b = "hei på dere";
    //testp.onRecieve(b, 12);
    tpc::Client_TCRequestPacket test;
    test.payload = "Hellofeqf3 4there2 2442 1 sfe2f 3 1431";
    test.payload_size = test.payload.size();
    auto [data, data_size] = test.toBuffer();
    std::cout << "toCharBuffer: " << data << std::endl;

    tpc::Client_TCRequestPacket test2 = tpc::Client_TCRequestPacket::fromBuffer(data, data_size);
    auto[data2, data_size2] = test2.toBuffer();
    std::cout << "toCharBuffer2: " << data2 << std::endl;
    
    herosockets::TCPClient client_test;
    tpc::TC tc("127.0.0.1", 9001);
    int a;
    std::cin >> a;
    tc.quit.store(true);
}