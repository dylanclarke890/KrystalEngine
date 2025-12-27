#pragma once
#include "Krystal.Lib/String/StringRef.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types/Map.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <cassert>

namespace Krys
{
  class StringInterner
  {
    NO_COPY_MOVE(StringInterner)

  private:
    List<utf8_string> _strings;
    Map<utf8_stringview, size_t> _map;

  public:
    StringInterner() = default;

    ~StringInterner() = default;

    NO_DISCARD StringRef Intern(const char8_t *str)
    {
      auto it = _map.find(str);
      if (it != _map.end())
      {
        return {it->second};
      }

      _strings.emplace_back(str);
      size_t id = _strings.size();
      _map.emplace(_strings.back(), id);

      return {id};
    }

    NO_DISCARD StringRef Intern(const char *str)
    {
      return Intern(reinterpret_cast<const char8_t *>(str));
    }

    NO_DISCARD const utf8_string &Get(StringRef ref) const
    {
      assert(ref.Id - 1u < _strings.size());
      return _strings[ref.Id - 1u];
    }
  };
}
