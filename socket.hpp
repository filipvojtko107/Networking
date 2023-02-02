#ifndef __NETWORKING_SOCKET_T_HPP__
#define __NETWORKING_SOCKET_T_HPP__


namespace networking
{
    class socket_t
    {
        public:
            enum { NONE = -1 };

        public:
            socket_t() = default;
            socket_t(const int& socket);
            socket_t(const socket_t& obj);
            socket_t(socket_t&& obj);
            ~socket_t() = default;

            socket_t& operator=(const int& socket);
            socket_t& operator=(const socket_t& obj);
            socket_t& operator=(socket_t&& obj);
            inline operator int() const { return socket_; }
            inline int& get() { return socket_; }
            inline const int& get() const { return socket_; }


        private:
            int socket_ = socket_t::NONE;
    };
}



#endif