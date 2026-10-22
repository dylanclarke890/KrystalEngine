#include "Krystal.Booey/Editing/SplitTextNodeContainingElementCommand.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"

namespace krys::boo
{
  SplitTextNodeContainingElementCommand::SplitTextNodeContainingElementCommand(dom::Text &text,
                                                                               uint32 offset) noexcept
      : CompositeEditCommand(text.NodeDocument()), _text(ShareRef(text)), _offset(offset)
  {
    krys_debug_assert(_text->Length() > 0);
  }

  Ref<SplitTextNodeContainingElementCommand>
    SplitTextNodeContainingElementCommand::Create(dom::Text &text, uint32 offset) noexcept
  {
    return CreateRef<SplitTextNodeContainingElementCommand>(text, offset);
  }

  void SplitTextNodeContainingElementCommand::DoApply() noexcept
  {
    krys_debug_assert(_offset > 0);

    SplitTextNode(*_text, _offset);

    RefPtr<dom::Element> parent = ShareRefPtr(_text->ParentElement());
    if (!parent)
    {
      return;
    }

    auto *parentParent = parent->ParentElement();
    if (!parentParent || !parentParent->HasEditableStyle())
    {
      return;
    }

    auto *parentRenderer = parent->Renderer();
    if (!parentRenderer || !parentRenderer->IsInline())
    {
      WrapContentsInDummySpan(*parent);
      auto *firstChild = DynamicDowncast<dom::Element>(parent->FirstChild());
      if (!firstChild)
      {
        return;
      }
      parent = ShareRefPtr(firstChild);
    }

    SplitElement(*parent, *_text);
  }
}
