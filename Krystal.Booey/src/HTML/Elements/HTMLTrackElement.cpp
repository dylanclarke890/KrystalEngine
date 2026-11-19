#include "Krystal.Booey/HTML/Elements/HTMLTrackElement.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/#attr-track-kind
  template <>
  struct html::EnumeratedAttributeTraits<"kind", HTMLTrackElement>
  {
    using State = html::KindAttributeState;
    static inline const html::KeywordStateMapping<State> StateMappings[] = {
      {u8"subtitles", State::Subtitles},
      {u8"captions", State::Captions},
      {u8"descriptions", State::Descriptions},
      {u8"chapters", State::Chapters},
      {u8"metadata", State::Metadata}};

    constexpr static State MissingValueDefault = State::Subtitles;
    constexpr static State InvalidValueDefault = State::Metadata;
  };

  HTMLTrackElement::HTMLTrackElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Track)
  {
  }

#pragma region HTMLTrackElement

  DOMString HTMLTrackElement::Kind() const noexcept
  {
    return html::ReflectEnumerated<"kind", HTMLTrackElement>(*this);
  }

  void HTMLTrackElement::Kind(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"kind", krys::move(value));
  }

  dom::USVString HTMLTrackElement::Src() const noexcept
  {
    return html::ReflectUrl<dom::USVString>(*this, u8"src");
  }

  void HTMLTrackElement::Src(dom::USVString value) noexcept
  {
    return html::ReflectUrl<dom::USVString>(*this, u8"src", krys::move(value));
  }

  DOMString HTMLTrackElement::SrcLang() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"srclang").value_or(u8"");
  }

  void HTMLTrackElement::SrcLang(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"srclang", krys::move(value));
  }

  DOMString HTMLTrackElement::Label() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"label").value_or(u8"");
  }

  void HTMLTrackElement::Label(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"label", krys::move(value));
  }

  bool HTMLTrackElement::Default() const noexcept
  {
    return html::Reflect<bool>(*this, u8"default");
  }

  void HTMLTrackElement::Default(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"default", krys::move(value));
  }

#pragma endregion
}
