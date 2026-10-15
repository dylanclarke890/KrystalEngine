#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/MatchResult.hpp"
#include "Krystal.Booey/Style/Relations.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::style
{
  /// @brief An element's style as the builder computed it, before the adjuster.
  struct UnadjustedStyle
  {
    UniquePtr<RenderStyle> Style;
    UniquePtr<style::Relations> Relations {};
    RefPtr<const style::MatchResult> MatchResult {};
  };

  /// @brief An element's resolved style, with the relations and matched declarations it came from.
  struct ResolvedStyle
  {
    UniquePtr<RenderStyle> Style;
    UniquePtr<style::Relations> Relations {};
    RefPtr<const style::MatchResult> MatchResult {};
  };
}
