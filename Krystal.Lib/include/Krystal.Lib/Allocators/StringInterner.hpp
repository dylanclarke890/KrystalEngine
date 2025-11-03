#pragma once
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include <cassert>
#include <compare>
#include <cstdint>
#include <cstring>
#include <memory>

namespace Krys
{
  struct StringRef
  {
    size_t Index {0u};

    constexpr auto operator<=>(const StringRef &other) const noexcept = default;
  };

  class StringInterner
  {
    NO_COPY_MOVE(StringInterner)

  private:
    List<string> _strings;
    Map<stringview, size_t> _map;

  public:
    StringInterner() = default;

    ~StringInterner() = default;

    NO_DISCARD StringRef Intern(const char *str)
    {
      auto it = _map.find(str);
      if (it != _map.end())
        return {it->second};
      _strings.emplace_back(str);
      size_t id = _strings.size() - 1;
      _map.emplace(_strings.back(), id);
      return {id};
    }

    NO_DISCARD const string &Get(StringRef ref) const
    {
      assert(ref.Index < _strings.size());
      return _strings[ref.Index];
    }
  };
}
