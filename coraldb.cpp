#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <fstream>
#include <chrono>
#include <thread>
#include <stdlib.h>

#define VERSION "1.0.0"

char ERROR_MSG[] = "ERROR.\r\n";
char OK_MSG[] = "OK.\r\n";
char FOUND_MSG[] = "FOUND.\r\n";
char NOT_FOUND_MSG[] = "NOT-FOUND.\r\n";
int CHECKPOINT_TIME = 1000 * 60 * 10;

std::string global_filename = "";
unsigned int global_port = 26725;
std::unordered_map<std::string, std::string> global_db;
std::mutex global_db_mutex;
std::mutex checkpoint_file_mutex;

enum switch_types
{
    port,
    file,
    none,
    checkpoint,
};

void print_version()
{
    std::cout << "CoralDB, version " << VERSION << std::endl;
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

void set_key(const std::string &key, const std::string &value)
{
    // Average: O(1)
    // Worst case: O(n)
    global_db_mutex.lock();
    if (global_db.find(key) == global_db.end())
        global_db.emplace(key, value);
    else
        global_db[key] = value;
    global_db_mutex.unlock();
}

bool read_key(const std::string &key, std::string &out)
{
    // Average: O(1)
    // Worst case: O(n)
    bool found;
    global_db_mutex.lock();
    std::unordered_map<std::string, std::string>::const_iterator it = global_db.find(key);
    found = !(it == global_db.end());
    if (found)
    {
        out = it->second;
    }
    global_db_mutex.unlock();
    return found;
}

void drop_key(const std::string &key)
{
    // Average: O(1)
    // Worst case: O(n)
    global_db_mutex.lock();
    global_db.erase(key);
    global_db_mutex.unlock();
}

bool probe_key(const std::string &key)
{
    // Average: O(1)
    // Worst case: O(n)
    bool found;
    global_db_mutex.lock();
    found = (global_db.find(key) != global_db.end());
    global_db_mutex.unlock();
    return found;
}

int test_save_file_permissions()
{
    FILE *fp = fopen(global_filename.c_str(), "a+");
    if (fp == NULL)
    {
        if (errno == EACCES)
        {
            std::cerr << "CoralDB: Cannot write to checkpoint file " << global_filename << ", permission denied." << std::endl;
        }
        else
        {
            std::cerr << "CoralDB: Cannot write to checkpoint file " << global_filename << ": " << strerror(errno) << "." << std::endl;
        }
        return 1;
    }
    return 0;
}

void save_datafile()
{
    //TODO compress checkpoint data
    std::lock(global_db_mutex, checkpoint_file_mutex);
    std::ofstream db_file;
    std::cout << "CoralDB: Performing checkpoint to " << global_filename << std::endl;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    db_file.open(global_filename);
    for (auto &it : global_db)
    {
        db_file << "SET " << it.first << " \"" << it.second << "\"\r\n";
    }
    db_file.flush();
    db_file.close();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "CoralDB: Checkpoint performed: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;
    checkpoint_file_mutex.unlock();
    global_db_mutex.unlock();
}

void checkpoint_handler()
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(CHECKPOINT_TIME));
        save_datafile();
    }
}

int execute_command(std::string &command, unsigned int socket_fd)
{
    std::vector<std::string> tokens;
    std::string current_token = "";
    bool escape_next_char = false;
    bool string_open = false;
    for (unsigned int i = 0; i < command.length(); ++i)
    {
        if (escape_next_char)
        {
            current_token += command[i];
            escape_next_char = false;
        }
        else if (command[i] == '\\')
        {
            current_token += command[i];
            escape_next_char = true;
        }
        else if (command[i] == '"')
        {
            string_open = !string_open;
            if (!string_open)
            {
                tokens.push_back(current_token);
                current_token.clear();
            }
            if (escape_next_char)
                current_token += command[i];
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
    // Command format: COMMAND argument1 argument2 argument3 etc
    /*
    for (unsigned int i = 0; i < tokens.size(); ++i)
    {
        std::cout << "TOKEN " << i << "): " << tokens[i] << std::endl;
    }
    //*/
    if (tokens.empty())
        return 1;
    if (tokens[0] == "SET" || tokens[0] == "set")
    {
        if (tokens.size() < 3)
            return 1;
        else
            set_key(tokens[1], tokens[2]);
        return 0;
    }
    else if (tokens[0] == "GET" || tokens[0] == "get")
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
    else if (tokens[0] == "PROBE" || tokens[0] == "probe")
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
    else if (tokens[0] == "DROP" || tokens[0] == "drop")
    {
        if (tokens.size() < 2)
            return 1;
        drop_key(tokens[1]);
        return 0;
    }
    else if (tokens[0] == "PING" || tokens[0] == "ping")
    {
        return 0;
    }
    else if (tokens[0] == "CHECKPOINT" || tokens[0] == "checkpoint")
    {
        save_datafile();
        return 0;
    }
    return 1;
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
                std::cerr << "CoralDB: Unknown switch: " << argument << std::endl;
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
                    std::cerr << "CoralDB: Invalid port number: " << argument << std::endl;
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
                    std::cerr << "CoralDB: Invalid checkpoint timeout value: " << argument << std::endl;
                    return 1;
                }
            }
            requested_switch = none;
        }
    }
    if (requested_switch != none)
    {
        std::cerr << "CoralDB: Expected argument for switch: " << argument << std::endl;
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
    char buffer[10];
    size_t bytes_read;
    bool escape_next_char = false;
    bool string_open = false;
    bool request_read_end = false;
    while ((bytes_read = recv(new_socket_fd, buffer, sizeof(buffer), 0)) > 0)
    {
        for (unsigned int i = 0; i < bytes_read; ++i)
        {
            if (escape_next_char)
            {
                escape_next_char = false;
            }
            else if (buffer[i] == '\\')
            {
                escape_next_char = true;
            }
            else if (buffer[i] == '"')
            {
                string_open = !string_open;
            }
            else if (buffer[i] == '\n' || buffer[i] == '\r')
            {
                if (!string_open)
                {
                    request_read_end = true;
                    break;
                }
            }
            str_buffer += buffer[i];
        }
        memset(&buffer, 0, sizeof(buffer));
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
        std::cerr << "CoralDB: Socket creation failed" << std::endl;
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
        std::cerr << "CoralDB: Socket binding failed: " << strerror(errno) << std::endl;
        exit(1);
    }
    getsockname(sockfd, (struct sockaddr *)&address, &len);
    global_port = ntohs(address.sin_port);
    // Inform the user
    std::cout << "CoralDB running on port " << global_port << ", backed to " << global_filename << std::endl;
    // Listen socket
    int new_socket_fd;
    if (listen(sockfd, 10) < 0)
    {
        std::cerr << "CoralDB: Read error: " << strerror(errno) << std::endl;
    }
    while (true)
    {
        if ((new_socket_fd = accept(sockfd, (struct sockaddr *)&address, &len)) < 0)
        {
            std::cerr << "CoralDB: Accept error" << std::endl;
        }
        std::thread child_thread(handle_socket, new_socket_fd);
        child_thread.detach();
    }
    close(sockfd);
}

void load_checkpoint()
{
    size_t loaded = 0;
    std::string line;
    std::ifstream checkpoint_file(global_filename);
    std::cout << "CoralDB: Attempting to load checkpoint file " << global_filename << std::endl;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    while (std::getline(checkpoint_file, line))
    {
        // TODO fix multiline value loading
        if (execute_command(line, 0) == 0)
            ++loaded;
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "CoralDB: Loaded " << loaded << " values in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;
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