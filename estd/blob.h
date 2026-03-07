#pragma once

#include "files.h"

namespace estd
{
  struct blob
  {
    uint8_t* data = nullptr;
    uint64_t size = 0;
  };

  bool write_blob(const estd::path& path, const blob& object, bool force);
  blob read_blob(const estd::path& path, uint64_t offset, uint64_t size, uint8_t* buffer);
}