#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  namespace dom
  {
    class Element;
  }

  class LocalFrame;
  class RenderObject;
  class RenderView;

  enum class RenderAsTextFlag : uint16
  {
    ShowAllLayers,        // Dump all layers, not just those that would paint.
    ShowLayerNesting,     // Annotate the layer lists.
    ShowCompositedLayers, // Show which layers are composited.
    ShowOverflow,         // Print layout and visual overflow.
    ShowSVGGeometry,      // Print additional geometry for SVG objects.
    ShowLayerFragments,   // Print additional info about fragmented RenderLayers
    ShowAddresses,        // Show layer and renderer addresses.
    ShowIDAndClass,       // Show id and class attributes
    PrintingMode,         // Dump the tree in printing mode.
    // Don't update layout, to make it safe to call showLayerTree() from the debugger inside layout or
    // painting code.
    DontUpdateLayout,
    ShowLayoutState, // Print the various 'needs layout' bits on renderers.
  };

  KRYS_NODISCARD TextStream CreateTextStream(const RenderView &view) noexcept;

  /// @brief The render tree and layer tree of @p frame's document, laid out, as text: the dump layout tests
  /// compare.
  KRYS_NODISCARD utf8_string ExternalRepresentation(LocalFrame *frame,
                                                    EnumSet<RenderAsTextFlag> behavior = {}) noexcept;
  /// @brief The render tree and layer tree under @p element's box, laid out, as text.
  KRYS_NODISCARD utf8_string ExternalRepresentation(dom::Element *element,
                                                    EnumSet<RenderAsTextFlag> behavior = {}) noexcept;
  void ExternalRepresentationForLocalFrame(TextStream &ts, LocalFrame &frame,
                                           EnumSet<RenderAsTextFlag> behavior = {}) noexcept;

  void Write(TextStream &ts, const RenderObject &renderer, EnumSet<RenderAsTextFlag> behavior = {}) noexcept;
  void WriteDebugInfo(TextStream &ts, const RenderObject &object,
                      EnumSet<RenderAsTextFlag> behavior = {}) noexcept;

  class RenderTreeAsText
  {
    // FIXME: This is a cheesy hack to allow easy access to RenderStyle colors.  It won't be needed if we
    // convert it to use visitedDependentColor instead. (This just involves rebaselining many results though,
    // so for now it's not being done).
  public:
    static void WriteRenderObject(TextStream &ts, const RenderObject &o,
                                  EnumSet<RenderAsTextFlag> behavior) noexcept;
  };

  // Helper function shared with SVGRenderTreeAsText
  KRYS_NODISCARD utf8_string QuoteAndEscapeNonPrintables(utf8_stringview s) noexcept;

  KRYS_NODISCARD utf8_string CounterValueForElement(dom::Element *element) noexcept;
  KRYS_NODISCARD utf8_string MarkerTextForListItem(dom::Element *element) noexcept;
}
