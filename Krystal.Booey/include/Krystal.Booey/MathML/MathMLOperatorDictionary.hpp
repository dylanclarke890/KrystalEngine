#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::mathml
{
  /// @brief The operator dictionary: the default form, spacing and properties of each operator.
  /// @see https://w3c.github.io/mathml-core/#operator-dictionary
  struct MathMLOperatorDictionary
  {
    enum class Form : uint8
    {
      Infix,
      Prefix,
      Postfix,
    };

    enum Flag : uint16
    {
      Accent = 0x1,
      // This has no visual effect but allows to expose semantic information via the accessibility tree.
      Fence = 0x2,
      LargeOp = 0x4,
      MovableLimits = 0x8,
      // This has no visual effect but allows to expose semantic information via the accessibility tree.
      Separator = 0x10,
      Stretchy = 0x20,
      Symmetric = 0x40,
    };

    static constexpr uint16 AllFlags =
      Accent | Fence | LargeOp | MovableLimits | Separator | Stretchy | Symmetric;

    struct Property
    {
      mathml::MathMLOperatorDictionary::Form Form {mathml::MathMLOperatorDictionary::Form::Infix};
      // Default leading and trailing spaces are "thickmathspace".
      uint16 LeadingSpaceInMathUnit {5};
      uint16 TrailingSpaceInMathUnit {5};
      // Default operator properties are all set to "false".
      uint16 Flags {0};

      KRYS_NODISCARD bool operator==(const Property &) const noexcept = default;
    };

    /// @brief The entry for @p character in the form @p form. If there's none and the form wasn't explicit,
    /// the entry for its first form of infix, prefix and postfix.
    KRYS_NODISCARD static Maybe<Property> Search(char32 character, Form form, bool explicitForm) noexcept;

    /// @brief Whether the operator @p character stretches vertically, rather than horizontally.
    KRYS_NODISCARD static bool IsVertical(char32 character) noexcept;
  };
}
