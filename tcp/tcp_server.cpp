#include "tcp_server.hpp"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <mutex>


networking::tcp_server::tcp_server(const std::string& ip_address, const std::uint16_t& port, 
    const networking::communication& communication_type, const std::uint16_t& max_connections, const std::string& log_file_path) :
    tcp(ip_address, port, communication_type, log_file_path), max_connections_(max_connections), threads_(max_connections)
{

}


void networking::tcp_server::reset()
{
    tcp::reset();
    end();
    threads_.reset();
    max_connections_ = 0;
}


void networking::tcp_server::reload(const std::string& ip_address, const std::uint16_t& port, 
    const networking::communication& communication_type, 
    const std::uint16_t& max_connections, const std::string& log_file_path)
{
    end();

    tcp::reload(ip_address, port, communication_type, log_file_path);
    max_connections_ = max_connections;
    threads_.threads_count(max_connections_);

    start();
}


void networking::tcp_server::start()
{
    if (!is_running())
    {
        tcp::start();

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

        if (listen(server_.socket_, max_connections_) == -1)
        {
            last_error_ = networking::error::LISTEN_ON_SOCKET_ERROR;
            const char* const message = make_log(last_error_, strerror(errno));
            close(server_.socket_);
            server_.socket_ = networking::socket_t::NONE;
            throw networking::networking_error(message);
        }

        threads_.run();

        make_log(networking::netbase::log::SERVER_STARTED_LOG, server_.info());
    }
}


void networking::tcp_server::end()
{
    if (is_running())
    {
        const std::string server_info = server_.info();
        tcp::end();

        lock_.lock();
        for (const auto& client : clients_)
        {
            if (close(client.socket_) == -1)
            {
                last_error_ = networking::error::CLOSE_SOCKET_ERROR;
                lock_.unlock();
                const char* message = make_log(last_error_);
                throw networking::networking_error(message);
            }
        }
        lock_.unlock();

        threads_.wait();
        threads_.stop();

        lock_.lock();
        last_connections_.clear();
        clients_.clear();
        lock_.unlock();

        make_log(networking::netbase::log::SERVER_STOPPED_LOG, server_info);
    }
}


void networking::tcp_server::end(const networking::socket_t& sock)
{
    if (is_running() && sock != networking::socket_t::NONE)
    {
        lock_.lock_shared();
        auto client = clients_.begin();
        
        for (; client != clients_.end(); client++)
        {
            if ((*client).socket_ == sock) { break; }
        }
        lock_.unlock_shared();

        lock_.lock();  
        if (client != clients_.end())
        {
            if (close(sock) == -1)
            {
                clients_.erase(client);
                last_error_ = networking::error::CLOSE_SOCKET_ERROR;
                lock_.unlock();
                const char* message = make_log(last_error_);        
                throw networking::networking_error(message);
            }

            const std::string client_info_str = client->info();
            clients_.erase(client);

            lock_.unlock();
            make_log(networking::netbase::log::CLIENT_DISCONNECTED_LOG, client_info_str);
            return;
        }
        
        lock_.unlock();
    }
}


networking::socket_t networking::tcp_server::handle(const std::function<void()>& task)
{
    if (is_running() && clients_.size() < max_connections_)
    {
        tcp_server::connection client;
        socklen_t client_size = sizeof(client.connection_);

        if ((client.socket_ = accept(server_.socket_, (sockaddr*) &client.connection_, &client_size)) == -1)
        {
            last_error_ = networking::error::ACCEPT_CONNECTION_ERROR;
            const char* message = make_log(last_error_, strerror(errno));
            throw networking::networking_error(message);
        }

        lock_.lock();

        clients_.push_back(std::move(client));
        threads_.add_task(task);
        last_connections_.push_back(clients_.back().socket_);
        client.socket_ = clients_.back().socket_;

        lock_.unlock();

        make_log(networking::netbase::log::CLIENT_CONNECTED_LOG, clients_.back().info());

        return client.socket_;
    }

    return networking::socket_t::NONE;
}


networking::socket_t networking::tcp_server::last_connection()
{
    if (is_running())
    {
        std::unique_lock<std::shared_mutex> lock(lock_);
        if (last_connections_.empty()) { return networking::socket_t::NONE; }

        const networking::socket_t last_sock = last_connections_.front();
        last_connections_.pop_front();

        return last_sock;
    }

    return networking::socket_t::NONE;
}


bool networking::tcp_server::is_connected(const networking::socket_t& sock) const
{
    if (is_running() && sock != networking::socket_t::NONE)
    {
        std::shared_lock<std::shared_mutex> lock(lock_);
        int buffer = 0;
        const int result = recv(sock, &buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT);

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


bool networking::tcp_server::is_running() const
{
    std::shared_lock<std::shared_mutex> lock(lock_);
    return (server_.socket_ != networking::socket_t::NONE && threads_.is_running());
}
