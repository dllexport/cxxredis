#include "Persistence/Restore.h"
#include "Net/Server.h"
#include "State/Memory.h"
#include "Net/IOExecutor.h"
int main() {
    Memory::GetInstance();
    Restore::RESTORE();
    Server::GetInstance()->Init();
    IOExecutor::GetInstance()->Run();
    return 0;
}
