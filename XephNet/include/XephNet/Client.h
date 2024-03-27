#pragma once
#include <SFML/Network.hpp>

#include <functional>
#include <thread>
#include <mutex>

namespace xe
{
	class Client
	{
	public:
		~Client() { Disconnect(); }
		Client(const Client&) = delete;
		Client(const Client&&) = delete;
		Client operator=(const Client&) = delete;
		Client operator=(const Client&&) = delete;

		static void Connect(const std::string& ipAddress, const uint16_t port, std::function<void(std::vector<uint8_t>)> callback = nullptr);
		static void Disconnect();

		static void SendToServer(std::vector<uint8_t> data);


		static void SetConnectCallback(std::function<void(void)> callback);
		static void SetReceiveCallback(std::function<void(std::vector<uint8_t>)> callback);
		static void SetDisconnectCallback(std::function<void(void)> callback);

		static bool IsConnected();

	private:
		Client() = default;
		static Client& Get() { static Client instance; return instance; }

		void FromServerLoop();
		void SetIsConnected(const bool isConnected);

		std::mutex m_isConnectedMutex;
		bool m_isConnected = false;

		sf::TcpSocket m_server;
		std::thread m_thread;

		std::function<void(std::vector<uint8_t>)> m_receiveCallback;
		std::function<void(void)> m_connectCallback;
		std::function<void(void)> m_disconnectCallback;

		std::mutex m_receiveMutex;
		std::mutex m_connectMutex;
		std::mutex m_disconnectMutex;

		
	};
}

