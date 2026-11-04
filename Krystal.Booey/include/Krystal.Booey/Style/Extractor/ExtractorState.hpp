#pragma once

#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class RenderElement;
  class RenderStyle;
}

namespace krys::boo::css
{
  class ValuePool;
}

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::style
{
  /// @brief What a computed value is extracted from.
  struct ExtractorState
  {
    enum class PropertyValueType : bool
    {
      Resolved,
      Computed
    };
    PropertyValueType ValueType;

    const RenderStyle &Style;

    Ref<dom::Element> Element;
    const Maybe<style::PseudoElementIdentifier> &PseudoElementIdentifier;

    const RenderElement *Renderer;

    bool AllowVisitedStyle;

    css::ValuePool &Pool;
  };
}
