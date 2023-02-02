#include "tcp.hpp"
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


networking::tcp::~tcp()
{

}


networking::tcp::tcp(const std::string& ip_address, const std::uint16_t& port, const networking::communication& communication_type, const std::string& log_file_path) :
    networking::netbase(ip_address, port, communication_type, log_file_path)
{

}


void networking::tcp::start()
{
    if (!is_running())
    {
        server_.socket_ = socket(static_cast<int>(communication_type_), SOCK_STREAM, 0);
        if (server_.socket_ == networking::socket_t::NONE)
        {
            last_error_ = networking::error::OPEN_SOCKET_ERROR;
            const char* message = make_log(last_error_, strerror(errno)); 
            throw networking::networking_error(message);
        }
    }
}


bool networking::tcp::receive(const networking::socket_t& sock, void* const data, const std::size_t& size)
{
    bool received = false;
    if (is_running())
    {                 
        lock_.lock();
        if (recv(sock, data, size, MSG_WAITALL) < 0)
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
        make_log(networking::netbase::log::DATA_RECEIVED_LOG, "RX: " + std::to_string(size));

        received = true;
    }

    return received;
}


bool networking::tcp::transfer(const networking::socket_t& sock, void* const data, const std::size_t& size)
{
    bool sent = false;
    if (is_running())
    {
        if (data != nullptr && size > 0)
        {
            std::size_t count;
            if (!is_big_endian())
            {
                reverse_byte_order((unsigned char* const) data, size);
                count = htonl(size);
            }
            else { count = size; }

            lock_.lock();
            if (send(sock, &count, sizeof(count), MSG_NOSIGNAL) < 0)
            {
                last_error_ = networking::error::TRANSFER_ERROR;
                lock_.unlock();
                const char* message = make_log(last_error_, strerror(errno));
                throw networking::networking_error(message);
            }

            if (send(sock, data, size, MSG_NOSIGNAL) < 0)
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

            make_log(networking::netbase::log::DATA_TRANSMITTED_LOG, "TX: " + std::to_string(size));

            sent = true;
        }
    }

    return sent;
}


void networking::tcp::receive_byte_count(const networking::socket_t& sock, std::size_t& count)
{
    if (is_running())
    {
        lock_.lock();
        if (recv(sock, &count, sizeof(count), MSG_WAITALL) < 0)
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


bool networking::tcp::is_data_to_receive(const networking::socket_t& sock) const
{
    if (sock != networking::socket_t::NONE && is_running())
    {
        std::shared_lock<std::shared_mutex> lock(lock_);
        int buffer = 0;
        const int result = recv(sock, &buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT);
        return (result > 0);
    }

    return false;
}