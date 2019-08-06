#include <iostream>
#include <thread>

static bool s_Finished = false;

// void DoWork()
// {
//     using namespace std::literals::chrono_literals;
//
//     while (!s_Finished)
//     {
//         std::cout << "working..." << std::endl;
//         std::this_thread::sleep_for(1s);
//     }
// }

int main()
{
    std::string name;

    std::cout << "Hello " << std::endl;

    // std::thread worker(DoWork);
    //
    // std::cin.get();
    // s_Finished = true;

    std::cout << "Enter your name" << std::endl;

    std::cin >> name;

    std::cout << "OK " << name << ", welcome to da' par-tay!!" << std::endl;

    // worker.join();
    // std::cin.get();

    unsigned int n = std::thread::hardware_concurrency();
    std::cout << n << " concurrent threads are supported." << std::endl;

    return 0;
}
