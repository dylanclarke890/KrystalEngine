#include "Krystal.Booey/HTML/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/BeforeTextInsertedEvent.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Rendering/RenderTextControl.hpp"
#include "Krystal.Booey/Rendering/RenderTextControlSingleLine.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/ResolvedStyle.hpp"

namespace krys::boo::html
{
#pragma region TextControlInnerContainer

  TextControlInnerContainer::TextControlInnerContainer(dom::Document &document) noexcept
      : HTMLDivElement(document, dom::NodeFlags::HasCustomStyleResolveCallbacks)
  {
  }

  Ref<TextControlInnerContainer> TextControlInnerContainer::Create(dom::Document &document) noexcept
  {
    return CreateRef<TextControlInnerContainer>(document);
  }

  RenderPtr<RenderElement> TextControlInnerContainer::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderTextControlInnerContainer>(*this, krys::move(style));
  }

  Maybe<style::UnadjustedStyle>
    TextControlInnerContainer::ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                                                  const RenderStyle *shadowHostStyle) noexcept
  {
    // FIXME: Strong password fields' container wraps and clips (isStrongPasswordTextField), with autofill.
    auto style = ResolveStyle(resolutionContext);
    if (shadowHostStyle)
    {
      RenderTheme::Singleton().AdjustTextControlInnerContainerStyle(*style.Style, *shadowHostStyle,
                                                                    dom::TreeQueries::ShadowHost(*this));
    }
    return style;
  }

#pragma endregion

#pragma region TextControlInnerElement

  TextControlInnerElement::TextControlInnerElement(dom::Document &document) noexcept
      : HTMLDivElement(document, dom::NodeFlags::HasCustomStyleResolveCallbacks)
  {
  }

  Ref<TextControlInnerElement> TextControlInnerElement::Create(dom::Document &document) noexcept
  {
    return CreateRef<TextControlInnerElement>(document);
  }

  Maybe<style::UnadjustedStyle>
    TextControlInnerElement::ResolveCustomStyle(const style::ResolutionContext &,
                                                const RenderStyle *shadowHostStyle) noexcept
  {
    auto newStyle = RenderStyle::CreatePtr();
    newStyle->InheritFrom(*shadowHostStyle);
    newStyle->SetFlexGrow(style::FlexGrow {1});

    // Needed for correct shrinking.
    newStyle->SetLogicalMinWidth(style::MinimumSize::Fixed {0});

    newStyle->SetDisplay(DisplayType::Block);
    newStyle->SetDirection(TextDirection::LTR);
    // We don't want the shadow DOM to be editable, so we set this block to read-only in case the input itself
    // is editable.
    newStyle->SetUserModify(UserModify::ReadOnly);

    // FIXME: Strong password fields' inner block (isStrongPasswordTextField), with autofill.

    return style::UnadjustedStyle {krys::move(newStyle)};
  }

#pragma endregion

#pragma region TextControlInnerTextElement

  TextControlInnerTextElement::TextControlInnerTextElement(dom::Document &document) noexcept
      : HTMLDivElement(document, dom::NodeFlags::HasCustomStyleResolveCallbacks)
  {
  }

  Ref<TextControlInnerTextElement> TextControlInnerTextElement::Create(dom::Document &document,
                                                                       bool isEditable) noexcept
  {
    auto result = CreateRef<TextControlInnerTextElement>(document);
    result->UpdateInnerTextElementEditabilityImpl(isEditable);
    return result;
  }

  void TextControlInnerTextElement::UpdateInnerTextElementEditabilityImpl(bool isEditable) noexcept
  {
    (void)SetAttribute(u8"contenteditable", dom::DOMString(isEditable ? u8"plaintext-only" : u8"false"));
  }

  void TextControlInnerTextElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    // FIXME: In the future, we should add a way to have default event listeners.
    // Then we would add one to the text field's inner div, and we wouldn't need this subclass.
    // Or possibly we could just use a normal event listener.
    if (event.IsBeforeTextInsertedEvent())
    {
      // A TextControlInnerTextElement can have no host if its been detached,
      // but kept alive by an EditCommand. In this case, an undo/redo can
      // cause events to be sent to the TextControlInnerTextElement. To
      // prevent an infinite loop, we must check for this case before sending
      // the event up the chain.
      if (RefPtr<dom::Element> host = ShareRefPtr(dom::TreeQueries::ShadowHost(*this)))
      {
        host->DefaultEventHandler(event);
      }
    }
    if (!event.DefaultHandled())
    {
      HTMLDivElement::DefaultEventHandler(event);
    }
  }

  RenderPtr<RenderElement> TextControlInnerTextElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderTextControlInnerBlock>(*this, krys::move(style));
  }

  RenderTextControlInnerBlock *TextControlInnerTextElement::Renderer() const noexcept
  {
    return Downcast<RenderTextControlInnerBlock>(HTMLDivElement::Renderer());
  }

  Maybe<style::UnadjustedStyle>
    TextControlInnerTextElement::ResolveCustomStyle(const style::ResolutionContext &,
                                                    const RenderStyle *shadowHostStyle) noexcept
  {
    auto &shadowHost = *dom::TreeQueries::ShadowHost(*this);
    auto style = Downcast<HTMLTextFormControlElement>(shadowHost).CreateInnerTextStyle(*shadowHostStyle);

    RenderTheme::Singleton().AdjustTextControlInnerTextStyle(style, *shadowHostStyle, &shadowHost);

    return style::UnadjustedStyle {CreateUnique<RenderStyle>(krys::move(style))};
  }

#pragma endregion

#pragma region TextControlPlaceholderElement

  TextControlPlaceholderElement::TextControlPlaceholderElement(dom::Document &document) noexcept
      : HTMLDivElement(document, dom::NodeFlags::HasCustomStyleResolveCallbacks)
  {
  }

  Ref<TextControlPlaceholderElement> TextControlPlaceholderElement::Create(dom::Document &document) noexcept
  {
    auto element = CreateRef<TextControlPlaceholderElement>(document);
    element->SetUserAgentPart(css::UserAgentParts::Placeholder());
    return element;
  }

  Maybe<style::UnadjustedStyle>
    TextControlPlaceholderElement::ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                                                      const RenderStyle *shadowHostStyle) noexcept
  {
    auto style = ResolveStyle(resolutionContext);

    auto &controlElement =
      Downcast<HTMLTextFormControlElement>(*dom::TreeQueries::ContainingShadowRoot(*this)->Host());
    auto &styleStyle = *style.Style;
    styleStyle.SetDisplay(controlElement.IsPlaceholderVisible() ? DisplayType::Block : DisplayType::None);
    // The user agent sheet can't say this (`-webkit-text-security` isn't parsed), so a password field's mask
    // is turned off here.
    styleStyle.SetTextSecurity(TextSecurity::None);

    if (auto *inputElement = DynamicDowncast<HTMLInputElement>(controlElement))
    {
      styleStyle.SetTextOverflow(inputElement->ShouldTruncateText(*shadowHostStyle) ? TextOverflow::Ellipsis
                                                                                    : TextOverflow::Clip);
      styleStyle.SetPaddingTop(style::PaddingEdge::Fixed {0});
      styleStyle.SetPaddingBottom(style::PaddingEdge::Fixed {0});
    }

    RenderTheme::Singleton().AdjustTextControlInnerPlaceholderStyle(styleStyle, *shadowHostStyle,
                                                                    &controlElement);

    return style;
  }

#pragma endregion

#pragma region SearchFieldResultsButtonElement

  namespace
  {
    KRYS_NODISCARD bool
      SearchFieldStyleHasExplicitlySpecifiedTextFieldAppearance(const RenderStyle &style) noexcept
    {
      auto appearance = style.Appearance();
      return appearance == StyleAppearance::TextField && appearance == style.UsedAppearance();
    }
  }

  SearchFieldResultsButtonElement::SearchFieldResultsButtonElement(dom::Document &document) noexcept
      : HTMLDivElement(document, dom::NodeFlags::HasCustomStyleResolveCallbacks)
  {
  }

  Ref<SearchFieldResultsButtonElement>
    SearchFieldResultsButtonElement::Create(dom::Document &document) noexcept
  {
    return CreateRef<SearchFieldResultsButtonElement>(document);
  }

  Maybe<style::UnadjustedStyle>
    SearchFieldResultsButtonElement::ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                                                        const RenderStyle *shadowHostStyle) noexcept
  {
    _canAdjustStyleForAppearance = true;

    if (!shadowHostStyle)
    {
      return null;
    }

    if (SearchFieldStyleHasExplicitlySpecifiedTextFieldAppearance(*shadowHostStyle))
    {
      auto elementStyle = ResolveStyle(resolutionContext);
      elementStyle.Style->SetDisplay(DisplayType::None);
      return elementStyle;
    }

    // By default, input[type=search] can use either the searchfield or textfield appearance depending
    // on the platform and writing mode. Only adjust the style when that default is used.
    auto usedAppearance = shadowHostStyle->UsedAppearance();
    if (usedAppearance != StyleAppearance::SearchField && usedAppearance != StyleAppearance::TextField)
    {
      _canAdjustStyleForAppearance = false;
      return ResolveStyle(resolutionContext);
    }

    return null;
  }

  void SearchFieldResultsButtonElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    // On mousedown, bring up a menu, if needed
    if (RefPtr<HTMLInputElement> input =
          ShareRefPtr(DynamicDowncast<HTMLInputElement>(dom::TreeQueries::ShadowHost(*this))))
    {
      auto *mouseEvent = DynamicDowncast<dom::MouseEvent>(event);
      if (event.Type() == EventNames::MouseDown && mouseEvent && mouseEvent->Button() == 0)
      {
        input->Focus();
        input->Select();
        // FIXME: Showing and hiding the recent searches popup (RenderSearchField), with recent searches.
        event.SetDefaultHandled();
      }
    }

    if (!event.DefaultHandled())
    {
      HTMLDivElement::DefaultEventHandler(event);
    }
  }

#pragma endregion

#pragma region SearchFieldCancelButtonElement

  SearchFieldCancelButtonElement::SearchFieldCancelButtonElement(dom::Document &document) noexcept
      : HTMLDivElement(document, dom::NodeFlags::HasCustomStyleResolveCallbacks)
  {
  }

  Ref<SearchFieldCancelButtonElement> SearchFieldCancelButtonElement::Create(dom::Document &document) noexcept
  {
    auto element = CreateRef<SearchFieldCancelButtonElement>(document);
    element->SetUserAgentPart(css::UserAgentParts::WebkitSearchCancelButton());
    // FIXME: The accessibility label (aria-label), with accessibility.
    (void)element->SetAttribute(u8"role", dom::DOMString(u8"button"));
    return element;
  }

  Maybe<style::UnadjustedStyle>
    SearchFieldCancelButtonElement::ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                                                       const RenderStyle *shadowHostStyle) noexcept
  {
    auto elementStyle = ResolveStyle(resolutionContext);
    auto &inputElement = Downcast<HTMLInputElement>(*dom::TreeQueries::ShadowHost(*this));
    elementStyle.Style->SetVisibility(elementStyle.Style->UsedVisibility() == Visibility::Hidden
                                          || inputElement.Value().empty()
                                        ? Visibility::Hidden
                                        : Visibility::Visible);

    if (shadowHostStyle && SearchFieldStyleHasExplicitlySpecifiedTextFieldAppearance(*shadowHostStyle))
    {
      elementStyle.Style->SetDisplay(DisplayType::None);
    }

    return elementStyle;
  }

  void SearchFieldCancelButtonElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    RefPtr<HTMLInputElement> input =
      ShareRefPtr(DynamicDowncast<HTMLInputElement>(dom::TreeQueries::ShadowHost(*this)));
    if (!input || !input->IsMutable())
    {
      if (!event.DefaultHandled())
      {
        HTMLDivElement::DefaultEventHandler(event);
      }
      return;
    }

    if (auto *mouseEvent = DynamicDowncast<dom::MouseEvent>(event);
        event.Type() == EventNames::MouseDown && mouseEvent && mouseEvent->Button() == 0)
    {
      input->Focus();
      input->Select();
      event.SetDefaultHandled();
    }

    if (dom::IsAnyClick(event))
    {
      (void)input->SetValue({}, TextFieldEventBehavior::DispatchChangeEvent);
      event.SetDefaultHandled();
    }

    if (!event.DefaultHandled())
    {
      HTMLDivElement::DefaultEventHandler(event);
    }
  }

#pragma endregion
}
