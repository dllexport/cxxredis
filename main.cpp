#include "Persistence/Restore.h"
#include "Net/Server.h"
int main() {
    Restore::RESTORE();
    Server server;
    server.Run();
    return 0;
}
