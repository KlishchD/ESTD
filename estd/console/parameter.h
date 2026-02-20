#pragma once

namespace estd
{
  namespace console
  {
    class parameter
    {
    public:
      using name_type = stack_string_128;
      using help_type = stack_string_512;
      using type_type = stack_string_64;

      parameter(const char* name) : name(name), help("none"), mandatory(false)
      {
        assert_condition(name, "Must provide name for console parameter.");
      }

      virtual type_type get_type() = 0;
      virtual bool extract(json& destination) const = 0;
      virtual bool insert(const json& source) = 0;

      virtual bool process(const char* data) = 0;
      virtual bool was_processed() const = 0;

      const name_type& get_name() const { return name; }

      const help_type& get_help() const { return help; }
      void set_help(const char* value) { help = value; }

      bool is_mandatory() const { return mandatory; }
      void set_mandatory(bool status) { mandatory = status; }

      virtual ~parameter() = default;
    protected:
      name_type name;
      help_type help;
      bool mandatory;
    };

    template <typename child_type>
    class base_parameter : public parameter
    {
    public:
      using return_type = child_type;

      base_parameter(const char* name) : parameter(name)
      { }

      return_type& set_help(const char* value) { parameter::set_help(value); return *reinterpret_cast<return_type*>(this); }
      return_type& set_mandatory(bool status) { parameter::set_mandatory(status); return *reinterpret_cast<return_type*>(this); }
    };

    template <typename parent_type>
    class inline_parameter : public parent_type
    {
    public:
      using value_type = parent_type::value_type;
      using inherited = parent_type;

      inline_parameter(const char* name, const value_type& default_value) : parent_type(name, &value)
      {
        value = default_value;
      }
    protected:
      value_type value;
    };

    class path_parameter : public base_parameter<path_parameter>
    {
    public:
      using inherited = base_parameter<path_parameter>;
      using return_type = inherited::return_type;
      using value_type = path_string;

      path_parameter(const char* name, estd::path* destination)
        : inherited(name), path(&(destination->get())), directory_test(false), file_test(false)
      {
        assert_condition(path, "Provided parameter destination didn't exist");
      }

      path_parameter(const char* name, value_type* destination)
        : inherited(name), path(destination), directory_test(false), file_test(false)
      {
        assert_condition(path, "Provided parameter destination didn't exist");
      }

      virtual type_type get_type() override
      {
        return "path";
      }

      virtual bool extract(json& destination) const override
      {
        destination = path->c_str();
        return true;
      }

      virtual bool insert(const json& source) override
      {
        estd::path desired_path = fetch_c_str(source);
        return set_value(desired_path);
      }

      virtual bool process(const char* data) override
      {
        if (!data) { log("No data provided to parameter path parameter [{}].", inherited::name); return false; }

        estd::path provided_path = data;

        std::error_code error = provided_path.to_absolute();
        if (error) { log("Provieded string is not an actual path [{}].", data); return false; }

        return set_value(provided_path);
      }

      virtual bool was_processed() const override
      {
        return path->size();
      }

      return_type& set_directory(bool test)
      {
        assert_condition(file_test ^ test, "Can not test paramter [{}] for file and directory at the time.", inherited::name);
        directory_test = test;
        return *reinterpret_cast<return_type*>(this);
      }

      return_type& set_file(bool test)
      {
        assert_condition(directory_test ^ test, "Can not test paramter [{}] for file and directory at the time.", inherited::name);
        file_test = test;
        return *reinterpret_cast<return_type*>(this);
      }

      bool set_value(const value_type& desired_path)
      {
        return set_value(estd::path(desired_path));
      }

      bool set_value(const estd::path& desired_path)
      {
        if (desired_path.exists())
        {
          if (directory_test)
          {
            const bool directory = desired_path.is_directory();
            if (!directory) { log("Parameter [{}] failed directory check [{}].", inherited::name, desired_path); return false; }
          }

          if (file_test)
          {
            const bool file = desired_path.is_file();
            if (!file) { log("Parameter [{}] failed file check [{}].", inherited::name, desired_path); return false; }
          }
        }

        (*path) = desired_path.get();

        return true;
      }

      const value_type& get_value() const
      {
        return *path;
      }

      const value_type& operator()() const
      {
        return *path;
      }

      value_type& operator()()
      {
        return *path;
      }
    protected:
      value_type* path;
      bool directory_test;
      bool file_test;
    };

    template <typename value_type> requires(std::is_integral_v<value_type>)
    class integral_parameter : public base_parameter<integral_parameter<value_type>>
    {
    public:
      using inherited = base_parameter<integral_parameter<value_type>>;
      using return_type = inherited::return_type;
      using store_type = std::conditional_t<std::is_unsigned_v<value_type>, unsigned long long, long long>;
      using value_type = value_type;

      integral_parameter(const char* name, value_type* destination) 
        : inherited(name), value(destination),
        range_test(false),
        min(std::numeric_limits<value_type>::min()),
        max(std::numeric_limits<value_type>::max()),
        processed(false)
      {
        assert_condition(destination, "Must provide destination for integral parameter.");
      }

      virtual type_type get_type() override
      {
        if constexpr (std::is_same_v<value_type, uint8_t>)
        {
          return "u8";
        }
        else if constexpr (std::is_same_v<value_type, int8_t>)
        {
          return "i8";
        }
        else if constexpr (std::is_same_v<value_type, uint16_t>)
        {
          return "u16";
        }
        else if constexpr (std::is_same_v<value_type, int16_t>)
        {
          return "i16";
        }
        else if constexpr (std::is_same_v<value_type, uint32_t>)
        {
          return "u32";
        }
        else if constexpr (std::is_same_v<value_type, int32_t>)
        {
          return "i32";
        }
        else if constexpr (std::is_same_v<value_type, uint64_t>)
        {
          return "u64";
        }
        else if constexpr (std::is_same_v<value_type, int64_t>)
        {
          return "i64";
        }
        else
        {
          static_assert(0, "Failed to match integral type.");
        }
      }

      virtual bool extract(json& destination) const override
      {
        destination = *value;
        return true;
      }

      virtual bool insert(const json& source) override
      {
        return set_value(source.get<value_type>());
      }

      virtual bool process(const char* data) override
      {
        if (!data) { log("No data provided to parameter integral parameter [{}].", inherited::name); return false; }

        constexpr const char* format = std::is_unsigned_v<value_type> ? "%llu" : "lld";

        store_type store;
        int32_t parsed = std::sscanf(data, format, &store);

        if (parsed != 1) { log("Failed to parse value [{}] for integral parameter [{}].", data, inherited::name); return false; }

        return set_value(store);
      }

      virtual bool was_processed() const override
      {
        return processed;
      }

      return_type& set_range(value_type desired_min, value_type desired_max)
      {
        range_test = true;
        min = desired_min;
        max = desired_max;
        return *this;
      }

      bool set_value(value_type desired_value)
      {
        bool is_in_range = min <= desired_value && desired_value <= max;
        if (!is_in_range) { log("Integral parameter [{}] failed range test [{}] - [{}-{}].", inherited::name, desired_value, min, max); return false; }

        (*value) = desired_value;
        processed = true;

        return true;
      }

      value_type get_value() const
      {
        return *value;
      }

      value_type get_min() const
      {
        return min;
      }

      value_type get_max() const
      {
        return max;
      }

      const value_type& operator()() const
      {
        return *value;
      }

      value_type& operator()()
      {
        return *value;
      }
    protected:
      value_type* value;

      value_type min;
      value_type max;
      bool range_test;

      bool processed;
    };

    template <typename value_type> requires(std::is_floating_point_v<value_type>)
    class floating_point_parameter : public base_parameter<floating_point_parameter<value_type>>
    {
    public:
      using inherited = base_parameter<floating_point_parameter<value_type>>;
      using return_type = inherited::return_type;
      using value_type = value_type;

      floating_point_parameter(const char* name, value_type* destination)
        : inherited(name), value(destination),
        range_test(false),
        min(std::numeric_limits<value_type>::min()),
        max(std::numeric_limits<value_type>::max()),
        processed(false)
      {
        assert_condition(destination, "Must provide destination for floating point parameter.");
      }

      virtual type_type get_type() override
      {
        if constexpr (std::is_same_v<value_type, float>)
        {
          return "f32";
        }
        else if constexpr (std::is_same_v<value_type, double>)
        {
          return "f64";
        }
        else
        {
          static_assert(0, "Failed to match floating point type.");
        }
      }

      virtual bool extract(json& destination) const override
      {
        destination = *value;
        return true;
      }

      virtual bool insert(const json& source) override
      {
        return set_value(source.get<value_type>());
      }

      virtual bool process(const char* data) override
      {
        if (!data) { log("No data provided to parameter floating point parameter [{}].", inherited::name); return false; }

        double store;

        int32_t parsed = std::sscanf(data, "%lf", &store);
        if (parsed != 1) { log("Failed to parse value [{}] for floating point parameter [{}].", data, inherited::name); return false; }

        return set_value(store);
      }

      virtual bool was_processed() const override
      {
        return processed;
      }

      return_type& set_range(value_type desired_min, value_type desired_max)
      {
        range_test = true;
        min = desired_min;
        max = desired_max;
        return *this;
      }

      bool set_value(value_type desired_value)
      {
        bool is_in_range = min <= desired_value && desired_value <= max;
        if (!is_in_range) { log("Floating point parameter [{}] failed range test [{}] - [{}-{}].", inherited::name, desired_value, min, max); return false; }

        (*value) = desired_value;

        processed = true;
        return true;
      }

      value_type get_value() const
      {
        return *value;
      }

      value_type get_min() const
      {
        return min;
      }

      value_type get_max() const
      {
        return max;
      }

      const value_type& operator()() const
      {
        return *value;
      }

      value_type& operator()()
      {
        return *value;
      }
    protected:
      value_type* value;

      value_type min;
      value_type max;
      bool range_test;

      bool processed;
    };

    class bool_parameter : public base_parameter<bool_parameter>
    {
    public:
      using inherited = base_parameter<bool_parameter>;
      using return_type = inherited::return_type;
      using value_type = bool;

      bool_parameter(const char* name, bool* destination)
        : inherited(name), value(destination), processed(false)
      {
        assert_condition(destination, "Must provide destination to bool parameter.");
      }

      virtual type_type get_type() override
      {
        return "bool";
      }

      virtual bool extract(json& destination) const override
      {
        destination = *value;
        return true;
      }

      virtual bool insert(const json& source) override
      {
        return set_value(source.get<bool>());
      }

      virtual bool process(const char* data) override
      {
        if (!data) { log("No data provided to parameter bool parameter [{}].", inherited::name); return false; }

        constexpr const char* incorrect_format = "Failed to parser value [{}] for bool parameter [{}], acceptable values are [0, 1, on, off, true, false].";

        const std::size_t size = std::strlen(data);
        if (size == 1)
        {
          bool is_on = data[0] == '1';
          bool is_off = data[0] == '0';

          const bool incorrect = !is_on && !is_off;
          if (incorrect) { log(incorrect_format, data, inherited::name); return false; }

          set_value(is_on);
        }
        else if (size == 2)
        {
          const bool is_on =
            capitalize(data[0]) == 'O' &&
            capitalize(data[1]) == 'N';

          if (!is_on) { log(incorrect_format, data, inherited::name); return false; }

          set_value(true);
        }
        else if (size == 3)
        {
          const bool is_off =
            capitalize(data[0]) == 'O' &&
            capitalize(data[1]) == 'F' &&
            capitalize(data[1]) == 'F';

          if (!is_off) { log(incorrect_format, data, inherited::name); return false; }

          set_value(false);
        }
        else if (size == 4)
        {
          const bool is_true =
            capitalize(data[0]) == 'T' &&
            capitalize(data[1]) == 'R' &&
            capitalize(data[2]) == 'U' &&
            capitalize(data[3]) == 'E';

          if (!is_true) { log(incorrect_format, data, inherited::name); return false; }

          set_value(true);
        }
        else if (size == 5)
        {
          const bool is_false =
            capitalize(data[0]) == 'F' &&
            capitalize(data[1]) == 'A' &&
            capitalize(data[2]) == 'L' &&
            capitalize(data[3]) == 'S' &&
            capitalize(data[4]) == 'E';

          if (!is_false) { log(incorrect_format, data, inherited::name); return false; }

          set_value(false);
        }
        else
        {
          log(incorrect_format, data, inherited::name);
          return false;
        }

        return true;
      }

      virtual bool was_processed() const override
      {
        return processed;
      }

      bool set_value(bool desired_value)
      {
        (*value) = desired_value;
        processed = true;
        return true;
      }

      bool get_value() const
      {
        return *value;
      }

      const value_type& operator()() const
      {
        return *value;
      }

      value_type& operator()()
      {
        return *value;
      }
    protected:
      bool* value;
      bool processed;
    };

    class marker_parameter : public base_parameter<marker_parameter>
    {
    public:
      using inherited = base_parameter<marker_parameter>;
      using return_type = inherited::return_type;
      using value_type = char;

      marker_parameter(const char* name, char* destination)
        : inherited(name), value(destination), processed(false)
      {
        assert_condition(destination, "Must provide destination to bool parameter.");
      }

      virtual type_type get_type() override
      {
        return "marker";
      }

      virtual bool extract(json& destination) const override
      {
        destination = *value;
        return true;
      }

      virtual bool insert(const json& source) override
      {
        return set_value(source.get<char>());
      }

      virtual bool process(const char* data) override
      {
        if (!data) { log("No data provided to parameter marker parameter [{}].", inherited::name); return false; }

        std::size_t size = std::strlen(data);
        if (size != 1) { log("Failed to parse non marker string [{}] for marker parameter [{}].", data, inherited::name); return false; }

        return set_value(data[0]);
      }

      virtual bool was_processed() const override
      {
        return processed;
      }

      bool set_value(char desired_value)
      {
        (*value) = desired_value;
        processed = true;

        return true;
      }

      const value_type& operator()() const
      {
        return *value;
      }

      value_type& operator()()
      {
        return *value;
      }
    protected:
      char* value;
      bool processed;
    };

    using unsigned_integer_parameter = integral_parameter<uint32_t>;
    using integer_parameter = integral_parameter<int32_t>;
    using float_parameter = floating_point_parameter<float>;
    using double_parameter = floating_point_parameter<double>;

    using inline_path_parameter = inline_parameter<path_parameter>;
    using inline_unsigned_integer_parameter = inline_parameter<integral_parameter<uint32_t>>;
    using inline_integer_parameter = inline_parameter<integral_parameter<int32_t>>;
    using inline_float_parameter = inline_parameter<floating_point_parameter<float>>;
    using inline_double_parameter = inline_parameter<floating_point_parameter<double>>;
    using inline_bool_parameter = inline_parameter<bool_parameter>;
    using inline_marker_parameter = inline_parameter<marker_parameter>;
  }
}
