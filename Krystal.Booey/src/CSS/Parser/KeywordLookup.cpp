#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/Array.hpp"

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

    Array<char8, MaxValueKeywordLength> buffer;
    if (!ToASCIILowerInto(name, buffer))
    {
      return css::ValueId::Invalid;
    }

    return FindValueKeyword(CSSOMStringView(buffer.data(), name.size()));
  }

  bool IsCustomPropertyName(CSSOMStringView propertyName) noexcept
  {
    return propertyName.size() > 2uz && propertyName[0] == u8'-' && propertyName[1] == u8'-';
  }
}
