#include <XephNet/Server.h>

#include <iostream>
#include <random>

using namespace xe;

void xe::Server::Open(const uint16_t port, std::function<void(std::vector<uint8_t>, uint32_t)> callback)
{
	Server& server = Get();
	if (server.m_listener != nullptr)
	{
		std::cout << "Server is already active" << std::endl;
		return;
	}
	server.GenerateNextID();

	server.m_listener = new sf::TcpListener();
	server.m_port = port;

	if (server.m_listener->listen(port) != sf::Socket::Done)
	{
		std::cout << "Server could not open port " << port << std::endl;
		delete server.m_listener;
		return;
	}

	server.m_listener->setBlocking(false);

	server.SetIsOpen(true);

	if (callback)
	{
		server.m_updateMutex.lock();
		server.m_updateCallback = callback;
		server.m_updateMutex.unlock();
	}

	server.m_thread = std::thread(std::bind(&Server::Update, &server));
}

void xe::Server::Close()
{
	Server& server = Get();
	server.m_clientsMutex.lock();
	if (server.m_listener == nullptr)
	{
		if (server.m_clients.size() > 0)
		{
			std::cout << "Server client list was not empty while listener was null" << std::endl;
		}
		server.m_clientsMutex.unlock();
		return;
	}

	server.SetIsOpen(false);
	server.m_thread.join();

	for (auto client : server.m_clients)
	{
		server.m_disconnectMutex.lock();
		if (server.m_disconnectCallback)
			server.m_disconnectCallback(client.first);
		server.m_disconnectMutex.unlock();

		client.second->disconnect();
		delete client.second;
	}
	server.m_clients.clear();
	server.m_clientsMutex.unlock();

	delete server.m_listener;
	server.m_listener = nullptr;
}

void xe::Server::SetConnectCallback(std::function<void(size_t)> callback)
{
	Get().m_connectMutex.lock();
	Get().m_connectCallback = callback;
	Get().m_connectMutex.unlock();
}

void xe::Server::SetUpdateCallback(std::function<void(std::vector<uint8_t>, uint32_t)> callback)
{
	Get().m_updateMutex.lock();
	Get().m_updateCallback = callback;
	Get().m_updateMutex.unlock();
}

void xe::Server::SetDisconnectCallback(std::function<void(size_t)> callback)
{
	Get().m_disconnectMutex.lock();
	Get().m_disconnectCallback = callback;
	Get().m_disconnectMutex.unlock();
}

bool xe::Server::IsOpen()
{
	bool result{};
	Get().m_isOpenMutex.lock();
	result = Get().m_isOpen;
	Get().m_isOpenMutex.unlock();
	return result;
}

void xe::Server::SendToAll(const std::vector<uint8_t>& data)
{
	Server& server = Get();
	server.m_allDataMutex.lock();
	server.m_allDataQueue.push_back(data);
	server.m_allDataMutex.unlock();
}

void xe::Server::SendToOne(uint32_t id, const std::vector<uint8_t>& data)
{
	Server& server = Get();
	server.m_oneDataMutex.lock();
	server.m_oneDataQueue.push_back({ id, data });
	server.m_oneDataMutex.unlock();
}

void xe::Server::Kick(uint32_t id)
{
	Server& server = Get();
	server.m_disconnectMutex.lock();
	server.m_disconnectQueue.push_back(id);
	server.m_disconnectMutex.unlock();
}

void xe::Server::Update()
{
	bool isOpen = true;
	while (isOpen)
	{
		HandleIncoming();
		HandleDisconnections();
		HandleOutgoing();

		isOpen = IsOpen();
	}
}

void xe::Server::HandleIncoming()
{
	// Check for new Client Connection
	sf::TcpSocket* client = new sf::TcpSocket;
	if (m_listener->accept(*client) == sf::Socket::Done)
	{
		m_clientsMutex.lock();
		client->setBlocking(false);
		m_clients[m_nextID] = client;
		m_clientsMutex.unlock();

		m_connectMutex.lock();
		if (m_connectCallback)
			m_connectCallback(m_nextID);
		m_connectMutex.unlock();

		GenerateNextID();
	}
	else
	{
		delete client;
	}

	//Check new data from clients
	for (auto iter = m_clients.begin(); iter != m_clients.end(); ++iter)
	{
		sf::TcpSocket* client = iter->second;
		sf::Packet packet;
		sf::Socket::Status status = client->receive(packet);

		//There was data received;
		if (status == sf::Socket::Done)
		{
			std::vector<uint8_t> data;
			data.resize(packet.getDataSize());
			memcpy(data.data(), packet.getData(), data.size());

			m_updateMutex.lock();
			if (m_updateCallback)
				m_updateCallback(data, iter->first);
			m_updateMutex.unlock();
		}

		//Delete Connection if Disconnected
		else if (status == sf::Socket::Disconnected)
		{
			m_disconnectMutex.lock();
			m_disconnectQueue.push_back(iter->first);
			m_disconnectMutex.unlock();
		}
	}
}

void xe::Server::HandleDisconnections()
{
	m_clientsMutex.lock();
	m_disconnectMutex.lock();
	for (const uint32_t id : m_disconnectQueue)
	{
		if (m_disconnectCallback)
			m_disconnectCallback(id);

		delete m_clients[id];
		m_clients.erase(id);
	}
	m_disconnectQueue.clear();
	m_disconnectMutex.unlock();
	m_clientsMutex.unlock();
}

void xe::Server::HandleOutgoing()
{
	//Single Client
	m_oneDataMutex.lock();
	for (const auto& iter : m_oneDataQueue)
	{
		uint32_t id = iter.first;
		const std::vector<uint8_t>& data = iter.second;
		if (m_clients.find(id) == m_clients.end())
		{
			std::cout << "Couldn't find client with ID " << id << " to send data" << std::endl;
			return;
		}

		sf::Packet packet;
		packet.append(data.data(), data.size());
		if (m_clients[id]->send(packet) != sf::Socket::Done)
		{
			std::cout << "Couldn't send data to client with ID " << id << std::endl;
		}
	}
	m_oneDataQueue.clear();
	m_oneDataMutex.unlock();

	//All Clients
	m_allDataMutex.lock();
	for (const auto& data : m_allDataQueue)
	{
		sf::Packet packet;
		packet.append(data.data(), data.size());
		for (const auto client : m_clients)
		{
			if (client.second->send(packet) != sf::Socket::Done)
			{
				std::cout << "Couldn't send data to client with ID " << client.first << std::endl;
			}
		}
	}
	m_allDataQueue.clear();
	m_allDataMutex.unlock();
}

void xe::Server::SetIsOpen(const bool isOpen)
{
	m_isOpenMutex.lock();
	m_isOpen = isOpen;
	m_isOpenMutex.unlock();
}

void xe::Server::GenerateNextID()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<uint32_t> dis(0, UINT32_MAX);
	m_nextID = dis(gen);
}
