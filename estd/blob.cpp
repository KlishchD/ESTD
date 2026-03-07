#include "blob.h"
#include "logging.h"

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
  blob object;

  if (path.exists())
  {
    if (path.is_file())
    {
      if (buffer) object.data = buffer;
      else object.data = new uint8_t[size];
      object.size = size;

      std::ifstream file(path.c_str(), std::ios::binary);
      file.seekg(offset, std::ios::beg);
      file.read(reinterpret_cast<char*>(object.data), size);

      estd::log("Successfully read blob [{}].", path.c_str());
    }
    else
    {
      estd::log("Requested blob [{}] is not a file.", path.c_str());
    }
  }
  else
  {
    estd::log("Requested blob [{}] doesn't exists.", path.c_str());
  }

  return object;
}
