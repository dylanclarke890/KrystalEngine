#include "Krystal.Booey/MathML/MathMLOperatorElement.hpp"
#include "Krystal.Booey/MathML/MathMLNames.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLOperator.hpp"
#include "Krystal.Booey/Rendering/RenderPtr.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/Unicode.hpp"

namespace krys::boo::mathml
{
  MathMLOperatorElement::MathMLOperatorElement(dom::Document &document) noexcept
      : MathMLTokenElement(document, MathMLElementInterface::Operator)
  {
  }

  MathMLOperatorElement::OperatorChar
    MathMLOperatorElement::ParseOperatorChar(dom::DOMStringView string) noexcept
  {
    OperatorChar operatorChar;
    auto view = string;
    while (!view.empty() && text::IsASCIIWhitespaceWithoutFF(view.front()))
    {
      view.remove_prefix(1);
    }
    while (!view.empty() && text::IsASCIIWhitespaceWithoutFF(view.back()))
    {
      view.remove_suffix(1);
    }

    // FIXME: This operator dictionary does not accept multiple characters (https://webkit.org/b/124828).
    if (!view.empty())
    {
      auto decoded = text::DecodeUTF8(Span<const char8>(view.data(), view.size()));
      if (decoded.IsValid && decoded.Length == view.size())
      {
        auto character = decoded.CodePoint;
        // The minus sign renders better than the hyphen sign used in some MathML formulas.
        if (character == text::HyphenMinus)
        {
          character = text::MinusSign;
        }
        operatorChar.Character = character;
        operatorChar.IsVertical = MathMLOperatorDictionary::IsVertical(operatorChar.Character);
      }
    }
    return operatorChar;
  }

  const MathMLOperatorElement::OperatorChar &MathMLOperatorElement::GetOperatorChar() noexcept
  {
    if (!_operatorChar)
    {
      _operatorChar = ParseOperatorChar(TextContent().value_or(dom::DOMString {}));
    }
    return *_operatorChar;
  }

  MathMLOperatorDictionary::Property MathMLOperatorElement::ComputeDictionaryProperty() noexcept
  {
    MathMLOperatorDictionary::Property dictionaryProperty;

    // We first determine the form attribute and use the default spacing and properties.
    auto value = AttributeValue(MathMLNames::FormAttr());
    bool explicitForm = true;
    if (value == u8"prefix")
    {
      dictionaryProperty.Form = MathMLOperatorDictionary::Form::Prefix;
    }
    else if (value == u8"infix")
    {
      dictionaryProperty.Form = MathMLOperatorDictionary::Form::Infix;
    }
    else if (value == u8"postfix")
    {
      dictionaryProperty.Form = MathMLOperatorDictionary::Form::Postfix;
    }
    else
    {
      // FIXME: We should use more advanced heuristics indicated in the specification to determine the
      // operator form (https://bugs.webkit.org/show_bug.cgi?id=124829).
      explicitForm = false;
      if (!PreviousSibling() && NextSibling())
      {
        dictionaryProperty.Form = MathMLOperatorDictionary::Form::Prefix;
      }
      else if (PreviousSibling() && !NextSibling())
      {
        dictionaryProperty.Form = MathMLOperatorDictionary::Form::Postfix;
      }
      else
      {
        dictionaryProperty.Form = MathMLOperatorDictionary::Form::Infix;
      }
    }

    // We then try and find an entry in the operator dictionary to override the default values.
    if (auto entry = MathMLOperatorDictionary::Search(GetOperatorChar().Character, dictionaryProperty.Form,
                                                      explicitForm))
    {
      dictionaryProperty = *entry;
    }

    return dictionaryProperty;
  }

  const MathMLOperatorDictionary::Property &MathMLOperatorElement::DictionaryProperty() noexcept
  {
    if (!_dictionaryProperty)
    {
      _dictionaryProperty = ComputeDictionaryProperty();
    }
    return *_dictionaryProperty;
  }

  namespace
  {
    KRYS_NODISCARD const dom::QualifiedName &
      PropertyFlagToAttributeName(MathMLOperatorDictionary::Flag flag) noexcept
    {
      switch (flag)
      {
        case MathMLOperatorDictionary::Accent:
        {
          return MathMLNames::AccentAttr();
        }
        case MathMLOperatorDictionary::Fence:
        {
          return MathMLNames::FenceAttr();
        }
        case MathMLOperatorDictionary::LargeOp:
        {
          return MathMLNames::LargeopAttr();
        }
        case MathMLOperatorDictionary::MovableLimits:
        {
          return MathMLNames::MovablelimitsAttr();
        }
        case MathMLOperatorDictionary::Separator:
        {
          return MathMLNames::SeparatorAttr();
        }
        case MathMLOperatorDictionary::Stretchy:
        {
          return MathMLNames::StretchyAttr();
        }
        case MathMLOperatorDictionary::Symmetric:
        {
          return MathMLNames::SymmetricAttr();
        }
      }
      krys_unreachable();
    }
  }

  void MathMLOperatorElement::ComputeOperatorFlag(MathMLOperatorDictionary::Flag flag) noexcept
  {
    krys_debug_assert(_properties.DirtyFlags & flag);

    Maybe<BooleanValue> property;
    const auto &name = PropertyFlagToAttributeName(flag);
    auto value = CachedBooleanAttribute(name, property);
    switch (value)
    {
      case BooleanValue::True:
      {
        _properties.Flags |= flag;
        break;
      }
      case BooleanValue::False:
      {
        _properties.Flags &= ~flag;
        break;
      }
      case BooleanValue::Default:
      {
        // By default, we use the value specified in the operator dictionary.
        if (DictionaryProperty().Flags & flag)
        {
          _properties.Flags |= flag;
        }
        else
        {
          _properties.Flags &= ~flag;
        }
        break;
      }
    }
  }

  bool MathMLOperatorElement::HasProperty(MathMLOperatorDictionary::Flag flag) noexcept
  {
    if (_properties.DirtyFlags & flag)
    {
      ComputeOperatorFlag(flag);
      _properties.DirtyFlags &= ~flag;
    }
    return _properties.Flags & flag;
  }

  MathMLElement::Length MathMLOperatorElement::DefaultLeadingSpace() noexcept
  {
    return Length {.Type = LengthType::MathUnit,
                   .Value = static_cast<float>(DictionaryProperty().LeadingSpaceInMathUnit)};
  }

  MathMLElement::Length MathMLOperatorElement::DefaultTrailingSpace() noexcept
  {
    return Length {.Type = LengthType::MathUnit,
                   .Value = static_cast<float>(DictionaryProperty().TrailingSpaceInMathUnit)};
  }

  const MathMLElement::Length &MathMLOperatorElement::LeadingSpace() noexcept
  {
    return CachedMathMLLength(MathMLNames::LspaceAttr(), _leadingSpace);
  }

  const MathMLElement::Length &MathMLOperatorElement::TrailingSpace() noexcept
  {
    return CachedMathMLLength(MathMLNames::RspaceAttr(), _trailingSpace);
  }

  const MathMLElement::Length &MathMLOperatorElement::MinSize() noexcept
  {
    return CachedMathMLLength(MathMLNames::MinsizeAttr(), _minSize);
  }

  const MathMLElement::Length &MathMLOperatorElement::MaxSize() noexcept
  {
    return CachedMathMLLength(MathMLNames::MaxsizeAttr(), _maxSize);
  }

  void MathMLOperatorElement::UpdateRendererFromElement() noexcept
  {
    if (auto *renderer = Renderer())
    {
      Downcast<RenderMathMLOperator>(*renderer).UpdateFromElement();
    }
  }

  RenderPtr<RenderElement> MathMLOperatorElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderMathMLOperator>(*this, krys::move(style));
  }

  void MathMLOperatorElement::OnChildrenChanged() noexcept
  {
    _operatorChar.reset();
    _dictionaryProperty.reset();
    _properties.DirtyFlags = MathMLOperatorDictionary::AllFlags;
    MathMLTokenElement::OnChildrenChanged();
  }

  void MathMLOperatorElement::AttributeChanged(const dom::QualifiedName &name,
                                               Maybe<dom::DOMStringView> oldValue,
                                               Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == MathMLNames::FormAttr())
    {
      _dictionaryProperty.reset();
      _properties.DirtyFlags = MathMLOperatorDictionary::AllFlags;
    }
    else if (name == MathMLNames::LspaceAttr())
    {
      _leadingSpace.reset();
      UpdateRendererFromElement();
    }
    else if (name == MathMLNames::RspaceAttr())
    {
      _trailingSpace.reset();
      UpdateRendererFromElement();
    }
    else if (name == MathMLNames::MinsizeAttr())
    {
      _minSize.reset();
    }
    else if (name == MathMLNames::MaxsizeAttr())
    {
      _maxSize.reset();
    }
    else if (name == MathMLNames::StretchyAttr())
    {
      _properties.DirtyFlags |= MathMLOperatorDictionary::Stretchy;
      UpdateRendererFromElement();
    }
    else if (name == MathMLNames::MovablelimitsAttr())
    {
      _properties.DirtyFlags |= MathMLOperatorDictionary::MovableLimits;
      UpdateRendererFromElement();
    }
    else if (name == MathMLNames::AccentAttr())
    {
      _properties.DirtyFlags |= MathMLOperatorDictionary::Accent;
    }
    else if (name == MathMLNames::FenceAttr())
    {
      _properties.DirtyFlags |= MathMLOperatorDictionary::Fence;
    }
    else if (name == MathMLNames::LargeopAttr())
    {
      _properties.DirtyFlags |= MathMLOperatorDictionary::LargeOp;
    }
    else if (name == MathMLNames::SeparatorAttr())
    {
      _properties.DirtyFlags |= MathMLOperatorDictionary::Separator;
    }
    else if (name == MathMLNames::SymmetricAttr())
    {
      _properties.DirtyFlags |= MathMLOperatorDictionary::Symmetric;
    }

    MathMLTokenElement::AttributeChanged(name, oldValue, value);
  }
}
