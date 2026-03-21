#pragma once

#include <string>
#include <vector>

template <typename T>
struct PaginationResponse {
    T data;
    int page;
    int totalPage;
    int total;
};