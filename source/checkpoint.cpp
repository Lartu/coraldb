/*
 * -- checkpoint.cpp --
 * This file contains implementations for the checkpoint load and save
 * functions.
 */

#include <fstream>
#include <string>
#include <mutex>

// Global Objects
int CHECKPOINT_TIME = 1000 * 60 * 10;
std::string global_filename = "";
std::mutex checkpoint_file_mutex;

// Function implementations
void load_checkpoint()
{
    size_t loaded = 0;
    std::string line;
    std::ifstream checkpoint_file(global_filename);
    std::cout << "CoralDB: Attempting to load checkpoint file " << global_filename << std::endl;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    while (std::getline(checkpoint_file, line))
    {
        if (execute_command(line, 0) == 0)
        {
            ++loaded;
        }
        else
        {
            err_malformed_command_load(line);
        }
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "CoralDB: Executed " << loaded << " startup commands in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;
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
    // Here we just use \n because we are going to load the values with getline
    for (auto &it : global_db)
    {
        db_file << "SET " << it.first << " \"" << it.second << "\"\n";
    }
    if (!database_key.empty())
    {
        db_file << "SETKEY " << database_key << "\n";
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
    /* This function waits until it's time for the checkpoint. When it is,
     * it requests the saving of the checkpoint file.
     */
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(CHECKPOINT_TIME));
        save_datafile();
    }
}