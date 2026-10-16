#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::dom
{
  class Document;
  class Element;
  class EventTarget;
}

namespace krys::boo::svg
{
  class SVGElement;
}

namespace krys::boo::style
{
  class Update;

  /// @brief Adjusts a computed style for its element and parent: blockification and inlinification, used
  /// z-index, overflow, text decorations in effect, and the element-specific rules.
  class Adjuster
  {
    CheckedRef<const dom::Document> _document;
    const RenderStyle &_parentStyle;
    const RenderStyle &_parentBoxStyle;
    RefPtr<dom::Element> _element;

  public:
    Adjuster(const dom::Document &document, const RenderStyle &parentStyle, const RenderStyle *parentBoxStyle,
             dom::Element *element) noexcept;

    static void AdjustFromBuilder(RenderStyle &style) noexcept;
    void Adjust(RenderStyle &style) const noexcept;

    static void AdjustFirstLetterStyle(RenderStyle &style) noexcept;
    static void AdjustFirstLineStyle(RenderStyle &style) noexcept;
    static void AdjustSVGElementStyle(RenderStyle &style, const svg::SVGElement &svgElement) noexcept;
    static bool AdjustEventListenerRegionTypesForRootStyle(RenderStyle &rootStyle,
                                                           const dom::Document &document) noexcept;
    /// @brief Propagates the writing mode and direction of the document element (or `<body>`) to the initial
    /// containing block and the document element's used style.
    static void PropagateToDocumentElementAndInitialContainingBlock(Update &update,
                                                                    const dom::Document &document) noexcept;
    KRYS_NODISCARD static UniquePtr<RenderStyle>
      RestoreUsedDocumentElementStyleToComputed(const RenderStyle &style) noexcept;

  private:
    void AdjustDisplayContentsStyle(RenderStyle &style) const noexcept;

    void AdjustThemeStyle(RenderStyle &style, const RenderStyle &parentStyle) const noexcept;

    KRYS_NODISCARD static EnumSet<EventListenerRegionType>
      ComputeEventListenerRegionTypes(const dom::Document &document, const RenderStyle &style,
                                      const dom::EventTarget &eventTarget,
                                      EnumSet<EventListenerRegionType> parentTypes) noexcept;
  };
}
