#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"

namespace krys::boo::html
{
  HTMLInputElement::HTMLInputElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Input)
  {
  }

#pragma region HTMLInputElement

  dom::DOMString HTMLInputElement::Accept() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"accept").value_or(u8"");
  }

  void HTMLInputElement::Accept(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"accept", krys::move(value));
  }

  bool HTMLInputElement::Alpha() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"alpha");
  }

  void HTMLInputElement::Alpha(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"alpha", krys::move(value));
  }

  dom::DOMString HTMLInputElement::Alt() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"alt").value_or(u8"");
  }

  void HTMLInputElement::Alt(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"alt", krys::move(value));
  }

  dom::DOMString HTMLInputElement::Autocomplete() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"autocomplete").value_or(u8"");
  }

  void HTMLInputElement::Autocomplete(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"autocomplete", krys::move(value));
  }

  bool HTMLInputElement::DefaultChecked() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"checked");
  }

  void HTMLInputElement::DefaultChecked(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"checked", krys::move(value));
  }

  bool HTMLInputElement::Checked() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"checked");
  }

  void HTMLInputElement::Checked(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"checked", krys::move(value));
  }

  dom::DOMString HTMLInputElement::ColorSpace() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"colorspace").value_or(u8"");
  }

  void HTMLInputElement::ColorSpace(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"colorspace", krys::move(value));
  }

  dom::DOMString HTMLInputElement::DirName() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"dirname").value_or(u8"");
  }

  void HTMLInputElement::DirName(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"dirname", krys::move(value));
  }

  bool HTMLInputElement::Disabled() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLInputElement::Disabled(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  dom::ExceptionOr<dom::USVString> HTMLInputElement::FormAction() const noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"formaction");
  }

  dom::ExceptionOr<void> HTMLInputElement::FormAction(dom::USVString &&value) noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"formaction", krys::move(value));
  }

  dom::DOMString HTMLInputElement::FormEnctype() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"formenctype").value_or(u8"");
  }

  void HTMLInputElement::FormEnctype(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"formenctype", krys::move(value));
  }

  dom::DOMString HTMLInputElement::FormMethod() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"formmethod").value_or(u8"");
  }

  void HTMLInputElement::FormMethod(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"formmethod", krys::move(value));
  }

  bool HTMLInputElement::FormNoValidate() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"formnovalidate");
  }

  void HTMLInputElement::FormNoValidate(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"formnovalidate", krys::move(value));
  }

  dom::DOMString HTMLInputElement::FormTarget() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"formtarget").value_or(u8"");
  }

  void HTMLInputElement::FormTarget(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"formtarget", krys::move(value));
  }

  uint32 HTMLInputElement::Height() const noexcept
  {
    return Reflection::Reflect<uint32>(*this, u8"height");
  }

  void HTMLInputElement::Height(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"height", krys::move(value));
  }

  bool HTMLInputElement::Indeterminate() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"indeterminate");
  }

  void HTMLInputElement::Indeterminate(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"indeterminate", krys::move(value));
  }

  dom::DOMString HTMLInputElement::Max() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"max").value_or(u8"");
  }

  void HTMLInputElement::Max(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"max", krys::move(value));
  }

  int32 HTMLInputElement::MaxLength() const noexcept
  {
    return Reflection::Reflect<int32>(*this, u8"maxlength");
  }

  void HTMLInputElement::MaxLength(int32 value) noexcept
  {
    Reflection::Reflect<int32>(*this, u8"maxlength", krys::move(value));
  }

  dom::DOMString HTMLInputElement::Min() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"min").value_or(u8"");
  }

  void HTMLInputElement::Min(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"min", krys::move(value));
  }

  int32 HTMLInputElement::MinLength() const noexcept
  {
    return Reflection::Reflect<int32>(*this, u8"minlength");
  }

  void HTMLInputElement::MinLength(int32 value) noexcept
  {
    Reflection::Reflect<int32>(*this, u8"minlength", krys::move(value));
  }

  bool HTMLInputElement::Multiple() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"multiple");
  }

  void HTMLInputElement::Multiple(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"multiple", krys::move(value));
  }

  dom::DOMString HTMLInputElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLInputElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::DOMString HTMLInputElement::Pattern() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"pattern").value_or(u8"");
  }

  void HTMLInputElement::Pattern(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"pattern", krys::move(value));
  }

  dom::DOMString HTMLInputElement::Placeholder() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"placeholder").value_or(u8"");
  }

  void HTMLInputElement::Placeholder(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"placeholder", krys::move(value));
  }

  bool HTMLInputElement::ReadOnly() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"readonly");
  }

  void HTMLInputElement::ReadOnly(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"readonly", krys::move(value));
  }

  bool HTMLInputElement::Required() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"required");
  }

  void HTMLInputElement::Required(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"required", krys::move(value));
  }

  uint32 HTMLInputElement::Size() const noexcept
  {
    return Reflection::Reflect<uint32>(*this, u8"size");
  }

  void HTMLInputElement::Size(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"size", krys::move(value));
  }

  dom::ExceptionOr<dom::USVString> HTMLInputElement::Src() const noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"src");
  }

  dom::ExceptionOr<void> HTMLInputElement::Src(dom::USVString &&value) noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"src", krys::move(value));
  }

  dom::DOMString HTMLInputElement::Step() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"step").value_or(u8"");
  }

  void HTMLInputElement::Step(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"step", krys::move(value));
  }

  dom::DOMString HTMLInputElement::Type() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLInputElement::Type(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

  bool HTMLInputElement::IsPasswordField() const noexcept
  {
    auto type = GetAttribute(u8"type");
    return type && infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*type, u8"password");
  }

  bool HTMLInputElement::IsSearchField() const noexcept
  {
    auto type = GetAttribute(u8"type");
    return type && infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*type, u8"search");
  }

  bool HTMLInputElement::IsImageButton() const noexcept
  {
    auto type = GetAttribute(u8"type");
    return type && infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*type, u8"image");
  }

  bool HTMLInputElement::IsReplaced(const RenderStyle *) const noexcept
  {
    return IsImageButton();
  }

  dom::DOMString HTMLInputElement::DefaultValue() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"defaultvalue").value_or(u8"");
  }

  void HTMLInputElement::DefaultValue(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"defaultvalue", krys::move(value));
  }

  dom::DOMString HTMLInputElement::Value() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"value").value_or(u8"");
  }

  void HTMLInputElement::Value(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"value", krys::move(value));
  }

  uint32 HTMLInputElement::Width() const noexcept
  {
    return Reflection::Reflect<uint32>(*this, u8"width");
  }

  void HTMLInputElement::Width(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"width", krys::move(value));
  }

#pragma endregion

#pragma region HTMLInputElement Obsolete members

  dom::DOMString HTMLInputElement::Align() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLInputElement::Align(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  dom::DOMString HTMLInputElement::UseMap() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"usemap").value_or(u8"");
  }

  void HTMLInputElement::UseMap(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"usemap", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLInputElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                            dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null())
    {
      // The type decides which of the other attributes map to style.
      if (localName == u8"vspace" || localName == u8"hspace" || localName == u8"width"
          || localName == u8"height" || localName == u8"type")
      {
        return true;
      }
      if (localName == u8"border")
      {
        return IsImageButton();
      }
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLInputElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                                dom::DOMStringAtom namespaceURI,
                                                                dom::DOMStringView value,
                                                                css::MutableStyleProperties &style) noexcept
  {
    using namespace presentational_hints;
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    // Only image buttons respect the align, width and height attributes.
    if (localName == u8"vspace")
    {
      if (IsImageButton())
      {
        AddHTMLLengthToStyle(style, css::PropertyId::MarginTop, value);
        AddHTMLLengthToStyle(style, css::PropertyId::MarginBottom, value);
      }
      return;
    }
    if (localName == u8"hspace")
    {
      if (IsImageButton())
      {
        AddHTMLLengthToStyle(style, css::PropertyId::MarginLeft, value);
        AddHTMLLengthToStyle(style, css::PropertyId::MarginRight, value);
      }
      return;
    }
    if (localName == u8"align")
    {
      if (IsImageButton())
      {
        ApplyAlignmentAttributeToStyle(value, style);
      }
      return;
    }
    if (localName == u8"width")
    {
      if (IsImageButton())
      {
        AddHTMLLengthToStyle(style, css::PropertyId::Width, value);
        ApplyAspectRatioFromWidthAndHeightAttributesToStyle(value, AttributeOrEmpty(u8"height"), style);
      }
      return;
    }
    if (localName == u8"height")
    {
      if (IsImageButton())
      {
        AddHTMLLengthToStyle(style, css::PropertyId::Height, value);
        ApplyAspectRatioFromWidthAndHeightAttributesToStyle(AttributeOrEmpty(u8"width"), value, style);
      }
      return;
    }
    if (localName == u8"border")
    {
      if (IsImageButton())
      {
        ApplyBorderAttributeToStyle(value, style);
      }
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
