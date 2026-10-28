#include "Krystal.Booey/DOM/DOMPointReadOnly.hpp"
#include "Krystal.Booey/CSS/DOMMatrixReadOnly.hpp"
#include "Krystal.Booey/DOM/DOMPoint.hpp"

namespace krys::boo::dom
{
  ExceptionOr<Ref<DOMPoint>> DOMPointReadOnly::MatrixTransform(css::DOMMatrixInit &&matrixInit) const noexcept
  {
    auto matrixOrException = css::DOMMatrixReadOnly::FromMatrix(krys::move(matrixInit));
    if (matrixOrException.HasException())
    {
      return matrixOrException.ReleaseException();
    }

    auto matrix = matrixOrException.ReleaseValue();
    double x = X();
    double y = Y();
    double z = Z();
    double w = W();
    matrix->GetTransformationMatrix().Map4ComponentPoint(x, y, z, w);
    return DOMPoint::Create(x, y, z, w);
  }
}
