#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Booey/Style/Calc/Tree.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  enum class Category : uint8;
}

namespace krys::boo::style
{
  struct ZoomFactor;
  struct ZoomNeeded;
}

namespace krys::boo::style::calc
{
  /// @brief Boxes a `style::calc::Tree` along with the `css::Category` and `css::Range` used to construct it.
  class Value final : public RefCounted<Value>
  {
  private:
    css::Category _category;
    css::Range _range;
    calc::Tree _tree;

  public:
    KRYS_NODISCARD static Ref<Value> Create(css::Category, css::Range, calc::Tree &&) noexcept;

    ~Value() noexcept;

    KRYS_NODISCARD double Evaluate(double percentResolutionLength, const ZoomFactor &usedZoom) const noexcept;
    KRYS_NODISCARD double Evaluate(double percentResolutionLength, const ZoomNeeded &) const noexcept;

    KRYS_NODISCARD css::Category GetCategory() const noexcept
    {
      return _category;
    }

    KRYS_NODISCARD css::Range GetRange() const noexcept
    {
      return _range;
    }

    KRYS_NODISCARD const calc::Tree &GetTree() const noexcept
    {
      return _tree;
    }

    KRYS_NODISCARD calc::Tree CopyTree() const noexcept;
    KRYS_NODISCARD Child CopyRoot() const noexcept;

    KRYS_NODISCARD bool operator==(const Value &) const noexcept;

  private:
    explicit Value(css::Category, css::Range, calc::Tree &&) noexcept;
  };
}
