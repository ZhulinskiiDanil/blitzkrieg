#pragma once

#include "./PaginationResponse.hpp"
#include "./StartposLevel.hpp"

using GetLevelsResponse = PaginationResponse<std::vector<StartposLevel>>;