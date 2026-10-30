#include "Krystal.Booey/Graphics/Transforms/TransformOperations.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Transforms/IdentityTransformOperation.hpp"
#include "Krystal.Booey/Graphics/Transforms/Matrix3DTransformOperation.hpp"
#include "Krystal.Booey/Graphics/Transforms/MatrixTransformOperation.hpp"
#include "Krystal.Booey/Graphics/Transforms/PerspectiveTransformOperation.hpp"
#include "Krystal.Booey/Graphics/Transforms/RotateTransformOperation.hpp"
#include "Krystal.Booey/Graphics/Transforms/ScaleTransformOperation.hpp"
#include "Krystal.Booey/Graphics/Transforms/SkewTransformOperation.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformOperationsSharedPrimitivesPrefix.hpp"
#include "Krystal.Booey/Graphics/Transforms/TranslateTransformOperation.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using Type = TransformOperation::Type;

    KRYS_NODISCARD TransformationMatrix Applied(const TransformOperation &operation) noexcept
    {
      TransformationMatrix matrix;
      operation.Apply(matrix);
      return matrix;
    }
  }

  TEST_CASE("TransformOperation", "[Graphics][Transforms][TransformOperations]")
  {
    SECTION("apply")
    {
      auto translate = Applied(*TranslateTransformOperation::Create(10, 20, Type::Translate));
      CHECK(translate.E() == 10);
      CHECK(translate.F() == 20);

      auto scale = Applied(*ScaleTransformOperation::Create(2, 3, Type::Scale));
      CHECK(scale.A() == 2);
      CHECK(scale.D() == 3);

      auto rotate = Applied(*RotateTransformOperation::Create(90, Type::Rotate));
      CHECK(rotate.B() == Catch::Approx(1));
      CHECK(rotate.C() == Catch::Approx(-1));

      auto skew = Applied(*SkewTransformOperation::Create(45, 0, Type::SkewX));
      CHECK(skew.C() == Catch::Approx(1));

      auto perspective = Applied(*PerspectiveTransformOperation::Create(100.f));
      CHECK(perspective.M34() == Catch::Approx(-0.01));
      CHECK(Applied(*PerspectiveTransformOperation::Create(std::nullopt)).IsIdentity());

      auto matrix = Applied(*MatrixTransformOperation::Create(1, 0, 0, 1, 5, 6));
      CHECK(matrix.F() == 6);
      CHECK(Applied(*IdentityTransformOperation::Create()).IsIdentity());
    }

    SECTION("equality")
    {
      CHECK((*TranslateTransformOperation::Create(1, 2, Type::Translate)
             == *TranslateTransformOperation::Create(1, 2, Type::Translate)));
      CHECK_FALSE((*TranslateTransformOperation::Create(1, 2, Type::Translate)
                   == *TranslateTransformOperation::Create(1, 2, Type::TranslateX)));
      CHECK_FALSE((*ScaleTransformOperation::Create(1, 2, Type::Scale)
                   == *TranslateTransformOperation::Create(1, 2, Type::Translate)));
      auto rotate = RotateTransformOperation::Create(1, 0, 0, 30, Type::Rotate3D);
      CHECK((*rotate == *static_cast<const TransformOperation &>(*rotate).Clone()));
    }

    SECTION("shared primitives")
    {
      auto translate = TranslateTransformOperation::Create(1, 2, Type::Translate);
      CHECK(translate->SharedPrimitiveType(Type::Translate3D) == Type::Translate3D);
      CHECK(translate->SharedPrimitiveType(Type::Translate) == Type::Translate);
      CHECK_FALSE(translate->SharedPrimitiveType(Type::Rotate));
      CHECK(TranslateTransformOperation::Create(0, 2, Type::TranslateY)->PrimitiveType() == Type::Translate);
      CHECK(TranslateTransformOperation::Create(0, 0, 3, Type::TranslateZ)->PrimitiveType()
            == Type::Translate3D);
      // A missing operation blends with the identity of the other one.
      CHECK(translate->SharedPrimitiveType(nullptr) == Type::Translate);
      CHECK(TransformOperation::IsRotateTransformOperationType(Type::RotateX));
      CHECK_FALSE(TransformOperation::IsScaleTransformOperationType(Type::SkewY));
    }

    SECTION("blend")
    {
      // `Blend` returns a new operation; keep it alive while checking it.
      auto from = TranslateTransformOperation::Create(0, 0, Type::Translate);
      auto to = TranslateTransformOperation::Create(10, 20, Type::Translate);
      auto blended = to->Blend(from.get(), BlendingContext {0.5});
      CHECK(Downcast<TranslateTransformOperation>(*blended).X() == 5);
      CHECK(Downcast<TranslateTransformOperation>(*blended).Y() == 10);

      auto toIdentity = to->Blend(nullptr, BlendingContext {0.25}, true);
      CHECK(Downcast<TranslateTransformOperation>(*toIdentity).X() == 7.5f);

      auto scale = ScaleTransformOperation::Create(3, 3, Type::Scale)->Blend(nullptr, BlendingContext {0.5});
      CHECK(Downcast<ScaleTransformOperation>(*scale).X() == 2);

      auto rotated =
        RotateTransformOperation::Create(90, Type::Rotate)->Blend(nullptr, BlendingContext {0.5});
      CHECK(Downcast<RotateTransformOperation>(*rotated).GetAngle() == 45);

      // Perspectives blend through their inverses.
      auto perspective =
        static_cast<const TransformOperation &>(*PerspectiveTransformOperation::Create(100.f))
          .Blend(nullptr, BlendingContext {0.5});
      REQUIRE(Downcast<PerspectiveTransformOperation>(*perspective).Perspective());
      CHECK(*Downcast<PerspectiveTransformOperation>(*perspective).Perspective() == Catch::Approx(200));
    }
  }

  TEST_CASE("TransformOperations", "[Graphics][Transforms][TransformOperations]")
  {
    TransformOperations operations {
      List<Ref<TransformOperation>> {TranslateTransformOperation::Create(10, 0, Type::Translate),
                                     ScaleTransformOperation::Create(2, 2, Type::Scale)}};
    CHECK(operations.size() == 2);
    CHECK(operations.HasTransformOfType<Type::Scale>());
    CHECK_FALSE(operations.HasTransformOfType<Type::Rotate>());
    CHECK(operations.IsInvertible());
    CHECK_FALSE(operations.ContainsNonInvertibleMatrix());
    CHECK(operations == operations.Clone());
    CHECK(operations.At(5) == nullptr);

    TransformationMatrix matrix;
    operations.Apply(matrix);
    CHECK(matrix.E() == 10);
    CHECK(matrix.A() == 2);

    SECTION("blend")
    {
      TransformOperations to {
        List<Ref<TransformOperation>> {TranslateTransformOperation::Create(30, 0, Type::Translate),
                                       ScaleTransformOperation::Create(4, 4, Type::Scale)}};
      auto blended = Blend(operations, to, BlendingContext {0.5});
      REQUIRE(blended.size() == 2);
      CHECK(Downcast<TranslateTransformOperation>(*blended[0]).X() == 20);
      CHECK(Downcast<ScaleTransformOperation>(*blended[1]).X() == 3);

      // A shorter list blends its missing operations from identity.
      auto fromShorter =
        Blend(TransformOperations {TranslateTransformOperation::Create(10, 0, Type::Translate)}, to,
              BlendingContext {0.5});
      REQUIRE(fromShorter.size() == 2);
      CHECK(Downcast<ScaleTransformOperation>(*fromShorter[1]).X() == 2.5);

      // Mismatched primitives fall back to a matrix blend.
      auto mismatched =
        Blend(TransformOperations {RotateTransformOperation::Create(90, Type::Rotate)},
              TransformOperations {TranslateTransformOperation::Create(10, 0, Type::Translate)},
              BlendingContext {0.5});
      REQUIRE(mismatched.size() == 1);
      CHECK(mismatched[0]->GetType() == Type::Matrix3D);
    }

    SECTION("non-invertible")
    {
      TransformOperations singular {ScaleTransformOperation::Create(0, 1, Type::Scale)};
      CHECK_FALSE(singular.IsInvertible());
      TransformOperations matrixSingular {MatrixTransformOperation::Create(0, 0, 0, 0, 0, 0)};
      CHECK(matrixSingular.ContainsNonInvertibleMatrix());
    }

    SECTION("shared primitives prefix")
    {
      TransformOperationsSharedPrimitivesPrefix<Type> prefix;
      prefix.Update(TransformOperations {
        List<Ref<TransformOperation>> {TranslateTransformOperation::Create(1, 0, Type::TranslateX),
                                       RotateTransformOperation::Create(1, Type::Rotate)}});
      prefix.Update(TransformOperations {
        List<Ref<TransformOperation>> {TranslateTransformOperation::Create(0, 0, 1, Type::TranslateZ),
                                       ScaleTransformOperation::Create(1, 1, Type::Scale)}});
      CHECK(prefix.HadIncompatibleTransformFunctions());
      REQUIRE(prefix.Primitives().size() == 1);
      CHECK(prefix.Primitives()[0] == Type::Translate3D);
    }
  }
}
