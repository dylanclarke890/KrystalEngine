#pragma once

#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderBlock;
  class RenderBoxModelObject;

  /// @brief Used to represent a text substring of an element, e.g., for text runs that are split because of
  /// first letter and that must therefore have different styles (and positions in the render tree).
  /// We cache offsets so that text transformations can be applied in such a way that we can recover
  /// the original unaltered string from our corresponding DOM node.
  // FIXME: Selection (canBeSelectionLeaf) comes with editing.
  class RenderTextFragment final : public RenderText
  {
    KRYS_NON_COPY_MOVABLE(RenderTextFragment);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTextFragment);

  private:
    size_t _start;
    size_t _end;
    // Alternative description that can be used for accessibility instead of the native text.
    utf8_string _altText;
    utf8_string _contentString;
    WeakPtr<RenderBoxModelObject> _firstLetter;

  public:
    RenderTextFragment(dom::Text &textNode, utf8_stringview text, size_t startOffset, size_t length) noexcept;
    RenderTextFragment(dom::Document &document, utf8_stringview text, size_t startOffset,
                       size_t length) noexcept;
    RenderTextFragment(dom::Document &document, utf8_string text) noexcept;
    ~RenderTextFragment() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderTextFragment";
    }

    /// @brief The byte offset of the fragment in the original text.
    KRYS_NODISCARD size_t Start() const noexcept
    {
      return _start;
    }
    /// @brief The length of the fragment, in bytes.
    KRYS_NODISCARD size_t End() const noexcept
    {
      return _end;
    }

    KRYS_NODISCARD RenderBoxModelObject *FirstLetter() const noexcept;
    void SetFirstLetter(RenderBoxModelObject &firstLetter) noexcept;

    KRYS_NODISCARD RenderBlock *BlockForAccompanyingFirstLetter() noexcept;

    void SetContentString(utf8_string text) noexcept;
    KRYS_NODISCARD const utf8_string &ContentString() const noexcept
    {
      return _contentString;
    }

    KRYS_NODISCARD const utf8_string &AltText() const noexcept
    {
      return _altText;
    }
    void SetAltText(utf8_string altText) noexcept
    {
      _altText = krys::move(altText);
    }

  private:
    void SetTextInternal(utf8_string text, bool force) noexcept override;

    KRYS_NODISCARD utf8_string PreviousCharacter() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTextFragment)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTextFragment();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
