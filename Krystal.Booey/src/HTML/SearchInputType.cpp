#include "Krystal.Booey/HTML/SearchInputType.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/HTML/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::html
{
  SearchInputType::SearchInputType(HTMLInputElement &element) noexcept
      : BaseTextInputType(Type::Search, element)
  {
    krys_debug_assert(NeedsShadowSubtree());
  }

  bool SearchInputType::NeedsContainer() const noexcept
  {
    return true;
  }

  void SearchInputType::CreateShadowSubtree() noexcept
  {
    krys_debug_assert(NeedsShadowSubtree());
    krys_debug_assert(!_resultsButton);
    krys_debug_assert(!_cancelButton);
    krys_debug_assert(Element());

    TextFieldInputType::CreateShadowSubtree();
    auto &document = Element()->NodeDocument();
    auto *container = ContainerElement();
    auto *textWrapper = InnerBlockElement();
    krys_debug_assert(container);
    krys_debug_assert(textWrapper);

    // Without the `results` attribute (which isn't standard), the results button is only a decoration.
    auto resultsButton = SearchFieldResultsButtonElement::Create(document);
    (void)container->InsertBefore(*resultsButton, textWrapper);
    resultsButton->SetUserAgentPart(css::UserAgentParts::WebkitSearchDecoration());
    _resultsButton = krys::move(resultsButton);

    auto cancelButton = SearchFieldCancelButtonElement::Create(document);
    (void)container->InsertBefore(*cancelButton, textWrapper->NextSibling());
    _cancelButton = krys::move(cancelButton);
  }

  HTMLElement *SearchInputType::ResultsButtonElement() const noexcept
  {
    return _resultsButton.get();
  }

  HTMLElement *SearchInputType::CancelButtonElement() const noexcept
  {
    return _cancelButton.get();
  }

  InputType::ShouldCallBaseEventHandler
    SearchInputType::HandleKeydownEvent(dom::KeyboardEvent &event) noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    if (!element.IsMutable())
    {
      return TextFieldInputType::HandleKeydownEvent(event);
    }

    if (event.KeyIdentifier() == u8"U+001B")
    {
      (void)element.SetValue({}, TextFieldEventBehavior::DispatchChangeEvent);
      event.SetDefaultHandled();
      return ShouldCallBaseEventHandler::Yes;
    }
    return TextFieldInputType::HandleKeydownEvent(event);
  }

  void SearchInputType::RemoveShadowSubtree() noexcept
  {
    TextFieldInputType::RemoveShadowSubtree();
    _resultsButton = nullptr;
    _cancelButton = nullptr;
  }

  void SearchInputType::DidSetValueByUserEdit() noexcept
  {
    krys_debug_assert(Element());
    // The cancel button's visibility follows whether the field is empty (RenderSearchField's
    // updateCancelButtonVisibility), which its style resolution decides.
    if (_cancelButton)
    {
      _cancelButton->InvalidateStyleInternal();
    }

    TextFieldInputType::DidSetValueByUserEdit();
  }

  bool SearchInputType::SizeShouldIncludeDecoration(int32, int32 &preferredSize) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    preferredSize = static_cast<int32>(element.Size());
    // https://html.spec.whatwg.org/multipage/input.html#the-size-attribute
    // If the attribute is present, then its value must be parsed using the rules for parsing non-negative
    // integers, and if the result is a number greater than zero, then the user agent should ensure that at
    // least that many characters are visible.
    auto size = element.GetAttribute(u8"size");
    if (!size)
    {
      return false;
    }
    if (auto parsedSize = Numbers::ParseNonNegativeInteger(*size); parsedSize.Success())
    {
      return static_cast<int32>(parsedSize.Value) == preferredSize;
    }
    return false;
  }

  float SearchInputType::DecorationWidth(float) const noexcept
  {
    float width = 0;
    if (_resultsButton && _resultsButton->ExistingComputedStyle())
    {
      // FIXME: Document what invariant holds to allow only using fixed logical widths?
      auto *renderStyle = _resultsButton->ExistingComputedStyle();
      if (auto fixedLogicalWidth = renderStyle->LogicalWidth().TryFixed())
      {
        width += fixedLogicalWidth->ResolveZoom(renderStyle->UsedZoomForLength());
      }
    }
    if (_cancelButton && _cancelButton->ExistingComputedStyle())
    {
      // FIXME: Document what invariant holds to allow only using fixed logical widths?
      auto *renderStyle = _cancelButton->ExistingComputedStyle();
      if (auto fixedLogicalWidth = renderStyle->LogicalWidth().TryFixed())
      {
        width += fixedLogicalWidth->ResolveZoom(renderStyle->UsedZoomForLength());
      }
    }
    return width;
  }

  void SearchInputType::SetValue(dom::DOMStringView sanitizedValue, bool valueChanged,
                                 TextFieldEventBehavior eventBehavior,
                                 TextControlSetValueSelection selection) noexcept
  {
    bool emptinessChanged = valueChanged && sanitizedValue.empty() != Element()->Value().empty();

    BaseTextInputType::SetValue(sanitizedValue, valueChanged, eventBehavior, selection);

    if (!emptinessChanged)
    {
      return;
    }

    if (_cancelButton)
    {
      _cancelButton->InvalidateStyleInternal();
    }
  }
}
