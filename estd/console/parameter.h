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

      parameter(const char* name) : name(name), help("none"), mandatory(false)
      {
        assert_condition(name, "Must provide name for console parameter.");
      }

      virtual bool process(const char* data) = 0;
      virtual bool was_processed() const = 0;

      const name_type& get_name() const { return name; }

      const help_type& get_help() const { return help; }
      void set_help(const char* value) { help = value; }

      bool is_mandatory() const { return mandatory; }
      void set_madatory(bool status) { mandatory = status; }

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
      return_type& set_mandatory(bool status) { parameter::set_madatory(status); return *reinterpret_cast<return_type*>(this); }
    };

    template <typename parent_type>
    class inline_parameter : public parent_type
    {
    public:
      using value_type = parent_type::value_type;
      using inherited = parent_type;

      inline_parameter(const char* name, value_type default_value) : parent_type(name, &value)
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

      using path_type = path_string;
      using value_type = path_type;

      path_parameter(const char* name, estd::path* destination)
        : inherited(name), path(&(destination->get())), directory_test(false), file_test(false)
      {
        assert_condition(path, "Provided parameter destination didn't exist");
      }

      path_parameter(const char* name, path_type* destination)
        : inherited(name), path(destination), directory_test(false), file_test(false)
      {
        assert_condition(path, "Provided parameter destination didn't exist");
      }

      virtual bool process(const char* data) override
      {
        if (!data) { log("No data provided to parameter path parameter [{}].", name); return false; }

        std::error_code error;
        std::filesystem::path absolute_path = std::filesystem::absolute(data, error);
        if (error) { log("Provieded string is no an actual path [{}].", data); return false; }

        const bool exists = std::filesystem::exists(data);
        if (exists)
        {
          if (directory_test)
          {
            const bool directory = std::filesystem::is_directory(data);
            if (!directory) { log("Parameter [{}] failed directory check [{}].", name, data); return false; }
          }

          if (file_test)
          {
            const bool file = std::filesystem::is_regular_file(data);
            if (!file) { log("Parameter [{}] failed file check [{}].", name, data); return false; }
          }
        }

        (*path) = absolute_path.string().c_str();

        return true;
      }

      virtual bool was_processed() const override
      {
        return path->size();
      }

      return_type& set_directory(bool test)
      {
        assert_condition(file_test ^ test, "Can not test paramter [{}] for file and directory at the time.", name);
        directory_test = test;
        return *reinterpret_cast<return_type*>(this);
      }

      return_type& set_file(bool test)
      {
        assert_condition(directory_test ^ test, "Can not test paramter [{}] for file and directory at the time.", name);
        file_test = test;
        return *reinterpret_cast<return_type*>(this);
      }
    protected:
      path_type* path;
      bool directory_test;
      bool file_test;
    };

    using inline_path_parmeter = inline_parameter<path_parameter>;

    template <typename value_type> requires(std::is_integral_v<value_type>)
    class integral_parameter : public base_parameter<integral_parameter<value_type>>
    {
    public:
      using inherited = base_parameter<integral_parameter<value_type>>;
      using return_type = inherited::return_type;
      using store_type = std::conditional_t<std::is_unsigned_v<value_type>, unsigned long long, long long>;

      integral_parameter(const char* name, value_type* destination) 
        : inherited(name), value(destination),
        range_test(false),
        min(std::numeric_limits<value_type>::min()),
        max(std::numeric_limits<value_type>::max()),
        processed(false)
      {
        assert_condition(destination, "Must provide destination for integral parameter.");
      }

      virtual bool process(const char* data) override
      {
        if (!data) { log("No data provided to parameter integral parameter [{}].", inherited::name); return false; }

        constexpr const char* format = std::is_unsigned_v<value_type> ? "%llu" : "lld";

        store_type store;
        int32_t parsed = std::sscanf(data, format, &store);

        if (parsed != 1) { log("Failed to parse value [{}] for integral parameter [{}].", data, inherited::name); return false; }

        bool is_in_range = min <= store && store <= max;
        if (!is_in_range) { log("Integral parameter [{}] failed range test [{}] - [{}-{}].", inherited::name, store, min, max); return false; }

        (*value) = store;

        processed = true;
        return true;
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

      floating_point_parameter(const char* name, value_type* destination)
        : inherited(name), value(destination),
        range_test(false),
        min(std::numeric_limits<value_type>::min()),
        max(std::numeric_limits<value_type>::max()),
        processed(false)
      {
        assert_condition(destination, "Must provide destination for floating point parameter.");
      }

      virtual bool process(const char* data) override
      {
        if (!data) { log("No data provided to parameter floating point parameter [{}].", inherited::name); return false; }

        double store;

        int32_t parsed = std::sscanf(data, "%lf", &store);
        if (parsed != 1) { log("Failed to parse value [{}] for floating point parameter [{}].", data, inherited::name); return false; }

        bool is_in_range = min <= store && store <= max;
        if (!is_in_range) { log("Floating point parameter [{}] failed range test [{}] - [{}-{}].", inherited::name, store, min, max); return false; }

        (*value) = store;

        processed = true;
        return true;
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
    protected:
      value_type* value;

      value_type min;
      value_type max;
      bool range_test;

      bool processed;
    };


    using unsigned_integer_parameter = integral_parameter<uint32_t>;
    using integer_parameter = integral_parameter<int32_t>;

    using float_parameter = floating_point_parameter<float>;
    using double_parameter = floating_point_parameter<double>;

    class bool_parameter : public base_parameter<bool_parameter>
    {
    public:
      using inherited = base_parameter<bool_parameter>;
      using return_type = inherited::return_type;

      bool_parameter(const char* name, bool* destination)
        : inherited(name), value(destination), processed(false)
      {
        assert_condition(destination, "Must provide destination to bool parameter.");
      }

      virtual bool process(const char* data) override
      {
        if (!data) { log("No data provided to parameter bool parameter [{}].", name); return false; }

        constexpr const char* incorrect_format = "Failed to parser value [{}] for bool parameter [{}], acceptable values are [0, 1, on, off, true, false].";

        const std::size_t size = std::strlen(data);
        if (size == 1)
        {
          bool is_on = data[0] == '1';
          bool is_off = data[0] == '0';

          const bool incorrect = !is_on && !is_off;
          if (incorrect) { log(incorrect_format, data, name); return false; }

          (*value) = is_on;
        }
        else if (size == 2)
        {
          const bool is_on =
            capitalize(data[0]) == 'O' &&
            capitalize(data[1]) == 'N';

          if (!is_on) { log(incorrect_format, data, name); return false; }

          (*value) = true;
        }
        else if (size == 3)
        {
          const bool is_off =
            capitalize(data[0]) == 'O' &&
            capitalize(data[1]) == 'F' &&
            capitalize(data[1]) == 'F';

          if (!is_off) { log(incorrect_format, data, name); return false; }

          (*value) = false;
        }
        else if (size == 4)
        {
          const bool is_true =
            capitalize(data[0]) == 'T' &&
            capitalize(data[1]) == 'R' &&
            capitalize(data[2]) == 'U' &&
            capitalize(data[3]) == 'E';

          if (!is_true) { log(incorrect_format, data, name); return false; }

          (*value) = true;
        }
        else if (size == 5)
        {
          const bool is_false =
            capitalize(data[0]) == 'F' &&
            capitalize(data[1]) == 'A' &&
            capitalize(data[2]) == 'L' &&
            capitalize(data[3]) == 'S' &&
            capitalize(data[4]) == 'E';

          if (!is_false) { log(incorrect_format, data, name); return false; }

          (*value) = false;
        }
        else
        {
          log(incorrect_format, data, name);
          return false;
        }

        processed = true;

        return true;
      }

      virtual bool was_processed() const override
      {
        return processed;
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

      marker_parameter(const char* name, char* destination)
        : inherited(name), value(destination), processed(false)
      {
        assert_condition(destination, "Must provide destination to bool parameter.");
      }

      virtual bool process(const char* data) override
      {
        if (!data) { log("No data provided to parameter marker parameter [{}].", name); return false; }

        std::size_t size = std::strlen(data);
        if (size != 1) { log("Failed to parse non marker string [{}] for marker parameter [{}].", data, name); return false; }

        (*value) = data[0];

        processed = true;
        return true;
      }

      virtual bool was_processed() const override
      {
        return processed;
      }

    protected:
      char* value;
      bool processed;
    };
  };
};