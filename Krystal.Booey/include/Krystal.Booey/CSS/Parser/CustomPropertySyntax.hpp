/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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

    /// @brief The syntax string the syntax was parsed from, normalised: `*`, or its components joined by ` |
    /// `.
    /// @see https://drafts.css-houdini.org/css-properties-values-api/#syntax-strings
    KRYS_NODISCARD CSSOMString SyntaxString() const noexcept;

    /// @brief The syntax serialised as a `<css-type>`: a single component's syntax string, otherwise the
    /// syntax string in `type()`.
    /// @see https://drafts.csswg.org/css-mixins/#serialize-a-css-type
    KRYS_NODISCARD CSSOMString CSSTypeString() const noexcept;

  private:
    KRYS_NODISCARD static Maybe<Component> ParseComponent(CSSOMStringView component) noexcept;
    KRYS_NODISCARD static Type TypeForTypeName(CSSOMStringView dataTypeName) noexcept;
  };
}
