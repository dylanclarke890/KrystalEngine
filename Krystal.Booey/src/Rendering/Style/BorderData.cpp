#include "Krystal.Booey/Rendering/Style/BorderData.hpp"

namespace krys::boo
{
  BorderData::BorderData() noexcept : BorderImage {StyleBorderImageData::Create()}
  {
  }

  bool BorderData::ContainsCurrentColor() const noexcept
  {
    return Edges.AnyOf([](const auto &edge)
                       { return edge.IsVisible() && edge.Color.ContainsCurrentColor(); });
  }
}
