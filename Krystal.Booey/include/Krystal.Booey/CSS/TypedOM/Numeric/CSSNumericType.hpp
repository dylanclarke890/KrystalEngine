#pragma once

#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSNumericBaseType.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::css
{
  class CSSNumericValue;
  enum class UnitType : uint8;

  /// @brief A numeric value's type: an exponent for each base type it has, and the base type a percentage
  /// resolves against.
  /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-type
  class CSSNumericType
  {
  public:
    using BaseTypeStorage = Maybe<int32>;
    BaseTypeStorage Length;
    BaseTypeStorage Angle;
    BaseTypeStorage Time;
    BaseTypeStorage Frequency;
    BaseTypeStorage Resolution;
    BaseTypeStorage Flex;
    BaseTypeStorage Percent;
    Maybe<CSSNumericBaseType> PercentHint;

    /// @see https://drafts.css-houdini.org/css-typed-om/#cssnumericvalue-create-a-type
    KRYS_NODISCARD static Maybe<CSSNumericType> Create(UnitType unit, int32 exponent = 1) noexcept;
    KRYS_NODISCARD friend bool operator==(const CSSNumericType &, const CSSNumericType &) noexcept = default;
    KRYS_NODISCARD static Maybe<CSSNumericType> AddTypes(const List<Ref<CSSNumericValue>> &values) noexcept;
    /// @see https://drafts.css-houdini.org/css-typed-om/#cssnumericvalue-add-two-types
    KRYS_NODISCARD static Maybe<CSSNumericType> AddTypes(CSSNumericType a, CSSNumericType b) noexcept;
    KRYS_NODISCARD static Maybe<CSSNumericType>
      MultiplyTypes(const List<Ref<CSSNumericValue>> &values) noexcept;
    /// @see https://drafts.css-houdini.org/css-typed-om/#cssnumericvalue-multiply-two-types
    KRYS_NODISCARD static Maybe<CSSNumericType> MultiplyTypes(const CSSNumericType &a,
                                                              const CSSNumericType &b) noexcept;
    KRYS_NODISCARD BaseTypeStorage &ValueForType(CSSNumericBaseType type) noexcept;
    KRYS_NODISCARD const BaseTypeStorage &ValueForType(CSSNumericBaseType type) const noexcept
    {
      return const_cast<CSSNumericType *>(this)->ValueForType(type);
    }
    /// @see https://drafts.css-houdini.org/css-typed-om/#apply-the-percent-hint
    void ApplyPercentHint(CSSNumericBaseType hint) noexcept;
    KRYS_NODISCARD size_t NonZeroEntryCount() const noexcept;

    template <CSSNumericBaseType Type>
    KRYS_NODISCARD bool Matches() const noexcept
    {
      // https://drafts.css-houdini.org/css-typed-om/#cssnumericvalue-match
      return (Type == CSSNumericBaseType::Percent || !PercentHint) && NonZeroEntryCount() == 1
             && ValueForType(Type) && *ValueForType(Type);
    }

    KRYS_NODISCARD bool MatchesNumber() const noexcept
    {
      // https://drafts.css-houdini.org/css-typed-om/#cssnumericvalue-match
      return !NonZeroEntryCount() && !PercentHint;
    }

    template <CSSNumericBaseType Type>
    KRYS_NODISCARD bool MatchesTypeOrPercentage() const noexcept
    {
      return Matches<Type>() || Matches<CSSNumericBaseType::Percent>();
    }

    KRYS_NODISCARD utf8_string DebugString() const noexcept;
  };
}
