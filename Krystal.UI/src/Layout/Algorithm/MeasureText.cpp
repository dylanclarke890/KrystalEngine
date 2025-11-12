#include "Krystal.UI/Layout/Algorithm/MeasureText.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.Gfx/Resources/Font.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI/Document.hpp"
#include "Krystal.UI/Elements/Element.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"

namespace Krys::UI
{
  Size TextMeasureFunc(NodeConstRef node, float width, MeasureMode widthMode, float height,
                       MeasureMode heightMode)
  {
    auto &element = *static_cast<Element *>(NodeGetContext(node));
    auto &document = *static_cast<Document *>(NodeGetContext(element.LayoutNode));
    auto &context = *static_cast<Gfx::IContext *>(ConfigGetContext(element.LayoutConfig));
    auto &text = context.Strings().Get(element.GetText());
    auto fontDesc = document.ElementGetFontDesc(element.Handle);
    auto font = context.Fonts().Get(fontDesc);
    const auto &characters = context.Fonts().GetCharacterMap(font);
    const auto &metrics = context.Fonts().GetMetrics(font);
    float lineHeight = metrics.Height;

    float maxWidth = 0.0f;
    float currentLineWidth = 0.0f;
    float totalHeight = lineHeight;

    for (char c : text)
    {
      if (c == '\n')
      {
        maxWidth = std::max(maxWidth, currentLineWidth);
        totalHeight += lineHeight;
        currentLineWidth = 0.0f;
        continue;
      }

      const auto &ch = characters.at(Gfx::Codepoint(c));

      // Wrap if constrained and over width
      if (widthMode != MeasureMode::Undefined && currentLineWidth + ch.Advance > width)
      {
        maxWidth = std::max(maxWidth, currentLineWidth);
        totalHeight += lineHeight;
        currentLineWidth = 0.0f;
      }

      currentLineWidth += ch.Advance;
    }

    maxWidth = std::max(maxWidth, currentLineWidth);

    return {maxWidth, totalHeight};
  }
}