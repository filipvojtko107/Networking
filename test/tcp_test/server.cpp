#include "parametres.hpp"
#include "tcp_server.hpp"
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <vector>
#include <string>


static networking::tcp_server server(ip_address, port, networking::communication::REMOTE, max_connections, log_file_path_server);


void server_terminate(int sig)
{
    int exit_val = EXIT_SUCCESS;
    try
    {
        server.end();
        std::clog << "Server turned off\n";
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
        signal(SIGINT, &server_terminate);
        signal(SIGTERM, &server_terminate);
        
        std::clog << "Starting...\n";
        server.start();
        std::clog << "Server started\n";

        while (server.is_running())
        {
            server.handle([]()
            {
                try
                {
                    const networking::socket_t sock = server.last_connection();
                    std::vector<char> buffer;
                    char server_reply[] = "Data arrived OK";

                    while (server.is_connected(sock))
                    {
                        if (server.is_data_to_receive(sock))
                        {
                            buffer = server.receive<char, std::vector<char>>(sock);
                            if (!buffer.empty())
                            {
                                for (const auto& i : buffer) { std::cout << i; }
                                putchar('\n');
                                buffer.clear();
                                
                                server.transfer<char>(sock, server_reply, sizeof(server_reply));
                            }
                        }
                    }

                    server.end(sock);
                }

                catch (const std::exception& err) {
                    std::cerr << err.what() << std::endl;
                }
            });
        }

        server.end();
        std::clog << "Server turned off\n";
    }

    catch (const std::exception& err)
    {
        try
        {
            server.end();
            std::clog << "Server turned off\n";
        }
        
        catch (const std::exception& err2) {
            std::cerr << err2.what() << '\n';
        }

        std::cerr << err.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}