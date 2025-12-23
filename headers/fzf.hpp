#pragma once
#include <string>
#include <vector>

std::string fzf_select(
    const std::vector<std::string>& options,
    const std::string& query
);