#include "Krystal.Booey/Editing/HTMLInterchange.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"

namespace krys::boo
{
  // Offsets into the text are UTF-8 bytes; collapsible whitespace is ASCII, so each is one byte.
  utf8_string ConvertHTMLTextToInterchangeFormat(utf8_stringview in, const dom::Text *node) noexcept
  {
    // Assume all the text comes from node.
    if (node->Renderer() && node->Renderer()->Style().PreserveNewline())
    {
      return utf8_string(in);
    }

    const utf8_string convertedSpaceString =
      utf8_string(u8"<span class=\"") + utf8_string(AppleConvertedSpace) + u8"\"> </span>";

    utf8_string s;
    size_t i = 0;
    size_t consumed = 0;
    while (i < in.size())
    {
      consumed = 1;
      if (DeprecatedIsCollapsibleWhitespace(in[i]))
      {
        // count number of adjoining spaces
        size_t j = i + 1;
        while (j < in.size() && DeprecatedIsCollapsibleWhitespace(in[j]))
        {
          j++;
        }
        size_t count = j - i;
        consumed = count;
        while (count)
        {
          size_t add = count % 3;
          switch (add)
          {
            case 0:
              s += convertedSpaceString;
              s += u8' ';
              s += convertedSpaceString;
              add = 3;
              break;
            case 1:
              if (i == 0 || i + 1 == in.size()) // at start or end of string
              {
                s += convertedSpaceString;
              }
              else
              {
                s += u8' ';
              }
              break;
            case 2:
              if (i == 0)
              {
                // at start of string
                s += convertedSpaceString;
                s += u8' ';
              }
              else if (i + 2 == in.size())
              {
                // at end of string
                s += convertedSpaceString;
                s += convertedSpaceString;
              }
              else
              {
                s += convertedSpaceString;
                s += u8' ';
              }
              break;
          }
          count -= add;
        }
      }
      else
      {
        s += in[i];
      }
      i += consumed;
    }
    return s;
  }
}
