#pragma once

#include "parameter.h"

namespace estd
{
  namespace console
  {
    using parameters_list = std::vector<parameter*>;

    class console
    {
    public:

      template <typename parameter_type>
      parameter_type& add_parameter(const char* name)
      {
        parameter_type* result = new parameter_type(name);
        parameters.push_back(result);
        return *result;
      }

      const parameters_list& get_parameters() const { return parameters; }

      template <typename parameter_type, typename... arguments_types>
      parameter_type& add_parameter(const char* name, arguments_types... arguments)
      {
        parameter_type* result = new parameter_type(name, std::forward<arguments_types>(arguments)...);
        parameters.push_back(result);
        return *result;
      }

      void parse(int32_t count, const char** arguments)
      {
#pragma message("Can use hash map if becomes too expensive.")
        for (uint32_t index{ 0 }; index + 1 < count; ++index)
        {
          for (auto* parameter : parameters)
          {
            if (parameter->get_name() == arguments[index])
            {
              parameter->process(arguments[index + 1]);
              break;
            }
          }
        }
      }

      void verify_mandatory() const
      {
        for (const auto* parameter : parameters)
        {
          if (parameter->is_mandatory() && !parameter->was_processed())
          {
            throw_error<std::logic_error>("Parameter was not provided [{}].", parameter->get_name());
          }
        }
      }

      virtual ~console()
      {
        for (auto& parameter : parameters)
        {
          delete parameter;
          parameter = nullptr;
        }
      }
    protected:
      parameters_list parameters;
    };
  }
};