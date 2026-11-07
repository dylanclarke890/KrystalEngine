#include "Krystal.Booey/HTML/Shadow/DataListButtonElement.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Events/MouseEvent.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDataListElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/ResolvedStyle.hpp"

namespace krys::boo::html
{
  DataListButtonElement::DataListButtonElement(dom::Document &document, DataListButtonOwner &owner) noexcept
      : HTMLDivElement(document, dom::NodeFlags::HasCustomStyleResolveCallbacks), _owner(&owner)
  {
  }

  Ref<DataListButtonElement> DataListButtonElement::Create(dom::Document &document,
                                                           DataListButtonOwner &owner) noexcept
  {
    auto element = CreateRef<DataListButtonElement>(document, owner);
    element->SetUserAgentPart(css::UserAgentParts::WebkitListButton());
    return element;
  }

  void DataListButtonElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    if (!Is<dom::MouseEvent>(event))
    {
      if (!event.DefaultHandled())
      {
        HTMLDivElement::DefaultEventHandler(event);
      }
      return;
    }

    if (dom::IsAnyClick(event))
    {
      if (_owner)
      {
        _owner->DataListButtonElementWasClicked();
      }
      event.SetDefaultHandled();
    }

    if (!event.DefaultHandled())
    {
      HTMLDivElement::DefaultEventHandler(event);
    }
  }

  Maybe<style::UnadjustedStyle>
    DataListButtonElement::ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                                              const RenderStyle *) noexcept
  {
    auto elementStyle = ResolveStyle(resolutionContext);
    auto *input = DynamicDowncast<HTMLInputElement>(dom::TreeQueries::ShadowHost(*this));
    if (!input || !input->List())
    {
      elementStyle.Style->SetDisplay(DisplayType::None);
    }
    return elementStyle;
  }

  bool DataListButtonElement::IsDisabledFormControl() const noexcept
  {
    auto *host = dom::TreeQueries::ShadowHost(*this);
    return host && host->IsDisabledFormControl();
  }
}
