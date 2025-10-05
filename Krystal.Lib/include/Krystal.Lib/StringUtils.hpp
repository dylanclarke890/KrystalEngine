#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/String.hpp"

namespace Krys
{
  struct StringUtils
  {
    STATIC_CLASS(StringUtils)

    NO_DISCARD static bool StartsWith(const string &fullString, const string &starting)
    {
      if (fullString.length() >= starting.length())
      {
        return fullString.compare(0, starting.length(), starting) == 0;
      }

      return false;
    }

    NO_DISCARD static bool EndsWith(const string &fullString, const string &ending)
    {
      if (fullString.length() >= ending.length())
      {
        return fullString.compare(fullString.length() - ending.length(), ending.length(), ending) == 0;
      }

      return false;
    }

    NO_DISCARD static List<string> Split(const string &text, const string &delimiter) noexcept
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

    NO_DISCARD static string Replace(string value, const string &from, const string &to) noexcept
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