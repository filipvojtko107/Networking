#include "socket.hpp"


networking::socket_t::socket_t(const int& socket) :
    socket_(socket)
{

}


networking::socket_t::socket_t(const socket_t& obj) :
    socket_(obj.socket_)
{

}


networking::socket_t::socket_t(socket_t&& obj) :
    socket_(obj.socket_)
{
    if (&obj != this) {
        obj.socket_ = networking::socket_t::NONE;
    }
}


networking::socket_t& networking::socket_t::operator=(const int& socket)
{
    socket_ = socket;
    return *this;
}


networking::socket_t& networking::socket_t::operator=(const networking::socket_t& obj)
{
    socket_ = obj.socket_;
    return *this;
}


networking::socket_t& networking::socket_t::operator=(networking::socket_t&& obj)
{
    if (&obj != this)
    {
        socket_ = obj.socket_;
        obj.socket_ = networking::socket_t::NONE;
    }

    return *this;
}