#pragma once

#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Style/Values/Content/Content.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::css
{
  class CounterStyle;
}

namespace krys::boo
{
  class CounterNode;

  /// @brief The text of a `counter()` or `counters()` in generated content.
  class RenderCounter final : public RenderText
  {
    friend class CounterNode;
    KRYS_NON_COPY_MOVABLE(RenderCounter);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderCounter);

  private:
    style::Content::Counter _counter;
    WeakPtr<CounterNode> _counterNode;
    WeakPtr<RenderCounter> _nextForSameCounter;

  public:
    RenderCounter(dom::Document &document, const style::Content::Counter &counter) noexcept;
    ~RenderCounter() noexcept override;

    static void DestroyCounterNodes(RenderElement &owner) noexcept;
    static void DestroyCounterNode(RenderElement &owner, const css::CSSOMStringAtom &identifier) noexcept;
    static void RendererStyleChanged(RenderElement &renderer, const RenderStyle *oldStyle,
                                     const RenderStyle &newStyle) noexcept;

    void UpdateCounter() noexcept;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override;
    KRYS_NODISCARD utf8_string OriginalText() const noexcept override;

  private:
    void WillBeDestroyed() noexcept override;
    static void RendererStyleChangedSlowCase(RenderElement &renderer, const RenderStyle *oldStyle,
                                             const RenderStyle &newStyle) noexcept;

    KRYS_NODISCARD Ref<css::CounterStyle> GetCounterStyle() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderCounter)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderCounter();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
