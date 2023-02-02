#ifndef __NETWORKING_TCP_SERVER_HPP__
#define __NETWORKING_TCP_SERVER_HPP__
#include "tcp.hpp"
#include "networking_error.hpp"
#include "thread_pool.hpp"
#include <cerrno>
#include <cstring>
#include <list>


namespace networking
{
    class tcp_server : public tcp
    {
        public:
            tcp_server() = default;
            tcp_server(const std::string& ip_address, const std::uint16_t& port, 
                const networking::communication& communication_type, 
                const std::uint16_t& max_connections, const std::string& log_file_path = "");
            tcp_server(const tcp_server& obj) = delete;
            tcp_server(tcp_server&& obj) = delete;
            ~tcp_server() = default;

            tcp_server& operator=(const tcp_server& obj) = delete;
            tcp_server& operator=(tcp_server&& obj) = delete;

            void reset() override;
            void reload(const std::string& ip_address, const std::uint16_t& port, 
                const networking::communication& communication_type, 
                const std::uint16_t& max_connections, const std::string& log_file_path = "");
            void start() override;
            void end() override;
            void end(const networking::socket_t& sock);
            networking::socket_t handle(const std::function<void()>& task);
            networking::socket_t last_connection();
            bool is_connected(const networking::socket_t& sock) const;
            bool is_running() const override;

            template<typename T>
            bool transfer(const networking::socket_t& sock, T* const data, const std::size_t& count)
            {
                return tcp::transfer(sock, data, (sizeof(T) * count));
            }


            template<typename T, typename RT>
            RT receive(const networking::socket_t& sock)
            {
                std::size_t count = 0;
                receive_byte_count(sock, count);
        
                if (count > 0)
                {
                    RT buffer(count / sizeof(T));
                    if (tcp::receive(sock, buffer.data(), count))
                    {
                        return buffer;
                    }
                }

                return RT();
            }


        private:
            std::list<connection> clients_;
            std::list<networking::socket_t> last_connections_;
            std::uint16_t max_connections_ = 0;
            thread_pool threads_;
    };
}


#endif