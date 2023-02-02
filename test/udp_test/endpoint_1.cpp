#include "udp.hpp"
#include "parametres.hpp"
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <vector>
#include <string>
#include <thread> 


static networking::udp endpoint(ip_address_1, port_1, networking::communication::REMOTE, log_file_path_1);


void endpoint_terminate(int sig)
{
    int exit_val = EXIT_SUCCESS;
    try 
    {
        endpoint.end();
        std::clog << "Endpoint closed\n";
    }

    catch (const std::exception& err) {
        std::cerr << err.what() << '\n';
        exit_val = EXIT_FAILURE;
    }

    exit(exit_val);
}


int main()
{
    try
    {
        signal(SIGINT, &endpoint_terminate);
        signal(SIGTERM, &endpoint_terminate);

        std::clog << "Starting...\n";
 
        endpoint.start();
        endpoint.set_destination(ip_address_2, port_2);

        std::clog << "Endpoint opened\n";

        if (endpoint.is_running())
        {
            std::thread send_t([]()
            {
                std::string data_to_send;
                while (endpoint.is_running())
                {
                    std::getline(std::cin, data_to_send);
                    endpoint.transfer<char>(data_to_send.data(), data_to_send.size());
                    data_to_send.clear();
                }
            });

            std::thread receive_t([]()
            {
                std::vector<char> buffer;
                while (endpoint.is_running())
                {
                    if (endpoint.is_data_to_receive())
                    {
                        buffer = endpoint.receive<char, std::vector<char>>();
                        if (!buffer.empty())
                        {
                            std::cout << "Reply: ";
                            for (const auto& i : buffer) { std::cout << i; }
                            putchar('\n');
                            buffer.clear();
                        }
                    
                    }
                }
            });

            send_t.join();
            receive_t.join();
        }

        endpoint.end();
        std::clog << "Endpoint closed\n";
    }

    catch (const std::exception& err)
    {
        try {
            endpoint.end();
            std::clog << "Endpoint closed\n";
        }

        catch (const std::exception& err2) {
            std::cerr << err2.what() << '\n';
        }

        std::cerr << err.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}