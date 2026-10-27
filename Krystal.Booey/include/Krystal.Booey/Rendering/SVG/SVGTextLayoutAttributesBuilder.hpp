#pragma once

#include "Krystal.Booey/Rendering/SVG/SVGTextLayoutAttributes.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextMetricsBuilder.hpp"
#include "Krystal.Booey/SVG/SVGTextPositioningElement.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::svg
{
  class SVGTextPositioningElement;
}

namespace krys::boo
{
  class RenderBoxModelObject;
  class RenderSVGInlineText;
  class RenderSVGText;

  /// @brief Performs the first layout phase for SVG text.
  ///
  /// It extracts the x/y/dx/dy/rotate values from the SVGTextPositioningElements in the DOM. These values are
  /// propagated to the corresponding RenderSVGInlineText renderers. The first layout phase only extracts the
  /// relevant information needed in the line layout to create the inline box tree based on text chunk
  /// boundaries & bidi information. The second layout phase is carried out by SVGTextLayoutEngine.
  class SVGTextLayoutAttributesBuilder
  {
    KRYS_NON_COPYABLE(SVGTextLayoutAttributesBuilder);

    struct TextPosition
    {
      explicit TextPosition(svg::SVGTextPositioningElement *newElement, uint32 newStart = 0,
                            uint32 newLength = 0) noexcept;

      CheckedPtr<svg::SVGTextPositioningElement> Element;
      /// @brief In characters, from the start of the whole text.
      uint32 Start;
      /// @brief In characters.
      uint32 Length;
    };

  private:
    /// @brief In characters.
    uint32 _textLength {0};
    List<TextPosition> _textPositions;
    SVGCharacterDataMap _characterDataMap;
    SVGTextMetricsBuilder _metricsBuilder;

  public:
    SVGTextLayoutAttributesBuilder() noexcept;
    ~SVGTextLayoutAttributesBuilder() noexcept;

    bool BuildLayoutAttributesForForSubtree(RenderSVGText &) noexcept;
    void BuildLayoutAttributesForTextRenderer(RenderSVGInlineText &) noexcept;

    void RebuildMetricsForSubtree(RenderSVGText &) noexcept;

    /// @brief Invoked whenever the underlying DOM tree changes, so that the text positions are rebuilt.
    void ClearTextPositioningElements() noexcept
    {
      _textPositions.clear();
    }
    KRYS_NODISCARD uint32 NumberOfTextPositioningElements() const noexcept
    {
      return static_cast<uint32>(_textPositions.size());
    }

  private:
    void BuildCharacterDataMap(RenderSVGText &) noexcept;
    void CollectTextPositioningElements(RenderBoxModelObject &, bool &lastCharacterWasSpace) noexcept;
    void FillCharacterDataMap(const TextPosition &) noexcept;
  };
}
