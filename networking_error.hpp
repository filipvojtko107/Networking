#ifndef __NETWORKING_UDP_ERROR_HPP__
#define __NETWORKING_UDP_ERROR_HPP__
#include <string>
#include <exception>


namespace networking
{   
    class networking_error : public std::exception
    {
        public:
            networking_error() = default;
            networking_error(const char* const message);
            networking_error(const std::string& message);
            networking_error(std::string&& message);
            networking_error(const networking::networking_error& obj);
            networking_error(networking::networking_error&& obj);
            virtual ~networking_error();

            networking_error& operator=(const char* const message);
            networking_error& operator=(const std::string& message);
            networking_error& operator=(std::string&& message);
            networking_error& operator=(const networking::networking_error& obj);
            networking_error& operator=(networking::networking_error&& obj);

            const char* what() const noexcept override;

        private:
            std::string message_;
    };
}


#endif