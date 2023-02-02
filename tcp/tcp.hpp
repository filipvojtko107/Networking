#ifndef __NETWORKING_TCP_HPP__
#define __NETWORKING_TCP_HPP__
#include "netbase.hpp"


namespace networking
{   
    class tcp : public netbase
    {
        public:
            tcp() = default;
            tcp(const std::string& ip_address, const std::uint16_t& port, 
                const networking::communication& communicaton_type, const std::string& log_file_path = "");
            tcp(const tcp& obj) = delete;
            tcp(tcp&& obj) = delete;
            virtual ~tcp() = 0;

            tcp& operator=(const tcp& obj) = delete;
            tcp& operator=(tcp&& obj) = delete;

            virtual void start() override;
            virtual bool is_data_to_receive(const networking::socket_t& sock) const override;


        protected:
            bool receive(const networking::socket_t& sock, void* const data, const std::size_t& size) override;
            bool transfer(const networking::socket_t& sock, void* const data, const std::size_t& size) override;
            void receive_byte_count(const networking::socket_t& sock, std::size_t& count) override;
    };
}


#endif