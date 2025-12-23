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

struct RepoConfig {
    std::string owner;
    std::string repo;
    std::string branch;
    std::string host_type;
    std::string base_url;
};

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

RepoConfig get_repo_config() {
    const char* custom_repo = std::getenv("CPF_REPO");
    std::string repo_url = custom_repo ? custom_repo : "https://github.com/heykulthe/cp-templates";

    RepoConfig config;
    config.branch = "main";

    if (repo_url.find("github.com") != std::string::npos) {
        config.host_type = "github";

        size_t start = repo_url.find("github.com");
        if (start != std::string::npos) {
            start = repo_url.find_first_of("/:", start) + 1;
            std::string path = repo_url.substr(start);

            if (path.length() >= 4 && path.substr(path.length() - 4) == ".git") {
                path = path.substr(0, path.length() - 4);
            }

            size_t slash = path.find('/');
            if (slash != std::string::npos) {
                config.owner = path.substr(0, slash);
                config.repo = path.substr(slash + 1);
            }
        }
        config.base_url = "https://api.github.com/repos/" + config.owner + "/" + config.repo;
    }
    else if (repo_url.find("gitlab.com") != std::string::npos) {
        config.host_type = "gitlab";

        size_t start = repo_url.find("gitlab.com");
        if (start != std::string::npos) {
            start = repo_url.find_first_of("/:", start) + 1;
            std::string path = repo_url.substr(start);

            if (path.length() >= 4 && path.substr(path.length() - 4) == ".git") {
                path = path.substr(0, path.length() - 4);
            }

            std::string encoded_path;
            for (char c : path) {
                if (c == '/') encoded_path += "%2F";
                else encoded_path += c;
            }

            config.base_url = "https://gitlab.com/api/v4/projects/" + encoded_path;
        }
    }
    else if (repo_url.find("bitbucket.org") != std::string::npos) {
        config.host_type = "bitbucket";

        size_t start = repo_url.find("bitbucket.org");
        if (start != std::string::npos) {
            start = repo_url.find('/', start) + 1;
            std::string path = repo_url.substr(start);

            if (path.length() >= 4 && path.substr(path.length() - 4) == ".git") {
                path = path.substr(0, path.length() - 4);
            }

            size_t slash = path.find('/');
            if (slash != std::string::npos) {
                config.owner = path.substr(0, slash);
                config.repo = path.substr(slash + 1);
            }

            config.base_url = "https://api.bitbucket.org/2.0/repositories/" + config.owner + "/" + config.repo;
        }
    }
    else {
        config.host_type = "raw";
        config.base_url = repo_url;
        if (config.base_url.back() == '/') {
            config.base_url.pop_back();
        }
    }

    return config;
}

fs::path cache_path() {
    const char* home = getenv("HOME");
    return fs::path(home) / ".cache/cpf/index.txt";
}

bool is_cache_stale() {
    if (!fs::exists(cache_path())) return true;

    auto last_write = fs::last_write_time(cache_path());
    auto now = fs::file_time_type::clock::now();
    auto age = std::chrono::duration_cast<std::chrono::minutes>(now - last_write);

    return age.count() >= 30;
}

std::vector<std::string> fetch_remote_index() {
    fs::create_directories(cache_path().parent_path());
    auto config = get_repo_config();

    std::string json;

    if (config.host_type == "github") {
        json = exec(
            "curl -s " + config.base_url + "/git/trees/" + config.branch + "?recursive=1"
        );
    } else if (config.host_type == "gitlab") {
        json = exec(
            "curl -s " + config.base_url + "/repository/tree?recursive=true&ref=" + config.branch
        );
    } else if (config.host_type == "bitbucket") {
        json = exec(
            "curl -s " + config.base_url + "/src/" + config.branch + "/?pagelen=100"
        );
    } else {
        throw std::runtime_error("Cannot fetch index for raw URL repositories. Please use GitHub, GitLab, or Bitbucket.");
    }

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

std::vector<std::string> load_cache_index() {
    std::vector<std::string> files;
    if (!fs::exists(cache_path())) return files;
    std::ifstream in(cache_path());
    std::string s;
    while (std::getline(in, s)) files.push_back(s);
    return files;
}

std::vector<std::string> load_or_fetch_index(bool force_refresh) {
    if (force_refresh) return fetch_remote_index();

    auto index = load_cache_index();
    if (!index.empty() && !is_cache_stale()) return index;
    return fetch_remote_index();
}

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

    local_index = fetch_remote_index();
    for (auto& s : local_index)
        if (s.find(query) != std::string::npos)
            return s;

    throw std::runtime_error("No template matched: " + query);
}

std::string fetch_template(const std::string& path) {
    auto config = get_repo_config();
    std::string content;

    if (config.host_type == "github") {
        std::string api_url = config.base_url + "/contents/" + path;
        content = exec("curl -s -H 'Accept: application/vnd.github.v3.raw' " + api_url);
    } else if (config.host_type == "gitlab") {
        std::string api_url = config.base_url + "/repository/files/" + path + "/raw?ref=" + config.branch;
        std::string encoded_path;
        for (char c : path) {
            if (c == '/') encoded_path += "%2F";
            else encoded_path += c;
        }
        api_url = config.base_url + "/repository/files/" + encoded_path + "/raw?ref=" + config.branch;
        content = exec("curl -s " + api_url);
    } else if (config.host_type == "bitbucket") {
        std::string raw_url = "https://bitbucket.org/" + config.owner + "/" + config.repo + "/raw/" + config.branch + "/" + path;
        content = exec("curl -s " + raw_url);
    } else {
        std::string raw_url = config.base_url + "/" + path;
        content = exec("curl -s " + raw_url);
    }

    if (content.empty()) {
        throw std::runtime_error("Failed to fetch template: " + path);
    }

    return content;
}

std::vector<std::string> fetch_templates_batch(const std::vector<std::string>& paths) {
    std::vector<std::string> contents(paths.size());
    std::vector<std::thread> threads;
    std::mutex mtx;
    std::string error_msg;

    auto config = get_repo_config();

    for (size_t i = 0; i < paths.size(); ++i) {
        threads.emplace_back([&, i]() {
            try {
                std::string content;

                if (config.host_type == "github") {
                    std::string api_url = config.base_url + "/contents/" + paths[i];
                    content = exec("curl -s -H 'Accept: application/vnd.github.v3.raw' " + api_url);
                } else if (config.host_type == "gitlab") {
                    std::string encoded_path;
                    for (char c : paths[i]) {
                        if (c == '/') encoded_path += "%2F";
                        else encoded_path += c;
                    }
                    std::string api_url = config.base_url + "/repository/files/" + encoded_path + "/raw?ref=" + config.branch;
                    content = exec("curl -s " + api_url);
                } else if (config.host_type == "bitbucket") {
                    std::string raw_url = "https://bitbucket.org/" + config.owner + "/" + config.repo + "/raw/" + config.branch + "/" + paths[i];
                    content = exec("curl -s " + raw_url);
                } else {
                    std::string raw_url = config.base_url + "/" + paths[i];
                    content = exec("curl -s " + raw_url);
                }

                if (content.empty()) {
                    std::lock_guard<std::mutex> lock(mtx);
                    error_msg = "Failed to fetch template: " + paths[i] + " (empty response)";
                    return;
                }

                if (content.find("\"message\"") != std::string::npos &&
                    (content.find("Not Found") != std::string::npos ||
                     content.find("404") != std::string::npos ||
                     content.find("Bad credentials") != std::string::npos ||
                     content.find("rate limit") != std::string::npos)) {
                    std::lock_guard<std::mutex> lock(mtx);

                    size_t msg_start = content.find("\"message\"");
                    std::string error_detail = "API error";
                    if (msg_start != std::string::npos) {
                        size_t colon = content.find(":", msg_start);
                        size_t quote_start = content.find("\"", colon) + 1;
                        size_t quote_end = content.find("\"", quote_start);
                        if (quote_start != std::string::npos && quote_end != std::string::npos) {
                            error_detail = content.substr(quote_start, quote_end - quote_start);
                        }
                    }

                    error_msg = "Failed to fetch template '" + paths[i] + "': " + error_detail;
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