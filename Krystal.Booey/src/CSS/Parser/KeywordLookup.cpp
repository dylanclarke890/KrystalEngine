#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include <cstring>

namespace krys::boo::css
{
  namespace
  {
    /// @brief Lowercases `name` into `buffer`.
    /// @return False if `name` contains a NUL or non-ASCII character.
    template <size_t N>
    KRYS_NODISCARD bool ToASCIILowerInto(CSSOMStringView name, Array<char8, N> &buffer) noexcept
    {
      krys_debug_assert(name.size() <= N);

      for (size_t i = 0uz; i != name.size(); ++i)
      {
        auto character = name[i];
        if (!character || !krys::text::IsASCII(character))
        {
          return false;
        }
        buffer[i] = krys::text::ToASCIILower(character);
      }

      return true;
    }

    // FIXME: Remove this mechanism entirely once we can do it without breaking the web.
    KRYS_NODISCARD bool IsAppleLegacyValueKeyword(CSSOMStringView characters) noexcept
    {
      return characters.substr(1).starts_with(u8"apple-") && !characters.substr(7).starts_with(u8"system")
          && !characters.substr(7).starts_with(u8"pay") && !characters.substr(7).starts_with(u8"wireless");
    }
  }

  css::PropertyId PropertyIdFromName(CSSOMStringView name) noexcept
  {
    if (name.empty() || name.size() > MaxPropertyNameLength)
    {
      return css::PropertyId::Invalid;
    }

    Array<char8, MaxPropertyNameLength> buffer;
    if (!ToASCIILowerInto(name, buffer))
    {
      return css::PropertyId::Invalid;
    }

    return FindProperty(CSSOMStringView(buffer.data(), name.size()));
  }

  css::ValueId ValueIdFromName(CSSOMStringView name) noexcept
  {
    if (name.empty() || name.size() > MaxValueKeywordLength)
    {
      return css::ValueId::Invalid;
    }

    // One extra character to turn "apple" into "webkit".
    Array<char8, MaxValueKeywordLength + 1uz> buffer;
    if (!ToASCIILowerInto(name, buffer))
    {
      return css::ValueId::Invalid;
    }

    // In most cases, if the prefix is -apple-, change it to -webkit-. This makes the string one character
    // longer.
    auto length = name.size();
    if (buffer[0] == u8'-' && IsAppleLegacyValueKeyword(CSSOMStringView(buffer.data(), length)))
    {
      std::memmove(buffer.data() + 7, buffer.data() + 6, length - 6);
      std::memcpy(buffer.data() + 1, u8"webkit", 6);
      ++length;
    }

    return FindValueKeyword(CSSOMStringView(buffer.data(), length));
  }
}
