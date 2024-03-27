#include <iostream>

#include <XephNet/Server.h>

void Connect(int index);
void Update(std::vector<uint8_t> data, uint32_t id);
void Disconnect(int index);

int main(int argc, char* argv[])
{
    xe::Server::SetConnectCallback(Connect);
    xe::Server::SetDisconnectCallback(Disconnect);
    xe::Server::Open(98798, Update);

    while (true)
    {

    }

    xe::Server::Close();

    return 0;
}

void Connect(int index)
{
    std::cout << "Connection at index " << index << std::endl;
}

void Update(std::vector<uint8_t> data, uint32_t id)
{
    const char* str = (const char*)data.data();
    std::cout << str << std::endl;

    std::string response = std::string("Server Received: ") + str;

    std::vector<uint8_t> respData;
    respData.resize(response.length() + 1);
    memcpy(respData.data(), response.c_str(), respData.size());

    xe::Server::SendToOne(id, respData);
}

void Disconnect(int index)
{
    std::cout << "Disconnection at index " << index << std::endl;
}
