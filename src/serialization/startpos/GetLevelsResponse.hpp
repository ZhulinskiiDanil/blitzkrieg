#pragma once

#include "./PaginationResponse.hpp"
#include "./StartPosLevel.hpp"

using GetLevelsResponse = PaginationResponse<std::vector<StartPosLevel>>;