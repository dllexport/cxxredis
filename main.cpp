#include "String.h"
#include "Persistence/Dump.h"
#include "Persistence/Restore.h"

#include "Net/Server.h"
int main() {

    auto t1 = std::chrono::high_resolution_clock::now();
//    String::SET(0, "key", "123");
//    String::SET(0, "key2", "1234");
//    String::SET(0, "key3", "12345");
//
//    Dump::BGSAVE();
    auto t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    Restore::RESTORE();
    Server server;
    server.Run();
    return 0;
}
