/*
 * -- map_functions.cpp --
 * This file contains implementations for all required threadsafe operations
 * on the unordered map, as well as the global declarations of the unordered
 * map object and its mutex.
 */

#include <unordered_map>
#include <mutex>
#include <string>

// Global Objects
std::unordered_map<std::string, std::string> global_db;
std::mutex global_db_mutex;

// Function implementations
void set_key(const std::string &key, const std::string &value)
{
    /* Sets the value of a key into the dictionary. If the key is not found,
     * it's added. Otherwise, it's value is changed.
     * Time complexity:
     * - Average: O(1)
     * - Worst case: O(n)
     */
    global_db_mutex.lock();
    if (global_db.find(key) == global_db.end())
        global_db.emplace(key, value);
    else
        global_db[key] = value;
    global_db_mutex.unlock();
}

bool read_key(const std::string &key, std::string &out)
{
    /* Returns the value of a key via the out parameter. If a value could be
     * read, the function returns true. Otherwise it returns false.
     * Time complexity:
     * - Average: O(1)
     * - Worst case: O(n)
     */
    bool found;
    global_db_mutex.lock();
    std::unordered_map<std::string, std::string>::const_iterator it = global_db.find(key);
    found = it != global_db.end();
    if (found)
    {
        out = it->second;
    }
    global_db_mutex.unlock();
    return found;
}

void drop_key(const std::string &key)
{
    /* Erases the element with the key from the dictionary.
     * Time complexity:
     * - Average: O(1)
     * - Worst case: O(n)
     */
    global_db_mutex.lock();
    global_db.erase(key);
    global_db_mutex.unlock();
}

bool probe_key(const std::string &key)
{
    /* Returns true if the key exists in the dictionary, false otherwise.
     * Time complexity:
     * - Average: O(1)
     * - Worst case: O(n)
     */
    bool found;
    global_db_mutex.lock();
    found = global_db.find(key) != global_db.end();
    global_db_mutex.unlock();
    return found;
}