#pragma once

#include "parameter.h"

namespace estd
{
  namespace console
  {
    using parameters_list = std::map<parameter::name_type, parameter*>;

    class console
    {
    public:

      template <typename parameter_type>
      parameter_type& add_parameter(const char* name)
      {
        assert_condition(!parameters.count(name), "Parameter with the same name [{}] was already added.", name);

        auto* parameter = new parameter_type(name);
        parameters[name] = parameter;
        return *parameter;
      }

      template <typename parameter_type, typename... arguments_types>
      parameter_type& add_parameter(const char* name, arguments_types... arguments)
      {
        assert_condition(!parameters.count(name), "Parameter with the same name [{}] was already added.", name);

        auto* parameter = new parameter_type(name, std::forward<arguments_types>(arguments)...);
        parameters[name] = parameter;
        return *parameter;
      }

      const parameters_list& get_parameters() const { return parameters; }

      void parse(int32_t count, const char** arguments)
      {
#pragma message("Can use hash map if becomes too expensive.")
        for (uint32_t index{ 0 }; index + 1 < count; ++index)
        {
          const char* name = arguments[index];
          const char* value = arguments[index + 1];

          auto parameter_iterator = parameters.find(name);
          if (parameter_iterator != parameters.end())
          {
            auto* parameter = parameter_iterator->second;
            parameter->process(value);
          }
        }
      }

      void parse(int32_t count, const char** arguments, const char* parameter_name)
      {
        uint32_t argument_index = 0;
        while (argument_index < count && std::strcmp(arguments[argument_index], parameter_name))
        {
          ++argument_index;
        }

        const bool argument_was_not_provided = argument_index >= count;
        if (argument_was_not_provided) return;

        auto parameter_iterator = parameters.find(parameter_name);
        if (parameter_iterator != parameters.end())
        {
          auto* parameter = parameter_iterator->second;
          parameter->process(arguments[argument_index + 1]);
        }
      }

      void verify_mandatory() const
      {
        for (const auto& [parameter_name, parameter] : parameters)
        {
          if (parameter->is_mandatory() && !parameter->was_processed())
          {
            throw_error<std::logic_error>("Parameter was not provided [{}].", parameter->get_name());
          }
        }
      }

      bool insert(const char* name, const estd::json& object)
      {
        for (auto& [parameter_name, parameter] : parameters)
        {
          if (parameter_name == name)
          {
            return parameter->insert(object);
          }
        }

        return false;
      }

      bool extract(const char* name, estd::json& object) const
      {
        for (const auto& [parameter_name, parameter] : parameters)
        {
          if (parameter_name == name)
          {
            return parameter->extract(object);
          }
        }

        return false;
      }

      virtual ~console()
      {
        for (auto& [_, parameter] : parameters)
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