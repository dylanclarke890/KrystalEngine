#pragma once

#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Strings/StringAtom.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Css/Properties/PropertyId.hpp"
#include "Krystal.Web/Css/Types/CSSOMString.hpp"
#include "Krystal.Web/Css/Values/Primitives/Units.hpp"
#include "Krystal.Web/Css/Values/Units.hpp"
#include "Krystal.Web/Css/Values/Value.hpp"
#include "Krystal.Web/Css/Values/ValueId.hpp"

namespace krys::web::css::calc
{
  class Value;
}

namespace krys::web::css
{
  class ToLengthConversionData;

  /// @brief A value with no parts: a keyword, a property name, a number with its unit, a string, a custom
  /// identifier, a font family name or a `calc()`.
  /// @details Keywords, and the whole numbers 0 to 255 in `px`, `%` or no unit, come from the static pool.
  /// The quantity checks (`IsLength`, `IsAngle`, ...) and the `ResolveAs` functions see a `calc()` as the
  /// quantity it resolves to; `IsPx` and the font checks look at the units written.
  /// @see https://www.w3.org/TR/css-values-4/
  class PrimitiveValue final : public Value
  {
    friend class StaticValuePool;
    friend class Value;

    using PrimitiveKind = detail::PrimitiveKind;

  private:
    // Chosen by `_header.PrimitiveKind`; a number's unit is `_header.Unit`. `String` and `Calc` each hold a
    // reference, released by the destructor.
    union Payload
    {
      css::PropertyId PropertyId;
      css::ValueId ValueId;
      double Number;
      StringAtomStorage String;
      const calc::Value *Calc;
    } _payload;

  public:
    ~PrimitiveValue() noexcept;

    // Creation.

    /// @brief A number in @p unit.
    KRYS_NODISCARD static Ref<PrimitiveValue> Create(double value, UnitType unit = UnitType::Number) noexcept;

    /// @brief An `<integer>`.
    KRYS_NODISCARD static Ref<PrimitiveValue> CreateInteger(double value) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> Create(Ref<calc::Value> value) noexcept;

    /// @brief A keyword, from the static pool.
    /// @note Defined in ValuePool.hpp.
    KRYS_NODISCARD static inline Ref<PrimitiveValue> Create(css::ValueId keyword) noexcept;

    /// @brief A property's name, as `transition-property` and `will-change` take.
    KRYS_NODISCARD static Ref<PrimitiveValue> Create(css::PropertyId property) noexcept;

    /// @brief A `<string>`.
    KRYS_NODISCARD static Ref<PrimitiveValue> Create(const CSSOMString &value) noexcept;

    /// @brief A `<string>`.
    KRYS_NODISCARD static Ref<PrimitiveValue> Create(CSSOMStringAtom value) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> CreateCustomIdent(const CSSOMString &value) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> CreateCustomIdent(CSSOMStringAtom value) noexcept;

    /// @brief A font family's name, serialised as identifiers where it can be and as a string otherwise.
    KRYS_NODISCARD static Ref<PrimitiveValue> CreateFontFamily(const CSSOMString &value) noexcept;

    /// @brief The `initial` that a declaration block gives a longhand its shorthand left out.
    /// @note Defined in ValuePool.hpp.
    KRYS_NODISCARD static inline PrimitiveValue &ImplicitInitialValue() noexcept;

    // What it holds.

    KRYS_NODISCARD bool IsValueId() const noexcept
    {
      return _header.PrimitiveKind == PrimitiveKind::ValueId;
    }

    /// @brief The keyword, or `ValueId::Invalid` if this isn't one.
    KRYS_NODISCARD css::ValueId ValueId() const noexcept
    {
      return IsValueId() ? _payload.ValueId : css::ValueId::Invalid;
    }

    KRYS_NODISCARD bool IsPropertyId() const noexcept
    {
      return _header.PrimitiveKind == PrimitiveKind::PropertyId;
    }

    /// @brief The property, or `PropertyId::Invalid` if this isn't one.
    KRYS_NODISCARD css::PropertyId PropertyId() const noexcept
    {
      return IsPropertyId() ? _payload.PropertyId : css::PropertyId::Invalid;
    }

    KRYS_NODISCARD bool IsString() const noexcept
    {
      return _header.PrimitiveKind == PrimitiveKind::String;
    }

    KRYS_NODISCARD bool IsCustomIdent() const noexcept
    {
      return _header.PrimitiveKind == PrimitiveKind::CustomIdent;
    }

    KRYS_NODISCARD bool IsFontFamily() const noexcept
    {
      return _header.PrimitiveKind == PrimitiveKind::FontFamily;
    }

    /// @brief The text of a string, custom identifier or font family, or the name of a keyword or property;
    /// empty for a number or `calc()`.
    KRYS_NODISCARD CSSOMString StringValue() const noexcept;

    KRYS_NODISCARD bool IsCalc() const noexcept
    {
      return _header.PrimitiveKind == PrimitiveKind::Calc;
    }

    /// @brief The `calc()`, or null if this isn't one.
    KRYS_NODISCARD const calc::Value *CalcValue() const noexcept
    {
      return IsCalc() ? _payload.Calc : nullptr;
    }

    // Its unit.

    /// @brief The unit of a number, or the canonical unit of what a `calc()` resolves to (`px` for a
    /// length); `UnitType::Unknown` for the other kinds, and for a `calc()` mixing a percentage with another
    /// quantity.
    KRYS_NODISCARD UnitType Unit() const noexcept;

    KRYS_NODISCARD bool IsNumber() const noexcept
    {
      return Unit() == UnitType::Number;
    }

    KRYS_NODISCARD bool IsInteger() const noexcept
    {
      return Unit() == UnitType::Integer;
    }

    KRYS_NODISCARD bool IsNumberOrInteger() const noexcept
    {
      return IsNumber() || IsInteger();
    }

    KRYS_NODISCARD bool IsPercentage() const noexcept
    {
      return Unit() == UnitType::Percentage;
    }

    KRYS_NODISCARD bool IsLength() const noexcept
    {
      return css::IsLength(Unit());
    }

    KRYS_NODISCARD bool IsAngle() const noexcept
    {
      return CategoryForUnitType(Unit()) == Category::Angle;
    }

    KRYS_NODISCARD bool IsTime() const noexcept
    {
      return CategoryForUnitType(Unit()) == Category::Time;
    }

    KRYS_NODISCARD bool IsResolution() const noexcept
    {
      return CategoryForUnitType(Unit()) == Category::Resolution;
    }

    KRYS_NODISCARD bool IsFlex() const noexcept
    {
      return Unit() == UnitType::fr;
    }

    /// @brief Whether this is a `calc()` adding a percentage to a length, such as `calc(50% - 1em)`.
    KRYS_NODISCARD bool IsCalcPercentageWithLength() const noexcept;

    /// @brief Whether this is a number written in `px`; never a `calc()`.
    KRYS_NODISCARD bool IsPx() const noexcept
    {
      return StoredUnit() == UnitType::px;
    }

    /// @brief Whether this is an absolute length, which nothing but zoom affects; a `calc()` is if all its
    /// lengths are.
    KRYS_NODISCARD bool IsFontIndependentLength() const noexcept;

    /// @brief Whether this is a length relative to a font, the root's included; a `calc()` is if any of its
    /// lengths is.
    KRYS_NODISCARD bool IsFontRelativeLength() const noexcept;

    /// @brief Whether this depends on the parent's font size: a percentage, or a length relative to the
    /// element's own font; a `calc()` is if any of its parts is.
    KRYS_NODISCARD bool IsParentFontRelativeLength() const noexcept;

    /// @brief Whether this is a length relative to the root's font; a `calc()` is if any of its lengths is.
    KRYS_NODISCARD bool IsRootFontRelativeLength() const noexcept;

    // Its sign, as written. A calc() isn't range-checked when it's parsed, and its sign isn't known until it
    // is resolved, so these are null for one.
    // https://www.w3.org/TR/css-values-4/#calc-range

    KRYS_NODISCARD Maybe<bool> IsZero() const noexcept
    {
      return IsCalc() ? Maybe<bool> {} : Maybe<bool> {_payload.Number == 0.0};
    }

    KRYS_NODISCARD Maybe<bool> IsOne() const noexcept
    {
      return IsCalc() ? Maybe<bool> {} : Maybe<bool> {_payload.Number == 1.0};
    }

    KRYS_NODISCARD Maybe<bool> IsPositive() const noexcept
    {
      return IsCalc() ? Maybe<bool> {} : Maybe<bool> {_payload.Number > 0.0};
    }

    KRYS_NODISCARD Maybe<bool> IsNegative() const noexcept
    {
      return IsCalc() ? Maybe<bool> {} : Maybe<bool> {_payload.Number < 0.0};
    }

    // Resolving. Each `ResolveAs` function needs the matching quantity check to hold, and converts to that
    // quantity's canonical unit, or the one given. Results are clamped to @p T's range.

    /// @pre `IsInteger()`.
    template <typename T = int64>
    KRYS_NODISCARD T ResolveAsInteger(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsInteger());
      return Resolve<T>(conversionData);
    }

    /// @brief The integer as written, or null for a `calc()`.
    /// @pre `IsInteger()`.
    template <typename T = int64>
    KRYS_NODISCARD Maybe<T> ResolveAsIntegerIfNotCalc() const noexcept
    {
      krys_debug_assert(IsInteger());
      return ValueIfNotCalc<T>();
    }

    /// @pre `IsNumberOrInteger()`.
    template <typename T = double>
    KRYS_NODISCARD T ResolveAsNumber(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsNumberOrInteger());
      return Resolve<T>(UnitType::Number, conversionData);
    }

    /// @brief The number as written, or null for a `calc()`.
    /// @pre `IsNumberOrInteger()`.
    template <typename T = double>
    KRYS_NODISCARD Maybe<T> ResolveAsNumberIfNotCalc() const noexcept
    {
      krys_debug_assert(IsNumberOrInteger());
      return ValueIfNotCalc<T>();
    }

    /// @brief The percentage, as 50 for `50%`.
    /// @pre `IsPercentage()`.
    template <typename T = double>
    KRYS_NODISCARD T ResolveAsPercentage(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsPercentage());
      return Resolve<T>(conversionData);
    }

    /// @pre `IsPercentage()`, and this isn't a `calc()`.
    template <typename T = double>
    KRYS_NODISCARD T ResolveAsPercentageWithoutConversionData() const noexcept
    {
      krys_debug_assert(IsPercentage());
      return ResolveWithoutConversionData<T>();
    }

    /// @pre `IsAngle()`.
    template <typename T = double, AngleUnit TUnit = UnitTraits<AngleUnit>::Canonical>
    KRYS_NODISCARD T ResolveAsAngle(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsAngle());
      return ClampTo<T>(ConvertUnit(ResolveDouble(conversionData), Unit(), ToUnitType(TUnit)).value_or(0));
    }

    /// @pre `IsTime()`.
    template <typename T = double, TimeUnit TUnit = UnitTraits<TimeUnit>::Canonical>
    KRYS_NODISCARD T ResolveAsTime(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsTime());
      return ClampTo<T>(ConvertUnit(ResolveDouble(conversionData), Unit(), ToUnitType(TUnit)).value_or(0));
    }

    /// @pre `IsResolution()`.
    template <typename T = double, ResolutionUnit TUnit = UnitTraits<ResolutionUnit>::Canonical>
    KRYS_NODISCARD T ResolveAsResolution(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsResolution());
      return ClampTo<T>(ConvertUnit(ResolveDouble(conversionData), Unit(), ToUnitType(TUnit)).value_or(0));
    }

    /// @pre `IsFlex()`.
    template <typename T = double>
    KRYS_NODISCARD T ResolveAsFlex(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsFlex());
      return Resolve<T>(conversionData);
    }

    /// @brief The length in CSS pixels. Integer types take it through `style::RoundForImpreciseConversion`.
    /// @pre `IsLength()`.
    /// @note Instantiated in PrimitiveValue.cpp for the 16-, 32- and 64-bit integers, `float`, `double` and
    /// `LayoutUnit`.
    template <typename T = double>
    KRYS_NODISCARD T ResolveAsLength(const ToLengthConversionData &conversionData) const noexcept;

    /// @pre `IsLength()` with an absolute unit, and this isn't a `calc()`.
    template <typename T = double>
    KRYS_NODISCARD T ResolveAsLengthWithoutConversionData() const noexcept
    {
      krys_debug_assert(IsLength());
      return ResolveWithoutConversionData<T>(UnitType::px);
    }

    /// @brief The number in its own unit, unconverted.
    template <typename T = double>
    KRYS_NODISCARD T Resolve(const ToLengthConversionData &conversionData) const noexcept
    {
      return ClampTo<T>(ResolveDouble(conversionData));
    }

    /// @brief The number in its own unit, unconverted.
    /// @pre This isn't a `calc()`.
    template <typename T = double>
    KRYS_NODISCARD T ResolveWithoutConversionData() const noexcept
    {
      return ClampTo<T>(ResolveDoubleWithoutConversionData());
    }

    /// @brief A number, or a percentage as a fraction (0.5 for `50%`).
    /// @pre `IsNumberOrInteger()` or `IsPercentage()`.
    template <typename T = double>
    KRYS_NODISCARD
    T ResolveDividingBy100IfPercentage(const ToLengthConversionData &conversionData) const noexcept
    {
      return ClampTo<T>(ResolveDoubleDividingBy100IfPercentage(conversionData));
    }

    // The Value operations.

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const PrimitiveValue &other) const noexcept;

    void CollectComputedStyleDependencies(css::ComputedStyleDependencies &dependencies) const noexcept;

  private:
    /// @brief Selects the constructor of `ImplicitInitialValue()`.
    struct CreateImplicitInitialValueTag
    {
    };

    constexpr static CreateImplicitInitialValueTag CreateImplicitInitialValue {};

    PrimitiveValue(double value, UnitType unit) noexcept;

    explicit PrimitiveValue(css::PropertyId property) noexcept;

    /// @pre @p kind is `String`, `CustomIdent` or `FontFamily`.
    PrimitiveValue(CSSOMStringAtom value, PrimitiveKind kind) noexcept;

    explicit PrimitiveValue(Ref<calc::Value> value) noexcept;

    PrimitiveValue(StaticValueTag, css::ValueId keyword) noexcept;

    PrimitiveValue(StaticValueTag, double value, UnitType unit) noexcept;

    PrimitiveValue(StaticValueTag, CreateImplicitInitialValueTag) noexcept;

    /// @brief The unit written: a number's, and `Unknown` for every other kind, `calc()` included.
    KRYS_NODISCARD UnitType StoredUnit() const noexcept
    {
      return _header.Unit;
    }

    KRYS_NODISCARD double ResolveDouble(const ToLengthConversionData &conversionData) const noexcept;

    /// @brief The number converted to @p targetUnit, or 0 if it can't be.
    KRYS_NODISCARD double ResolveDouble(UnitType targetUnit,
                                        const ToLengthConversionData &conversionData) const noexcept;

    KRYS_NODISCARD double ResolveDoubleWithoutConversionData() const noexcept
    {
      krys_debug_assert(!IsCalc());
      return _payload.Number;
    }

    /// @brief The number converted to @p targetUnit, or 0 if it can't be.
    KRYS_NODISCARD double ResolveDoubleWithoutConversionData(UnitType targetUnit) const noexcept;

    KRYS_NODISCARD double
      ResolveDoubleDividingBy100IfPercentage(const ToLengthConversionData &conversionData) const noexcept;

    KRYS_NODISCARD double ResolveAsLengthDouble(const ToLengthConversionData &conversionData) const noexcept;

    template <typename T = double>
    KRYS_NODISCARD T Resolve(UnitType targetUnit, const ToLengthConversionData &conversionData) const noexcept
    {
      return ClampTo<T>(ResolveDouble(targetUnit, conversionData));
    }

    template <typename T = double>
    KRYS_NODISCARD T ResolveWithoutConversionData(UnitType targetUnit) const noexcept
    {
      return ClampTo<T>(ResolveDoubleWithoutConversionData(targetUnit));
    }

    template <typename T = double>
    KRYS_NODISCARD Maybe<T> ValueIfNotCalc() const noexcept
    {
      return IsCalc() ? Maybe<T> {} : Maybe<T> {static_cast<T>(_payload.Number)};
    }

    /// @brief The text of a string, custom identifier or font family.
    KRYS_NODISCARD utf8_stringview StringView() const noexcept
    {
      return _payload.String->View();
    }

    KRYS_NODISCARD CSSOMString SerialiseNumber() const noexcept;

    KRYS_NODISCARD bool AddDerivedHash(size_t &seed) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::css::PrimitiveValue)
  KRYS_NODISCARD static bool IsType(const krys::web::css::Value &value) noexcept
  {
    return value.Type() == krys::web::css::ValueType::Primitive;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

namespace krys::web::css
{
#pragma region Value inline definitions

  inline bool Value::IsValueId() const noexcept
  {
    auto *value = DynamicDowncast<PrimitiveValue>(*this);
    return value && value->IsValueId();
  }

  inline css::ValueId Value::ValueId() const noexcept
  {
    auto *value = DynamicDowncast<PrimitiveValue>(*this);
    return value ? value->ValueId() : css::ValueId::Invalid;
  }

  inline bool Value::IsCustomIdent() const noexcept
  {
    auto *value = DynamicDowncast<PrimitiveValue>(*this);
    return value && value->IsCustomIdent();
  }

#pragma endregion
}
