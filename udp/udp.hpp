#ifndef __NETWORKING_UDP_HPP__
#define __NETWORKING_UDP_HPP__
#include "netbase.hpp"
#include "networking_error.hpp"
#include <sys/socket.h>
#include <string>
#include <cinttypes>
#include <vector>
#include <mutex>


namespace networking
{
    class udp : public netbase
    {
        public:
            udp();
            udp(const std::string& ip_address, const std::uint16_t& port, 
                const networking::communication& communication_type, const std::string& log_file_path = "");
            udp(const udp& obj) = delete;
            udp(udp&& obj) = delete;
            ~udp();

            udp& operator=(const udp& obj) = delete;
            udp& operator=(udp&& obj) = delete;

            void start() override;
            void reset() override;
            void end() override;
            void set_destination(const std::string& ip_address, const std::uint16_t& port);
            void unset_destination();
            bool is_running() const override;
            bool is_data_to_receive() const;
            bool is_destination() const;
            std::string destination_ip_address() const;
            std::uint16_t destination_port() const;

            template<typename T>
            bool transfer(T* const data, const std::size_t& count)
            {
                return transfer(server_.socket_, data, (sizeof(T) * count));
            }

            
            template<typename T, typename RT>
            RT receive()
            {
                std::size_t count = 0;
                receive_byte_count(server_.socket_, count);
        
                if (count > 0)
                {
                    RT buffer(count / sizeof(T));
                    if (receive(server_.socket_, buffer.data(), count))
                    {
                        return buffer;
                    }
                }

                return RT();
            }


        private:
            bool is_data_to_receive(const networking::socket_t& sock) const override;
            bool receive(const networking::socket_t& sock, void* const data, const std::size_t& size) override;
            bool transfer(const networking::socket_t& sock, void* const data, const std::size_t& size) override;
            void receive_byte_count(const networking::socket_t& sock, std::size_t& count) override;


        private:
            bool is_destination_ = false;
            networking::netbase::connection destination_;
    };
}


inline bool networking::udp::is_destination() const
{
    return is_destination_;
}

inline std::string networking::udp::destination_ip_address() const
{
    return inet_ntoa(destination_.connection_.sin_addr);
}

inline std::uint16_t networking::udp::destination_port() const
{
    return ntohs(destination_.connection_.sin_port);
}


#endif