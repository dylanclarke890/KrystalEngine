#pragma once

#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  /// @see https://drafts.fxtf.org/geometry/#dictdef-dommatrix2dinit
  struct DOMMatrix2DInit
  {
    Maybe<double> A;
    Maybe<double> B;
    Maybe<double> C;
    Maybe<double> D;
    Maybe<double> E;
    Maybe<double> F;
    Maybe<double> M11;
    Maybe<double> M12;
    Maybe<double> M21;
    Maybe<double> M22;
    Maybe<double> M41;
    Maybe<double> M42;
  };
}
