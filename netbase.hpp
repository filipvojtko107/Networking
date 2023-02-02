#ifndef __NETWORKING_NETBASE_HPP__
#define __NETWORKING_NETBASE_HPP__
#include "socket.hpp"
#include <string>
#include <cinttypes>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <shared_mutex>
#include <vector>


namespace networking
{
    enum class error
    {
        NONE, OPEN_SOCKET_ERROR, CLOSE_SOCKET_ERROR, SET_SOCKET_OPTIONS_ERROR, 
        BIND_TO_SOCKET_ERROR, LISTEN_ON_SOCKET_ERROR, ACCEPT_CONNECTION_ERROR, 
        CONNECT_ERROR, TRANSFER_ERROR, RECEIVE_ERROR, LOG_FILE_ERROR
    };

    enum class communication
    {
        NONE, LOCAL, REMOTE
    };

    class netbase
    {
        public:
            netbase() = default;
            netbase(const std::string& ip_address, const std::uint16_t& port, 
                const networking::communication& communicaton_type, const std::string& log_file_path = "");
            netbase(const netbase& obj) = delete;
            netbase(const netbase&& obj) = delete;
            virtual ~netbase() = 0;

            virtual void reset();
            virtual void reload(const std::string& ip_address, const std::uint16_t& port, 
                const networking::communication& communicaton_type, const std::string& log_file_path = "");
            virtual void start() = 0;
            virtual void end();
            virtual bool is_running() const = 0;
            virtual bool is_data_to_receive(const networking::socket_t& sock) const = 0;

            networking::error last_error() const;
            std::string ip_address() const;
            std::uint16_t port() const;
            networking::communication communicaton_type() const;
            std::string log_file_path() const;

            static bool is_big_endian();


        protected:
            struct connection
            {
                public:
                    connection() = default;
                    connection(const connection& obj) = delete;
                    connection(connection&& obj);
                    ~connection();

                    connection& operator=(const connection& obj) = delete;
                    connection& operator=(connection&& obj);

                    void reset();
                    std::string info() const;

                    sockaddr_in connection_ = {0};
                    networking::socket_t socket_;
            };

            enum class log
            {
                SERVER_STARTED_LOG, SERVER_STOPPED_LOG, 
                CLIENT_CONNECTED_LOG, CLIENT_DISCONNECTED_LOG, 
                ENDPOINT_READY_LOG, ENDPOINT_CLOSED_LOG, 
                DATA_RECEIVED_LOG, DATA_TRANSMITTED_LOG
            };

            const char* make_log(const networking::netbase::log& log_type, const std::string& append_message = "");
            const char* make_log(const networking::error& error_type, const std::string& append_message = "");

            virtual bool receive(const networking::socket_t& sock, void* const data, const std::size_t& size) = 0;
            virtual bool transfer(const networking::socket_t& sock, void* const data, const std::size_t& size) = 0;
            virtual void receive_byte_count(const networking::socket_t& sock, std::size_t& count) = 0;
            void reverse_byte_order(unsigned char* const data, const std::size_t& size);


        private:
            const char* get_log_message(const networking::netbase::log& log_type);
            const char* get_error_message(const networking::error& error_type);
            unsigned char reverse_bit_order(unsigned char b);


        protected:
            networking::netbase::connection server_;
            networking::communication communication_type_ = networking::communication::NONE;
            std::string log_file_path_;
            networking::error last_error_ = networking::error::NONE;
            mutable std::shared_mutex lock_;
    };
}


inline networking::error networking::netbase::last_error() const
{
    return last_error_;
}

inline std::string networking::netbase::ip_address() const
{
    return inet_ntoa(server_.connection_.sin_addr);
}

inline std::uint16_t networking::netbase::port() const
{
    return ntohs(server_.connection_.sin_port);
}

inline networking::communication networking::netbase::communicaton_type() const
{
    return communication_type_;
}

inline std::string networking::netbase::log_file_path() const
{
    return log_file_path_;
}


#endif