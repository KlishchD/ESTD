#pragma once

#include <string>
#include "exceptions.h"

namespace estd
{
  std::string fetch_executable_path();
  uint64_t fetch_sector_size();
}