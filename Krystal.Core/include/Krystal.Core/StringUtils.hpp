#pragma once

#include "Krystal.Core/Core.hpp"

namespace Krys
{
  struct StringUtils
  {
    STATIC_CLASS(StringUtils)

    static List<string> Split(const string &text, const string &delimiter)
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
  };
}