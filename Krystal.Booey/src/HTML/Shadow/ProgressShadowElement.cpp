#include "Krystal.Booey/HTML/Shadow/ProgressShadowElement.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/HTML/HTMLProgressElement.hpp"
#include "Krystal.Booey/Rendering/RenderProgress.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::html
{
  ProgressShadowElement::ProgressShadowElement(dom::Document &document) noexcept : HTMLDivElement(document)
  {
  }

  HTMLProgressElement *ProgressShadowElement::ProgressElement() const noexcept
  {
    return Downcast<HTMLProgressElement>(dom::TreeQueries::ShadowHost(*this));
  }

  bool ProgressShadowElement::RendererIsNeeded(const RenderStyle &style) noexcept
  {
    if (auto *progressRenderer = ProgressElement()->Renderer())
    {
      return !progressRenderer->Style().HasUsedAppearance() && HTMLDivElement::RendererIsNeeded(style);
    }
    return false;
  }

  ProgressInnerElement::ProgressInnerElement(dom::Document &document) noexcept
      : ProgressShadowElement(document)
  {
  }

  RenderPtr<RenderElement> ProgressInnerElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderProgress>(*this, krys::move(style));
  }

  bool ProgressInnerElement::RendererIsNeeded(const RenderStyle &style) noexcept
  {
    auto *progressRenderer = ProgressElement()->Renderer();
    return progressRenderer && !progressRenderer->Style().HasUsedAppearance()
           && HTMLDivElement::RendererIsNeeded(style);
  }

  ProgressBarElement::ProgressBarElement(dom::Document &document) noexcept : ProgressShadowElement(document)
  {
  }

  ProgressValueElement::ProgressValueElement(dom::Document &document) noexcept
      : ProgressShadowElement(document)
  {
  }

  void ProgressValueElement::SetInlineSizePercentage(double size) noexcept
  {
    (void)SetInlineStyleProperty(css::PropertyId::InlineSize, std::max(0.0, size), css::UnitType::Percentage);
  }

  Ref<ProgressInnerElement> ProgressInnerElement::Create(dom::Document &document) noexcept
  {
    auto result = CreateRef<ProgressInnerElement>(document);
    result->SetUserAgentPart(css::UserAgentParts::WebkitProgressInnerElement());
    return result;
  }

  Ref<ProgressBarElement> ProgressBarElement::Create(dom::Document &document) noexcept
  {
    auto result = CreateRef<ProgressBarElement>(document);
    result->SetUserAgentPart(css::UserAgentParts::WebkitProgressBar());
    return result;
  }

  Ref<ProgressValueElement> ProgressValueElement::Create(dom::Document &document) noexcept
  {
    auto result = CreateRef<ProgressValueElement>(document);
    result->SetUserAgentPart(css::UserAgentParts::WebkitProgressValue());
    return result;
  }
}
