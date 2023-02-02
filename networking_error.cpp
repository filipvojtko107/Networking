#include "networking_error.hpp"


networking::networking_error::networking_error(const char* const message) :
    message_(message)
{

}


networking::networking_error::networking_error(const std::string& message) :
    message_(message)
{

}


networking::networking_error::networking_error(std::string&& message) :
    message_(std::move(message))
{

}


networking::networking_error::networking_error(const networking::networking_error& obj) :
    message_(obj.message_)
{

}


networking::networking_error::networking_error(networking::networking_error&& obj) :
    message_(std::move(obj.message_))
{

}


networking::networking_error::~networking_error()
{
    
}


networking::networking_error& networking::networking_error::operator=(const char* const message)
{
    message_ = message;
    return *this;
}


networking::networking_error& networking::networking_error::operator=(const std::string& message)
{
    message_ = message;
    return *this;
}


networking::networking_error& networking::networking_error::operator=(std::string&& message)
{
    message_ = std::move(message);
    return *this;
}


networking::networking_error& networking::networking_error::operator=(const networking::networking_error& obj)
{
    message_ = obj.message_;
    return *this;
}


networking::networking_error& networking::networking_error::operator=(networking::networking_error&& obj)
{
    message_ = std::move(obj.message_);
    return *this;
}


const char* networking::networking_error::what() const noexcept
{
    return message_.data();
}