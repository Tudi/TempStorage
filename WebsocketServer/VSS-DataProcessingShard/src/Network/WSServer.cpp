#include <iostream>
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"
#include "websocketpp/common/thread.hpp"
#include "Network/NetworkSession.h"
#include "Network/NetworkSessionManager.h"
#include "WSServer.h"
#include "LogManager.h"
#include "ResourceManager/ServerHealthReportingManager.h"

//#define UPDATED_WEBSOCKET_LIB_AND_DID_NOT_APPLY_CUSTOM_CHANGES

class WebSocketServer 
{
public:
    WebSocketServer()
    {
        server.init_asio();
        server.set_message_handler(std::bind(&WebSocketServer::on_message, this, std::placeholders::_1, std::placeholders::_2));
        server.set_open_handler(std::bind(&WebSocketServer::on_open, this, std::placeholders::_1));
        server.set_close_handler(std::bind(&WebSocketServer::on_close, this, std::placeholders::_1));

#ifdef _DEBUG
        server.set_error_channels(websocketpp::log::elevel::all);
        server.set_access_channels(websocketpp::log::alevel::all);
        server.clear_access_channels(websocketpp::log::alevel::frame_header);
        server.clear_access_channels(websocketpp::log::alevel::frame_payload);
#else
        server.clear_access_channels(websocketpp::log::alevel::all);
        server.clear_error_channels(websocketpp::log::elevel::all);
#endif

    }

    void start_server(uint16_t port) 
    {
        server.listen(port);
        server.start_accept();
        server_thread = std::thread([&]() { server.run(); });
    }

    void stop_server() 
    {
        server.stop();
        server_thread.join();
    }

    bool send_message(const char *data, const size_t len, const __int64 client_id)
    {
        // broadcast ?
        if (client_id == 0)
        {
            for (auto itr = hdl_map.begin(); itr != hdl_map.end(); itr++)
            {
                sServerHealthReportingManager.ReportPacketSent(len + 14 + 20 + 20 + 14); // Ethernet + IP + TCP + WS header
                server.send(itr->second, data, len, websocketpp::frame::opcode::binary);
            }
            return true;
        }
        else
        {
            // send to specific client only
            auto itr = hdl_map.find(client_id);
            if (itr != hdl_map.end())
            {
                std::error_code ec;
                sServerHealthReportingManager.ReportPacketSent(len + 14 + 20 + 20 + 14); // Ethernet + IP + TCP + WS header
                server.send(itr->second, data, len, websocketpp::frame::opcode::binary, ec);
                if (ec)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
        }
        return false;
    }

    size_t GetClientConnectedCount() { return sess_map.size(); }
private:
    void on_open(websocketpp::connection_hdl hdl) 
    {
        // Store the connection handle and client ID in the map when a new client connects
        NetworkSession *ns = sNetworkSessionManager.CreateSession();
        hdl_map[ns->GetSessionId()] = hdl;

        void* ptr = hdl.lock().get();
        sess_map[ptr] = ns;

        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWSServer, 0, 0,
            "WSServer:Client id %d connected to the server.", ns->GetSessionId());

#ifndef UPDATED_WEBSOCKET_LIB_AND_DID_NOT_APPLY_CUSTOM_CHANGES
        std::shared_ptr<websocketpp::connection<websocketpp::config::asio>> con = server.get_con_from_hdl(hdl);
        if (con) 
        { 
            con->set_user_data(ns);
        }
        else
        {
            AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceWSServer, 0, 0,
                "WSServer:Failed to get pointer. Aborting.");
        }
#endif
    }

    void on_close(websocketpp::connection_hdl hdl) 
    {
        hdl;
		// TODO : Check if this needs to be thread safe
        // Remove the entry from the map when a client disconnects
        void* removePtr = hdl.lock().get();
        auto it = sess_map.find(removePtr);
        __int64 removeId = 0;
        if (it != sess_map.end())
        {
            NetworkSession* ns = it->second;
            removeId = ns->GetSessionId();
            sess_map.erase(it);
            sNetworkSessionManager.DestroySession(ns);
        }
        auto it2 = hdl_map.find(removeId);
        if (it2 != hdl_map.end())
        {
            hdl_map.erase(it2);
        }
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWSServer, 0, 0,
            "WSServer:Client id %d disconnected from the server.", removeId);
    }
    void on_message(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg) 
    {
#ifndef UPDATED_WEBSOCKET_LIB_AND_DID_NOT_APPLY_CUSTOM_CHANGES
        sServerHealthReportingManager.ReportPacketRecv(msg->get_raw_payload().size() + 14 + 20 + 20 + 14); // Ethernet + IP + TCP + WS header
        std::shared_ptr<websocketpp::connection<websocketpp::config::asio>> con = server.get_con_from_hdl(hdl);
        if (con)
        {
            NetworkSession* ns = (NetworkSession *)con->get_user_data();
            ns->OnMessage(msg->get_raw_payload());

            sNetworkSessionManager.WakeUpWorkerThread();
        }
        else
        {
            AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceWSServer, 0, 0,
                "WSServer:Failed to get pointer. Aborting.");
        }
#else
        void* ptr = hdl.lock().get();
        auto itr = sess_map.find(ptr);
        if (itr != sess_map.end())
        {
            itr->second->OnMessage(msg->get_raw_payload());
            sNetworkSessionManager.WakeUpWorkerThread();
        }
        else
        {
            AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceWSServer, 0, 0,
                "WSServer:Failed to find connection info for received packet. Aborting.");
        }
#endif
    }

    websocketpp::server<websocketpp::config::asio> server;
    std::thread server_thread;
    // would be great if we could just assign a session object to a connection. Maybe some other library ..
    // would be great to be able to copy connection_hdl to sessions ... maybe some other version
    std::map<__int64, websocketpp::connection_hdl> hdl_map;
    std::map<void *, NetworkSession *> sess_map;
};

VSSWSServer::VSSWSServer()
{
    m_pWSServer = NULL;
}

bool VSSWSServer::StartServer(unsigned short port)
{
    if (m_pWSServer == NULL)
    {
        WebSocketServer *wsserver = new WebSocketServer();
        wsserver->start_server(port);
        m_pWSServer = wsserver;
        return true;
    }

    return false;
}

bool VSSWSServer::StopServer()
{
    WebSocketServer* wsserver = (WebSocketServer*)m_pWSServer;
    wsserver->stop_server();
    delete wsserver;
    m_pWSServer = NULL;
    return true;
}

bool VSSWSServer::SendMsg(const char* data, const size_t len, const __int64 ClientOnly)
{
    if (m_pWSServer)
    {
        WebSocketServer* wsserver = (WebSocketServer*)m_pWSServer;
        return wsserver->send_message(data, len, ClientOnly);
    }

    return false;
}

size_t VSSWSServer::GetClientConnectedCount()
{
    WebSocketServer* wsserver = (WebSocketServer*)m_pWSServer;
    return wsserver->GetClientConnectedCount();
}