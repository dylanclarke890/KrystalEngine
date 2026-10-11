#pragma once

#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Values/NoConversionDataRequiredTag.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style
{
  class RenderStyle;
}

namespace krys::boo::style::calc
{
  class Value;
}

namespace krys::boo::css
{
  class TokenRange;
  class ToLengthConversionData;

  struct ComputedStyleDependencies;
  struct ParserContext;
  struct PropertyParserOptions;
  struct PropertyParserState;
  struct Range;

  enum class Category : uint8;
  enum class UnitType : uint8;
  enum class ValueId : uint16;
}

namespace krys::boo::css::calc
{
  class AllowedSymbols;
  class SymbolTable;

  /// @brief Boxes a `calc::Tree` along with the `css::Category` and `css::Range` used to construct it.
  class Value final : public RefCounted<Value>
  {
  private:
    css::Category _category;
    css::Range _range;
    calc::Tree _tree;

    explicit Value(css::Category, css::Range, calc::Tree &&tree);

  public:
    ~Value() noexcept;

    KRYS_NODISCARD static RefPtr<Value> Parse(TokenRange &tokens, PropertyParserState &state,
                                              Category category, Range range, AllowedSymbols,
                                              PropertyParserOptions) noexcept;

    KRYS_NODISCARD static Ref<Value> Create(const style::calc::Value &, const style::RenderStyle &) noexcept;
    KRYS_NODISCARD static Ref<Value> Create(Category category, Range range, calc::Tree &&tree) noexcept;

    /// @brief Creates a copy of the calc::Tree with non-canonical dimensions and any symbols present in the
    /// provided symbol table resolved.
    KRYS_NODISCARD Ref<Value> CopySimplified(const ToLengthConversionData &) const noexcept;

    KRYS_NODISCARD Ref<Value> CopySimplified(const ToLengthConversionData &,
                                             const SymbolTable &symbols) const noexcept;

    KRYS_NODISCARD Ref<Value> CopySimplified(NoConversionDataRequiredTag) const noexcept;

    KRYS_NODISCARD Ref<Value> CopySimplified(NoConversionDataRequiredTag,
                                             const SymbolTable &symbols) const noexcept;

    KRYS_NODISCARD css::Category Category() const noexcept
    {
      return _category;
    }

    KRYS_NODISCARD css::Range Range() const noexcept
    {
      return _range;
    }

    KRYS_NODISCARD const calc::Tree &Tree() const noexcept
    {
      return _tree;
    }

    KRYS_NODISCARD UnitType PrimitiveType() const noexcept;

    /// @brief Checks whether calc::Tree requires `ToLengthConversionData` to fully resolve.
    KRYS_NODISCARD bool RequiresConversionData() const
    {
      return _tree.RequiresConversionData;
    };

    KRYS_NODISCARD double DoubleValue(const ToLengthConversionData &) const noexcept;

    KRYS_NODISCARD double DoubleValue(const ToLengthConversionData &,
                                      const SymbolTable &symbols) const noexcept;

    KRYS_NODISCARD double DoubleValue(NoConversionDataRequiredTag) const noexcept;

    KRYS_NODISCARD double DoubleValue(NoConversionDataRequiredTag, const SymbolTable &symbols) const noexcept;

    KRYS_NODISCARD double DoubleValueDeprecated() const noexcept;

    KRYS_NODISCARD double ComputeLengthPx(const ToLengthConversionData &) const noexcept;

    KRYS_NODISCARD double ComputeLengthPx(const ToLengthConversionData &,
                                          const SymbolTable &symbols) const noexcept;

    KRYS_NODISCARD Ref<style::calc::Value> CreateCalculationValue(NoConversionDataRequiredTag) const noexcept;

    KRYS_NODISCARD Ref<style::calc::Value> CreateCalculationValue(NoConversionDataRequiredTag,
                                                                  const SymbolTable &symbols) const noexcept;

    KRYS_NODISCARD Ref<style::calc::Value> CreateCalculationValue(const ToLengthConversionData &) const;

    KRYS_NODISCARD Ref<style::calc::Value> CreateCalculationValue(const ToLengthConversionData &,
                                                                  const SymbolTable &symbols) const;

    KRYS_NODISCARD bool Equals(const Value &value) const noexcept;

    CSSOMString CssText(const SerialisationContext &context) const noexcept;

    void CollectComputedStyleDependencies(ComputedStyleDependencies &dependencies) const noexcept;

  private:
    KRYS_NODISCARD double ClampToPermittedRange(double value) const noexcept;
  };
}
