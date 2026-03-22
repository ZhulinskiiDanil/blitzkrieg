#pragma once

#include <string>
#include <vector>

template <typename T>
struct PaginationResponse
{
    T data;
    int page;
    int totalPages;
    int total;
};