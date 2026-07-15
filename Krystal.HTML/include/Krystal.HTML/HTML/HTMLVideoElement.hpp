#pragma once

#include "Krystal.HTML/HTML/HTMLMediaElement.hpp"
#include "Krystal.HTML/DOM/Types/USVString.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlvideoelement
  class HTMLVideoElement : public HTMLMediaElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLVideoElement);

  public:
    HTMLVideoElement(Document &document) noexcept;

#pragma region HTMLVideoElement - https://html.spec.whatwg.org/#htmlvideoelement

    /// @see https://html.spec.whatwg.org/#dom-video-width
    KRYS_NODISCARD uint32 Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-video-width
    void Width(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-video-height
    KRYS_NODISCARD uint32 Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-video-height
    void Height(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-video-videowidth
    KRYS_NODISCARD uint32 VideoWidth() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-video-videoheight
    KRYS_NODISCARD uint32 VideoHeight() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-video-poster
    KRYS_NODISCARD ExceptionOr<USVString> Poster() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-video-poster
    ExceptionOr<void> Poster(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-video-playsinline
    KRYS_NODISCARD bool PlaysInline() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-video-playsinline
    void PlaysInline(bool value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLVideoElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLVideoElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();