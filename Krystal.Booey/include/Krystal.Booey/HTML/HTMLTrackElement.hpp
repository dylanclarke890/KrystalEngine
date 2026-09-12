#pragma once

#include "Krystal.Booey/HTML/Enums/TrackReadyState.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmltrackelement
  class HTMLTrackElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTrackElement);

  private:
    TrackReadyState _readyState : BitCount<TrackReadyState>() {TrackReadyState::None};

  public:
    HTMLTrackElement(Document &document) noexcept;

#pragma region HTMLTrackElement - https://html.spec.whatwg.org/#htmltrackelement

    /// @see https://html.spec.whatwg.org/#dom-track-kind
    KRYS_NODISCARD dom::DOMString Kind() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-track-kind
    void Kind(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-track-src
    KRYS_NODISCARD dom::ExceptionOr<USVString> Src() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-track-src
    dom::ExceptionOr<void> Src(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-track-srclang
    KRYS_NODISCARD dom::DOMString SrcLang() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-track-srclang
    void SrcLang(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-track-label
    KRYS_NODISCARD dom::DOMString Label() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-track-label
    void Label(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-track-default
    KRYS_NODISCARD bool Default() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-track-default
    void Default(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-track-readystate
    KRYS_NODISCARD TrackReadyState ReadyState() const noexcept
    {
      return _readyState;
    }

    // TODO(HTMLTrackElement, HTML): implement Track().
    // readonly attribute TextTrack track;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTrackElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTrackElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();