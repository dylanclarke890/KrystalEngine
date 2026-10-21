#include "Krystal.Booey/Graphics/Filters/SourceGraphic.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<SourceGraphic> SourceGraphic::Create() noexcept
  {
    return AdoptRef(*new SourceGraphic());
  }

  SourceGraphic::SourceGraphic() noexcept : FilterEffect(FilterEffect::Type::SourceGraphic)
  {
  }

  EnumSet<FilterRenderingMode> SourceGraphic::SupportedFilterRenderingModes(
    EnumSet<FilterRenderingMode> preferredFilterRenderingModes) const noexcept
  {
    // What's drawn is the graphics context's own source: nothing to apply.
    return EnumSet<FilterRenderingMode> {FilterRenderingMode::GraphicsContext}
           & preferredFilterRenderingModes;
  }

  TextStream &SourceGraphic::ExternalRepresentation(TextStream &ts, FilterRepresentation) const noexcept
  {
    ts << Indent << u8"[SourceGraphic]\n";
    return ts;
  }
}
