#pragma once

namespace krys::boo::dom
{
  /// @see https://drafts.fxtf.org/geometry/#dictdef-dompointinit
  struct DOMPointInit
  {
    double X {0};
    double Y {0};
    double Z {0};
    double W {1};
  };
}
