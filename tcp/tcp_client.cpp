#include "tcp_client.hpp"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <shared_mutex>


networking::tcp_client::tcp_client(const std::string& ip_address, const std::uint16_t& port,
    const networking::communication& communicaton_type, const std::string& log_file_path) :
    tcp(ip_address, port, communicaton_type, log_file_path)
{

}


void networking::tcp_client::start()
{
    if (!is_running())
    {
        tcp::start();
        
        if (connect(server_.socket_, (sockaddr*) &server_.connection_, sizeof(server_.connection_)) == -1)
        {
            last_error_ = networking::error::CONNECT_ERROR;
            const char* message = make_log(last_error_, strerror(errno));
            close(server_.socket_);
            throw networking::networking_error(message);
        }

        make_log(networking::netbase::log::CLIENT_CONNECTED_LOG);
    }
}


void networking::tcp_client::end()
{
    const std::string server_info = server_.info();
    tcp::end();
    make_log(networking::netbase::log::CLIENT_DISCONNECTED_LOG, server_info);
}


bool networking::tcp_client::is_running() const
{
    std::shared_lock<std::shared_mutex> lock(lock_);
    if (server_.socket_ != networking::socket_t::NONE)
    {
        int buffer = 0;
        const int result = recv(server_.socket_, &buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT);

        if (result == 0) { return false; }
        else if (result < 0)
        { 
            if (errno == EAGAIN || errno == EWOULDBLOCK) { return true; }
            else { return false; }
        }
        else { return true; }
    }

    return false;
}


bool networking::tcp_client::is_data_to_receive() const
{
    return tcp::is_data_to_receive(server_.socket_);
}