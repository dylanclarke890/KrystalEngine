#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderRuby.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderInline.hpp"

namespace krys::boo
{
  RenderTreeBuilder::Ruby::Ruby(RenderTreeBuilder &builder) noexcept : _builder(builder)
  {
  }

  RenderStyle CreateAnonymousStyleForRuby(const RenderStyle &parentStyle, DisplayType display) noexcept
  {
    krys_debug_assert(display == DisplayType::Ruby || display == DisplayType::RubyBase);

    auto style = RenderStyle::CreateAnonymousStyleWithDisplay(parentStyle, display);
    style.SetUnicodeBidi(UnicodeBidi::Isolate);
    if (display == DisplayType::RubyBase)
    {
      style.SetTextWrapMode(TextWrapMode::NoWrap);
    }
    return style;
  }

  namespace
  {
    KRYS_NODISCARD RenderPtr<RenderElement> CreateAnonymousRendererForRuby(RenderElement &parent,
                                                                           DisplayType display) noexcept
    {
      auto style = CreateAnonymousStyleForRuby(parent.Style(), display);
      auto ruby =
        CreateRenderer<RenderInline>(RenderObject::Type::Inline, parent.GetDocument(), krys::move(style));
      ruby->InitializeStyle();
      return ruby;
    }
  }

  RenderElement &RenderTreeBuilder::Ruby::FindOrCreateParentForStyleBasedRubyChild(
    RenderElement &parent, const RenderObject &child, RenderObject *&beforeChild) noexcept
  {
    // There are no continuations, so the before child's ancestor is the parent.
    RenderElement *beforeChildAncestor = &parent;

    if (!child.IsRenderText() && child.Style().Display() == DisplayType::Ruby
        && beforeChildAncestor->Style().Display() == DisplayType::RubyBlock)
    {
      return *beforeChildAncestor;
    }

    if (beforeChildAncestor->Style().Display() == DisplayType::RubyBlock)
    {
      // See if we have an anonymous ruby box already.
      // FIXME: It should be the immediate child but continuations can break this assumption.
      for (auto *first = beforeChildAncestor->FirstChild(); first; first = first->FirstChildSlow())
      {
        if (!first->IsAnonymous())
        {
          // <ruby blockified><ruby> is valid and still requires construction of an anonymous inline ruby box.
          krys_debug_assert(first->Style().Display() == DisplayType::Ruby);
          break;
        }
        if (first->Style().Display() == DisplayType::Ruby)
        {
          return Downcast<RenderElement>(*first);
        }
      }
    }

    if (beforeChildAncestor->Style().Display() != DisplayType::Ruby)
    {
      auto rubyContainer = CreateAnonymousRendererForRuby(*beforeChildAncestor, DisplayType::Ruby);
      auto *newParent = rubyContainer.get();
      _builder.Attach(parent, krys::move(rubyContainer), beforeChild);
      beforeChild = nullptr;
      return *newParent;
    }

    if (!child.IsRenderText()
        && (child.Style().Display() == DisplayType::RubyBase
            || child.Style().Display() == DisplayType::RubyAnnotation))
    {
      return *beforeChildAncestor;
    }

    if (beforeChild && beforeChild->Parent()->Style().Display() == DisplayType::RubyBase)
    {
      return *beforeChild->Parent();
    }

    auto *previous = beforeChild ? beforeChild->PreviousSibling() : beforeChildAncestor->LastChild();
    if (previous && previous->Style().Display() == DisplayType::RubyBase)
    {
      beforeChild = nullptr;
      return Downcast<RenderElement>(*previous);
    }

    auto rubyBase = CreateAnonymousRendererForRuby(*beforeChildAncestor, DisplayType::RubyBase);
    rubyBase->InitializeStyle();
    auto *newParent = rubyBase.get();
    _builder.InlineBuilder().Attach(Downcast<RenderInline>(parent), krys::move(rubyBase), beforeChild);
    beforeChild = nullptr;
    return *newParent;
  }

  void RenderTreeBuilder::Ruby::AttachForStyleBasedRuby(RenderElement &parent, RenderPtr<RenderObject> child,
                                                        RenderObject *beforeChild) noexcept
  {
    if (parent.Style().Display() == DisplayType::RubyBlock)
    {
      krys_debug_assert(child->Style().Display() == DisplayType::Ruby);
      _builder.AttachToRenderElementInternal(parent, krys::move(child), beforeChild);
      return;
    }
    krys_debug_assert(parent.Style().Display() == DisplayType::Ruby);
    krys_debug_assert(child->Style().Display() == DisplayType::RubyBase
                      || child->Style().Display() == DisplayType::RubyAnnotation);

    while (beforeChild && beforeChild->Parent() && beforeChild->Parent() != &parent)
    {
      beforeChild = beforeChild->Parent();
    }

    if (child->Style().Display() == DisplayType::RubyAnnotation)
    {
      // Create an empty anonymous base if it is missing.
      auto *previous = beforeChild ? beforeChild->PreviousSibling() : parent.LastChild();
      if (!previous || previous->Style().Display() != DisplayType::RubyBase)
      {
        auto rubyBase = CreateAnonymousRendererForRuby(parent, DisplayType::RubyBase);
        _builder.AttachToRenderElementInternal(parent, krys::move(rubyBase), beforeChild);
      }
    }
    _builder.AttachToRenderElementInternal(parent, krys::move(child), beforeChild);
  }
}
