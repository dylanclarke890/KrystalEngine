#include "Krystal.Booey/Rendering/RenderTextFragment.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Rendering/RenderAncestorIterator.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"

namespace krys::boo
{
  RenderTextFragment::RenderTextFragment(dom::Text &textNode, utf8_stringview text, size_t startOffset,
                                         size_t length) noexcept
      : RenderText(Type::TextFragment, textNode, utf8_string(text.substr(startOffset, length))),
        _start(startOffset), _end(length)
  {
  }

  RenderTextFragment::RenderTextFragment(dom::Document &document, utf8_stringview text, size_t startOffset,
                                         size_t length) noexcept
      : RenderText(Type::TextFragment, document, utf8_string(text.substr(startOffset, length))),
        _start(startOffset), _end(length)
  {
  }

  RenderTextFragment::RenderTextFragment(dom::Document &document, utf8_string text) noexcept
      : RenderText(Type::TextFragment, document, text), _start(0), _end(text.size()),
        _contentString(krys::move(text))
  {
  }

  bool RenderTextFragment::CanBeSelectionLeaf() const noexcept
  {
    return TextNode() && TextNode()->HasEditableStyle();
  }

  RenderTextFragment::~RenderTextFragment() noexcept
  {
    krys_debug_assert(!_firstLetter);
  }

  RenderBoxModelObject *RenderTextFragment::FirstLetter() const noexcept
  {
    return _firstLetter.get();
  }

  void RenderTextFragment::SetFirstLetter(RenderBoxModelObject &firstLetter) noexcept
  {
    _firstLetter = CreateWeakPtr(&firstLetter);
  }

  void RenderTextFragment::SetTextInternal(utf8_string newText, bool force) noexcept
  {
    RenderText::SetTextInternal(krys::move(newText), force);

    _start = 0;
    _end = Text().size();
    if (!_firstLetter)
    {
      return;
    }
    if (auto *builder = RenderTreeBuilder::Current())
    {
      builder->Destroy(*_firstLetter);
    }
    else
    {
      RenderTreeBuilder(*GetDocument().GetRenderView()).Destroy(*_firstLetter);
    }
    krys_debug_assert(!_firstLetter);
    krys_debug_assert(!TextNode() || TextNode()->Renderer() == this);
  }

  utf8_string RenderTextFragment::PreviousCharacter() const noexcept
  {
    if (Start())
    {
      utf8_stringview original = TextNode() ? utf8_stringview(TextNode()->Data()) : _contentString;
      if (Start() <= original.size())
      {
        // The code point that ends where the fragment starts.
        auto start = Start() - 1;
        while (start > 0 && (original[start] & 0xC0) == 0x80)
        {
          --start;
        }
        return utf8_string(original.substr(start, Start() - start));
      }
    }
    return RenderText::PreviousCharacter();
  }

  RenderBlock *RenderTextFragment::BlockForAccompanyingFirstLetter() noexcept
  {
    if (!_firstLetter)
    {
      return nullptr;
    }
    for (auto &block : AncestorsOfType<RenderBlock>(*_firstLetter))
    {
      // FIXME: Stop at a multi-column flow, once there are multi-column flows.
      if (block.Style().HasPseudoStyle(PseudoElementType::FirstLetter) && block.CanHaveChildren())
      {
        return &block;
      }
    }
    return nullptr;
  }

  void RenderTextFragment::SetContentString(utf8_string text) noexcept
  {
    _contentString = text;
    SetText(krys::move(text));
  }
}
