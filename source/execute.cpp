/*
 * -- execute.cpp --
 * This file contains the implementation for the command parser and runner.
 */

#define MIN_WAIT 1
#define MAX_WAIT 4

// Included files
#include <stdlib.h>

// Global Objects
char FOUND_MSG[] = "FOUND.\r\n";
char NOT_FOUND_MSG[] = "NOT-FOUND.\r\n";
bool display_tokens = false;
std::string database_key = "";

// Function implementations
int execute_command(std::string &command, unsigned int socket_fd)
{
    std::vector<std::string> tokens;
    std::string current_token = "";
    bool string_open = false;
    for (unsigned int i = 0; i < command.length(); ++i)
    {
        if (command[i] == '\\' && i < command.length() - 1 && command[i + 1] == '"')
        {
            current_token += command[i + 1];
            ++i;
        }
        else if (command[i] == '"')
        {
            string_open = !string_open;
            if (!string_open)
            {
                tokens.push_back(current_token);
                current_token.clear();
            }
        }
        else if (command[i] == ' ' && !string_open)
        {
            if (!current_token.empty())
            {
                tokens.push_back(current_token);
                current_token.clear();
            }
        }
        else
        {
            current_token += command[i];
        }
    }
    if (!current_token.empty())
    {
        tokens.push_back(current_token);
        current_token.clear();
    }

    if (display_tokens)
    {
        std::cerr << "\x1B[97;42m Message Received: " << command.length() << " bytes + \\n \033[0m" << std::endl;
        std::cerr << "\x1B[97;100m" << command << "\033[0m" << std::endl;
        std::cerr << "\x1B[33mThe message contains " << tokens.size() << " tokens.\033[0m" << std::endl;
        for (unsigned int i = 0; i < tokens.size(); ++i)
        {
            std::cerr << "\x1B[32mToken Number " << (i + 1) << ":\033[0m ";
            std::cerr << "\x1B[97;100m" << tokens[i] << "\033[0m" << std::endl;
        }
    }

    // Check for authentication key
    bool authenticated = false;
    if (tokens[0] == "KEY")
    {
        if (tokens.size() < 2)
            return 1;
        std::string received_key = tokens[1];
        std::string comparing_key = database_key;
        bool is_key_equal = true;
        // Fill buffers for full-buffer comparison
        while (received_key.length() < comparing_key.length())
        {
            received_key += ' ';
        }
        while (comparing_key.length() < received_key.length())
        {
            comparing_key += ' ';
        }
        // Compare buffers
        for (size_t i = 0; i < comparing_key.length(); ++i)
        {
            if (comparing_key[i] != received_key[i])
            {
                is_key_equal = false;
            }
        }
        if (is_key_equal)
        {
            authenticated = true;
            tokens.erase(tokens.begin()); // Remove KEY command
            tokens.erase(tokens.begin()); // Remove key
        }
    }

    // If not authenticated, throw error
    if (!authenticated && !database_key.empty())
    {
        srand(time(NULL));
        sleep((rand() % (MAX_WAIT * 10)) * 0.1 + MIN_WAIT); // Wait a random number of seconds
        return 3;
    }

    // If no command was sent, throw error
    if (tokens.empty())
    {
        return 1;
    }

    // Execute command
    if (tokens[0] == "SET")
    {
        if (tokens.size() < 3)
            return 1;
        else
            set_key(tokens[1], tokens[2]);
        return 0;
    }
    else if (tokens[0] == "GET")
    {
        if (tokens.size() < 2)
            return 1;
        std::string value;
        if (read_key(tokens[1], value))
        {
            send(socket_fd, "\"", 1, 0);
            send(socket_fd, value.c_str(), value.length(), 0);
            send(socket_fd, "\"\r\n", 3, 0);
            return 2;
        }
        else
        {
            return 1;
        }
    }
    else if (tokens[0] == "PROBE")
    {
        if (tokens.size() < 2)
            return 1;
        if (probe_key(tokens[1]))
        {
            send(socket_fd, FOUND_MSG, strlen(FOUND_MSG), 0);
        }
        else
        {
            send(socket_fd, NOT_FOUND_MSG, strlen(NOT_FOUND_MSG), 0);
        }
        return 2;
    }
    else if (tokens[0] == "DROP")
    {
        if (tokens.size() < 2)
            return 1;
        drop_key(tokens[1]);
        return 0;
    }
    else if (tokens[0] == "PING")
    {
        return 0;
    }
    else if (tokens[0] == "CHECKPOINT")
    {
        save_datafile();
        return 0;
    }
    else if (tokens[0] == "DEBUG")
    {
        if (tokens.size() < 2)
            return 1;
        if (tokens[1] == "ON")
        {
            display_tokens = true;
            return 0;
        }
        else if (tokens[1] == "OFF")
        {
            display_tokens = false;
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else if (tokens[0] == "SETKEY")
    {
        if (tokens.size() < 2)
            return 1;
        database_key = tokens[1];
        return 0;
    }
    else if (tokens[0] == "DELKEY")
    {
        database_key = "";
        return 0;
    }
    return 1;
}