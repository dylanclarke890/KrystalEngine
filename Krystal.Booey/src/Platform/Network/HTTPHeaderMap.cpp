#include "Krystal.Booey/Platform/Network/HTTPHeaderMap.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>

namespace krys::boo
{
  Maybe<utf8_stringview> HTTPHeaderMap::Get(utf8_stringview name) const noexcept
  {
    auto found = std::ranges::find_if(_headers, [&](const auto &header)
                                      { return text::IsASCIICaselessEqual(header.first, name); });
    if (found == _headers.end())
    {
      return null;
    }
    return utf8_stringview(found->second);
  }

  Pair<utf8_string, utf8_string> *HTTPHeaderMap::Find(utf8_stringview name) noexcept
  {
    auto found = std::ranges::find_if(_headers, [&](const auto &header)
                                      { return text::IsASCIICaselessEqual(header.first, name); });
    return found == _headers.end() ? nullptr : &*found;
  }

  void HTTPHeaderMap::Set(utf8_stringview name, utf8_stringview value) noexcept
  {
    if (auto *header = Find(name))
    {
      header->second = utf8_string(value);
      return;
    }
    _headers.emplace_back(utf8_string(name), utf8_string(value));
  }

  void HTTPHeaderMap::Add(utf8_stringview name, utf8_stringview value) noexcept
  {
    if (auto *header = Find(name))
    {
      header->second += u8", ";
      header->second += value;
      return;
    }
    _headers.emplace_back(utf8_string(name), utf8_string(value));
  }

  void HTTPHeaderMap::Remove(utf8_stringview name) noexcept
  {
    std::erase_if(_headers,
                  [&](const auto &header) { return text::IsASCIICaselessEqual(header.first, name); });
  }
}
