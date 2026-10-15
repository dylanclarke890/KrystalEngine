#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Rendering/TransformOperationData.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Masking/Clip.hpp"
#include "Krystal.Booey/Style/Values/Masking/ClipPath.hpp"
#include "Krystal.Booey/Style/Values/Masking/MaskBorder.hpp"
#include "Krystal.Booey/Style/Values/Masking/MaskLayers.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;

  TEST_CASE_METHOD(BuilderFixture, "Style clip", "[Style][Values][Masking]")
  {
    CHECK(Convert<Clip>(PropertyId::Clip, u8"auto").IsAuto());

    auto rect = Convert<Clip>(PropertyId::Clip, u8"rect(1px, auto, 3px, 4px)");
    REQUIRE(rect.IsRect());
    CHECK(Serialised(rect) == "rect(1px, auto, 3px, 4px)");

    // A rect whose edges are all `auto` is `auto`.
    CHECK(Serialised(Convert<Clip>(PropertyId::Clip, u8"rect(auto, auto, auto, auto)")) == "auto");

    SECTION("blending")
    {
      auto from = Convert<Clip>(PropertyId::Clip, u8"rect(0px, 10px, 20px, 0px)");
      auto to = Convert<Clip>(PropertyId::Clip, u8"rect(10px, 30px, 40px, 20px)");
      CHECK(Blending<Clip> {}.CanBlend(from, to));
      CHECK(Serialised(Blending<Clip> {}.Blend(from, to, BlendingContext {0.5}))
            == "rect(5px, 20px, 30px, 10px)");

      // An `auto` edge on one side blends discretely.
      auto withAuto = Convert<Clip>(PropertyId::Clip, u8"rect(auto, 30px, 40px, 20px)");
      CHECK_FALSE(Blending<Clip> {}.CanBlend(from, withAuto));
      CHECK(Blending<Clip> {}.RequiresInterpolationForAccumulativeIteration(from, withAuto) == false);
      CHECK_FALSE(Blending<Clip> {}.CanBlend(from, Clip {css::keywords::Auto {}}));
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style clip-path", "[Style][Values][Masking]")
  {
    CHECK(Convert<ClipPath>(PropertyId::ClipPath, u8"none").IsNone());

    auto box = Convert<ClipPath>(PropertyId::ClipPath, u8"padding-box");
    CHECK(box.IsBox());
    CHECK(Serialised(box) == "padding-box");
    REQUIRE(box.TryBox());
    CHECK(box.TryBox()->ReferenceBox() == CSSBoxType::PaddingBox);

    auto shape = Convert<ClipPath>(PropertyId::ClipPath, u8"circle(10px at 20px 30px) content-box");
    REQUIRE(shape.IsBasicShape());
    CHECK(Serialised(shape) == "circle(10px at 20px 30px) content-box");
    CHECK(Created(shape) == "circle(10px at 20px 30px) content-box");
    CHECK(shape.HoldsAlternative<BasicShapePath>());
    CHECK_FALSE(shape.HoldsAlternative<BoxPath>());

    // A basic shape without a motion path is resolved against the bounding box.
    auto path = TryPath(shape, TransformOperationData {FloatRect {0, 0, 100, 100}});
    REQUIRE(path);
    CHECK(path->BoundingRect() == FloatRect {10, 20, 20, 20});

    // `path()` is allowed here, unlike `shape-outside`.
    auto svgPath = Convert<ClipPath>(PropertyId::ClipPath, u8"path('M 10 10 h 30 v 40 Z')");
    REQUIRE(svgPath.IsBasicShape());
    CHECK(Serialised(svgPath) == "path(\"M 10 10 h 30 v 40 Z\")");
    CHECK(TryPath(svgPath, TransformOperationData {FloatRect {0, 0, 100, 100}})->BoundingRect()
          == FloatRect {10, 10, 30, 40});

    auto reference = Convert<ClipPath>(PropertyId::ClipPath, u8"url(#clip)");
    REQUIRE(reference.IsReference());
    CHECK(reference.TryReference()->Fragment().View() == u8"clip");
    // Without SVG geometry the reference has no path yet.
    CHECK_FALSE(TryPath(reference, TransformOperationData {FloatRect {0, 0, 100, 100}}));

    // Copies share the operation; equality compares the operations.
    auto copy = shape;
    CHECK(copy == shape);
    CHECK(Convert<ClipPath>(PropertyId::ClipPath, u8"circle(10px at 20px 30px) content-box") == shape);
    CHECK_FALSE(shape == box);

    SECTION("blending")
    {
      auto from = Convert<ClipPath>(PropertyId::ClipPath, u8"inset(0px)");
      auto to = Convert<ClipPath>(PropertyId::ClipPath, u8"inset(20px)");
      CHECK(Blending<ClipPath> {}.CanBlend(from, to));
      CHECK(Serialised(Blending<ClipPath> {}.Blend(from, to, BlendingContext {0.5})) == "inset(10px)");
      CHECK_FALSE(Blending<ClipPath> {}.CanBlend(from, box));
      CHECK_FALSE(Blending<ClipPath> {}.CanBlend(from, ClipPath {css::keywords::None {}}));
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style mask-border", "[Style][Values][Masking]")
  {
    MaskBorder initial;
    CHECK(initial.Source.IsNone());
    CHECK(Serialised(initial.Slice) == "0");
    CHECK(Serialised(initial.Width) == "auto");
    CHECK(Serialised(initial.Outset) == "0");
    CHECK(Serialised(initial.Repeat) == "stretch");
    CHECK(Serialised(initial) == "none");

    CHECK(Serialised(Convert<MaskBorderSlice>(PropertyId::MaskBorderSlice, u8"10 20% fill"))
          == "10 20% fill");
    CHECK(Serialised(Convert<MaskBorderWidth>(PropertyId::MaskBorderWidth, u8"1px 2")) == "1px 2");
    CHECK(Serialised(Convert<MaskBorderOutset>(PropertyId::MaskBorderOutset, u8"1px 2")) == "1px 2");
    CHECK(Serialised(Convert<MaskBorderRepeat>(PropertyId::MaskBorderRepeat, u8"round space"))
          == "round space");

    auto slice = Convert<MaskBorderSlice>(PropertyId::MaskBorderSlice, u8"10");
    CHECK(Created(slice) == "10");
    auto width = Convert<MaskBorderWidth>(PropertyId::MaskBorderWidth, u8"auto");
    CHECK(Created(width) == "auto");

    SECTION("blending")
    {
      auto from = Convert<MaskBorderWidth>(PropertyId::MaskBorderWidth, u8"0px");
      auto to = Convert<MaskBorderWidth>(PropertyId::MaskBorderWidth, u8"10px");
      CHECK(Blending<MaskBorderWidth> {}.CanBlend(from, to));
      CHECK(Serialised(Blending<MaskBorderWidth> {}.Blend(from, to, BlendingContext {0.5})) == "5px");
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style mask layers", "[Style][Values][Masking]")
  {
    MaskLayer layer;
    CHECK(layer.IsInitial());
    CHECK(layer.GetOrigin() == FillBox::BorderBox);
    CHECK(layer.GetClip() == FillBox::BorderBox);
    CHECK(layer.GetComposite() == CompositeOperator::SourceOver);
    CHECK(layer.GetMaskMode() == MaskMode::MatchSource);
    CHECK(MaskLayer::GetAttachment() == FillAttachment::ScrollBackground);
    CHECK(MaskLayer::GetBlendMode() == BlendMode::Normal);

    layer.SetComposite(CompositeOperator::SourceIn);
    layer.SetMaskMode(MaskMode::Luminance);
    CHECK(layer.GetComposite() == CompositeOperator::SourceIn);
    CHECK(layer.GetMaskMode() == MaskMode::Luminance);
    // The last layer ignores its compositing operator.
    CHECK(layer.CompositeForPainting(false) == CompositeOperator::SourceIn);
    CHECK(layer.CompositeForPainting(true) == CompositeOperator::SourceOver);

    CHECK_FALSE(layer == MaskLayer {});
    CHECK(MaskLayer::Clone(layer) == layer);

    MaskLayers layers {MaskLayer {}, MaskLayer {}};
    layers.PrepareForUse();
    CHECK(layers[0].IsInitial());
  }
}
