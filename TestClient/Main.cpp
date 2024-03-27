#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <XephNet/Client.h>

void ReceiveData(std::vector<uint8_t> data);

int main(int argc, char* argv[])
{
    std::ifstream file("ip.txt");
    std::string ip;
    file >> ip;

    xe::Client::Connect(ip, 98798, ReceiveData);
    std::string line;
    while (line != "exit")
    {
        std::getline(std::cin, line);
        std::vector<uint8_t> data;
        data.resize(line.length() + 1);
        memcpy(data.data(), line.c_str(), data.size());

        xe::Client::SendToServer(data);
    }

    return 0;
}

void ReceiveData(std::vector<uint8_t> data)
{
    const char* str = (const char*)data.data();
    std::cout << str << std::endl;
}
