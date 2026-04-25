#pragma once

#include "nlohmann\json.hpp"
#include "strings.h"
#include "files.h"

namespace estd
{
  using json = nlohmann::json;

  template <typename value_type>
  const value_type* fetch_value(const json& input, const char* value_name)
  {
    if (!input.contains(value_name)) return nullptr;
    return input[value_name].get_ptr<const value_type*>();
  }

  template <typename value_type>
  const value_type* fetch_value(const json& input, std::size_t index)
  {
    if (index >= input.size()) return nullptr;
    return input[index].get_ptr<const value_type*>();
  }

  inline const char* fetch_c_str(const json& input, const char* value_name)
  {
    const std::string* value = fetch_value<std::string>(input, value_name);
    return value ? value->c_str() : nullptr;
  }

  inline const char* fetch_c_str(const json& input, std::size_t index)
  {
    const std::string* value = fetch_value<std::string>(input, index);
    return value ? value->c_str() : nullptr;
  }

  inline const char* fetch_c_str(const json& input)
  {
    const std::string* value = input.get_ptr<const std::string*>();
    return value ? value->c_str() : nullptr;
  }

  template <typename value_type>
  inline value_type fetch_or_default(const json& input, const char* value_name, const value_type& default_value)
  {
    return input.contains(value_name) ? static_cast<value_type>(input[value_name]) : default_value;
  }

  json read_json(const path_string& path);
  json read_json(const path& path);

  void write_json(const path_string& path, const json& object);
  void write_json(const path& path, const json& object);
}