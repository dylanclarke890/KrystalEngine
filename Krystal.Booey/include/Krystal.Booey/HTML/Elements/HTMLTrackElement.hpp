#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Core/Enum.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/multipage/media.html#dom-track-none
  enum class TrackReadyState : uint8
  {
    None,
    Loading,
    Loaded,
    Error,
  };

  consteval TrackReadyState MaxEnumValue(TrackReadyState) noexcept
  {
    return TrackReadyState::Error;
  }
}

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/media.html#htmltrackelement
  class HTMLTrackElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTrackElement);

  private:
    html::TrackReadyState _readyState : EnumBitWidth<html::TrackReadyState> {html::TrackReadyState::None};

  public:
    HTMLTrackElement(Document &document) noexcept;

#pragma region HTMLTrackElement - https://html.spec.whatwg.org/multipage/media.html#htmltrackelement

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-track-kind
    KRYS_NODISCARD DOMString Kind() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-track-kind
    void Kind(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-track-src
    KRYS_NODISCARD dom::USVString Src() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-track-src
    void Src(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-track-srclang
    KRYS_NODISCARD DOMString SrcLang() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-track-srclang
    void SrcLang(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-track-label
    KRYS_NODISCARD DOMString Label() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-track-label
    void Label(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-track-default
    KRYS_NODISCARD bool Default() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-track-default
    void Default(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-track-readystate
    KRYS_NODISCARD html::TrackReadyState ReadyState() const noexcept
    {
      return _readyState;
    }

    // FIXME: Implement Track().
    // readonly attribute TextTrack track;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLTrackElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLTrackElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
