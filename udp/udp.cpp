#include "udp.hpp"
#include "networking_error.hpp"
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>


networking::udp::udp()
{
    if (communication_type_ == networking::communication::LOCAL) { destination_.connection_.sin_family = AF_UNIX; }
    else { destination_.connection_.sin_family = AF_INET; } 
}


networking::udp::~udp()
{

}


networking::udp::udp(const std::string& ip_address, const std::uint16_t& port, 
    const networking::communication& communication_type, const std::string& log_file_path) :
    networking::netbase(ip_address, port, communication_type, log_file_path)
{
    if (communication_type_ == networking::communication::LOCAL) { destination_.connection_.sin_family = AF_UNIX; }
    else { destination_.connection_.sin_family = AF_INET; }
}


/*networking::udp::udp(udp&& obj) :
    server_(std::move(obj.server_), destination_(std::move(obj.destination_)), log_file_path_(std::move(obj.log_file_path_))
{
    
}*/


/*networking::udp& networking::udp::operator=(networking::udp&& obj)
{
    if (&obj != this)
    {
        server_ = std::move(obj.server_);
        destination_ = std::move(obj.destination_);
        log_file_path_ = std::move(obj.log_file_path_);
    }

    return *this;
}*/


void networking::udp::reset()
{
    networking::netbase::reset();
    destination_.reset();
    is_destination_ = false;
}


void networking::udp::start()
{
    if (!is_running())
    {
        server_.socket_ = socket(static_cast<int>(communication_type_), SOCK_DGRAM, 0);
        if (server_.socket_ == networking::socket_t::NONE)
        {
            last_error_ = networking::error::OPEN_SOCKET_ERROR;
            const char* message = make_log(last_error_); 
            throw networking::networking_error(message);
        }

        const int option = 1;
        if (setsockopt(server_.socket_, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
        {
            last_error_ = networking::error::SET_SOCKET_OPTIONS_ERROR;
            const char* message = make_log(last_error_, strerror(errno));
            close(server_.socket_);
            server_.socket_ = networking::socket_t::NONE;
            throw networking::networking_error(message);
        }

        if (bind(server_.socket_, (const sockaddr*) &server_.connection_, sizeof(server_.connection_)) == -1)
        {
            last_error_ = networking::error::BIND_TO_SOCKET_ERROR;
            const char* message = make_log(last_error_, strerror(errno)); 
            close(server_.socket_);
            server_.socket_ = networking::socket_t::NONE;
            throw networking::networking_error(message);
        }

        make_log(networking::netbase::log::ENDPOINT_READY_LOG, server_.info());
    }
}


void networking::udp::end()
{
    const std::string server_info = server_.info();
    networking::netbase::end();
    make_log(networking::netbase::log::ENDPOINT_CLOSED_LOG, server_info);
}


void networking::udp::set_destination(const std::string& ip_address, const std::uint16_t& port)
{
    destination_.connection_.sin_port = htons(port);
    inet_aton(ip_address.data(), &destination_.connection_.sin_addr);
    is_destination_ = true;
}


void networking::udp::unset_destination()
{
    destination_.reset();
    is_destination_ = false;
}


bool networking::udp::is_running() const
{
    std::shared_lock<std::shared_mutex> lock(lock_);
    return (server_.socket_ != networking::socket_t::NONE);
}


bool networking::udp::is_data_to_receive() const
{
    return udp::is_data_to_receive(server_.socket_);
}


bool networking::udp::is_data_to_receive(const networking::socket_t& sock) const
{
    if (sock != networking::socket_t::NONE && is_running())
    {
        std::shared_lock<std::shared_mutex> lock(lock_);
        socklen_t len =  sizeof(destination_.connection_);
        int buffer = 0;

        const int result = recvfrom(sock, &buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT, nullptr, &len);
        return (result > 0);
    }

    return false;
}


bool networking::udp::transfer(const networking::socket_t& sock, void* const data, const std::size_t& size)
{
    bool sent = false;
    if (is_destination() && is_running())
    {
        std::size_t count;
        if (!is_big_endian())
        {
            reverse_byte_order((unsigned char* const) data, size);
            count = htonl(size);
        }
        else { count = size; }

        lock_.lock();
        if (sendto(sock, &count, sizeof(count), 0, 
            (const sockaddr*) &destination_.connection_, sizeof(destination_.connection_)) < 0)
        {
            last_error_ = networking::error::TRANSFER_ERROR;
            lock_.unlock();
            const char* message = make_log(last_error_, strerror(errno)); 
            throw networking::networking_error(message);
        }

        if (sendto(sock, data, size, 0, 
            (const sockaddr*) &destination_.connection_, sizeof(destination_.connection_)) < 0)
        {
            last_error_ = networking::error::TRANSFER_ERROR;
            lock_.unlock();
            const char* message = make_log(last_error_, strerror(errno)); 
            throw networking::networking_error(message);
        }
    
        lock_.unlock();

        if (!is_big_endian()) {
            reverse_byte_order((unsigned char* const) data, size);
        }

        make_log(networking::netbase::log::DATA_TRANSMITTED_LOG, 
            "TX: " + std::to_string(size) + " | " + destination_.info());

        sent = true;
    }

    return sent;
}


bool networking::udp::receive(const networking::socket_t& sock, void* const data, const std::size_t& size)
{
    bool sent = false;
    if (is_destination() && is_running())
    {
        socklen_t len =  sizeof(destination_.connection_);
        networking::netbase::connection source;

        lock_.lock();
        if (recvfrom(sock, data, size, 0, 
            (sockaddr*) &source.connection_, &len) < 0)
        {
            last_error_ = networking::error::RECEIVE_ERROR;
            lock_.unlock();
            const char* message = make_log(last_error_, strerror(errno)); 
            throw networking::networking_error(message);
        }
        lock_.unlock();

        if (!is_big_endian()) {
            reverse_byte_order((unsigned char* const) data, size);
        }

        make_log(networking::netbase::log::DATA_RECEIVED_LOG, 
            "RX: " + std::to_string(size) + " | " + source.info());

        sent = true;
    }

    return sent;
}


void networking::udp::receive_byte_count(const networking::socket_t& sock, std::size_t& count)
{
    if (is_destination() && is_running())
    {
        lock_.lock();
        socklen_t len =  sizeof(destination_.connection_);
        
        if (recvfrom(sock, &count, sizeof(count), 0, nullptr, &len) < 0)
        {
            last_error_ = networking::error::RECEIVE_ERROR;
            lock_.unlock();
            const char* message = make_log(last_error_, strerror(errno)); 
            throw networking::networking_error(message);
        }
        lock_.unlock();

        if (!is_big_endian()) { count = ntohl(count); }
    }
}