/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotCommandServer.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/thread_pool.hpp>
#include <cstdlib>
#include <iostream>
#include <mutex>

#include "IoContext.h"
#include "Playerbots.h"

using boost::asio::ip::tcp;
typedef boost::shared_ptr<tcp::socket> socket_ptr;

std::mutex session_mutex;  // Global mutex to protect sessions
boost::asio::thread_pool pool(4);  // 4-thread thread pool

bool ReadLine(socket_ptr sock, std::string* buffer, std::string* line)
{
    // Check if the socket is valid before using it
    if (!sock || !sock->is_open())
    {
        LOG_ERROR("playerbots", "Invalid or closed socket.");
        return false;
    }

    // Does the actual reading until the buffer has a '\n'
    std::string::iterator pos;
    while ((pos = find(buffer->begin(), buffer->end(), '\n')) == buffer->end())
    {
        char buf[1025];
        boost::system::error_code error;

        // Read socket data
        size_t n = sock->read_some(boost::asio::buffer(buf), error);
        if (n == 0 || error == boost::asio::error::eof)
        {
            LOG_INFO("playerbots", "Connection closed by peer.");
            return false;
        }
        else if (error)
        {
            LOG_ERROR("playerbots", "Socket read error: {}", error.message());
            return false;  // Returns false in case of error.
        }

        buf[n] = 0;  // Ensures the buffer ends with null
        *buffer += buf;
    }

    *line = std::string(buffer->begin(), pos);
    *buffer = std::string(pos + 1, buffer->end());
    return true;
}

void session(socket_ptr sock)
{
    try
    {
        std::lock_guard<std::mutex> guard(session_mutex);  // Protect session with mutex

        std::string buffer, request;
        while (ReadLine(sock, &buffer, &request))
        {
            std::string const response = sRandomPlayerbotMgr->HandleRemoteCommand(request) + "\n";
            boost::asio::write(*sock, boost::asio::buffer(response.c_str(), response.size()));
            request = "";
        }
    }
    catch (std::exception& e)
    {
        LOG_ERROR("playerbots", "Session error: {}", e.what());
    }

    // Make sure to close the socket at the end of the session
    if (sock && sock->is_open())
    {
        boost::system::error_code ec;
        sock->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        sock->close(ec);
        if (ec)
        {
            LOG_ERROR("playerbots", "Error closing socket: {}", ec.message());
        }
    }
}

void server(Acore::Asio::IoContext& io_service, short port)
{
    tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
    for (;;)
    {
        socket_ptr sock(new tcp::socket(io_service));
        a.accept(*sock);
        boost::asio::post(pool, boost::bind(session, sock));  // Use thread pool instead of creating new threads
    }
}

void Run()
{
    if (!sPlayerbotAIConfig->commandServerPort)
    {
        return;
    }

    std::ostringstream s;
    s << "Starting Playerbots Command Server on port " << sPlayerbotAIConfig->commandServerPort;
    LOG_INFO("playerbots", "{}", s.str().c_str());

    try
    {
        Acore::Asio::IoContext io_service;
        server(io_service, sPlayerbotAIConfig->commandServerPort);
    }

    catch (std::exception& e)
    {
        LOG_ERROR("playerbots", "{}", e.what());
    }
}

void PlayerbotCommandServer::Start()
{
    std::thread serverThread(Run);
    serverThread.detach();
}
