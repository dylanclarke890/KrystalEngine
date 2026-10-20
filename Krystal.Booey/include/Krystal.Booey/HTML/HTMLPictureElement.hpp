#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  class HTMLSourceElement;

  /// @see https://html.spec.whatwg.org/#the-picture-element
  class HTMLPictureElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLPictureElement);

  public:
    HTMLPictureElement(dom::Document &document) noexcept;

    /// @brief Selects the source of each image child again, after a relevant mutation of its sources.
    void SourcesChanged() noexcept;

    /// @brief Updates the presentational hints of the images whose selected source is @p sourceElement.
    void SourceDimensionAttributesChanged(const HTMLSourceElement &sourceElement) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLPictureElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLPictureElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::html::HTMLElement>(target);
    return element && element->IsHTMLPictureElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
