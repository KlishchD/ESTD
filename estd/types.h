#pragma once

#include "strings.h"

namespace estd
{
  class marker_type
  {
  public:
    marker_type(char marker) : marker(marker)
    { }

    inline bool match(char other_marker) const
    {
      return other_marker == marker;
    }

    inline bool match(const char* markers) const
    {
      if (!markers) return false;

      while (markers[0] != '\0')
      {
        if (markers[0] == marker) return true;
        ++markers;
      }

      return false;
    }

    template <typename string_type>
    inline bool match(const string_type& markers) const
    {
      if (markers.empty()) return false;
      return match(markers.c_str());
    }

    inline char get_marker() const
    {
      return marker;
    }
  protected:
    char marker;
  };

  class named_marker_type : public marker_type
  {
  public:
    named_marker_type(char marker, const char* name) : marker_type(marker), name(name)
    { }

    const char* get_name() const
    {
      return name.c_str();
    }
  protected:
    stack_string_128 name;
  };
}