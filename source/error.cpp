/*
 * -- error.cpp --
 * This file contains implementations for all error messages and the general
 * error message printer function.
 */

#include <iostream>
#include <string>

// Function implementations
void err_header()
{
    std::cerr << "\x1B[97;41m CoralDB Error \033[0m " << std::endl;
}

void err_unknown_switch(const std::string &argument)
{
    err_header();
    std::cerr << "Unknown switch: " << argument << std::endl;
}

void err_malformed_port(const std::string &argument)
{
    err_header();
    std::cerr << "Invalid port number: " << argument << std::endl;
}

void err_malformed_checkpoint_timeout(const std::string &argument)
{
    err_header();
    std::cerr << "Invalid checkpoint timeout value: ";
    std::cerr << argument << std::endl;
}

void err_expected_switch_value(const std::string &argument)
{
    err_header();
    std::cerr << "Expected argument for switch: " << argument << std::endl;
}

void err_socket_creation(char *argument)
{
    err_header();
    std::cerr << "Socket binding failed: " << argument << std::endl;
}

void err_socket_read(char *argument)
{
    err_header();
    std::cerr << "Socket read failed: " << argument << std::endl;
}

void err_socket_accept(char *argument)
{
    err_header();
    std::cerr << "Socket accept failed: " << argument << std::endl;
}

void err_malformed_command_load(const std::string &argument)
{
    err_header();
    std::cerr << "Malformed command found in checkpoint file: " << argument << std::endl;
}