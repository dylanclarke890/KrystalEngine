#pragma once

#include "Krystal.Booey/Platform/PopupMenu.hpp"
#include "Krystal.Booey/Platform/PopupMenuClient.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
{
  class HTMLSelectElement;
}

namespace krys::boo
{
  class RenderText;

  /// @brief The renderer of a select drawn as a drop-down: the selected option's text in an anonymous block,
  /// with a popup menu of the options.
  class RenderMenuList final : public RenderFlexibleBox, private PopupMenuClient
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMenuList);

  private:
    WeakPtr<RenderText> _buttonText;
    WeakPtr<RenderBlock> _innerBlock;

    bool _needsOptionsWidthUpdate {true};
    int32 _optionsWidth {0};

    Maybe<int32> _lastActiveIndex;

    UniquePtr<RenderStyle> _optionStyle;

    RefPtr<PopupMenu> _popup;
    bool _popupIsVisible {false};

  public:
    RenderMenuList(html::HTMLSelectElement &element, RenderStyle &&style) noexcept;
    ~RenderMenuList() noexcept override;

    KRYS_NODISCARD html::HTMLSelectElement &SelectElement() const noexcept;

    KRYS_NODISCARD bool PopupIsVisible() const noexcept
    {
      return _popupIsVisible;
    }

    void ShowPopup() noexcept;
    void HidePopup() noexcept;

    void SetOptionsChanged(bool changed) noexcept
    {
      _needsOptionsWidthUpdate = changed;
    }

    void DidSetSelectedIndex(int32 listIndex) noexcept;

    KRYS_NODISCARD utf8_string Text() const noexcept;

    KRYS_NODISCARD RenderBlock *InnerRenderer() const noexcept
    {
      return _innerBlock.get();
    }

    void SetInnerRenderer(RenderBlock &innerRenderer) noexcept;

    void DidAttachChild(RenderObject &child, RenderObject *beforeChild) noexcept;

  private:
    void WillBeDestroyed() noexcept override;

    KRYS_NODISCARD bool CreatesAnonymousWrapper() const noexcept override
    {
      return true;
    }

    void UpdateFromElement() noexcept override;

    KRYS_NODISCARD LayoutRect ControlClipRect(const LayoutPoint &additionalOffset) const noexcept override;

    KRYS_NODISCARD bool HasControlClip() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD bool CanHaveGeneratedChildren() const noexcept override
    {
      return false;
    }

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderMenuList";
    }

    void ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                       LayoutUnit &maxLogicalWidth) const noexcept override;
    void ComputePreferredLogicalWidths() noexcept override;

    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;

#pragma region PopupMenuClient

    void ValueChanged(uint32 listIndex, bool fireOnChange = true) noexcept override;

    void SelectionChanged(uint32, bool) noexcept override
    {
    }

    void SelectionCleared() noexcept override
    {
    }

    KRYS_NODISCARD utf8_string ItemText(uint32 listIndex) const noexcept override;
    KRYS_NODISCARD utf8_string ItemLabel(uint32 listIndex) const noexcept override;
    KRYS_NODISCARD utf8_string ItemIcon(uint32 listIndex) const noexcept override;
    KRYS_NODISCARD utf8_string ItemToolTip(uint32 listIndex) const noexcept override;
    KRYS_NODISCARD utf8_string ItemAccessibilityText(uint32 listIndex) const noexcept override;
    KRYS_NODISCARD bool ItemIsEnabled(uint32 listIndex) const noexcept override;
    KRYS_NODISCARD PopupMenuStyle ItemStyle(uint32 listIndex) const noexcept override;
    KRYS_NODISCARD PopupMenuStyle MenuStyle() const noexcept override;
    KRYS_NODISCARD int32 ClientInsetLeft() const noexcept override;
    KRYS_NODISCARD int32 ClientInsetRight() const noexcept override;
    KRYS_NODISCARD LayoutUnit ClientPaddingLeft() const noexcept override;
    KRYS_NODISCARD LayoutUnit ClientPaddingRight() const noexcept override;
    KRYS_NODISCARD int32 ListSize() const noexcept override;
    KRYS_NODISCARD int32 SelectedIndex() const noexcept override;
    void PopupDidHide() noexcept override;
    KRYS_NODISCARD bool ItemIsSeparator(uint32 listIndex) const noexcept override;
    KRYS_NODISCARD bool ItemIsLabel(uint32 listIndex) const noexcept override;
    KRYS_NODISCARD bool ItemIsSelected(uint32 listIndex) const noexcept override;

    KRYS_NODISCARD bool ShouldPopOver() const noexcept override
    {
      return true;
    }

    void SetTextFromItem(uint32 listIndex) noexcept override;
    void ListBoxSelectItem(int32 listIndex, bool allowMultiplySelections, bool shift,
                           bool fireOnChangeNow = true) noexcept override;
    KRYS_NODISCARD bool Multiple() const noexcept override;

#pragma endregion

    KRYS_NODISCARD bool HasLineIfEmpty() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept override
    {
      return RenderBlock::FirstLineBaseline();
    }

    void GetItemBackgroundColor(uint32 listIndex, Color &itemBackgroundColor,
                                bool &itemHasCustomBackgroundColor) const noexcept;

    void AdjustInnerStyle() noexcept;
    void SetText(const utf8_string &text) noexcept;
    void SetTextFromOption(int32 optionIndex) noexcept;
    void UpdateOptionsWidth() noexcept;

    void DidUpdateActiveOption(int32 optionIndex) noexcept;

    KRYS_NODISCARD bool IsFlexibleBoxImpl() const noexcept override
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMenuList)

  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMenuList();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
