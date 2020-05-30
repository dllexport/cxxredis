#include "Persistence/Restore.h"
#include "Net/Server.h"
#include "State/Memory.h"

int main() {
    Memory::GetInstance();
    Restore::RESTORE();
    Server::GetInstance()->Run();
    return 0;
}
