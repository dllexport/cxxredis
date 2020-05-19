#include "Encoding.h"
#include "String.h"
#include "List.h"

int main() {
    String::Set(0, "key", "1.0");
    String::Incr(0, "key");
    String::Incr(0, "key");
    auto str = String::Get(0, "key");
    auto sub = String::GetRange(0, "key", 0, -3213131);

    auto t1 = std::chrono::high_resolution_clock::now();


    auto t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1 ).count();

    std::cout << duration;

    //    String::Set(0, "key", "val");
//    auto& res = String::Get(0, "key");
    int j = sizeof(String);
    return 0;
}
