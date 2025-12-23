#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <cstdlib>
#include <stdexcept>
#include <array>
#include <thread>
#include <mutex>
#include <chrono>
#include "headers/fetch.hpp"

namespace fs = std::filesystem;

static std::string exec(const std::string& cmd) {
    std::array<char, 4096> buf;
    std::string out;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw std::runtime_error("popen failed");
    while (fgets(buf.data(), buf.size(), pipe))
        out += buf.data();
    pclose(pipe);
    return out;
}

fs::path cache_path() {
    const char* home = getenv("HOME");
    return fs::path(home) / ".cache/cpf/index.txt";
}

// check if cache is stale (older than 1 hour)
bool is_cache_stale() {
    if (!fs::exists(cache_path())) return true;

    auto last_write = fs::last_write_time(cache_path());
    auto now = fs::file_time_type::clock::now();
    auto age = std::chrono::duration_cast<std::chrono::hours>(now - last_write);

    // Cache expires after 1 hour
    return age.count() >= 1;
}

// fetches remote repo tree and rewrites cache
std::vector<std::string> fetch_remote_index() {
    fs::create_directories(cache_path().parent_path());
    std::string json = exec(
        "curl -s https://api.github.com/repos/heykulthe/cp-templates/git/trees/main?recursive=1"
    );

    std::vector<std::string> files;
    std::ofstream cache(cache_path());

    std::istringstream ss(json);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.find("\"path\"") != std::string::npos &&
            line.find(".cpp") != std::string::npos) {
            auto start = line.find(": \"") + 3;
            auto end = line.find("\"", start);
            std::string path = line.substr(start, end - start);
            cache << path << "\n";
            files.push_back(path);
        }
    }
    return files;
}

// load cache from disk
std::vector<std::string> load_cache_index() {
    std::vector<std::string> files;
    if (!fs::exists(cache_path())) return files;
    std::ifstream in(cache_path());
    std::string s;
    while (std::getline(in, s)) files.push_back(s);
    return files;
}

// main entry: lazy refresh with staleness check
std::vector<std::string> load_or_fetch_index(bool force_refresh) {
    if (force_refresh) return fetch_remote_index();

    auto index = load_cache_index();
    if (!index.empty() && !is_cache_stale()) return index;
    return fetch_remote_index();
}

// resolve template with lazy refresh
std::string resolve_template(const std::string& query, std::vector<std::string>* index) {
    std::vector<std::string> local_index;
    if (index) {
        local_index = *index;
    } else {
        local_index = load_or_fetch_index(false);
    }

    for (auto& s : local_index)
        if (s.find(query) != std::string::npos)
            return s;

    // not found in cache → fetch remote once and try again
    local_index = fetch_remote_index();
    for (auto& s : local_index)
        if (s.find(query) != std::string::npos)
            return s;

    throw std::runtime_error("No template matched: " + query);
}

// fetch the actual template content from GitHub
std::string fetch_template(const std::string& path) {
    std::string url = "https://raw.githubusercontent.com/heykulthe/cp-templates/main/" + path;
    std::string content = exec("curl -s " + url);

    if (content.empty()) {
        throw std::runtime_error("Failed to fetch template: " + path);
    }

    return content;
}

// Batch fetch all templates in parallel
std::vector<std::string> fetch_templates_batch(const std::vector<std::string>& paths) {
    std::vector<std::string> contents(paths.size());
    std::vector<std::thread> threads;
    std::mutex mtx;
    std::string error_msg;

    // Get current timestamp for cache busting
    auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();

    for (size_t i = 0; i < paths.size(); ++i) {
        threads.emplace_back([&, i, timestamp]() {
            try {
                // Add timestamp query parameter and aggressive cache-busting headers
                std::string url = "https://raw.githubusercontent.com/heykulthe/cp-templates/main/"
                                + paths[i] + "?t=" + std::to_string(timestamp);
                std::string cmd = "curl -s "
                                "-H 'Cache-Control: no-cache, no-store, must-revalidate' "
                                "-H 'Pragma: no-cache' "
                                "-H 'Expires: 0' "
                                "'" + url + "'";
                std::string content = exec(cmd);

                if (content.empty()) {
                    std::lock_guard<std::mutex> lock(mtx);
                    error_msg = "Failed to fetch template: " + paths[i];
                    return;
                }

                contents[i] = content;
            } catch (const std::exception& e) {
                std::lock_guard<std::mutex> lock(mtx);
                error_msg = e.what();
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    if (!error_msg.empty()) {
        throw std::runtime_error(error_msg);
    }

    return contents;
}