#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"

namespace krys::boo::dom
{
  class Node;
}

namespace krys::boo::css
{
  class CSSImportRule;
  class MediaList;

  /// @brief The base of the style sheets exposed to scripts.
  /// @see https://drafts.csswg.org/cssom/#the-stylesheet-interface
  class StyleSheet : public RefCounted<StyleSheet>
  {
  public:
    virtual ~StyleSheet() noexcept = default;

    KRYS_NODISCARD virtual bool Disabled() const noexcept = 0;
    virtual void SetDisabled(bool disabled) noexcept = 0;
    KRYS_NODISCARD virtual dom::Node *OwnerNode() const noexcept = 0;

    KRYS_NODISCARD virtual StyleSheet *ParentStyleSheet() const noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD virtual CSSOMString Href() const noexcept = 0;
    KRYS_NODISCARD virtual CSSOMString Title() const noexcept = 0;

    KRYS_NODISCARD virtual MediaList *Media() const noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD virtual CSSOMString Type() const noexcept = 0;

    KRYS_NODISCARD virtual CSSImportRule *OwnerRule() const noexcept
    {
      return nullptr;
    }

    virtual void ClearOwnerNode() noexcept = 0;
    KRYS_NODISCARD virtual url::URL BaseURL() const noexcept = 0;
    KRYS_NODISCARD virtual bool IsLoading() const noexcept = 0;

    KRYS_NODISCARD virtual bool IsCSSStyleSheet() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsXSLStyleSheet() const noexcept
    {
      return false;
    }
  };
}
