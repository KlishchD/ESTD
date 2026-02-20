#pragma once

#include "strings.h"
#include <filesystem>

namespace estd
{
  using file_clock = std::chrono::file_clock;
  using file_time = std::filesystem::file_time_type;

  using path_string = stack_string_256;
  using extension_string = stack_string_64;
  using filename_string = stack_string_128;

  class path
  {
  public:
    path() : store()
    { }

    path(const char* value) : store(value)
    { }

    explicit path(const path& other) : store(other.store)
    { }

    template <typename string_type> requires(std::is_class_v<string_type>)
    explicit path(const string_type& other) : store(other.c_str())
    { }

    path(path&& other) : store(std::move(other.store))
    { }

    path& operator=(const path& other)
    {
      store = other.store;
      return *this;
    }

    path& operator=(path&& other)
    {
      store = std::move(other.store);
      return *this;
    }

    template <typename string_type> requires(std::is_class_v<string_type>)
    path& operator=(const string_type& other)
    {
      store = other.c_str();
      return *this;
    }

    path& operator=(const char* other)
    {
      store = other;
      return *this;
    }

    bool operator==(const path_string& other) const
    {
      return store == other;
    }

    bool operator==(const path& other) const
    {
      return store == other.store;
    }

    bool operator==(const char* other) const
    {
      return store == other;
    }

    bool operator!=(const path_string& other) const
    {
      return store != other;
    }

    bool operator!=(const path& other) const
    {
      return store != other.store;
    }

    bool operator!=(const char* other) const
    {
      return store != other;
    }

    bool operator<(const path_string& other) const
    {
      return store < other;
    }

    bool operator<(const path& other) const
    {
      return store < other.store;
    }

    bool operator>(const path& other) const
    {
      return store > other.store;
    }

    bool operator>(const path_string& other) const
    {
      return store > other;
    }

#pragma message("Platform dependent code.")
    path& append(const char* value)
    {
      if (!value) return *this;

      const bool not_extension = value[0] != '.';
      const bool needs_separator = store.size() && store.back() != '\\';
      if (not_extension && needs_separator)
      {
        store.push_back('\\');
      }

      store.append(value);

      return *this;
    }

    path& append(const path& relative)
    {
      return append(relative.c_str());
    }

    template <typename string_type> requires(std::is_class_v<string_type>)
    path& append(const string_type& relative)
    {
      return append(relative.c_str());
    }

    path& push_back(char c)
    {
      store.push_back(c);
      return *this;
    }

    path& replace_filename(const char* filename)
    {
#pragma message("Platform depenedant code.")
      while (store.size() && store.back() != '\\')
      {
        store.pop_back();
      }

      store.append(filename);

      return *this;
    }

    path& replace_extension(const char* extension)
    {
      std::size_t extension_index = get_extension_start_index();
      while (extension_index < store.size()) store.pop_back();

      store.append(extension);
      return *this;
    }

    path& pop()
    {
#pragma message("Platform dependant code.")
      while (store.size() && store.back() != '\\')
      {
        store.pop_back();
      }

      return *this;
    }

    extension_string get_extension() const
    {
      extension_string result;

      for (std::size_t index { get_extension_start_index() }; index < store.size(); ++index)
      {
        result.push_back(store[index]);
      }

      return result;
    }

    filename_string get_filename() const
    {
      filename_string result;

      for (std::size_t index{ get_filename_start_index() }; index < store.size(); ++index)
      {
        result.push_back(store[index]);
      }

      return result;
    }

    filename_string get_stem() const
    {
      filename_string result;

#pragma message("Platform dependant code.")
      for (std::size_t index{ get_filename_start_index() }; index < store.size(); ++index)
      {
        const bool end_marker = store[index] == '.' || store[index] == '\\';
        if (end_marker) break;

        result.push_back(store[index]);
      }

      return result;
    }

    std::size_t size() const
    {
      return store.size();
    }

    bool empty() const
    {
      return store.empty();
    }

    const char* data() const
    {
      return store.data();
    }

    const char* c_str() const
    {
      return store.c_str();
    }

    const path_string& get() const
    {
      return store;
    }

    path_string& get()
    {
      return store;
    }

    bool exists() const
    {
      return std::filesystem::exists(store.c_str());
    }

    bool is_file() const
    {
      if (!exists()) return true;
      return std::filesystem::is_regular_file(store.c_str());
    }

    bool is_directory() const
    {
      if (!exists()) return true;
      return std::filesystem::is_directory(store.c_str());
    }

    bool is_relative() const
    {
      return std::filesystem::path(store.c_str()).is_relative();
    }

    bool is_absolute() const
    {
      return std::filesystem::path(store.c_str()).is_absolute();
    }

#pragma message("Platform dependant code.")
    std::size_t get_filename_start_index() const
    {
      std::size_t index = store.size() - 1;

      while (index < store.size() && store[index] == '\\')
      {
        --index;
      }

      while (index < store.size() && store[index] != '\\')
      {
        --index;
      }

      return index >= store.size() ? 0 : index + 1;
    }

    std::size_t get_extension_start_index() const
    {
      std::size_t index = store.size() - 1;

      while (index < store.size() && store[index] != '\\' && store[index] != '.')
      {
        --index;
      }

      return index < store.size() && store[index] == '.' ? index : std::size_t(-1);
    }

    bool has_extension() const
    {
      return get_extension_start_index() < store.size();
    }

    bool remove() const
    {
      if (!exists()) return false;
      std::filesystem::remove(store.c_str());
      return true;
    }

    bool remove_all() const
    {
      if (!exists()) return false;
      std::filesystem::remove_all(store.c_str());
      return true;
    }

    void copy_from(const char* source) const
    {
      if (exists()) return;
      std::filesystem::copy(source, store.c_str());
    }

    void copy_from(const path& source) const
    {
      copy_from(source.c_str());
    }

    template <typename string_type> requires(std::is_class_v<string_type>)
    void copy_from(const string_type& source)
    {
      copy_from(source.c_str());
    }

    void copy_to(const char* destination) const
    {
      if (!exists()) return;
      std::filesystem::copy(store.c_str(), destination);
    }

    void copy_to(const path& destination) const
    {
      copy_to(destination.c_str());
    }

    template <typename string_type> requires(std::is_class_v<string_type>)
    void copy_to(const string_type& destination) const
    {
      copy_to(destination.c_str());
    }

    bool create_directory() const
    {
      if (exists()) return false;
      std::filesystem::create_directories(store.c_str());
      return true;
    }

    file_time get_last_write_time() const
    {
      return std::filesystem::last_write_time(store.c_str());
    }

    std::filesystem::recursive_directory_iterator iterate_recursively() const
    {
      return std::filesystem::recursive_directory_iterator(store.c_str());
    }

    std::filesystem::directory_iterator iterate() const
    {
      return std::filesystem::directory_iterator(store.c_str());
    }

    std::error_code to_absolute()
    {
      std::error_code error;
      std::filesystem::path result = std::filesystem::absolute(store.c_str(), error);

      if (!error)
      {
        store = result.string().c_str();
      }

      return error;
    }
  protected:
    path_string store;
  };

  template <>
  class std::formatter<path>
  {
  public:
    constexpr auto parse(const std::format_parse_context& context)
    {
      //assert(context.begin() == context.end() || *context.begin() == '}', "Failed to parse path format.");
      return context.begin();
    }

    template <typename format_context>
    auto format(const path& value, format_context& context) const
    {
      return std::format_to(context.out(), "{}", value.get());
    }
  };
}