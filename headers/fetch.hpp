#pragma once
#include <string>
#include <vector>

std::vector<std::string> load_or_fetch_index();
std::string resolve_template(const std::string& query,
                             std::vector<std::string>* index = nullptr);
std::string fetch_template(const std::string& path);
std::vector<std::string> fetch_templates_batch(const std::vector<std::string>& paths);