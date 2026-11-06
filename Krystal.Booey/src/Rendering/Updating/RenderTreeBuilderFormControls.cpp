#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderFormControls.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderButton.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderMenuList.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderBlock.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeUpdaterGeneratedContent.hpp"

namespace krys::boo
{
  RenderTreeBuilder::FormControls::FormControls(RenderTreeBuilder &builder) noexcept : _builder(builder)
  {
  }

  void RenderTreeBuilder::FormControls::Attach(RenderButton &parent, RenderPtr<RenderObject> child,
                                               RenderObject *beforeChild) noexcept
  {
    _builder.BlockBuilder().Attach(FindOrCreateParentForChild(parent), krys::move(child), beforeChild);
  }

  void RenderTreeBuilder::FormControls::Attach(RenderMenuList &parent, RenderPtr<RenderObject> child,
                                               RenderObject *beforeChild) noexcept
  {
    auto &newChild = *child.get();
    _builder.BlockBuilder().Attach(FindOrCreateParentForChild(parent), krys::move(child), beforeChild);
    parent.DidAttachChild(newChild, beforeChild);
  }

  RenderPtr<RenderObject> RenderTreeBuilder::FormControls::Detach(RenderMenuList &parent, RenderObject &child,
                                                                  WillBeDestroyed willBeDestroyed) noexcept
  {
    auto *innerRenderer = parent.InnerRenderer();
    if (!innerRenderer || &child == innerRenderer)
    {
      return _builder.BlockBuilder().Detach(parent, child, willBeDestroyed, CanCollapseAnonymousBlock::Yes);
    }
    return _builder.Detach(*innerRenderer, child, willBeDestroyed);
  }

  RenderPtr<RenderObject> RenderTreeBuilder::FormControls::Detach(RenderButton &parent, RenderObject &child,
                                                                  WillBeDestroyed willBeDestroyed) noexcept
  {
    auto *innerRenderer = parent.InnerRenderer();
    if (!innerRenderer || &child == innerRenderer || child.Parent() == &parent)
    {
      krys_debug_assert(&child == innerRenderer || !innerRenderer);
      return _builder.BlockBuilder().Detach(parent, child, willBeDestroyed, CanCollapseAnonymousBlock::Yes);
    }
    return _builder.Detach(*innerRenderer, child, willBeDestroyed);
  }

  RenderBlock &RenderTreeBuilder::FormControls::FindOrCreateParentForChild(RenderButton &parent) noexcept
  {
    auto *innerRenderer = parent.InnerRenderer();
    if (innerRenderer)
    {
      return *innerRenderer;
    }

    auto wrapper = Block::CreateAnonymousBlockWithStyle(parent.GetDocument(), parent.Style());
    innerRenderer = wrapper.get();
    _builder.BlockBuilder().Attach(parent, krys::move(wrapper), nullptr);
    parent.SetInnerRenderer(*innerRenderer);
    return *innerRenderer;
  }

  RenderBlock &RenderTreeBuilder::FormControls::FindOrCreateParentForChild(RenderMenuList &parent) noexcept
  {
    auto *innerRenderer = parent.InnerRenderer();
    if (innerRenderer)
    {
      return *innerRenderer;
    }

    auto wrapper = Block::CreateAnonymousBlockWithStyle(parent.GetDocument(), parent.Style());
    innerRenderer = wrapper.get();
    _builder.BlockBuilder().Attach(parent, krys::move(wrapper), nullptr);
    parent.SetInnerRenderer(*innerRenderer);
    return *innerRenderer;
  }

  void RenderTreeBuilder::FormControls::UpdateAfterDescendants(RenderElement &renderer) noexcept
  {
    auto *element = renderer.GetElement();
    if (!element)
    {
      return;
    }

    auto type = PseudoElementType::Checkmark;
    bool usesBaseAppearance = false;
    if (auto *select = DynamicDowncast<html::HTMLSelectElement>(*element))
    {
      type = PseudoElementType::PickerIcon;
      usesBaseAppearance = select->ContentsUseBaseAppearance();
    }
    else if (auto *option = DynamicDowncast<html::HTMLOptionElement>(*element))
    {
      auto *ownerSelect = option->OwnerSelectElement();
      usesBaseAppearance = ownerSelect && ownerSelect->ContentsUseBaseAppearance();
    }
    else
    {
      return;
    }

    for (auto *child = renderer.FirstChild(); child; child = child->NextSibling())
    {
      if (auto *childElement = DynamicDowncast<RenderElement>(*child);
          childElement && childElement->IsAnonymous() && childElement->Style().GetPseudoElementType() == type)
      {
        _builder.Destroy(*childElement);
        break;
      }
    }

    const auto *style =
      usesBaseAppearance ? renderer.GetCachedPseudoStyle({type}, &renderer.Style()) : nullptr;
    if (!style || style->Display() == DisplayType::None || style->GetContent().TryData() == nullptr)
    {
      return;
    }

    auto generated = boo::CreateRenderer<RenderBlockFlow>(RenderObject::Type::BlockFlow,
                                                          renderer.GetDocument(), RenderStyle::Clone(*style));
    generated->InitializeStyle();
    auto &generatedRenderer = *generated;
    // A checkmark comes before the option's contents, and an icon after the select's.
    _builder.Attach(renderer, krys::move(generated),
                    type == PseudoElementType::Checkmark ? renderer.FirstChild() : nullptr);
    RenderTreeUpdater::CreateContentRenderers(_builder, generatedRenderer, generatedRenderer.Style());
  }
}
