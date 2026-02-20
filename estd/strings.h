#pragma once

#include <string>
#include "exceptions.h"

namespace estd
{
  template <typename type, std::size_t capacity>
  struct stack_allocator
  {
    using value_type = type;

    template <class _Other>
    struct rebind {
      using other = stack_allocator<_Other, capacity>;
    };

    stack_allocator() = default;
    stack_allocator(const stack_allocator& other) {}

    value_type* allocate(std::size_t size)
    {
      assert_condition(capacity == size, "Expected size to be equal to capacity for static string allocation [{}!={}].", capacity, size);
      return data;
    }

    void deallocate(value_type* ptr, std::size_t size)
    {
      assert_condition(capacity == size, "Expected size to be equal to capacity for static string allocation. [{}!={}]", capacity, size);
    }
  protected:
    value_type data[capacity]{};
  };

#pragma message("Need to improve (make work) capacity/size limit reached messages.")
  template<std::size_t capacity>
  class stack_string : public std::basic_string<char, std::char_traits<char>, estd::stack_allocator<char, capacity>>
  {
    using inherited = std::basic_string<char, std::char_traits<char>, estd::stack_allocator<char, capacity>>;
  public:
    static constexpr bool is_stack_string = true;

    static inline constexpr std::size_t get_static_capacity()
    {
      return capacity;
    }

    stack_string() : inherited()
    {
      constexpr std::size_t small_string_capacity = 16;
      inherited::reserve(capacity - small_string_capacity);
    }

    stack_string(const char* begin, const char* end) : stack_string()
    {
      inherited::append(begin, end);
    }

    stack_string(const char* intial_value) : stack_string()
    {
      inherited::append(intial_value);
    }

    stack_string(const stack_string& other) : stack_string()
    {
      inherited::append(other.c_str());
    }

    stack_string(stack_string&& other) : stack_string()
    {
      inherited::append(other.c_str());
    }

    stack_string& operator=(const char* other)
    {
      if (data() != other)
      {
        inherited::clear();
        inherited::append(other);
      }

      return *this;
    }

    stack_string& operator=(const stack_string& other)
    {
      if (data() != other.data())
      {
        inherited::clear();
        inherited::append(other.c_str());
      }

      return *this;
    }

    stack_string& operator=(stack_string&& other)
    {
      if (data() != other.data())
      {
        inherited::clear();
        inherited::append(other.c_str());
      }

      return *this;
    }

    void append(const char* value)
    {
      inherited::append(value);
    }

    template <typename type> requires(std::is_class_v<type>)
    void append(const type& other)
    {
      inherited::append(other.c_str());
    }
  };

  using stack_string_64 = stack_string<64>;
  using stack_string_128 = stack_string<128>;
  using stack_string_256 = stack_string<256>;
  using stack_string_512 = stack_string<512>;
  using stack_string_1024 = stack_string<1024>;
  using stack_string_2048 = stack_string<2048>;
  using stack_string_4096 = stack_string<4096>;
  using stack_string_8192 = stack_string<8192>;
  using stack_string_16384 = stack_string<16384>;
  using stack_string_32768 = stack_string<32768>;

  template <typename string_type> requires(string_type::is_stack_string)
  class std::formatter<string_type>
  {
  public:
    constexpr auto parse(const std::format_parse_context& context)
    {
      //assert(context.begin() == context.end() || *context.begin() == '}', "Failed to parse path format.");
      return context.begin();
    }

    template <typename format_context>
    auto format(const string_type& value, format_context& context) const
    {
      auto it = context.out();

      for (char c : value)
      {
        (*it) = c;
        ++it;
      }

      return it;
    }
  };

  template<typename string_type>
  void remove_file_extension(string_type& path)
  {
    while (path.size() && path.back() != '.') path.pop_back();
    if (path.size()) path.pop_back();
  }

  template<typename input_string_type, typename output_string_type>
  void remove_file_extension(const input_string_type& input, const output_string_type& output)
  {
    output = input;
    remove_file_extension(output);
  }

  template <typename string_type>
  void replace_extension(string_type& path, const char* new_extension)
  {
    remove_file_extension(path);
    path.append(new_extension);
  }

  template<typename path_string_type>
  void append_filename(const char* source_path, path_string_type& destination)
  {
#pragma message("Platform dependent code.")
    if (!source_path) return;

    const char* filename_start = source_path;
    const char* current_marker = source_path;

    while (current_marker[0] != '\0')
    {
      const bool is_new_name = current_marker[0] == '\\' && current_marker[1] != '\0';
      if (is_new_name) filename_start = current_marker;
      ++current_marker;
    }

    current_marker = filename_start;
    while (current_marker[0] != '\0' && current_marker[0] != '.')
    {
      destination.push_back(current_marker[0]);
      ++current_marker;
    }
  }

  template<typename filename_string_type, typename path_string_type>
  void append_filename(const filename_string_type& filename, path_string_type& path)
  {
    append_filename(filename.c_str(), path);
  }

  inline char capitalize(char c)
  {
    return (c >= 'a' && c <= 'z') ? (c - 'a' + 'A') : c;
  }

  inline void capitalize_inline(char& c)
  {
    c = capitalize(c);
  }

  int32_t stoi(const char* start, const char* end);
}