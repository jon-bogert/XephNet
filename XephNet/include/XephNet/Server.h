#pragma once
#include <SFML/Network.hpp>

#include <functional>
#include <thread>
#include <mutex>
#include <map>

namespace xe
{
	class Server
	{
	public:
		~Server() { Close(); }
		Server(const Server&) = delete;
		Server(const Server&&) = delete;
		Server operator=(const Server&) = delete;
		Server operator=(const Server&&) = delete;

		static void Open(const uint16_t port, std::function<void(std::vector<uint8_t>, uint32_t)> callback = nullptr);
		static void Close();

		static void SetConnectCallback(std::function<void(size_t)> callback);
		static void SetUpdateCallback(std::function<void(std::vector<uint8_t>, uint32_t)> callback);
		static void SetDisconnectCallback(std::function<void(size_t)> callback);

		static bool IsOpen();
		static uint16_t Port() { return Get().m_port; }

		static void SendToAll(const std::vector<uint8_t>& data);
		static void SendToOne(uint32_t id, const std::vector<uint8_t>& data);
		static void Kick(uint32_t id);

	private:
		Server() = default;
		static Server& Get() { static Server instance; return instance; }

		void Update();
		void HandleIncoming();
		void HandleDisconnections();
		void HandleOutgoing();

		void SetIsOpen(const bool isOpen);
		void GenerateNextID();

		bool m_isOpen = false;
		std::mutex m_isOpenMutex;
		uint16_t m_port = 0;
		std::thread m_thread;
		sf::TcpListener* m_listener = nullptr;
		std::mutex m_clientsMutex;
		std::map<uint32_t, sf::TcpSocket*> m_clients;
		uint32_t m_nextID = 0;

		std::vector<uint32_t> m_disconnectQueue;
		std::vector<std::pair<uint32_t, std::vector<uint8_t>>> m_oneDataQueue;
		std::vector<std::vector<uint8_t>> m_allDataQueue;

		std::function<void(std::vector<uint8_t>, uint32_t)> m_updateCallback;
		std::function<void(uint32_t)> m_connectCallback;
		std::function<void(uint32_t)> m_disconnectCallback;

		std::mutex m_updateMutex;
		std::mutex m_connectMutex;
		std::mutex m_disconnectMutex;
		std::mutex m_oneDataMutex;
		std::mutex m_allDataMutex;
	};
}

