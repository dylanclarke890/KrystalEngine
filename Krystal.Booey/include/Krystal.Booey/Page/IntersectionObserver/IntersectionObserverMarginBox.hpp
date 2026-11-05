#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo
{
  // https://w3c.github.io/IntersectionObserver/#dom-intersectionobserverinit-rootmargin
  // https://w3c.github.io/IntersectionObserver/#dom-intersectionobserverinit-scrollmargin

  struct IntersectionObserverMarginEdge : style::LengthWrapperBase<style::LengthPercentage<>>
  {
    using Base::Base;
  };

  using IntersectionObserverMarginBox =
    css::MinimallySerialisingSpaceSeparatedRectEdges<IntersectionObserverMarginEdge>;
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::IntersectionObserverMarginEdge)
