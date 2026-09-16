#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::css
{
  class TokenRange;

  struct CustomPropertySyntax
  {
    enum class Type : uint8
    {
      Unknown,
      Length,
      LengthPercentage,
      Percentage,
      Integer,
      Number,
      Angle,
      Time,
      Resolution,
      Color,
      Image,
      URL,
      CustomIdent,
      String,
      TransformFunction,
      TransformList,
    };

    enum class Multiplier : uint8
    {
      Single,
      SpaceList,
      CommaList
    };

    struct Component
    {
      Type type;
      Multiplier multiplier {Multiplier::Single};
      CSSOMStringAtom ident {CSSOMStringAtom::Null()};
    };

    using CustomDefinition = SmallList<Component>;

    CustomDefinition Definition;

    KRYS_NODISCARD CustomDefinition Universal() const noexcept
    {
      return {};
    }

    KRYS_NODISCARD bool IsUniversal() const noexcept
    {
      return Definition.empty();
    }

    KRYS_NODISCARD Maybe<CustomPropertySyntax> Parse(CSSOMStringView value) noexcept;

    KRYS_NODISCARD Maybe<CustomPropertySyntax> ConsumeType(TokenRange &tokens) noexcept;

    KRYS_NODISCARD bool ContainsUnknownType() const noexcept;

  private:
    template <typename CharacterType>
    KRYS_NODISCARD static Maybe<Component> ParseComponent(Span<const CharacterType> component) noexcept;

    KRYS_NODISCARD static Type TypeForTypeName(CSSOMStringView typeName) noexcept;
  };
}