#include "Persistence/Restore.h"
#include "Net/Server.h"
#include "State/Memory.h"
#include "Net/IOExecutor.h"

#include "Command/CommandRegistry.h"

int main() {
    command::RegisterAll();
    Memory::GetInstance();
    Restore::RESTORE();
    Server::GetInstance()->Init();
    IOExecutor::GetInstance()->Run();
    return 0;
}
