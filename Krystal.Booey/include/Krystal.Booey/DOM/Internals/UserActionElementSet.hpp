#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/HashMap.hpp"

namespace krys::boo::dom
{
  class Element;

  /// @brief The user action state of a document's elements: which are active, hovered or focused. Only the
  /// elements with some state are kept; they're flagged (`Element::IsUserActionElement`).
  class UserActionElementSet
  {
    KRYS_NON_COPY_MOVABLE(UserActionElementSet);

    enum class Flag : uint8
    {
      IsActive,
      InActiveChain,
      IsHovered,
      IsFocused,
      IsBeingDragged,
      HasFocusVisible,
      HasFocusWithin,
    };

  private:
    HashMap<const Element *, EnumSet<Flag>> _elements;

  public:
    UserActionElementSet() noexcept = default;

    KRYS_NODISCARD bool IsActive(const Element &element) const noexcept
    {
      return HasFlag(element, Flag::IsActive);
    }

    KRYS_NODISCARD bool IsFocused(const Element &element) const noexcept
    {
      return HasFlag(element, Flag::IsFocused);
    }

    KRYS_NODISCARD bool IsHovered(const Element &element) const noexcept
    {
      return HasFlag(element, Flag::IsHovered);
    }

    KRYS_NODISCARD bool IsInActiveChain(const Element &element) const noexcept
    {
      return HasFlag(element, Flag::InActiveChain);
    }

    KRYS_NODISCARD bool IsBeingDragged(const Element &element) const noexcept
    {
      return HasFlag(element, Flag::IsBeingDragged);
    }

    KRYS_NODISCARD bool HasFocusVisible(const Element &element) const noexcept
    {
      return HasFlag(element, Flag::HasFocusVisible);
    }

    KRYS_NODISCARD bool HasFocusWithin(const Element &element) const noexcept
    {
      return HasFlag(element, Flag::HasFocusWithin);
    }

    void SetActive(Element &element, bool enable) noexcept
    {
      SetFlags(element, enable, Flag::IsActive);
    }

    void SetFocused(Element &element, bool enable) noexcept
    {
      SetFlags(element, enable, Flag::IsFocused);
    }

    void SetHovered(Element &element, bool enable) noexcept
    {
      SetFlags(element, enable, Flag::IsHovered);
    }

    void SetInActiveChain(Element &element, bool enable) noexcept
    {
      SetFlags(element, enable, Flag::InActiveChain);
    }

    void SetBeingDragged(Element &element, bool enable) noexcept
    {
      SetFlags(element, enable, Flag::IsBeingDragged);
    }

    void SetHasFocusVisible(Element &element, bool enable) noexcept
    {
      SetFlags(element, enable, Flag::HasFocusVisible);
    }

    void SetHasFocusWithin(Element &element, bool enable) noexcept
    {
      SetFlags(element, enable, Flag::HasFocusWithin);
    }

    void ClearActiveAndHovered(Element &element) noexcept
    {
      ClearFlags(element, {Flag::IsActive, Flag::InActiveChain, Flag::IsHovered});
    }

    void ClearAllForElement(Element &element) noexcept
    {
      ClearFlags(element, {Flag::IsActive, Flag::InActiveChain, Flag::IsHovered, Flag::IsFocused,
                           Flag::IsBeingDragged, Flag::HasFocusVisible, Flag::HasFocusWithin});
    }

    void Clear() noexcept;

    /// @brief Calls @p function for each element that is focused or has focus within.
    template <typename TFunction>
    void ForEachFocusedOrFocusWithinElement(TFunction &&function) const noexcept
    {
      for (auto &[element, flags] : _elements)
      {
        if (flags.ContainsAny({Flag::IsFocused, Flag::HasFocusWithin}))
        {
          function(const_cast<Element &>(*element));
        }
      }
    }

  private:
    void SetFlags(Element &element, bool enable, EnumSet<Flag> flags) noexcept
    {
      enable ? SetFlags(element, flags) : ClearFlags(element, flags);
    }

    void SetFlags(Element &element, EnumSet<Flag> flags) noexcept;
    void ClearFlags(Element &element, EnumSet<Flag> flags) noexcept;
    KRYS_NODISCARD bool HasFlag(const Element &element, Flag flag) const noexcept;
  };
}
