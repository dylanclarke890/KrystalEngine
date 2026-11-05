#pragma once

#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderTextFragment.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
{
  class HTMLFormControlElement;
}

namespace krys::boo
{
  // RenderButtons are just like normal flexboxes except that they will generate an anonymous block child.
  // For inputs, they will also generate an anonymous RenderText and keep its style and content up
  // to date as the button changes.
  /// @brief The renderer of `<button>` and the button input types.
  class RenderButton final : public RenderFlexibleBox
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderButton);

  private:
    WeakPtr<RenderTextFragment> _buttonText;
    WeakPtr<RenderBlock> _inner;

  public:
    RenderButton(html::HTMLFormControlElement &, RenderStyle &&) noexcept;
    ~RenderButton() noexcept override;

    KRYS_NODISCARD html::HTMLFormControlElement &FormControlElement() const noexcept;

    KRYS_NODISCARD bool CanBeSelectionLeaf() const noexcept override;

    KRYS_NODISCARD bool CreatesAnonymousWrapper() const noexcept override
    {
      return true;
    }

    void UpdateFromElement() noexcept override;

    KRYS_NODISCARD bool CanHaveGeneratedChildren() const noexcept override;
    KRYS_NODISCARD bool HasControlClip() const noexcept override;
    KRYS_NODISCARD LayoutRect ControlClipRect(const LayoutPoint &) const noexcept override;

    void UpdateAnonymousChildStyle(RenderStyle &) const noexcept override;

    void SetText(const utf8_string &) noexcept;
    KRYS_NODISCARD utf8_string Text() const noexcept;

    KRYS_NODISCARD RenderTextFragment *TextRenderer() const noexcept
    {
      return _buttonText.get();
    }

    KRYS_NODISCARD RenderBlock *InnerRenderer() const noexcept
    {
      return _inner.get();
    }

    void SetInnerRenderer(RenderBlock &) noexcept;

  private:
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderButton";
    }

    KRYS_NODISCARD bool HasLineIfEmpty() const noexcept override;

    KRYS_NODISCARD bool IsFlexibleBoxImpl() const noexcept override
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderButton)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderButton();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
