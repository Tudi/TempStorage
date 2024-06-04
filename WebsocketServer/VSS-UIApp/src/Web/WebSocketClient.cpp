#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <string>
#include <set>
#include "Network/VSSPacketDefines.h"
#include "Session/ApplicationSession.h"
#include "UserSession.h"
#include "Util/Allocator.h"
#include "ResourceManager/LogManager.h"
#include "WebSocketClient.h"
#include "ResourceManagers/RadarDataSourceManager.h"
#include "ResourceManagers/AlertCacheManager.h"
#include "ResourceManagers/stdafx.h"

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// Define the client type
//#define FIXED_TLS_HANDSHAKE_ISSUE
#ifdef FIXED_TLS_HANDSHAKE_ISSUE
    typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
#else
    typedef websocketpp::client<websocketpp::config::asio_client> client;
#endif

typedef websocketpp::config::asio_tls_client::message_type::ptr message_ptr;
typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;
using connection_ptr = client::connection_ptr;

#ifdef FIXED_TLS_HANDSHAKE_ISSUE
int strcasecmp(const char* s1, const char* s2)
{
    while (*s1 != '\0' && *s2 != '\0') {
        int diff = tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
        if (diff != 0) {
            return diff;
        }
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

bool verify_subject_alternative_name(const char* hostname, X509* cert) {
    STACK_OF(GENERAL_NAME)* san_names = NULL;

    san_names = (STACK_OF(GENERAL_NAME)*) X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
    if (san_names == NULL) {
        return false;
    }

    int san_names_count = sk_GENERAL_NAME_num(san_names);

    bool result = false;

    for (int i = 0; i < san_names_count; i++) {
        const GENERAL_NAME* current_name = sk_GENERAL_NAME_value(san_names, i);

        if (current_name->type != GEN_DNS) {
            continue;
        }

        char const* dns_name = (char const*)ASN1_STRING_get0_data(current_name->d.dNSName);

        // Make sure there isn't an embedded NUL character in the DNS name
        if (ASN1_STRING_length(current_name->d.dNSName) != strlen(dns_name)) {
            break;
        }
        // Compare expected hostname with the CN
        result = (strcasecmp(hostname, dns_name) == 0);
    }
    sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);

    return result;
}

bool verify_common_name(char const* hostname, X509* cert) {
    // Find the position of the CN field in the Subject field of the certificate
    int common_name_loc = X509_NAME_get_index_by_NID(X509_get_subject_name(cert), NID_commonName, -1);
    if (common_name_loc < 0) {
        return false;
    }

    // Extract the CN field
    X509_NAME_ENTRY* common_name_entry = X509_NAME_get_entry(X509_get_subject_name(cert), common_name_loc);
    if (common_name_entry == NULL) {
        return false;
    }

    // Convert the CN field to a C string
    ASN1_STRING* common_name_asn1 = X509_NAME_ENTRY_get_data(common_name_entry);
    if (common_name_asn1 == NULL) {
        return false;
    }

    char const* common_name_str = (char const*)ASN1_STRING_get0_data(common_name_asn1);

    // Make sure there isn't an embedded NUL character in the CN
    if (ASN1_STRING_length(common_name_asn1) != strlen(common_name_str)) {
        return false;
    }

    // Compare expected hostname with the CN
    return (strcasecmp(hostname, common_name_str) == 0);
}

bool verify_certificate(const char* hostname, bool preverified, boost::asio::ssl::verify_context& ctx) {
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // Retrieve the depth of the current cert in the chain. 0 indicates the
    // actual server cert, upon which we will perform extra validation
    // (specifically, ensuring that the hostname matches. For other certs we
    // will use the 'preverified' flag from Asio, which incorporates a number of
    // non-implementation specific OpenSSL checking, such as the formatting of
    // certs and the trusted status based on the CA certs we imported earlier.
    int depth = X509_STORE_CTX_get_error_depth(ctx.native_handle());

    // if we are on the final cert and everything else checks out, ensure that
    // the hostname is present on the list of SANs or the common name (CN).
    if (depth == 0 && preverified) {
        X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());

        if (verify_subject_alternative_name(hostname, cert)) {
            return true;
        }
        else if (verify_common_name(hostname, cert)) {
            return true;
        }
        else {
            return false;
        }
    }

    return preverified;
}

class PasswordCallback {
public:
    std::string operator()(std::size_t, boost::asio::ssl::context_base::password_purpose) const {
        return "1234";
    }
};

context_ptr on_tls_init(const char* hostname, websocketpp::connection_hdl) {
    context_ptr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);

    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
//            boost::asio::ssl::context::no_sslv2 |
//            boost::asio::ssl::context::no_sslv3 |
//            boost::asio::ssl::context::tlsv13 | 
//            boost::asio::ssl::context::tlsv13_client |
            boost::asio::ssl::context::single_dh_use
        );


//        ctx->set_verify_mode(boost::asio::ssl::verify_peer);
        ctx->set_verify_mode(boost::asio::ssl::verify_none);
        ctx->set_verify_callback(bind(&verify_certificate, hostname, ::_1, ::_2));

        // Here we load the CA certificates of all CA's that this client trusts.
//        ctx->load_verify_file("./Data/cert.pem");

        // Load the certificate chain from a PEM file
        ctx->use_certificate_chain_file("./Data/cert.pem");

        // Load the private key from a PEM file with password
        ctx->use_private_key_file("./Data/key.pem", boost::asio::ssl::context::pem);

        // Set the password callback
        PasswordCallback password_callback;
        ctx->set_password_callback(password_callback);

    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return ctx;
}
#endif

class WebSocketClientHolder
{
public:
    WebSocketClientHolder():
        m_ctx(NULL)
    {
        m_bIsUsable = false;
        m_pThread = NULL;
        m_bIsConnected = false;
        m_bShouldReconnect = false;
        m_OnConnect = NULL;
        m_OnDisconnect = NULL;
        m_OwnerPtr = NULL;
        m_bIsFailedOrDisconnected = false;
    }

    bool Init(const std::string& server_url)
    {
        m_sServer_url = server_url;
        // Set up access channels to log information
#ifdef _DEBUG
        m_client_.set_error_channels(websocketpp::log::elevel::all);
        m_client_.set_access_channels(websocketpp::log::alevel::all);
        m_client_.clear_access_channels(websocketpp::log::alevel::frame_header);
        m_client_.clear_access_channels(websocketpp::log::alevel::frame_payload);
#else
        m_client_.clear_access_channels(websocketpp::log::alevel::all);
        m_client_.clear_error_channels(websocketpp::log::elevel::all);
#endif

        // Set up handlers or configure the client as needed
        m_client_.init_asio();
//        m_client_.start_perpetual();

        m_client_.set_message_handler(bind(&WebSocketClientHolder::on_message, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
        m_client_.set_fail_handler(bind(&WebSocketClientHolder::on_fail, this, websocketpp::lib::placeholders::_1));
        m_client_.set_close_handler(bind(&WebSocketClientHolder::on_close, this, websocketpp::lib::placeholders::_1));
        m_client_.set_open_handler(bind(&WebSocketClientHolder::on_open, this, websocketpp::lib::placeholders::_1));

#ifdef FIXED_TLS_HANDSHAKE_ISSUE
        m_client_.set_tls_init_handler(bind(&on_tls_init, hostname, ::_1));
#endif

        return Reconnect();
    }

    bool Reconnect()
    {
        // Connect to the WebSocket server
        try {
            websocketpp::lib::error_code ec;
            m_con_ = m_client_.get_connection(m_sServer_url, ec);

            if (ec)
            {
                AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceWSClient, 0, 0,
                    "WSClient:Failed to connect to server : %s", m_sServer_url.c_str());
                return false;
            }
            else
            {
                AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWSClient, 0, 0,
                    "WSClient:Connected to server : %s", m_sServer_url.c_str());
            }
            m_client_.connect(m_con_);
        }
        catch (const std::exception& e) 
        {
            AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWSClient, 0, 0,
                "WSClient:Failed to Connected to server : %s. Exception", m_sServer_url.c_str(), e.what());
            return false;
        }
        return true;
    }

    ~WebSocketClientHolder() 
    {
        m_bShouldReconnect = false;
        m_bIsUsable = false;
        // Clean up the connection
//        m_client_.stop_perpetual();
        CloseConnection();
        if (m_pThread->joinable())
        {
            m_pThread->join();
            delete m_pThread;
            m_pThread = NULL;
        }
    }

    void CloseConnection()
    {
        if (m_con_ && m_con_->get_state() == websocketpp::session::state::open)
        {
            m_client_.close(m_con_, websocketpp::close::status::normal, "Closing connection");
        }
    }

    VSS_WS_SocketResults sendMessage(const char *data, size_t len, bool isBinary)
    {
        // Send a message on the stored connection
        websocketpp::lib::error_code ec;
        if (m_con_ && m_con_->get_state() == websocketpp::session::state::open)
        {
            if (isBinary)
            {
                m_client_.send(m_con_, data, len, websocketpp::frame::opcode::binary, ec);
            }
            else
            {
                m_client_.send(m_con_, data, len, websocketpp::frame::opcode::text, ec);
            }
            if (ec)
            {
                return VSS_WS_SocketResults::SR_DROPPED;
            }
            else
            {
                return VSS_WS_SocketResults::SR_NO_ERROR;
            }
        }
        return VSS_WS_SocketResults::SR_DROPPED;
    }

    void run() 
    {
        if (m_con_ 
            && m_con_->get_state() == websocketpp::session::state::connecting
            && m_pThread == NULL)
        {
            // Run the client event loop to process events
            m_pThread = new std::thread(runThread, &m_client_, &m_bIsUsable);
        }
        else
        {
            AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceWSClient, 0, 0,
                "WSClient:Failed to run worker thread for server : %s", m_sServer_url.c_str());
        }
    }

    // Define a handler to process messages from the server
    void on_message(websocketpp::connection_hdl hdl, message_ptr msg) 
    {
        // default packet handling
        const VSS_N_PacketHeader* pktHdr = (VSS_N_PacketHeader *)msg->get_payload().c_str();
        const size_t pktSize = msg->get_payload().size();
        if (pktHdr->Opcode < VSS_WS_Opcodes::VSSWSO_MAX)
        {
            VSSPacketHandlerBase* pktHndl = sPacketHandlers[pktHdr->Opcode];
            pktHndl->ParserFunc((char*)pktHdr, pktSize, &m_ctx);
        }
        else
        {
            AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourcePacketParser, 0, 0,
                "PacketParser:Invalid opcode %d, max %d", pktHdr->Opcode, VSS_WS_Opcodes::VSSWSO_MAX);
        }
    }

    // reconnect on close unless we are shutting down
    void on_fail(websocketpp::connection_hdl hdl) 
    {
        m_bIsFailedOrDisconnected = true;
        m_bIsConnected = false;
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourcePacketParser, 0, 0,
            "WSClient:Connection failed. Going to retry");
        if (m_bShouldReconnect)
        {
            Reconnect();
        }
    }

    void on_open(websocketpp::connection_hdl hdl) 
    {
        m_bIsFailedOrDisconnected = false;
        m_bIsConnected = true;

        if (m_OnConnect)
        {
            m_OnConnect(m_OwnerPtr);
        }

        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWSClient, 0, 0,
            "WSClient:Connected to server : %s", m_sServer_url.c_str());

        m_ctx.wsClient->SendHandshakeToServer();
    }

    // reconnect on close unless we are shutting down
    void on_close(websocketpp::connection_hdl hdl) 
    {
        m_bIsConnected = false;
        m_bIsFailedOrDisconnected = true;
        if (m_OnDisconnect)
        {
            m_OnDisconnect(m_OwnerPtr);
        }
    }

    bool IsConnected()
    {
        return m_bIsConnected;
    }

    void SetClientContext(ClientNetworkContext* pCtx)
    {
        m_ctx = *pCtx; // copy values
    }

    void SetOwner(void* p) { m_OwnerPtr = p; }
    void SetOnConnect(WS_SCB onConnect) { m_OnConnect = onConnect; }
    void SetOnDisconnect(WS_SCB onDisconnect) { m_OnDisconnect = onDisconnect; }
    bool IsFailedOrDisconnected() { return m_bIsFailedOrDisconnected; }
private:
    static void runThread(client *client, bool * bIsUsable)
    {
        *bIsUsable = true;
        client->run();
    }
    bool m_bIsUsable;
    bool m_bIsConnected;
    bool m_bIsFailedOrDisconnected;
    bool m_bShouldReconnect;
    client m_client_;
    connection_ptr m_con_;
    std::thread *m_pThread;
    std::string m_sServer_url;
    ClientNetworkContext m_ctx;
    void* m_OwnerPtr;
    WS_SCB m_OnConnect, m_OnDisconnect;
};

VSSWebSocketClient::VSSWebSocketClient()
{
    WebSocketClientHolder* ws;
    InternalNew(ws, WebSocketClientHolder);
    m_pWebsocketClient = ws;
    if (m_pWebsocketClient == NULL)
    {
        assert(false);
    }
}

VSSWebSocketClient::~VSSWebSocketClient()
{
    WebSocketClientHolder* c = (WebSocketClientHolder*)m_pWebsocketClient;
    InternalDelete(c);
}

bool VSSWebSocketClient::ConnectToServer(const char* hostname, unsigned short port)
{
    std::string ConnectionUrl = std::string(hostname) + ":" + std::to_string(port);
    return ConnectToServer(ConnectionUrl.c_str());
}

bool VSSWebSocketClient::ConnectToServer(const char* ConnectionURL)
{
    WebSocketClientHolder* ws = (WebSocketClientHolder*)m_pWebsocketClient;
#ifdef FIXED_TLS_HANDSHAKE_ISSUE
    std::string uri = "wss://" + ConnectionURL;
#else
    std::string uri = std::string("ws://") + ConnectionURL;
#endif
    m_sConnectionUrl = uri;

    if (ws->Init(uri))
    {
        // run the event loop in the background
        ws->run(); 

        return true;
    }
    else
    {
        return false;
    }
}

VSS_WS_SocketResults VSSWebSocketClient::Send(const char* data, size_t len, bool isBinary)
{
    WebSocketClientHolder* c = (WebSocketClientHolder*)m_pWebsocketClient;
    return c->sendMessage(data, len, isBinary);
}

bool VSSWebSocketClient::Disconnect()
{
    WebSocketClientHolder* c = (WebSocketClientHolder*)m_pWebsocketClient;
    c->CloseConnection();

    return true;
}

bool VSSWebSocketClient::IsConnected()
{
    WebSocketClientHolder* c = (WebSocketClientHolder*)m_pWebsocketClient;
    return c->IsConnected();
}

VSSWebSocketClientEx::VSSWebSocketClientEx()
{
    m_SessionInitState = WSSessionInitState::InitWaitingConnect;
    m_InitSentStamp = 0;
    m_ConnectRetries = 0;
    m_HandshakeRetries = 0;
    m_NextReconnectStamp = 0;
    m_ReconnectGiveUpStamp = 0;
    m_bShouldReconnect = false;
    WebSocketClientHolder* c = (WebSocketClientHolder*)m_pWebsocketClient;
    ClientNetworkContext ctx(this);
    c->SetClientContext(&ctx);
    c->SetOwner(this);
    c->SetOnConnect(VSSWebSocketClientEx::OnConnect);
    c->SetOnDisconnect(VSSWebSocketClientEx::OnDisconnect);
}

void VSSWebSocketClientEx::OnSessionInitReply(unsigned short sc)
{
    if (sc == VSS_WS_StatusCodes::VSSWSS_ServerSessionInitialized)
    {
        m_SessionInitState = WSSessionInitState::InitSuccess;

        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWSClient, 0, 0,
            "WSClient:Server handshake successfull to %s", m_sConnectionUrl.c_str());
    }
    else if (sc == VSS_WS_StatusCodes::VSSWSS_ServerSessionInitFailed)
    {
        m_SessionInitState = WSSessionInitState::InitFailed;

        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWSClient, 0, 0,
            "WSClient:Server handshake failed to %s", m_sConnectionUrl.c_str());
    }
}

void VSSWebSocketClientEx::OnConnect(void* this_)
{
    VSSWebSocketClientEx* self = (VSSWebSocketClientEx*)this_;
    self->m_ConnectRetries = 0;
    self->m_HandshakeRetries = 0;
    self->m_InitSentStamp = 0;
    self->m_SessionInitState = WSSessionInitState::InitNotSent;
}

void VSSWebSocketClientEx::OnDisconnect(void* this_)
{
    VSSWebSocketClientEx* self = (VSSWebSocketClientEx*)this_;
    self->m_SessionInitState = WSSessionInitState::InitDisconnected;
    self->m_NextReconnectStamp = GetTickCount64() + RECONNECT_COOLDOWN;
}

void VSSWebSocketClientEx::PeriodicUpdate()
{
    WebSocketClientHolder* c = (WebSocketClientHolder*)m_pWebsocketClient;
    if (m_bShouldReconnect)
    {
        if (c->IsConnected() == false)
        {
            unsigned __int64 TimeTick = GetTickCount64();
            if (m_ConnectRetries == 0)
            {
                m_ReconnectGiveUpStamp = TimeTick + MAX_RECONNECT_TIME;
            }
            if (m_ReconnectGiveUpStamp > TimeTick &&
                m_NextReconnectStamp < TimeTick)
            {
                m_NextReconnectStamp = GetTickCount64() + RECONNECT_COOLDOWN;
                if (sAppSession.IsApplicationRunning())
                {
                    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWSClient, 0, 0,
                        "WSClient:Trying to reconnect");
                    c->Reconnect();
                    m_ConnectRetries++;
                }
                else
                {
                    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWSClient, 0, 0,
                        "WSClient:Closing connection without reconnection");
                }
            }
        }
    }

    // maybe there is a way to fix this situation
    if (m_SessionInitState == WSSessionInitState::InitNotSent ||
        m_SessionInitState == WSSessionInitState::InitSent ||
        m_SessionInitState == WSSessionInitState::InitFailed )
    {
        if (m_HandshakeRetries >= MAX_HANDSHAKE_TRIES)
        {
            AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceWSClient, 0, 0,
                "WSClient: Aborting. To many connection tries to %s", m_sConnectionUrl.c_str());
        }
        else if (m_InitSentStamp + RESEND_HANDSHAKE_INTERVAL < GetTickCount64())
        {
            m_HandshakeRetries++;
            SendHandshakeToServer();
        }
    }
}

void VSSWebSocketClientEx::SetSessionInitState(WSSessionInitState ns)
{
    m_SessionInitState = ns;
    if (m_SessionInitState == WSSessionInitState::InitSent)
    {
        m_InitSentStamp = GetTickCount64();
    }
}

void VSSWebSocketClientEx::SendHandshakeToServer()
{
    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWSClient, 0, 0,
        "WSClient:Sending handshake to server");

    WebSocketClientHolder* c = (WebSocketClientHolder*)m_pWebsocketClient;
    GENERIC_PACKET_INIT(VSS_WS_Opcodes::VSSWSO_C_InitSession, VSS_N_InitSession, 1);

    // the actual construction of the packet
    pktOut->SessionId = sAppSession.SessionIdGet();
    pktOut->SessionSalt = sAppSession.SessionSaltGet();
    pktOut->UserId = sUserSession.GetUserId();

    // send the init packet
    c->sendMessage(packetBuff, pktSize, true);

    // advance in the state of the handshake
    SetSessionInitState(VSSWebSocketClientEx::WSSessionInitState::InitSent);
}

void VSSWebSocketClientEx::SubscribeToModules(std::set<unsigned __int64>& ModuleIds, 
    VSS_N_SubscribeModules::SubscriptionType IdType)
{
    size_t allocCount = ModuleIds.size();
    if (allocCount > 0)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWSClient, 0, 0,
            "WSClient:Subscribing to %lld Module Alerts at %s", allocCount, m_sConnectionUrl.c_str());

        WebSocketClientHolder* c = (WebSocketClientHolder*)m_pWebsocketClient;
        GENERIC_PACKET_INIT(VSS_WS_Opcodes::VSSWSO_C_SubscribeModules, VSS_N_SubscribeModules, allocCount);

        pktOut->SubscribeType = IdType;
        // the actual construction of the packet
        size_t ModulesAdded = 0;
        for (auto& itr : ModuleIds)
        {
            //        pktOut->ModuleDetails[ModulesAdded].DataRate = 0;
            pktOut->ModuleDetails[ModulesAdded].ModuleId = itr;

            ModulesAdded++;
        }

        // send the init packet
        c->sendMessage(packetBuff, pktSize, true);
    }
}

void VSSWebSocketClientEx::UnSubscribeFromModules(std::set<unsigned __int64>& ModuleIds,
    VSS_N_SubscribeModules::SubscriptionType IdType)
{
    size_t allocCount = ModuleIds.size();
    if (allocCount > 0)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWSClient, 0, 0,
            "WSClient:UnSubscribing to %lld Module Alerts at %s", allocCount, m_sConnectionUrl.c_str());

        WebSocketClientHolder* c = (WebSocketClientHolder*)m_pWebsocketClient;
        GENERIC_PACKET_INIT(VSS_WS_Opcodes::VSSWSO_C_UnSubscribeModules, VSS_N_UnSubscribeModules, allocCount);

        pktOut->SubscribeType = IdType;
        // the actual construction of the packet
        size_t ModulesAdded = 0;
        for (auto& itr : ModuleIds)
        {
            pktOut->ModuleIds[ModulesAdded] = itr;
            ModulesAdded++;
        }

        // send the init packet
        c->sendMessage(packetBuff, pktSize, true);
    }
}

void VSSWebSocketClientEx::OnModuleDataPkt(const VSS_N_ModuleObjectState* mos)
{
    for (size_t i = 0; i < mos->Counter; i++)
    {
        sWindowManager.GetLocationViewWindow()->OnModulePositionDataArrived(
            mos->ModuleID, mos->Timestamp, mos->ObjectStates[i].id, mos->ObjectStates[i].x, mos->ObjectStates[i].y);
    }
}

void VSSWebSocketClientEx::OnModuleAlertPkt(const VSS_N_ModuleAlertState* mas)
{
    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWSClient, 0, 0,
        "WSClient:Received Alert %lld from Module %s", mas->AlertId, m_sConnectionUrl.c_str());
    sAlertsCache.OnDPSAlertArrived(mas->AlertId, (int)mas->AlertType, mas->TriggerStamp, 
        mas->StateFlags, mas->LocationId);
}

bool VSSWebSocketClientEx::IsConsideredUnusable()
{
    WebSocketClientHolder* c = (WebSocketClientHolder*)m_pWebsocketClient;
    if (c->IsFailedOrDisconnected())
    {
        return true;
    }

    if (m_SessionInitState == WSSessionInitState::InitSuccess)
    {
        return false;
    }
    if (m_HandshakeRetries >= MAX_HANDSHAKE_TRIES)
    {
        return true;
    }

    return false;
}