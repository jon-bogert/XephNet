#include <XephNet/Client.h>

#include <iostream>

using namespace xe;

void xe::Client::Connect(const std::string& ipAddress, const uint16_t port, std::function<void(std::vector<uint8_t>)> callback)
{
	Client& client = Get();
	if (client.m_server.connect(ipAddress, port) != sf::Socket::Done)
	{
		std::cout << "Could not connect to server " << ipAddress << ":" << port << std::endl;

		client.m_disconnectMutex.lock();
		if (client.m_disconnectCallback)
			client.m_disconnectCallback();
		client.m_disconnectMutex.unlock();
		return;
	}

	if (callback)
	{
		client.m_receiveMutex.lock();
		client.m_receiveCallback = callback;
		client.m_receiveMutex.unlock();
	}

	client.m_server.setBlocking(false);
	client.SetIsConnected(true);

	client.m_connectMutex.lock();
	if (client.m_connectCallback)
		client.m_connectCallback();
	client.m_connectMutex.unlock();

	client.m_thread = std::thread(std::bind(&Client::FromServerLoop, &client));
}

void xe::Client::Disconnect()
{
	Client& client = Get();
	if (!client.IsConnected())
	{
		return;
	}

	client.SetIsConnected(false);
	client.m_thread.join();

	client.m_disconnectMutex.lock();
	if (client.m_disconnectCallback)
		client.m_disconnectCallback();
	client.m_disconnectMutex.unlock();

	client.m_server.disconnect();
}

void xe::Client::SendToServer(std::vector<uint8_t> data)
{
	Client& client = Get();
	if (!client.IsConnected())
	{
		std::cout << "Client is not connected to server" << std::endl;
		return;
	}
	sf::Packet packet;
	packet.append(data.data(), data.size());
	client.m_server.send(packet);
}

void xe::Client::SetConnectCallback(std::function<void(void)> callback)
{
	Get().m_connectMutex.lock();
	Get().m_connectCallback = callback;
	Get().m_connectMutex.unlock();
}

void xe::Client::SetReceiveCallback(std::function<void(std::vector<uint8_t>)> callback)
{
	Get().m_receiveMutex.lock();
	Get().m_receiveCallback = callback;
	Get().m_receiveMutex.unlock();
}

void xe::Client::SetDisconnectCallback(std::function<void(void)> callback)
{
	Get().m_disconnectMutex.lock();
	Get().m_disconnectCallback = callback;
	Get().m_disconnectMutex.unlock();
}

bool xe::Client::IsConnected()
{
	bool result{};
	Get().m_isConnectedMutex.lock();
	result = Get().m_isConnected;
	Get().m_isConnectedMutex.unlock();
	return result;
}

void xe::Client::FromServerLoop()
{
	bool isConnected = true;
	while (isConnected)
	{
		sf::Packet packet;
		sf::Socket::Status status = m_server.receive(packet);
		if (status == sf::Socket::Done)
		{
			std::vector<uint8_t> data;
			data.resize(packet.getDataSize());
			memcpy(data.data(), packet.getData(), data.size());

			m_receiveMutex.lock();
			if (m_receiveCallback)
				m_receiveCallback(data);
			m_receiveMutex.unlock();
		}
		else if (status == sf::Socket::Disconnected)
		{
			m_disconnectMutex.lock();
			if (m_disconnectCallback)
				m_disconnectCallback();
			m_disconnectMutex.unlock();
		}

		isConnected = IsConnected();
	}
}

void xe::Client::SetIsConnected(const bool isConnected)
{
	Get().m_isConnectedMutex.lock();
	Get().m_isConnected = isConnected;
	Get().m_isConnectedMutex.unlock();
}
