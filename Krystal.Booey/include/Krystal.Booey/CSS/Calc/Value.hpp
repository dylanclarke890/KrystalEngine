#pragma once

#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::style::calc
{
  class Value;
}

namespace krys::boo::css
{
  class AllowedSymbols;
  class SymbolTable;
  class TokenRange;
  class ToLengthConversionData;

  struct ComputedStyleDependencies;
  struct NoConversionDataRequiredTag;
  struct PropertyParserOptions;
  struct PropertyParserState;
  struct SerialisationContext;

  enum class Category : uint8;
  enum class UnitType : uint8;
}

namespace krys::boo::css::calc
{
  /// @brief Boxes a `calc::Tree` along with the `css::Category` and `css::Range` used to construct it.
  class Value final : public RefCounted<Value>
  {
  private:
    css::Category _category;
    css::Range _range;
    calc::Tree _tree;

  public:
    KRYS_NODISCARD static RefPtr<Value> Parse(TokenRange &, PropertyParserState &, css::Category, css::Range,
                                              AllowedSymbols, PropertyParserOptions) noexcept;

    KRYS_NODISCARD static Ref<Value> Create(const style::calc::Value &, const RenderStyle &) noexcept;
    KRYS_NODISCARD static Ref<Value> Create(css::Category, css::Range, calc::Tree &&) noexcept;

    ~Value() noexcept;

    // Creates a copy of the calc::Tree with non-canonical dimensions and any symbols present in the provided
    // symbol table resolved.
    KRYS_NODISCARD Ref<Value> CopySimplified(const ToLengthConversionData &) const noexcept;
    KRYS_NODISCARD Ref<Value> CopySimplified(const ToLengthConversionData &,
                                             const SymbolTable &) const noexcept;
    KRYS_NODISCARD Ref<Value> CopySimplified(NoConversionDataRequiredTag) const noexcept;
    KRYS_NODISCARD Ref<Value> CopySimplified(NoConversionDataRequiredTag, const SymbolTable &) const noexcept;

    KRYS_NODISCARD css::Category GetCategory() const noexcept
    {
      return _category;
    }

    KRYS_NODISCARD css::Range GetRange() const noexcept
    {
      return _range;
    }

    KRYS_NODISCARD UnitType PrimitiveType() const noexcept;

    // Returns whether the calc::Tree requires `ToLengthConversionData` to fully resolve.
    KRYS_NODISCARD bool RequiresConversionData() const noexcept
    {
      return _tree.RequiresConversionData;
    }

    KRYS_NODISCARD double DoubleValue(const ToLengthConversionData &) const noexcept;
    KRYS_NODISCARD double DoubleValue(const ToLengthConversionData &, const SymbolTable &) const noexcept;
    KRYS_NODISCARD double DoubleValue(NoConversionDataRequiredTag) const noexcept;
    KRYS_NODISCARD double DoubleValue(NoConversionDataRequiredTag, const SymbolTable &) const noexcept;
    KRYS_NODISCARD double DoubleValueDeprecated() const noexcept;

    KRYS_NODISCARD double ComputeLengthPx(const ToLengthConversionData &) const noexcept;
    KRYS_NODISCARD double ComputeLengthPx(const ToLengthConversionData &, const SymbolTable &) const noexcept;

    KRYS_NODISCARD Ref<style::calc::Value> CreateCalculationValue(NoConversionDataRequiredTag) const noexcept;
    KRYS_NODISCARD Ref<style::calc::Value> CreateCalculationValue(NoConversionDataRequiredTag,
                                                                  const SymbolTable &) const noexcept;
    KRYS_NODISCARD Ref<style::calc::Value>
      CreateCalculationValue(const ToLengthConversionData &) const noexcept;
    KRYS_NODISCARD Ref<style::calc::Value> CreateCalculationValue(const ToLengthConversionData &,
                                                                  const SymbolTable &) const noexcept;

    void CollectComputedStyleDependencies(ComputedStyleDependencies &) const noexcept;

    KRYS_NODISCARD CSSOMString CssText(const SerialisationContext &) const noexcept;
    KRYS_NODISCARD bool Equals(const Value &) const noexcept;

    KRYS_NODISCARD const calc::Tree &GetTree() const noexcept
    {
      return _tree;
    }

  private:
    explicit Value(css::Category, css::Range, calc::Tree &&) noexcept;

    KRYS_NODISCARD double ClampToPermittedRange(double) const noexcept;
  };
}
