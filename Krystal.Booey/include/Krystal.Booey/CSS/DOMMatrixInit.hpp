#pragma once

#include "Krystal.Booey/CSS/DOMMatrix2DInit.hpp"

namespace krys::boo::css
{
  /// @see https://drafts.fxtf.org/geometry/#dictdef-dommatrixinit
  struct DOMMatrixInit : DOMMatrix2DInit
  {
    double M13 {0};
    double M14 {0};
    double M23 {0};
    double M24 {0};
    double M31 {0};
    double M32 {0};
    double M33 {1};
    double M34 {0};
    double M43 {0};
    double M44 {1};
    Maybe<bool> Is2D;
  };
}
