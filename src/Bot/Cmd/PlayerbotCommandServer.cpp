/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PlayerbotCommandServer.h"
#include "IoContext.h"
#include "RandomPlayerbotMgr.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <cstdlib>

using boost::asio::ip::tcp;
typedef boost::shared_ptr<tcp::socket> socket_ptr;

bool ReadLine(socket_ptr sock, std::string* buffer, std::string* line)
{
    // Do the real reading from fd until buffer has '\n'.
    std::string::iterator pos;
    while ((pos = find(buffer->begin(), buffer->end(), '\n')) == buffer->end())
    {
        char buf[1025];
        boost::system::error_code error;
        size_t n = sock->read_some(boost::asio::buffer(buf), error);
        if (n == static_cast<size_t>(-1) || error == boost::asio::error::eof)
            return false;
        else if (error)
            throw boost::system::system_error(error);  // Some other error.

        buf[n] = 0;
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
        std::string buffer, request;
        while (ReadLine(sock, &buffer, &request))
        {
            std::string const response = RandomPlayerbotMgr::instance().HandleRemoteCommand(request) + "\n";
            boost::asio::write(*sock, boost::asio::buffer(response.c_str(), response.size()));
            request = "";
        }
    }
    catch (std::exception& e)
    {
        LOG_ERROR("playerbots", "{}", e.what());
    }
}

void server(Acore::Asio::IoContext& io_service, short port)
{
    tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
    for (;;)
    {
        socket_ptr sock(new tcp::socket(io_service));
        a.accept(*sock);
        boost::thread t(boost::bind(session, sock));
    }
}

void Run()
{
    if (!sPlayerbotAIConfig.CommandServerPort)
    {
        return;
    }

    std::ostringstream s;
    s << "Starting Playerbots Command Server on port " << sPlayerbotAIConfig.CommandServerPort;
    LOG_INFO("playerbots", "{}", s.str().c_str());

    try
    {
        Acore::Asio::IoContext io_service;
        server(io_service, sPlayerbotAIConfig.CommandServerPort);
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
