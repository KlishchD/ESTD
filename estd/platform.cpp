#include "platform.h"
#include <Windows.h>

std::string estd::fetch_executable_path()
{
  constexpr std::size_t buffer_size = MAX_PATH;
  char executable_path[buffer_size];

  std::size_t size = GetModuleFileNameA(nullptr, executable_path, buffer_size);
  estd::assert_condition(size, "Failed to fetch executable path.");

  return executable_path;
}

uint64_t estd::fetch_sector_size()
{
  static uint64_t result = 0;

  if (!result)
  {
    DWORD sectors_per_cluster;
    DWORD sector_size;
    DWORD free_clusters_count;
    DWORD clusters_count;
    GetDiskFreeSpaceA(nullptr, &sectors_per_cluster, &sector_size, &free_clusters_count, &clusters_count);

    result = sector_size;
  }

  return result;
}
