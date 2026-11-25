#include "Krystal.Booey/Accessibility/AXProgressIndicator.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMeterElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLProgressElement.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"

namespace krys::boo
{
  Ref<AXProgressIndicator> AXProgressIndicator::Create(AXId id, RenderObject &renderer,
                                                       AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AXProgressIndicator(id, &renderer, renderer.GetNode(), cache));
  }

  Ref<AXProgressIndicator> AXProgressIndicator::Create(AXId id, Element &element,
                                                       AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AXProgressIndicator(id, nullptr, &element, cache));
  }

  AXProgressIndicator::AXProgressIndicator(AXId id, RenderObject *renderer, Node *node,
                                           AXObjectCache &cache) noexcept
      : AXRenderObject(id, renderer, node, cache)
  {
  }

  AXRole AXProgressIndicator::DetermineAccessibilityRole() noexcept
  {
    return MeterElement() ? AXRole::Meter : AXRole::ProgressIndicator;
  }

  bool AXProgressIndicator::IsIndeterminate() const noexcept
  {
    // A progress bar with no value is indeterminate (https://html.spec.whatwg.org/multipage/form-elements.html#the-progress-element).
    auto *progress = ProgressElement();
    return progress && !progress->HasAttribute(u8"value");
  }

  float AXProgressIndicator::ValueForRange() const noexcept
  {
    // An indeterminate progress bar has a position of -1
    // (https://html.spec.whatwg.org/multipage/form-elements.html#dom-progress-position).
    if (auto *progress = ProgressElement())
    {
      return progress->Position() >= 0 ? static_cast<float>(progress->Value()) : 0;
    }
    if (auto *meter = MeterElement())
    {
      return static_cast<float>(meter->Value());
    }
    return 0;
  }

  float AXProgressIndicator::MaxValueForRange() const noexcept
  {
    if (auto *progress = ProgressElement())
    {
      return static_cast<float>(progress->Max());
    }
    if (auto *meter = MeterElement())
    {
      return static_cast<float>(meter->Max());
    }
    return 0;
  }

  float AXProgressIndicator::MinValueForRange() const noexcept
  {
    auto *meter = MeterElement();
    return meter ? static_cast<float>(meter->Min()) : 0;
  }

  DOMString AXProgressIndicator::ValueDescription() const noexcept
  {
    auto description = AXRenderObject::ValueDescription();
    if (description.empty())
    {
      if (auto *meter = MeterElement())
      {
        return meter->TextContent().value_or(DOMString {});
      }
    }
    return description;
  }

  HTMLProgressElement *AXProgressIndicator::ProgressElement() const noexcept
  {
    return DynamicDowncast<HTMLProgressElement>(GetNode());
  }

  HTMLMeterElement *AXProgressIndicator::MeterElement() const noexcept
  {
    return DynamicDowncast<HTMLMeterElement>(GetNode());
  }
}
