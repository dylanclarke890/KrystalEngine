#include "Krystal.Booey/Graphics/Filters/FilterOperations.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Filters/FEDropShadow.hpp"
#include "Krystal.Booey/Graphics/Filters/FEGaussianBlur.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using Type = FilterOperation::Type;
    using Catch::Approx;

    KRYS_NODISCARD bool IsIdentity(const FilterOperation &operation) noexcept
    {
      return operation.IsIdentity();
    }

    KRYS_NODISCARD SRGBA<float> Transformed(const FilterOperation &operation, SRGBA<float> color) noexcept
    {
      auto transformed = operation.TransformColor(color);
      CHECK(transformed);
      return color;
    }
  }

  TEST_CASE("FilterOperation", "[Graphics][Filters][FilterOperations]")
  {
    SECTION("equality and identity")
    {
      CHECK((*BlurFilterOperation::Create(4) == *BlurFilterOperation::Create(4)));
      CHECK_FALSE((*BlurFilterOperation::Create(4) == *BlurFilterOperation::Create(5)));
      CHECK_FALSE((*BasicColorMatrixFilterOperation::Create(0.5, Type::Grayscale)
                   == *BasicColorMatrixFilterOperation::Create(0.5, Type::Sepia)));
      CHECK(IsIdentity(*BlurFilterOperation::Create(0)));
      CHECK(IsIdentity(*BasicColorMatrixFilterOperation::Create(0, Type::Grayscale)));
      CHECK(IsIdentity(*BasicComponentTransferFilterOperation::Create(1, Type::Brightness)));
      CHECK_FALSE(IsIdentity(*BasicComponentTransferFilterOperation::Create(0.5, Type::Brightness)));
      CHECK(BlurFilterOperation::Create(1)->MovesPixels());
      CHECK(BasicComponentTransferFilterOperation::Create(0.5, Type::Opacity)->AffectsOpacity());

      auto clone = BlurFilterOperation::Create(3)->Clone();
      CHECK((*clone == *BlurFilterOperation::Create(3)));
    }

    SECTION("colour transforms")
    {
      SRGBA<float> color {1.0f, 0.5f, 0.25f, 1.0f};

      auto inverted = Transformed(*BasicComponentTransferFilterOperation::Create(1, Type::Invert), color);
      CHECK(inverted.Resolved().Red == Approx(0));
      CHECK(inverted.Resolved().Green == Approx(0.5));
      CHECK(inverted.Resolved().Blue == Approx(0.75));

      auto brightened =
        Transformed(*BasicComponentTransferFilterOperation::Create(0.5, Type::Brightness), color);
      CHECK(brightened.Resolved().Red == Approx(0.5));
      CHECK(brightened.Resolved().Blue == Approx(0.125));

      auto faded = Transformed(*BasicComponentTransferFilterOperation::Create(0.5, Type::Opacity), color);
      CHECK(faded.Resolved().Alpha == Approx(0.5));

      auto grey = Transformed(*BasicColorMatrixFilterOperation::Create(1, Type::Grayscale), color);
      CHECK(grey.Resolved().Red == Approx(grey.Resolved().Green));
      CHECK(grey.Resolved().Green == Approx(grey.Resolved().Blue));
    }

    SECTION("blending")
    {
      auto blurred = BlurFilterOperation::Create(10)->Blend(BlurFilterOperation::Create(0).get(), {0.5});
      REQUIRE(blurred);
      CHECK(Downcast<BlurFilterOperation>(*blurred).StdDeviation() == Approx(5));

      // Blending from nothing blends from the passthrough amount.
      auto grey = BasicColorMatrixFilterOperation::Create(1, Type::Grayscale)->Blend(nullptr, {0.5});
      REQUIRE(grey);
      CHECK(Downcast<BasicColorMatrixFilterOperation>(*grey).Amount() == Approx(0.5));

      auto brightness =
        BasicComponentTransferFilterOperation::Create(3, Type::Brightness)->Blend(nullptr, {0.5});
      REQUIRE(brightness);
      CHECK(Downcast<BasicComponentTransferFilterOperation>(*brightness).Amount() == Approx(2));
    }
  }

  TEST_CASE("FEGaussianBlur", "[Graphics][Filters][FilterOperations]")
  {
    CHECK(FEGaussianBlur::CalculateUnscaledKernelSize({4, 0}) == IntSize {8, 0});
    // Small deviations still use a kernel of at least two.
    CHECK(FEGaussianBlur::CalculateUnscaledKernelSize({0.1f, 0.1f}) == IntSize {2, 2});
    CHECK(FEGaussianBlur::CalculateUnscaledKernelSize({1'000, 1'000}) == IntSize {500, 500});
    CHECK(FEGaussianBlur::CalculateOutsetSize({4, 4}) == IntSize {12, 12});
    CHECK(FEGaussianBlur::CalculateOutsets({4, 4}) == IntOutsets {12, 12, 12, 12});
  }

  TEST_CASE("FilterOperations", "[Graphics][Filters][FilterOperations]")
  {
    FilterOperations empty;
    CHECK(empty.IsEmpty());
    CHECK_FALSE(empty.HasOutsets());

    FilterOperations operations {
      {BlurFilterOperation::Create(4), DropShadowFilterOperation::Create({4, 2}, 0, Color::black)}};
    REQUIRE(operations.size() == 2);
    CHECK(operations.First()->GetType() == Type::Blur);
    CHECK(operations.Last()->GetType() == Type::DropShadow);
    CHECK(operations.At(2) == nullptr);
    CHECK(operations.HasFilterThatMovesPixels());
    CHECK(operations.HasFilterOfType<Type::DropShadow>());
    CHECK_FALSE(operations.HasReferenceFilter());

    // The blur moves pixels 12px each way; the unblurred shadow only extends to the right and bottom.
    CHECK(operations.HasOutsets());
    CHECK(operations.Outsets() == IntOutsets {12, 16, 14, 12});

    CHECK((operations == operations.Clone()));
    CHECK(operations.OperationsMatch(FilterOperations {
      {BlurFilterOperation::Create(1), DropShadowFilterOperation::Create({}, 1, Color::white)}}));

    SECTION("interpolation")
    {
      FilterOperations from {{BlurFilterOperation::Create(0)}};
      FilterOperations to {
        {BlurFilterOperation::Create(10), BasicColorMatrixFilterOperation::Create(1, Type::Grayscale)}};
      CHECK(from.CanInterpolate(to, CompositeOperation::Replace));
      CHECK(empty.CanInterpolate(to, CompositeOperation::Replace));

      FilterOperations mismatched {{BasicColorMatrixFilterOperation::Create(1, Type::Sepia)}};
      CHECK_FALSE(mismatched.CanInterpolate(to, CompositeOperation::Replace));
      CHECK(mismatched.CanInterpolate(to, CompositeOperation::Add));

      auto blended = from.Blend(to, {0.5});
      REQUIRE(blended.size() == 2);
      CHECK(Downcast<BlurFilterOperation>(*blended[0]).StdDeviation() == Approx(5));
      CHECK(Downcast<BasicColorMatrixFilterOperation>(*blended[1]).Amount() == Approx(0.5));

      auto added = from.Blend(to, BlendingContext {1, false, CompositeOperation::Add});
      CHECK(added.size() == 3);
    }
  }
}
