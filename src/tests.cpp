#include "tpc.h"



bool test_participant_onVote_callback(int id, std::string payload){
    DEBUG_LOG("Received commit packet: " << payload)
    return true;
}

bool test_participant_onCommit_callback(int id){

}

int main()
{
    tpc::TC_ParticipantVotePacket pack1;
    tpc::Participant testp("127.0.0.1", 3000);
    auto[data, len] = pack1.toBuffer();
    testp.voteCallback = test_participant_onVote_callback;
    testp.onVote(pack1);
    //char* b = "hei på dere";
    //testp.onRecieve(b, 12);
}