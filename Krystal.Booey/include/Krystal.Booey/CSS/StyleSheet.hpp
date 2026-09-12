#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class CSSStyleSheet;
  class MediaList;

  /// @see hhttps://drafts.csswg.org/cssom/#the-stylesheet-interface
  class StyleSheet : public RefCounted<StyleSheet>
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();

  public:
    virtual ~StyleSheet() noexcept = default;

#pragma region StyleSheet - https://drafts.csswg.org/cssom/#stylesheet

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-type
    KRYS_NODISCARD virtual CSSOMString Type() const noexcept = 0;

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-href
    KRYS_NODISCARD virtual Maybe<dom::USVString> Href() const noexcept = 0;

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-ownernode
    KRYS_NODISCARD virtual RefPtr<dom::Node> OwnerNode() const noexcept = 0;

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-parentstylesheet
    KRYS_NODISCARD virtual RefPtr<StyleSheet> ParentStyleSheet() const noexcept
    {
      return nullptr;
    }

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-title
    KRYS_NODISCARD virtual Maybe<dom::DOMString> Title() const noexcept = 0;

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-media
    KRYS_NODISCARD virtual RefPtr<MediaList> Media() const noexcept
    {
      return nullptr;
    }

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-disabled
    KRYS_NODISCARD virtual bool Disabled() const noexcept = 0;

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-disabled
    virtual void Disabled(bool value) noexcept = 0;

#pragma endregion

  protected:
#pragma region Type Checks

    KRYS_NODISCARD virtual bool IsCSSStyleSheet() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsXSLStyleSheet() const noexcept
    {
      return false;
    }

#pragma endregion
  };
}