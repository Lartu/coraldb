/*
   ___              _ ___  ___ 
  / __|___ _ _ __ _| |   \| _ )     An extremely simple in-memory key-value
 | (__/ _ \ '_/ _` | | |) | _ \     database for distributed applications and
  \___\___/_| \__,_|_|___/|___/     microservices made with love by Lartu.

   www.github.com/lartu/coraldb     Released under the BSD-2 License.

*/

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <vector>
#include <thread>
#include <chrono>
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mutex>
#include "signatures.cpp"
#include "error.cpp"
#include "map_functions.cpp"
#include "execute.cpp"
#include "checkpoint.cpp"

#define VERSION "1.1.0"

unsigned int global_port = 26725;
char KEY_ERROR_MSG[] = "WRONG-KEY.\r\n";
char ERROR_MSG[] = "ERROR.\r\n";
char OK_MSG[] = "OK.\r\n";

enum switch_types
{
    port,
    file,
    none,
    checkpoint,
};

void print_version()
{
    std::cout << "\x1B[97;45m";
    std::cout << " CoralDB, version " << VERSION << " ";
    std::cout << "\033[0m " << std::endl;
}

void show_help()
{
    print_version();
    std::cout << "Usage: coraldb [switches]" << std::endl;
    std::cout << " -h, --help          shows this help" << std::endl;
    std::cout << " -p, --port          sets the database port" << std::endl;
    std::cout << " -f, --file          sets the database checkpoint file" << std::endl;
    std::cout << " -c, --checkpoint    sets the checkpoint timeout in seconds" << std::endl;
    std::cout << " -a, --anyport       use any available port" << std::endl;
    std::cout << " -v, --version       prints version information" << std::endl;
}

int parse_arguments(int argc, char **argv)
{
    std::string argument;
    switch_types requested_switch = none;
    for (int i = 1; i < argc; ++i)
    {
        argument = argv[i];
        if (requested_switch == none)
        {
            if (argument == "-h" || argument == "--help")
            {
                show_help();
                exit(0);
            }
            else if (argument == "-p" || argument == "--port")
            {
                requested_switch = port;
            }
            else if (argument == "-f" || argument == "--file")
            {
                requested_switch = file;
            }
            else if (argument == "-c" || argument == "--checkpoint")
            {
                requested_switch = checkpoint;
            }
            else if (argument == "-a" || argument == "--anyport")
            {
                global_port = 0;
            }
            else if (argument == "-v" || argument == "--version")
            {
                print_version();
                exit(0);
            }
            else
            {
                err_unknown_switch(argument);
            }
        }
        else
        {
            if (requested_switch == port)
            {
                try
                {
                    global_port = stoi(argument);
                }
                catch (const std::invalid_argument &ia)
                {
                    err_malformed_port(argument);
                    return 1;
                }
            }
            else if (requested_switch == file)
            {
                global_filename = argument;
            }
            else if (requested_switch == checkpoint)
            {
                try
                {
                    CHECKPOINT_TIME = stoi(argument) * 1000;
                }
                catch (const std::invalid_argument &ia)
                {
                    err_malformed_checkpoint_timeout(argument);
                    return 1;
                }
            }
            requested_switch = none;
        }
    }
    if (requested_switch != none)
    {
        err_expected_switch_value(argument);
        return 1;
    }
    if (global_filename == "")
    {
        const char *homedir = getenv("HOME");
        if (homedir == NULL)
        {
            homedir = getpwuid(getuid())->pw_dir;
        }
        global_filename = (std::string)homedir + "/coraldb.db";
    }
    return 0;
}

void handle_socket(int new_socket_fd)
{
    // TODO add alarm with timeout https://stackoverflow.com/questions/9163308/how-to-use-timeouts-with-read-on-a-socket-in-c-on-unix/9163476
    std::string str_buffer = "";
    char buffer[1024];
    size_t bytes_read;
    bool request_read_end = false;
    bool last_char_was_cr = false;
    while ((bytes_read = recv(new_socket_fd, buffer, sizeof(buffer), 0)) > 0)
    {
        for (unsigned int i = 0; i < bytes_read; ++i)
        {
            // \r\n terminates the message.
            if (buffer[i] == '\r')
            {
                last_char_was_cr = true;
            }
            else if (buffer[i] == '\n' && last_char_was_cr)
            {
                request_read_end = true;
                break;
            }
            else
            {
                if (last_char_was_cr)
                {
                    // If we found a CR but it wasn't a CRLF, add the CR to the buffer.
                    str_buffer += '\r';
                    last_char_was_cr = false;
                }
                str_buffer += buffer[i];
            }
        }
        if (request_read_end)
            break;
    }
    int retcode = execute_command(str_buffer, new_socket_fd);
    if (retcode == 1)
    {
        send(new_socket_fd, ERROR_MSG, strlen(ERROR_MSG), 0);
    }
    else if (retcode == 0)
    {
        send(new_socket_fd, OK_MSG, strlen(OK_MSG), 0);
    }
    else if (retcode == 3)
    {
        send(new_socket_fd, KEY_ERROR_MSG, strlen(KEY_ERROR_MSG), 0);
    }
    // retcode 2 means "don't send anything"
    shutdown(new_socket_fd, SHUT_RDWR);
    recv(new_socket_fd, buffer, sizeof(buffer), 0);
    close(new_socket_fd);
}

void start_socket()
{
    // Create socket
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        err_socket_creation(strerror(errno));
        exit(1);
    }
    // Bind Socket
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(global_port);
    socklen_t len = sizeof(address);
    if (bind(sockfd, (struct sockaddr *)&address, len) < 0)
    {
        err_socket_creation(strerror(errno));
        exit(1);
    }
    getsockname(sockfd, (struct sockaddr *)&address, &len);
    global_port = ntohs(address.sin_port);
    // Inform the user
    std::cout << "CoralDB running on port " << global_port << ", backed to " << global_filename << std::endl;
    // Listen socket
    int new_socket_fd;
    if (listen(sockfd, 256) < 0)
    {
        err_socket_read(strerror(errno));
    }
    while (true)
    {
        if ((new_socket_fd = accept(sockfd, (struct sockaddr *)&address, &len)) < 0)
        {
            err_socket_accept(strerror(errno));
        }
        std::thread child_thread(handle_socket, new_socket_fd);
        child_thread.detach();
    }
    close(sockfd);
}

int main(int argc, char **argv)
{
    int arg_out = parse_arguments(argc, argv);
    if (arg_out == 0)
    {
        print_version();
        if (test_save_file_permissions() == 0)
        {
            load_checkpoint();
            std::thread checkpoint_thread(checkpoint_handler);
            checkpoint_thread.detach();
            start_socket();
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return arg_out;
    }
    return 0;
}
