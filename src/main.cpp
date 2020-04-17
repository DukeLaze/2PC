#include "tpc.h"

int main(){
    TC tc("127.0.0.1", 9001);
    int a;
    std::cin >> a;
    tc.quit.store(true);
}