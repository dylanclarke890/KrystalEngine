#include "Krystal.Booey/HTML/Elements/HTMLTrackElement.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#attr-track-kind
  template <>
  struct EnumeratedAttributeTraits<"kind", HTMLTrackElement>
  {
    using State = KindAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {{u8"subtitles", State::Subtitles},
                                                                      {u8"captions", State::Captions},
                                                                      {u8"descriptions", State::Descriptions},
                                                                      {u8"chapters", State::Chapters},
                                                                      {u8"metadata", State::Metadata}};

    constexpr static State MissingValueDefault = State::Subtitles;
    constexpr static State InvalidValueDefault = State::Metadata;
  };

  HTMLTrackElement::HTMLTrackElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Track)
  {
  }

#pragma region HTMLTrackElement

  dom::DOMString HTMLTrackElement::Kind() const noexcept
  {
    return ReflectEnumerated<"kind", HTMLTrackElement>(*this);
  }

  void HTMLTrackElement::Kind(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"kind", krys::move(value));
  }

  dom::USVString HTMLTrackElement::Src() const noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"src");
  }

  void HTMLTrackElement::Src(dom::USVString value) noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"src", krys::move(value));
  }

  dom::DOMString HTMLTrackElement::SrcLang() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"srclang").value_or(u8"");
  }

  void HTMLTrackElement::SrcLang(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"srclang", krys::move(value));
  }

  dom::DOMString HTMLTrackElement::Label() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"label").value_or(u8"");
  }

  void HTMLTrackElement::Label(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"label", krys::move(value));
  }

  bool HTMLTrackElement::Default() const noexcept
  {
    return Reflect<bool>(*this, u8"default");
  }

  void HTMLTrackElement::Default(bool value) noexcept
  {
    Reflect<bool>(*this, u8"default", krys::move(value));
  }

#pragma endregion
}
