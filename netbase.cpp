#include "netbase.hpp"
#include "networking_error.hpp"
#include <cerrno>
#include <cstring>
#include <ctime>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <mutex>


networking::netbase::~netbase()
{

}


networking::netbase::netbase(const std::string& ip_address, const std::uint16_t& port, const networking::communication& communication_type, const std::string& log_file_path) :
    communication_type_(communication_type), log_file_path_(log_file_path), last_error_(networking::error::NONE)
{
    if (communication_type_ == networking::communication::LOCAL) { server_.connection_.sin_family = AF_UNIX; }
    else { server_.connection_.sin_family = AF_INET; }

    server_.connection_.sin_port = htons(port);
    inet_aton(ip_address.data(), &server_.connection_.sin_addr);
}


void networking::netbase::reset()
{
    server_.reset();
    log_file_path_.clear();
    communication_type_ = networking::communication::NONE;
    last_error_ = networking::error::NONE;
}


void networking::netbase::reload(const std::string& ip_address, const std::uint16_t& port, 
                const networking::communication& communication_type, const std::string& log_file_path)
{
    end();
    
    communication_type_ = communication_type;
    log_file_path_ = log_file_path;
    last_error_ = networking::error::NONE;

    if (communication_type_ == networking::communication::LOCAL) { server_.connection_.sin_family = AF_UNIX; }
    else { server_.connection_.sin_family = AF_INET; }
    
    server_.connection_.sin_port = htons(port);
    inet_aton(ip_address.data(), &server_.connection_.sin_addr);

    start();
}


void networking::netbase::end()
{
    if (is_running())
    {
        lock_.lock();
        if (close(server_.socket_) == -1)
        {
            last_error_ = networking::error::CLOSE_SOCKET_ERROR;
            lock_.unlock();
            const char* message = make_log(last_error_, strerror(errno)); 
            throw networking::networking_error(message);
        }
        
        server_.socket_ = networking::socket_t::NONE;
        lock_.unlock();
    }
}


const char* networking::netbase::get_error_message(const networking::error& error_type)
{
    const char* message = nullptr;
    switch (error_type)
    {
        case networking::error::NONE:
            message = "OK";
            break;
        case networking::error::OPEN_SOCKET_ERROR:
            message = "Failed to open a socket";
            break;
        case networking::error::CLOSE_SOCKET_ERROR: 
            message = "Failed to close a socket";
            break;
        case networking::error::SET_SOCKET_OPTIONS_ERROR: 
            message = "Faile to set the socket options";
            break;
        case networking::error::BIND_TO_SOCKET_ERROR: 
            message = "Failed to bind an IP address and port to the socket";
            break;
        case networking::error::LISTEN_ON_SOCKET_ERROR: 
            message = "Failed to create a connection queue on the socket";
            break;
        case networking::error::ACCEPT_CONNECTION_ERROR: 
            message = "Faile to accept a connection";
            break;
        case networking::error::CONNECT_ERROR: 
            message = "Failed to connect to the server";
        case networking::error::TRANSFER_ERROR: 
            message = "Failed to send a data";
            break;
        case networking::error::RECEIVE_ERROR:
            message = "Failed to receive a data";
            break;
        case networking::error::LOG_FILE_ERROR:
            message = "Failed to open the log file";
            break;
        default:
            message = "Unknown error";
    }

    return message;
}


const char* networking::netbase::get_log_message(const networking::netbase::log& log_type)
{
    const char* message = nullptr;    
    switch (log_type)
    {
        case networking::netbase::log::SERVER_STARTED_LOG: 
            message = "Server started";
            break;
        case networking::netbase::log::SERVER_STOPPED_LOG:
            message = "Server stopped";
            break;
        case networking::netbase::log::CLIENT_CONNECTED_LOG: 
            message = "Client connected to the server";
            break;
        case networking::netbase::log::CLIENT_DISCONNECTED_LOG: 
            message = "Client disconnected from the server";
            break;
        case networking::netbase::log::ENDPOINT_READY_LOG: 
            message = "Endpoint ready";
            break;
        case networking::netbase::log::ENDPOINT_CLOSED_LOG: 
            message = "Endpoint closed";
            break;
        case networking::netbase::log::DATA_RECEIVED_LOG:
            message = "Data have been received";
            break;
        case networking::netbase::log::DATA_TRANSMITTED_LOG:
            message = "Data have been transmitted";
            break;
        default:
            message = "Unknown log";
    }

    return message;
}


const char* networking::netbase::make_log(const networking::error& error_type, const std::string& append_message)
{
    const char* error_message = nullptr;
    if (!log_file_path_.empty())
    {
        error_message = get_error_message(error_type);
        const time_t time_now = time(nullptr);

        std::unique_lock<std::shared_mutex> lock(lock_);
        std::ofstream log_file(log_file_path_, std::ofstream::app);
        
        if (!log_file.is_open())
        { 
            last_error_ = networking::error::LOG_FILE_ERROR;
            throw std::runtime_error(get_error_message(last_error_));
        }

        char* time_now_str = ctime(&time_now);
        time_now_str[strlen(time_now_str) - 1] = '\0';

        log_file << '[' << time_now_str << "] [ERROR] " << error_message;
        if (!append_message.empty()) { log_file << " (" << append_message << ')'; }
        log_file << '\n';

        log_file.close();
    }

    return error_message;
}


const char* networking::netbase::make_log(const networking::netbase::log& log_type, const std::string& append_message)
{
    const char* log_message = nullptr;
    if (!log_file_path_.empty())
    {
        log_message = get_log_message(log_type);
        const time_t time_now = time(nullptr);

        std::unique_lock<std::shared_mutex> lock(lock_);
        std::ofstream log_file(log_file_path_, std::ofstream::app);

        if (!log_file.is_open())
        { 
            last_error_ = networking::error::LOG_FILE_ERROR;
            throw std::runtime_error(get_error_message(last_error_));
        }

        char* time_now_str = ctime(&time_now);
        time_now_str[strlen(time_now_str) - 1] = '\0';

        log_file << '[' << time_now_str << "] [INFO] " << log_message;
        if (!append_message.empty()) { log_file << " (" << append_message << ')'; }
        log_file << '\n';

        log_file.close();
    }

    return log_message;
}


void networking::netbase::reverse_byte_order(unsigned char* const data, const std::size_t& size)
{
    if (data != nullptr && size > 0)
    {
        unsigned char* ptr = data;
        for (std::size_t i = 0; i < size; ++i)
        {
            *ptr = reverse_bit_order(*ptr);
            ptr += 1;
        }
    }
}


unsigned char networking::netbase::reverse_bit_order(unsigned char b)
{
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;

   return b;
}


bool networking::netbase::is_big_endian()
{
    int n = 1;
    if (*(char*) &n == 1) { return false; }
    return true;
}



networking::netbase::connection::~connection()
{

}


networking::netbase::connection::connection(networking::netbase::connection&& obj)
{
    if (&obj != this)
    {
        connection_ = obj.connection_;
        socket_ = obj.socket_;
        obj.reset();
    }
}


networking::netbase::connection& networking::netbase::connection::operator=(networking::netbase::connection&& obj)
{
    if (&obj != this)
    {
        connection_ = obj.connection_;
        socket_ = obj.socket_;
        obj.reset();
    }

    return *this;
}


void networking::netbase::connection::reset()
{
    memset(&connection_, 0, sizeof(connection_));
    socket_ = networking::socket_t::NONE;
}


std::string networking::netbase::connection::info() const
{
    std::stringstream s_info;
    s_info << "IP address: " << inet_ntoa(connection_.sin_addr) << "   Port: " << ntohs(connection_.sin_port) << "   Socket: " << socket_;
    return s_info.str();
}