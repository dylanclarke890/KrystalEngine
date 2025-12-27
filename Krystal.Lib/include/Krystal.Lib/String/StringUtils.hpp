#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/String/String.hpp"

namespace Krys
{
  struct StringUtils
  {
    STATIC_CLASS(StringUtils)

    KRYS_NODISCARD static bool StartsWith(const string &fullString, const string &starting)
    {
      if (fullString.length() >= starting.length())
      {
        return fullString.compare(0, starting.length(), starting) == 0;
      }

      return false;
    }

    KRYS_NODISCARD static bool EndsWith(const string &fullString, const string &ending)
    {
      if (fullString.length() >= ending.length())
      {
        return fullString.compare(fullString.length() - ending.length(), ending.length(), ending) == 0;
      }

      return false;
    }

    KRYS_NODISCARD static List<string> Split(const string &text, const string &delimiter) noexcept
    {
      size_t start = 0;
      size_t end = 0;
      string token;

      List<string> results;
      while ((end = text.find(delimiter, start)) != string::npos)
      {
        token = text.substr(start, end - start);
        start = end + delimiter.length();
        if (token.length())
          results.push_back(token);
      }

      token = text.substr(start);
      if (token.length())
        results.push_back(token);

      return results;
    }

    KRYS_NODISCARD static string Replace(string value, const string &from, const string &to) noexcept
    {
      size_t pos = 0;
      while ((pos = value.find(from, pos)) != std::string::npos)
      {
        value.replace(pos, from.length(), to);
        pos += to.length();
      }
      return value;
    }
  };
}