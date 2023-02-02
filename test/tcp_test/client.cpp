#include "parametres.hpp"
#include "tcp_client.hpp"
#include "socket.hpp"
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <thread>


static networking::tcp_client client(ip_address, port, networking::communication::REMOTE, log_file_path_client);


void client_terminate(int sig)
{
    int exit_val = EXIT_SUCCESS;
    try
    {
        client.end();
        std::clog << "Disconnected\n";
    }

    catch (const std::exception& err)
    {
        std::cerr << err.what() << '\n';
        exit_val = EXIT_FAILURE;
    }

    exit(exit_val);
}


int main()
{
    try
    {
        signal(SIGINT, &client_terminate);
        signal(SIGTERM, &client_terminate);

        std::clog << "Connecting...\n";
        client.start();
        std::clog << "Connected\n";
        
        if (client.is_running())
        {
            std::thread send_t([]()
            {
                std::string data_to_send;
                while (client.is_running())
                {
                    std::getline(std::cin, data_to_send);
                    client.transfer<char>(data_to_send.data(), data_to_send.size());
                    data_to_send.clear();
                }
            });

            std::thread receive_t([]()
            {
                std::vector<char> buffer;
                while (client.is_running())
                {
                    if (client.is_data_to_receive())
                    {
                        buffer = client.receive<char, std::vector<char>>();
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

        client.end();
        std::clog << "Disconnected\n";
    }

    catch (const std::exception& err)
    {
        try
        {
            client.end();
            std::clog << "Disconnected\n";
        }

        catch (const std::exception& err2)
        {
            std::cerr << err2.what() << '\n';
        }

        std::cerr << err.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}