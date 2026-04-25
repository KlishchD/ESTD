#include "blob.h"
#include "logging.h"
#include "platform.h"
#include <Windows.h>

uint64_t estd::adjust_read_size(uint64_t size)
{
  const uint64_t sector_size = fetch_sector_size();
  const uint64_t blocks_count = (size + sector_size) / sector_size;
  return blocks_count * sector_size;
}

bool estd::write_blob(const estd::path& path, const estd::blob& object, bool force)
{
  if (!path.is_file())
  {
    estd::log("Blob write path [{}] must be a file.", path.c_str());
    return false;
  }

  if (path.exists())
  {
    if (force)
    {
      estd::log("Overwriting file blob [{}].", path.c_str());
      path.remove();
    }
    else
    {
      estd::log("Blob alredy exists at a location [{}].", path.c_str());
      return false;
    }
  }

  std::ofstream file(path.c_str(), std::ios::binary);
  file.write(reinterpret_cast<const char*>(object.data), object.size);

  return true;
}

estd::blob estd::read_blob(const estd::path& path, uint64_t offset, uint64_t size, uint8_t* buffer)
{
  blob object{};

#pragma message("Platform specific logic.")

  if (!path.exists())
  {
    estd::log("Requested blob [{}] doesn't exists.", path.c_str());
    return object;
  }

  if (!path.is_file())
  {
    estd::log("Requested blob [{}] is not a file.", path.c_str());
    return object;
  }

  if (buffer) object.data = buffer;
  else object.data = new uint8_t[size];
  object.size = size;

  const uint64_t sector_size = fetch_sector_size();
  const bool is_aligned_properly = size % sector_size == 0;

  bool unbuffered_oi_succeded = false;

  while (is_aligned_properly)
  {
    HANDLE file = CreateFileA(
      path.c_str(),
      GENERIC_READ,
      FILE_SHARE_READ,
      nullptr,
      OPEN_EXISTING,
      FILE_FLAG_NO_BUFFERING,
      0
    );

    if (file == INVALID_HANDLE_VALUE)
    {
      estd::log("Failed to open a file [{}].", path.c_str());
      break;
    }

    DWORD bytes_read;
    OVERLAPPED overlapped{ 0 };
    BOOL success = ReadFile(file, object.data, size, &bytes_read, &overlapped);

    if (!success)
    {
      estd::log("Failed to read file [{}][{}].", path.c_str(), GetLastError());
      break;
    }

    unbuffered_oi_succeded = true;
    break;
  }

  if (!unbuffered_oi_succeded)
  {
    std::fstream file;
    file.rdbuf()->pubsetbuf(0, 0);

    file.open(path.c_str(), std::ios::binary | std::ios::in);
    file.seekg(offset, std::ios::beg);
    file.read(reinterpret_cast<char*>(object.data), size);
  }

  return object;
}
