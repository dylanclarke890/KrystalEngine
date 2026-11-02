#include "Krystal.Booey/HTML/HTMLTrackElement.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#attr-track-kind
  template <>
  struct EnumeratedAttributeTraits<"kind", HTMLTrackElement>
  {
    static inline KeywordStateMappingArray<5> StateMappings = {
      KeywordStateMapping {u8"subtitles", AttributeState::Subtitles},
      KeywordStateMapping {u8"captions", AttributeState::Captions},
      KeywordStateMapping {u8"descriptions", AttributeState::Descriptions},
      KeywordStateMapping {u8"chapters", AttributeState::Chapters},
      KeywordStateMapping {u8"metadata", AttributeState::Metadata}};

    constexpr static AttributeState MissingValueDefault = AttributeState::Subtitles;
    constexpr static AttributeState InvalidValueDefault = AttributeState::Metadata;
  };

  HTMLTrackElement::HTMLTrackElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Track)
  {
  }

#pragma region HTMLTrackElement

  dom::DOMString HTMLTrackElement::Kind() const noexcept
  {
    return EnumeratedAttribute<"kind", HTMLTrackElement>::ResolveCanonicalKeyword<dom::DOMString>(
      Reflect<dom::DOMString>(*this, u8"kind"));
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
