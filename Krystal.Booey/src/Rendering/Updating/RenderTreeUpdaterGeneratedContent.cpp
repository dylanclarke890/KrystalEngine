#include "Krystal.Booey/Rendering/Updating/RenderTreeUpdaterGeneratedContent.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"
#include "Krystal.Booey/Style/Change.hpp"
#include "Krystal.Booey/Style/Update.hpp"

namespace krys::boo
{
  RenderTreeUpdater::GeneratedContent::GeneratedContent(RenderTreeUpdater &updater) noexcept
      : _updater(updater)
  {
  }

  void RenderTreeUpdater::GeneratedContent::UpdateBeforeOrAfterPseudoElement(
    dom::Element &current, const style::ElementUpdate &elementUpdate,
    PseudoElementType pseudoElementType) noexcept
  {
    krys_debug_assert(pseudoElementType == PseudoElementType::Before
                      || pseudoElementType == PseudoElementType::After);

    auto *pseudoElement = pseudoElementType == PseudoElementType::Before ? current.BeforePseudoElement()
                                                                         : current.AfterPseudoElement();

    if (auto *renderer = pseudoElement ? pseudoElement->Renderer() : nullptr)
    {
      _updater.GetRenderTreePosition().InvalidateNextSibling(*renderer);
    }

    auto *updateStyle =
      elementUpdate.Style ? elementUpdate.Style->GetCachedPseudoStyle({pseudoElementType}) : nullptr;

    // FIXME: If we end up losing a previous pseudo because the style got removed, cancel any animations that
    // were on it (cancelStyleOriginatedAnimations), and keep pseudo-elements that animations need
    // (needsPseudoElementForAnimation), once there are animations.

    krys_debug_assert(!Is<dom::PseudoElement>(current));
    if (!NeedsPseudoElement(updateStyle))
    {
      if (pseudoElement)
      {
        if (pseudoElementType == PseudoElementType::Before)
        {
          RemoveBeforePseudoElement(current, *_updater._builder);
        }
        else
        {
          RemoveAfterPseudoElement(current, *_updater._builder);
        }
      }
      return;
    }

    if (!updateStyle)
    {
      return;
    }

    auto *existingStyle = pseudoElement ? pseudoElement->RenderOrDisplayContentsStyle() : nullptr;

    auto styleChanges = existingStyle ? style::DetermineChanges(*updateStyle, *existingStyle)
                                      : EnumSet<style::Change> {style::Change::Renderer};
    if (styleChanges.IsEmpty())
    {
      return;
    }

    pseudoElement = &current.EnsurePseudoElement(pseudoElementType);

    if (updateStyle->Display() == DisplayType::Contents)
    {
      // For display:contents we create an inline wrapper that inherits its
      // style from the display:contents style.
      auto contentsStyle = RenderStyle::CreatePtr();
      contentsStyle->SetPseudoElementIdentifier(style::PseudoElementIdentifier {pseudoElementType});
      contentsStyle->InheritFrom(*updateStyle);
      contentsStyle->CopyContentFrom(*updateStyle);
      contentsStyle->CopyPseudoElementsFrom(*updateStyle);

      style::ElementUpdate contentsUpdate {krys::move(contentsStyle), styleChanges,
                                           elementUpdate.RecompositeLayer};
      _updater.UpdateElementRenderer(*pseudoElement, contentsUpdate);
      auto pseudoElementUpdateStyle = RenderStyle::CloneIncludingPseudoElements(*updateStyle);
      pseudoElement->StoreDisplayContentsOrNoneStyle(
        CreateUnique<RenderStyle>(krys::move(pseudoElementUpdateStyle)));
    }
    else
    {
      auto pseudoElementUpdateStyle = RenderStyle::CloneIncludingPseudoElements(*updateStyle);
      style::ElementUpdate pseudoElementUpdate {
        CreateUnique<RenderStyle>(krys::move(pseudoElementUpdateStyle)), styleChanges,
        elementUpdate.RecompositeLayer};
      _updater.UpdateElementRenderer(*pseudoElement, pseudoElementUpdate);
      if (updateStyle->Display() == DisplayType::None)
      {
        auto displayNoneStyle = RenderStyle::CloneIncludingPseudoElements(*updateStyle);
        pseudoElement->StoreDisplayContentsOrNoneStyle(
          CreateUnique<RenderStyle>(krys::move(displayNoneStyle)));
      }
      else
      {
        pseudoElement->ClearDisplayContentsOrNoneStyle();
      }
    }

    auto *pseudoElementRenderer = pseudoElement->Renderer();
    if (!pseudoElementRenderer)
    {
      return;
    }

    // FIXME: The content renderers (createContentRenderers when the renderer changes, else
    // updateStyleForContentRenderers), quotes and RenderTreeBuilder::updateAfterDescendants, once there's a
    // render tree.
  }

  bool RenderTreeUpdater::GeneratedContent::NeedsPseudoElement(const RenderStyle *style) noexcept
  {
    if (!style)
    {
      return false;
    }
    if (!_updater.GetRenderTreePosition().Parent().CanHaveGeneratedChildren())
    {
      return false;
    }
    if (!PseudoElementRendererIsNeeded(style))
    {
      return false;
    }
    return true;
  }

  void RenderTreeUpdater::GeneratedContent::RemoveBeforePseudoElement(dom::Element &element,
                                                                      RenderTreeBuilder &builder) noexcept
  {
    auto *pseudoElement = element.BeforePseudoElement();
    if (!pseudoElement)
    {
      return;
    }
    TearDownRenderers(*pseudoElement, TeardownType::Full, builder);
    element.ClearBeforePseudoElement();
  }

  void RenderTreeUpdater::GeneratedContent::RemoveAfterPseudoElement(dom::Element &element,
                                                                     RenderTreeBuilder &builder) noexcept
  {
    auto *pseudoElement = element.AfterPseudoElement();
    if (!pseudoElement)
    {
      return;
    }
    TearDownRenderers(*pseudoElement, TeardownType::Full, builder);
    element.ClearAfterPseudoElement();
  }
}
