#include "Object.h"
#include "String.h"
#include "List.h"
#include "Protocol/BProto.pb.h"
#include "Net/Server.h"
int main() {

    auto t1 = std::chrono::high_resolution_clock::now();


    auto t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    Server server;
    server.Run();
    return 0;
}
