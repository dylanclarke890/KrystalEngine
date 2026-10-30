#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class TokenRange;

  /// @brief The syntax of a registered custom property or a custom function parameter.
  /// @see https://drafts.css-houdini.org/css-properties-values-api/#syntax-strings
  struct CustomPropertySyntax
  {
    enum class Type : uint8
    {
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
      Unknown
    };

    enum class Multiplier : uint8
    {
      Single,
      SpaceList,
      CommaList
    };

    struct Component
    {
      CustomPropertySyntax::Type Type;
      CustomPropertySyntax::Multiplier Multiplier {CustomPropertySyntax::Multiplier::Single};
      CSSOMStringAtom Ident {CSSOMStringAtom::Null()};
    };

    using DefinitionType = List<Component>;

    DefinitionType Definition;

    KRYS_NODISCARD bool IsUniversal() const noexcept
    {
      return Definition.empty();
    }

    KRYS_NODISCARD static Maybe<CustomPropertySyntax> Parse(CSSOMStringView syntax) noexcept;
    KRYS_NODISCARD static Maybe<CustomPropertySyntax> ConsumeType(TokenRange &tokens) noexcept;

    KRYS_NODISCARD static CustomPropertySyntax Universal() noexcept
    {
      return {};
    }

    KRYS_NODISCARD bool ContainsUnknownType() const noexcept;

  private:
    KRYS_NODISCARD static Maybe<Component> ParseComponent(CSSOMStringView component) noexcept;
    KRYS_NODISCARD static Type TypeForTypeName(CSSOMStringView dataTypeName) noexcept;
  };
}
