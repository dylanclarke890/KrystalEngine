#pragma once

#include "Krystal.Booey/CSS/DOMMatrixInit.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::dom
{
  class DOMPoint;
  struct DOMPointInit;
}

namespace krys::boo::css
{
  class DOMMatrix;

  /// @brief A 4 by 4 matrix (or a 2D one) that can't be changed through it.
  /// @see https://drafts.fxtf.org/geometry/#dommatrixreadonly
  class DOMMatrixReadOnly : public RefCounted<DOMMatrixReadOnly>
  {
  protected:
    TransformationMatrix _matrix;
    bool _is2D {true};

  public:
    enum class Is2D : bool
    {
      No,
      Yes
    };

    /// @see https://drafts.fxtf.org/geometry/#dom-dommatrixreadonly-dommatrixreadonly
    KRYS_NODISCARD static dom::ExceptionOr<Ref<DOMMatrixReadOnly>>
      Create(Maybe<Variant<CSSOMString, List<double>>> &&init = null) noexcept;

    KRYS_NODISCARD static Ref<DOMMatrixReadOnly> Create(const TransformationMatrix &matrix,
                                                        Is2D is2D) noexcept
    {
      return AdoptRef(*new DOMMatrixReadOnly(matrix, is2D));
    }

    KRYS_NODISCARD static dom::ExceptionOr<Ref<DOMMatrixReadOnly>> FromMatrix(DOMMatrixInit &&) noexcept;
    KRYS_NODISCARD static dom::ExceptionOr<Ref<DOMMatrixReadOnly>>
      FromFloat32Array(Span<const float>) noexcept;
    KRYS_NODISCARD static dom::ExceptionOr<Ref<DOMMatrixReadOnly>>
      FromFloat64Array(Span<const double>) noexcept;

    /// @see https://drafts.fxtf.org/geometry/#matrix-validate-and-fixup
    KRYS_NODISCARD static dom::ExceptionOr<void> ValidateAndFixup(DOMMatrix2DInit &) noexcept;
    KRYS_NODISCARD static dom::ExceptionOr<void> ValidateAndFixup(DOMMatrixInit &) noexcept;

    KRYS_NODISCARD double A() const noexcept
    {
      return _matrix.A();
    }
    KRYS_NODISCARD double B() const noexcept
    {
      return _matrix.B();
    }
    KRYS_NODISCARD double C() const noexcept
    {
      return _matrix.C();
    }
    KRYS_NODISCARD double D() const noexcept
    {
      return _matrix.D();
    }
    KRYS_NODISCARD double E() const noexcept
    {
      return _matrix.E();
    }
    KRYS_NODISCARD double F() const noexcept
    {
      return _matrix.F();
    }

    KRYS_NODISCARD double M11() const noexcept
    {
      return _matrix.M11();
    }
    KRYS_NODISCARD double M12() const noexcept
    {
      return _matrix.M12();
    }
    KRYS_NODISCARD double M13() const noexcept
    {
      return _matrix.M13();
    }
    KRYS_NODISCARD double M14() const noexcept
    {
      return _matrix.M14();
    }
    KRYS_NODISCARD double M21() const noexcept
    {
      return _matrix.M21();
    }
    KRYS_NODISCARD double M22() const noexcept
    {
      return _matrix.M22();
    }
    KRYS_NODISCARD double M23() const noexcept
    {
      return _matrix.M23();
    }
    KRYS_NODISCARD double M24() const noexcept
    {
      return _matrix.M24();
    }
    KRYS_NODISCARD double M31() const noexcept
    {
      return _matrix.M31();
    }
    KRYS_NODISCARD double M32() const noexcept
    {
      return _matrix.M32();
    }
    KRYS_NODISCARD double M33() const noexcept
    {
      return _matrix.M33();
    }
    KRYS_NODISCARD double M34() const noexcept
    {
      return _matrix.M34();
    }
    KRYS_NODISCARD double M41() const noexcept
    {
      return _matrix.M41();
    }
    KRYS_NODISCARD double M42() const noexcept
    {
      return _matrix.M42();
    }
    KRYS_NODISCARD double M43() const noexcept
    {
      return _matrix.M43();
    }
    KRYS_NODISCARD double M44() const noexcept
    {
      return _matrix.M44();
    }

    KRYS_NODISCARD bool GetIs2D() const noexcept
    {
      return _is2D;
    }
    KRYS_NODISCARD bool IsIdentity() const noexcept;

    /// @see https://drafts.fxtf.org/geometry/#dom-dommatrix-setmatrixvalue
    dom::ExceptionOr<void> SetMatrixValue(const CSSOMString &) noexcept;

    KRYS_NODISCARD Ref<DOMMatrix> Translate(double tx = 0, double ty = 0, double tz = 0) const noexcept;
    KRYS_NODISCARD dom::ExceptionOr<Ref<DOMMatrix>> Multiply(DOMMatrixInit &&other) const noexcept;
    KRYS_NODISCARD Ref<DOMMatrix> FlipX() const noexcept;
    KRYS_NODISCARD Ref<DOMMatrix> FlipY() const noexcept;
    KRYS_NODISCARD Ref<DOMMatrix> Scale(double scaleX = 1, Maybe<double> scaleY = null, double scaleZ = 1,
                                        double originX = 0, double originY = 0,
                                        double originZ = 0) const noexcept;
    KRYS_NODISCARD Ref<DOMMatrix> Scale3d(double scale = 1, double originX = 0, double originY = 0,
                                          double originZ = 0) const noexcept;
    KRYS_NODISCARD Ref<DOMMatrix> ScaleNonUniform(double scaleX = 1, double scaleY = 1) const noexcept;
    /// @note Angles are in degrees.
    KRYS_NODISCARD Ref<DOMMatrix> Rotate(double rotX = 0, Maybe<double> rotY = null,
                                         Maybe<double> rotZ = null) const noexcept;
    KRYS_NODISCARD Ref<DOMMatrix> RotateFromVector(double x = 0, double y = 0) const noexcept;
    /// @note The angle is in degrees.
    KRYS_NODISCARD Ref<DOMMatrix> RotateAxisAngle(double x = 0, double y = 0, double z = 0,
                                                  double angle = 0) const noexcept;
    /// @note The angle is in degrees.
    KRYS_NODISCARD Ref<DOMMatrix> SkewX(double sx = 0) const noexcept;
    /// @note The angle is in degrees.
    KRYS_NODISCARD Ref<DOMMatrix> SkewY(double sy = 0) const noexcept;
    KRYS_NODISCARD Ref<DOMMatrix> Inverse() const noexcept;

    /// @see https://drafts.fxtf.org/geometry/#dom-dommatrixreadonly-transformpoint
    KRYS_NODISCARD Ref<dom::DOMPoint> TransformPoint(dom::DOMPointInit &&) const noexcept;

    KRYS_NODISCARD Array<float, 16> ToFloat32Array() const noexcept;
    KRYS_NODISCARD Array<double, 16> ToFloat64Array() const noexcept;

    /// @see https://drafts.fxtf.org/geometry/#dom-dommatrixreadonly-stringifier
    KRYS_NODISCARD dom::ExceptionOr<CSSOMString> ToString() const noexcept;

    KRYS_NODISCARD const TransformationMatrix &GetTransformationMatrix() const noexcept
    {
      return _matrix;
    }

    KRYS_NODISCARD Ref<DOMMatrix> CloneAsDOMMatrix() const noexcept;

  protected:
    DOMMatrixReadOnly() noexcept = default;
    DOMMatrixReadOnly(const TransformationMatrix &, Is2D) noexcept;

    struct AbstractMatrix
    {
      TransformationMatrix Matrix;
      bool Is2D {true};
    };

    KRYS_NODISCARD static dom::ExceptionOr<AbstractMatrix>
      ParseStringIntoAbstractMatrix(const CSSOMString &) noexcept;

    /// @brief A matrix made from a list of 6 or 16 values, or nothing for other lengths.
    template <typename T, typename TValue>
    KRYS_NODISCARD static dom::ExceptionOr<Ref<T>> FromArrayHelper(Span<const TValue>) noexcept;

    template <typename T>
    KRYS_NODISCARD static dom::ExceptionOr<Ref<T>> FromMatrixHelper(DOMMatrixInit &&) noexcept;
  };

  template <typename T, typename TValue>
  inline dom::ExceptionOr<Ref<T>> DOMMatrixReadOnly::FromArrayHelper(Span<const TValue> init) noexcept
  {
    if (init.size() == 6)
    {
      return T::Create(TransformationMatrix {init[0], init[1], init[2], init[3], init[4], init[5]},
                       Is2D::Yes);
    }
    if (init.size() == 16)
    {
      return T::Create(TransformationMatrix {init[0], init[1], init[2], init[3], init[4], init[5], init[6],
                                             init[7], init[8], init[9], init[10], init[11], init[12],
                                             init[13], init[14], init[15]},
                       Is2D::No);
    }
    return dom::Exception {dom::ExceptionCode::TypeError};
  }

  // https://drafts.fxtf.org/geometry/#create-a-dommatrix-from-the-dictionary
  template <typename T>
  inline dom::ExceptionOr<Ref<T>> DOMMatrixReadOnly::FromMatrixHelper(DOMMatrixInit &&init) noexcept
  {
    auto result = ValidateAndFixup(init);
    if (result.HasException())
    {
      return result.ReleaseException();
    }

    if (init.Is2D.value())
    {
      return T::Create(TransformationMatrix {init.M11.value(), init.M12.value(), init.M21.value(),
                                             init.M22.value(), init.M41.value(), init.M42.value()},
                       Is2D::Yes);
    }

    return T::Create(TransformationMatrix {init.M11.value(), init.M12.value(), init.M13, init.M14,
                                           init.M21.value(), init.M22.value(), init.M23, init.M24, init.M31,
                                           init.M32, init.M33, init.M34, init.M41.value(), init.M42.value(),
                                           init.M43, init.M44},
                     Is2D::No);
  }
}
