#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Graphics/Filters/FEBlend.hpp"
#include "Krystal.Booey/Graphics/Filters/FEComponentTransfer.hpp"
#include "Krystal.Booey/Graphics/Filters/FEComposite.hpp"
#include "Krystal.Booey/Graphics/Filters/FEDiffuseLighting.hpp"
#include "Krystal.Booey/Graphics/Filters/FEFlood.hpp"
#include "Krystal.Booey/Graphics/Filters/FEMerge.hpp"
#include "Krystal.Booey/Graphics/Filters/FEOffset.hpp"
#include "Krystal.Booey/Graphics/Filters/PointLightSource.hpp"
#include "Krystal.Booey/Graphics/Filters/SourceGraphic.hpp"
#include "Krystal.Booey/Graphics/Filters/SpotLightSource.hpp"
#include "Krystal.Booey/Graphics/NullGraphicsContext.hpp"
#include "Krystal.Booey/SVG/SVGFEBlendElement.hpp"
#include "Krystal.Booey/SVG/SVGFEColorMatrixElement.hpp"
#include "Krystal.Booey/SVG/SVGFECompositeElement.hpp"
#include "Krystal.Booey/SVG/SVGFEConvolveMatrixElement.hpp"
#include "Krystal.Booey/SVG/SVGFEDisplacementMapElement.hpp"
#include "Krystal.Booey/SVG/SVGFEGaussianBlurElement.hpp"
#include "Krystal.Booey/SVG/SVGFEMorphologyElement.hpp"
#include "Krystal.Booey/SVG/SVGFETurbulenceElement.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    /// @brief A document with an SVG holding a filter of @p primitives, laid out.
    struct PrimitiveFixture : ResolverFixture
    {
      explicit PrimitiveFixture(utf8_stringview primitives) noexcept
          : ResolverFixture(u8"<!DOCTYPE html><svg width=200 height=100><filter id=f>"
                            + utf8_string(primitives)
                            + u8"</filter><rect width=10 height=10 filter=url(#f) /></svg>")
      {
        (void)Document->UpdateLayout();
      }

      template <typename T = svg::SVGFilterPrimitiveStandardAttributes>
      KRYS_NODISCARD T &Primitive(dom::DOMStringView id) noexcept
      {
        return Downcast<T>(Downcast<svg::SVGElement>(ById(id)));
      }

      /// @brief The effect the primitive `#id` creates, given @p inputs, or null.
      KRYS_NODISCARD RefPtr<FilterEffect> Effect(dom::DOMStringView id,
                                                 const FilterEffectVector &inputs = {}) noexcept
      {
        NullGraphicsContext context;
        return Primitive(id).GetFilterEffect(inputs, context);
      }

      template <typename T>
      KRYS_NODISCARD T &EffectAs(dom::DOMStringView id) noexcept
      {
        auto effect = Effect(id);
        krys_assert(effect);
        return Downcast<T>(*effect);
      }

      void Set(dom::DOMStringView id, dom::DOMStringView name, dom::DOMStringView value) noexcept
      {
        (void)ById(id).SetAttribute(dom::DOMStringAtom(name), dom::DOMString(value));
        (void)Document->UpdateLayout();
      }
    };

    KRYS_NODISCARD List<utf8_string> Names(const List<dom::DOMStringAtom> &atoms) noexcept
    {
      List<utf8_string> names;
      for (const auto &atom : atoms)
      {
        names.emplace_back(atom.View());
      }
      return names;
    }
  }

  TEST_CASE("SVGFilterPrimitiveStandardAttributes: the subregion and the result",
            "[SVG][Filters][SVGFilterPrimitiveStandardAttributes]")
  {
    PrimitiveFixture fixture(u8"<feOffset id=a x=5 width=50% result=moved /><feOffset id=b />");

    auto &a = fixture.Primitive(u8"a");
    CHECK(a.X().ValueAsString() == u8"5");
    CHECK(a.Width().ValueAsString() == u8"50%");
    CHECK(a.Result() == u8"moved");
    CHECK(a.EffectGeometryFlags()
          == EnumSet<FilterEffectGeometry::Flags> {FilterEffectGeometry::Flags::HasX,
                                                   FilterEffectGeometry::Flags::HasWidth});

    // Without the attributes, the subregion is the filter region ("0%" and "100%"), and none are given.
    auto &b = fixture.Primitive(u8"b");
    CHECK(b.Y().ValueAsString() == u8"0%");
    CHECK(b.Height().ValueAsString() == u8"100%");
    CHECK(b.EffectGeometryFlags().IsEmpty());
  }

  TEST_CASE("SVGFilterPrimitiveStandardAttributes: inputs are named by in and in2",
            "[SVG][Filters][SVGFilterPrimitiveStandardAttributes]")
  {
    PrimitiveFixture fixture(
      u8"<feBlend id=blend in=SourceAlpha in2=other />"
      u8"<feMerge id=merge><feMergeNode in=a /><feMergeNode /><feMergeNode in=b /></feMerge>"
      u8"<feTurbulence id=turbulence />");

    CHECK(Names(fixture.Primitive(u8"blend").FilterEffectInputsNames())
          == List<utf8_string> {u8"SourceAlpha", u8"other"});
    CHECK(Names(fixture.Primitive(u8"merge").FilterEffectInputsNames())
          == List<utf8_string> {u8"a", u8"", u8"b"});
    CHECK(fixture.Primitive(u8"turbulence").FilterEffectInputsNames().empty());

    // A merge has an input per node.
    auto merge =
      fixture.Effect(u8"merge", {SourceGraphic::Create(), SourceGraphic::Create(), SourceGraphic::Create()});
    REQUIRE(merge);
    CHECK(merge->NumberOfImageInputs() == 3);
  }

  TEST_CASE("SVGFilterPrimitiveStandardAttributes: feBlend and feComposite",
            "[SVG][Filters][SVGFilterPrimitiveStandardAttributes]")
  {
    PrimitiveFixture fixture(u8"<feBlend id=multiply mode=multiply /><feBlend id=bad mode=plus-lighter-ish />"
                             u8"<feComposite id=arithmetic operator=arithmetic k1=1 k2=.5 k3=-2 k4=x />"
                             u8"<feComposite id=default operator=nope />");

    CHECK(fixture.EffectAs<FEBlend>(u8"multiply").GetBlendMode() == BlendMode::Multiply);
    CHECK(fixture.EffectAs<FEBlend>(u8"bad").GetBlendMode() == BlendMode::Normal);

    auto &arithmetic = fixture.EffectAs<FEComposite>(u8"arithmetic");
    CHECK(arithmetic.Operation() == CompositeOperationType::FECOMPOSITE_OPERATOR_ARITHMETIC);
    CHECK(arithmetic.K1() == 1.f);
    CHECK(arithmetic.K2() == .5f);
    CHECK(arithmetic.K3() == -2.f);
    CHECK(arithmetic.K4() == 0.f);
    CHECK(fixture.EffectAs<FEComposite>(u8"default").Operation()
          == CompositeOperationType::FECOMPOSITE_OPERATOR_OVER);
  }

  TEST_CASE("SVGFilterPrimitiveStandardAttributes: feGaussianBlur",
            "[SVG][Filters][SVGFilterPrimitiveStandardAttributes]")
  {
    PrimitiveFixture fixture(u8"<feGaussianBlur id=pair stdDeviation='2 3' edgeMode=wrap />"
                             u8"<feGaussianBlur id=one stdDeviation=4 edgeMode=sideways />"
                             u8"<feGaussianBlur id=zero />");

    auto &pair = fixture.Primitive<svg::SVGFEGaussianBlurElement>(u8"pair");
    CHECK(pair.StdDeviationX() == 2.f);
    CHECK(pair.StdDeviationY() == 3.f);
    CHECK(pair.EdgeMode() == EdgeModeType::Wrap);

    auto &one = fixture.Primitive<svg::SVGFEGaussianBlurElement>(u8"one");
    CHECK(one.StdDeviationX() == 4.f);
    CHECK(one.StdDeviationY() == 4.f);
    CHECK(one.EdgeMode() == EdgeModeType::None);

    CHECK(fixture.Primitive(u8"zero").IsIdentity());
    CHECK_FALSE(fixture.Primitive(u8"one").IsIdentity());
  }

  TEST_CASE("SVGFilterPrimitiveStandardAttributes: feColorMatrix",
            "[SVG][Filters][SVGFilterPrimitiveStandardAttributes]")
  {
    PrimitiveFixture fixture(u8"<feColorMatrix id=identity /><feColorMatrix id=saturate type=saturate />"
                             u8"<feColorMatrix id=rotate type=hueRotate values=90 />"
                             u8"<feColorMatrix id=short values='1 2 3' />");

    // Without values, a matrix is the identity, and a saturation leaves the colours as they are.
    auto &identity = fixture.EffectAs<FEColorMatrix>(u8"identity");
    CHECK(identity.GetType() == ColorMatrixType::FECOLORMATRIX_TYPE_MATRIX);
    REQUIRE(identity.Values().size() == 20);
    CHECK(identity.Values()[0] == 1.f);
    CHECK(identity.Values()[1] == 0.f);
    CHECK(identity.Values()[6] == 1.f);
    CHECK(identity.Values()[18] == 1.f);
    CHECK(fixture.EffectAs<FEColorMatrix>(u8"saturate").Values() == List<float> {1});

    auto &rotate = fixture.EffectAs<FEColorMatrix>(u8"rotate");
    CHECK(rotate.GetType() == ColorMatrixType::FECOLORMATRIX_TYPE_HUEROTATE);
    CHECK(rotate.Values() == List<float> {90});

    // A matrix needs 20 values.
    CHECK_FALSE(fixture.Effect(u8"short"));
  }

  TEST_CASE("SVGFilterPrimitiveStandardAttributes: feConvolveMatrix",
            "[SVG][Filters][SVGFilterPrimitiveStandardAttributes]")
  {
    PrimitiveFixture fixture(
      u8"<feConvolveMatrix id=sum kernelMatrix='1 2 1 2 4 2 1 2 1' bias=.5 preserveAlpha=true />"
      u8"<feConvolveMatrix id=sized order='2 1' kernelMatrix='1 -1' divisor=2 targetX=1 edgeMode=none />"
      u8"<feConvolveMatrix id=mismatch kernelMatrix='1 2 3' />"
      u8"<feConvolveMatrix id=target kernelMatrix='1 2 1 2 4 2 1 2 1' targetY=3 />");

    // The divisor defaults to the kernel's sum and the target to its centre.
    auto &sum = fixture.EffectAs<FEConvolveMatrix>(u8"sum");
    CHECK(sum.KernelSize() == IntSize {3, 3});
    CHECK(sum.Divisor() == 16.f);
    CHECK(sum.Bias() == .5f);
    CHECK(sum.TargetOffset() == IntPoint {1, 1});
    CHECK(sum.EdgeMode() == EdgeModeType::Duplicate);
    CHECK(sum.KernelUnitLength() == FloatPoint {1, 1});
    CHECK(sum.PreserveAlpha());

    auto &sized = fixture.EffectAs<FEConvolveMatrix>(u8"sized");
    CHECK(sized.KernelSize() == IntSize {2, 1});
    CHECK(sized.Divisor() == 2.f);
    CHECK(sized.TargetOffset() == IntPoint {1, 0});
    CHECK(sized.EdgeMode() == EdgeModeType::None);
    CHECK(sized.Kernel() == List<float> {1, -1});

    // The kernel must fill the order, and the target must be inside it.
    CHECK_FALSE(fixture.Effect(u8"mismatch"));
    CHECK_FALSE(fixture.Effect(u8"target"));
  }

  TEST_CASE("SVGFilterPrimitiveStandardAttributes: feComponentTransfer",
            "[SVG][Filters][SVGFilterPrimitiveStandardAttributes]")
  {
    PrimitiveFixture fixture(
      u8"<feComponentTransfer id=transfer><feFuncR type=linear slope=2 intercept=.25 />"
      u8"<feFuncA type=table tableValues='0 .5 1' /><feFuncG type=bogus />"
      u8"<feFuncR type=gamma amplitude=3 exponent=2 offset=.1 /></feComponentTransfer>");

    // The last function for a channel wins; a channel without one, or with an unknown type, is the identity.
    auto &transfer = fixture.EffectAs<FEComponentTransfer>(u8"transfer");
    auto red = transfer.RedFunction();
    CHECK(red.Type == ComponentTransferType::FECOMPONENTTRANSFER_TYPE_GAMMA);
    CHECK(red.Amplitude == 3.f);
    CHECK(red.Exponent == 2.f);
    CHECK(red.Offset == .1f);
    CHECK(transfer.AlphaFunction().Type == ComponentTransferType::FECOMPONENTTRANSFER_TYPE_TABLE);
    CHECK(transfer.AlphaFunction().TableValues == List<float> {0, .5f, 1});
    CHECK(transfer.GreenFunction().Type == ComponentTransferType::FECOMPONENTTRANSFER_TYPE_IDENTITY);
    CHECK(transfer.BlueFunction().Type == ComponentTransferType::FECOMPONENTTRANSFER_TYPE_UNKNOWN);
  }

  TEST_CASE("SVGFilterPrimitiveStandardAttributes: feMorphology, feDisplacementMap and feTurbulence",
            "[SVG][Filters][SVGFilterPrimitiveStandardAttributes]")
  {
    PrimitiveFixture fixture(
      u8"<feMorphology id=dilate operator=dilate radius='1 2' /><feMorphology id=none radius=-1 />"
      u8"<feDisplacementMap id=map xChannelSelector=R yChannelSelector=Q scale=10 />"
      u8"<feTurbulence id=noise type=fractalNoise baseFrequency='.1 .2' numOctaves=3 seed=4 "
      u8"stitchTiles=stitch />"
      u8"<feTurbulence id=bad numOctaves=x baseFrequency=-1 />");

    auto &dilate = fixture.EffectAs<FEMorphology>(u8"dilate");
    CHECK(dilate.MorphologyOperator() == MorphologyOperatorType::Dilate);
    CHECK(dilate.RadiusX() == 1.f);
    CHECK(dilate.RadiusY() == 2.f);
    CHECK(fixture.Primitive(u8"none").IsIdentity());
    CHECK_FALSE(fixture.Primitive(u8"dilate").IsIdentity());

    auto &map = fixture.EffectAs<FEDisplacementMap>(u8"map");
    CHECK(map.XChannelSelector() == ChannelSelectorType::CHANNEL_R);
    CHECK(map.YChannelSelector() == ChannelSelectorType::CHANNEL_A);
    CHECK(map.Scale() == 10.f);

    auto &noise = fixture.EffectAs<FETurbulence>(u8"noise");
    CHECK(noise.GetType() == TurbulenceType::FractalNoise);
    CHECK(noise.BaseFrequencyX() == .1f);
    CHECK(noise.BaseFrequencyY() == .2f);
    CHECK(noise.NumOctaves() == 3);
    CHECK(noise.Seed() == 4.f);
    CHECK(noise.StitchTiles());

    // A negative base frequency disables the effect; an octave count that isn't a number is the initial 1.
    CHECK(fixture.Primitive<svg::SVGFETurbulenceElement>(u8"bad").NumOctaves() == 1);
    CHECK_FALSE(fixture.Effect(u8"bad"));
  }

  TEST_CASE("SVGFilterPrimitiveStandardAttributes: feFlood and lighting take their colours from style",
            "[SVG][Filters][SVGFilterPrimitiveStandardAttributes]")
  {
    PrimitiveFixture fixture(
      u8"<feFlood id=flood flood-color=blue flood-opacity=.5 />"
      u8"<feDiffuseLighting id=lit lighting-color=red diffuseConstant=2 surfaceScale=3>"
      u8"<feSpotLight id=light x=1 y=2 z=3 pointsAtX=4 limitingConeAngle=30 /></feDiffuseLighting>"
      u8"<feDiffuseLighting id=dark />");

    auto &flood = fixture.EffectAs<FEFlood>(u8"flood");
    CHECK(flood.FloodColor() == Color {SRGBA<uint8> {0, 0, 255}});
    CHECK(flood.FloodOpacity() == .5f);

    auto &lit = fixture.EffectAs<FEDiffuseLighting>(u8"lit");
    CHECK(lit.LightingColor() == Color {SRGBA<uint8> {255, 0, 0}});
    CHECK(lit.DiffuseConstant() == 2.f);
    CHECK(lit.SurfaceScale() == 3.f);
    auto &spot = Downcast<SpotLightSource>(lit.GetLightSource());
    CHECK(spot.Position() == FloatPoint3D {1, 2, 3});
    CHECK(spot.LimitingConeAngle() == 30.f);

    // Lighting needs a light source.
    CHECK_FALSE(fixture.Effect(u8"dark"));
  }

  TEST_CASE("SVGFilterPrimitiveStandardAttributes: changing an attribute updates the effect",
            "[SVG][Filters][SVGFilterPrimitiveStandardAttributes]")
  {
    PrimitiveFixture fixture(u8"<feOffset id=offset dx=1 /><feTurbulence id=noise stitchTiles=stitch />"
                             u8"<feDiffuseLighting id=lit><fePointLight id=light x=1 /></feDiffuseLighting>");

    auto offset = fixture.Effect(u8"offset");
    REQUIRE(offset);
    fixture.Set(u8"offset", u8"dx", u8"5");
    CHECK(fixture.Effect(u8"offset") == offset);
    CHECK(Downcast<FEOffset>(*offset).Dx() == 5.f);

    // Stitching is turned off by noStitch.
    auto &noise = fixture.EffectAs<FETurbulence>(u8"noise");
    REQUIRE(noise.StitchTiles());
    fixture.Set(u8"noise", u8"stitchTiles", u8"noStitch");
    CHECK_FALSE(noise.StitchTiles());

    // A light's attribute updates its lighting's light source.
    auto &lit = fixture.EffectAs<FEDiffuseLighting>(u8"lit");
    fixture.Set(u8"light", u8"x", u8"7");
    CHECK(Downcast<PointLightSource>(lit.GetLightSource()).Position().X() == 7.f);
  }
}
