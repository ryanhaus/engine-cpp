#pragma once
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include <thread>

struct HeadUpdate
{
	char username[25];
	float head[6];
};

using boost::asio::ip::udp;
boost::asio::io_service io_service;
udp::socket s(io_service, udp::endpoint(udp::v4(), 0));

udp::resolver resolver(io_service);
udp::resolver::query query(udp::v4(), "localhost", "1234");
udp::resolver::iterator iterator = resolver.resolve(query);

enum { max_length = 1024 };

char* socketSend(const char* request)
{
	try
	{
		size_t request_length = std::strlen(request);
		s.send_to(boost::asio::buffer(request, request_length), *iterator);

		char reply[max_length];
		udp::endpoint sender_endpoint;
		size_t reply_length = s.receive_from(boost::asio::buffer(reply, max_length), sender_endpoint);
		char* data = new char[reply_length];
		std::memcpy(data, &reply, reply_length);
		return data;
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

void socketReceiveHeads(udp::socket* socket, std::map<std::string, std::array<float, 6>>* playerMap, std::vector<std::pair<std::string, std::array<float, 6>>>* newPlayers)
{
	for (;;)
	{
		try
		{
			char reply[max_length];
			udp::endpoint sender_endpoint;
			size_t reply_length = socket->receive_from(boost::asio::buffer(reply, max_length), sender_endpoint);

			struct HeadUpdate update;
			memcpy(&update, (struct HeadUpdate*)reply, sizeof(struct HeadUpdate));

			std::array<float, 6> head;
			std::copy(std::begin(update.head), std::end(update.head), head.begin());

			if (players.find(std::string(update.username)) == players.end())
				newPlayers->push_back(std::make_pair(std::string(update.username), head));

			playerMap->insert_or_assign(std::string(update.username), head);
		}
		catch (std::exception& e)
		{
			std::cerr << "Exception: " << e.what() << "\n";
		}
	}
}

void socketSendHead()
{
	try
	{
		struct HeadUpdate update =
		{
			"",
			{
				0.0f,
				0.0f,
				0.0f,
				0.0f,
				0.0f,
				0.0f
			}
		};

		lua_getglobal(Lua, "Game");
		lua_getfield(Lua, -1, "Local");
		lua_getfield(Lua, -1, "LocalPlayer");
		lua_getfield(Lua, -1, "Name");

		std::memcpy(update.username, lua_tostring(Lua, -1), 24);

		lua_settop(Lua, 0);
		lua_getglobal(Lua, "Game");
		lua_getfield(Lua, -1, "Local");
		lua_getfield(Lua, -1, "LocalPlayer");
		lua_getfield(Lua, -1, "Head");
		lua_getfield(Lua, -1, "Position");

		float c;
		lua_getfield(Lua, -1, "x");
		c = lua_tonumber(Lua, -1);
		std::memcpy(&update.head[0], &c, sizeof(float));
		lua_getfield(Lua, -2, "y");
		c = lua_tonumber(Lua, -1);
		std::memcpy(&update.head[1], &c, sizeof(float));
		lua_getfield(Lua, -3, "z");
		c = lua_tonumber(Lua, -1);
		std::memcpy(&update.head[2], &c, sizeof(float));

		lua_settop(Lua, 0);
		lua_getglobal(Lua, "Game");
		lua_getfield(Lua, -1, "Local");
		lua_getfield(Lua, -1, "LocalPlayer");
		lua_getfield(Lua, -1, "Head");
		lua_getfield(Lua, -1, "Orientation");

		lua_getfield(Lua, -1, "x");
		c = lua_tonumber(Lua, -1);
		std::memcpy(&update.head[3], &c, sizeof(float));
		lua_getfield(Lua, -2, "y");
		c = lua_tonumber(Lua, -1);
		std::memcpy(&update.head[4], &c, sizeof(float));
		lua_getfield(Lua, -3, "z");
		c = lua_tonumber(Lua, -1);
		std::memcpy(&update.head[5], &c, sizeof(float));

		unsigned char* buffer = (unsigned char*)malloc(sizeof(update));
		memcpy(buffer, (const unsigned char*)&update, sizeof(update));

		s.send_to(boost::asio::buffer(buffer, sizeof(update)), *iterator);
		free(buffer);
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
}