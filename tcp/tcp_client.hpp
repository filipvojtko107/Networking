#ifndef __NETWORKING_TCP_CLIENT_HPP__
#define __NETWORKING_TCP_CLIENT_HPP__
#include "tcp.hpp"
#include "networking_error.hpp"
#include <cerrno>
#include <cstring>
#include <vector>


namespace networking
{
    class tcp_client : public tcp
    {
        public:
            tcp_client() = default;
            tcp_client(const std::string& ip_address, const std::uint16_t& port, 
                const networking::communication& communicaton_type, const std::string& log_file_path);
            tcp_client(const tcp_client& obj) = delete;
            tcp_client(tcp_client&& obj) = delete;
            ~tcp_client() = default;

            tcp_client& operator=(const tcp_client& obj) = delete;
            tcp_client& operator=(tcp_client&& obj) = delete;

            void start() override;
            void end() override;
            bool is_running() const override;
            bool is_data_to_receive() const;

            template<typename T>
            bool transfer(T* const data, const std::size_t& count)
            {
                return tcp::transfer(server_.socket_, data, (sizeof(T) * count));
            }

            template<typename T, typename RT>
            RT receive()
            {
                std::size_t count = 0;
                receive_byte_count(server_.socket_, count);

                if (count > 0)
                {
                    RT buffer(count / sizeof(T));
                    if (tcp::receive(server_.socket_, buffer.data(), count))
                    {
                        return buffer;
                    }
                }

                return RT();
            }
    };
}


#endif