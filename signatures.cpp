/*
 * -- signatures.cpp --
 * This file contains signatures for all the functions included in other files.
 */

// checkpoint.cpp
void load_checkpoint();
int test_save_file_permissions();
void save_datafile();

// error.cpp
void err_header();
void err_unknown_switch(const std::string &argument);
void err_malformed_port(const std::string &argument);
void err_malformed_checkpoint_timeout(const std::string &argument);
void err_expected_switch_value(const std::string &argument);
void err_socket_creation(char *argument);
void err_socket_read(char *argument);
void err_socket_accept(char *argument);

// execute.cpp
int execute_command(std::string &command, unsigned int socket_fd);
void save_datafile();

// map_functions.cpp
void set_key(const std::string &key, const std::string &value);
bool read_key(const std::string &key, std::string &out);
void drop_key(const std::string &key);
bool probe_key(const std::string &key);