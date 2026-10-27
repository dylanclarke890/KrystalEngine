#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/Filters/DistantLightSource.hpp"
#include "Krystal.Booey/Graphics/Filters/FEBlend.hpp"
#include "Krystal.Booey/Graphics/Filters/FEComponentTransfer.hpp"
#include "Krystal.Booey/Graphics/Filters/FEComposite.hpp"
#include "Krystal.Booey/Graphics/Filters/FEDiffuseLighting.hpp"
#include "Krystal.Booey/Graphics/Filters/FEDisplacementMap.hpp"
#include "Krystal.Booey/Graphics/Filters/FEFlood.hpp"
#include "Krystal.Booey/Graphics/Filters/FEMorphology.hpp"
#include "Krystal.Booey/Graphics/Filters/FEOffset.hpp"
#include "Krystal.Booey/Graphics/Filters/FETurbulence.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterResults.hpp"
#include "Krystal.Booey/Graphics/Filters/PointLightSource.hpp"
#include "Krystal.Booey/Graphics/Filters/SourceGraphic.hpp"
#include "Krystal.Booey/Graphics/Filters/SpotLightSource.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Rendering/CSSFilterRenderer.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using Catch::Approx;

    /// @brief Applies nothing, numbering the results it's asked for.
    class CountingApplier final : public FilterEffectApplier
    {
    private:
      uint32 &_count;

    public:
      explicit CountingApplier(uint32 &count) noexcept : _count(count)
      {
      }

      KRYS_NODISCARD bool Apply(const Filter &, Span<const Ref<FilterImage>>,
                                FilterImage &result) const noexcept override
      {
        result.SetAcceleratedImage(_count++);
        return true;
      }
    };

    KRYS_NODISCARD FilterResults CountingResults(uint32 &count) noexcept
    {
      return FilterResults([&count](const FilterEffect &) -> UniquePtr<FilterEffectApplier>
                           { return CreateUnique<CountingApplier>(count); });
    }

    constexpr FloatRect FilterRegion {0, 0, 32, 32};

    KRYS_NODISCARD Ref<Filter> FilterOf(List<Ref<FilterFunction>> &&functions,
                                        FloatSize scale = {1, 1}) noexcept
    {
      return CSSFilterRenderer::Create(krys::move(functions), {FilterRegion, FilterRegion, scale},
                                       FilterRenderingMode::Accelerated);
    }

    template <typename T>
    KRYS_NODISCARD utf8_string Representation(const T &effect) noexcept
    {
      TextStream ts;
      static_cast<const FilterFunction &>(effect).ExternalRepresentation(ts,
                                                                         FilterRepresentation::TestOutput);
      return ts.Release();
    }
  }

  TEST_CASE("Filter::Apply", "[Graphics][Filters][FilterEffect]")
  {
    auto source = ImageBuffer::Create({16, 16}, 1);
    REQUIRE(source);
    uint32 count = 0u;
    auto results = CountingResults(count);

    SECTION("Each effect's image rect follows from its input's")
    {
      SECTION("An offset moves it")
      {
        auto filter = FilterOf({SourceGraphic::Create(), FEOffset::Create(4, 2)});
        auto result = filter->Apply(source.get(), {0, 0, 16, 16}, results);
        REQUIRE(result);
        CHECK(result->ImageRect() == FloatRect {4, 2, 16, 16});
        CHECK(result->AbsoluteImageRect() == IntRect {4, 2, 16, 16});
        CHECK(result->PrimitiveSubregion() == FilterRegion);
        CHECK(result->AcceleratedImage() == 1u);
        CHECK(count == 2u);
      }

      SECTION("A morphology inflates it, within the filter region")
      {
        auto filter =
          FilterOf({SourceGraphic::Create(), FEMorphology::Create(MorphologyOperatorType::Dilate, 2, 3)});
        auto result = filter->Apply(source.get(), {0, 0, 16, 16}, results);
        REQUIRE(result);
        CHECK(result->ImageRect() == FloatRect {0, 0, 18, 19});
      }

      SECTION("The filter scale scales the absolute rect")
      {
        auto filter = FilterOf({SourceGraphic::Create(), FEOffset::Create(4, 2)}, {2, 2});
        auto result = filter->Apply(source.get(), {0, 0, 16, 16}, results);
        REQUIRE(result);
        CHECK(result->AbsoluteImageRect() == IntRect {8, 4, 32, 32});
      }
    }

    SECTION("Results are in the effect's operating colour space")
    {
      auto offset = FEOffset::Create(0, 0);
      offset->SetOperatingColorSpace(DestinationColorSpace::LinearSRGB());
      auto filter = FilterOf({SourceGraphic::Create(), offset});
      auto result = filter->Apply(source.get(), {0, 0, 16, 16}, results);
      REQUIRE(result);
      CHECK(result->ColorSpace() == DestinationColorSpace::LinearSRGB());
    }

    SECTION("Without an accelerated backend, nothing is applied")
    {
      FilterResults none;
      auto filter = FilterOf({SourceGraphic::Create(), FEOffset::Create(4, 2)});
      CHECK_FALSE(filter->Apply(source.get(), {0, 0, 16, 16}, none));
    }

    SECTION("An effect without inputs takes its image rect from its subregion")
    {
      auto filter = FilterOf({});
      auto flood = FEFlood::Create(Color(SRGBA<uint8> {0, 0, 0}), 1);
      auto result = flood->Apply(*filter, {}, results);
      REQUIRE(result);
      CHECK(result->ImageRect() == FilterRegion);
    }
  }

  TEST_CASE("FilterEffect", "[Graphics][Filters][FilterEffect]")
  {
    auto image = [](float x)
    {
      return FilterImage::Create({}, {x, 0, 1, 1}, {}, false, true, DestinationColorSpace::SRGB());
    };

    SECTION("Inputs are taken from the top of the stack, the topmost first")
    {
      FilterImageVector stack {image(0), image(1), image(2)};
      auto inputs = FEBlend::Create(BlendMode::Multiply)->TakeImageInputs(stack);
      REQUIRE(inputs.size() == 2u);
      CHECK(inputs[0]->ImageRect().X() == 2.f);
      CHECK(inputs[1]->ImageRect().X() == 1.f);
      REQUIRE(stack.size() == 1u);
      CHECK(stack[0]->ImageRect().X() == 0.f);

      FilterImageVector tooFew {image(0)};
      CHECK(FEBlend::Create(BlendMode::Multiply)->TakeImageInputs(tooFew).empty());
    }

    SECTION("A displacement map's result is in its first input's colour space")
    {
      auto filter = FilterOf({});
      uint32 count = 0u;
      auto results = CountingResults(count);
      auto displacement =
        FEDisplacementMap::Create(ChannelSelectorType::CHANNEL_R, ChannelSelectorType::CHANNEL_G, 4);
      displacement->SetOperatingColorSpace(DestinationColorSpace::LinearSRGB());
      FilterImageVector inputs {FilterImage::Create(FilterRegion, FilterRegion, {0, 0, 32, 32}, false, true,
                                                    DestinationColorSpace::SRGB()),
                                FilterImage::Create(FilterRegion, FilterRegion, {0, 0, 32, 32}, false, true,
                                                    DestinationColorSpace::LinearSRGB())};
      auto result = displacement->Apply(*filter, inputs, results);
      REQUIRE(result);
      CHECK(result->ColorSpace() == DestinationColorSpace::SRGB());
    }

    SECTION("Equal lighting effects have equal lights")
    {
      auto a = FEDiffuseLighting::Create(Color(SRGBA<uint8> {255, 255, 255}), 1, 1, 0, 0,
                                         DistantLightSource::Create(10, 20));
      auto b = FEDiffuseLighting::Create(Color(SRGBA<uint8> {255, 255, 255}), 1, 1, 0, 0,
                                         DistantLightSource::Create(10, 20));
      auto c = FEDiffuseLighting::Create(Color(SRGBA<uint8> {255, 255, 255}), 1, 1, 0, 0,
                                         DistantLightSource::Create(10, 30));
      CHECK(*a == *b);
      CHECK_FALSE(*a == *c);
    }

    SECTION("Representations")
    {
      CHECK(Representation(
              *FEComposite::Create(CompositeOperationType::FECOMPOSITE_OPERATOR_ARITHMETIC, 1, 0, 0, 0.5f))
            == u8"[feComposite operation=\"ARITHMETIC\" k1=\"1.00\" k2=\"0.00\" k3=\"0.00\" k4=\"0.50\"]\n");
      CHECK(Representation(*FETurbulence::Create(TurbulenceType::FractalNoise, 0.1f, 0.2f, 3, 7, true))
            == u8"[feTurbulence type=\"NOISE\" baseFrequency=\"0.10, 0.20\" seed=\"7.00\" numOctaves=\"3\" "
               u8"stitchTiles=\"1\"]\n");
      CHECK(Representation(*FEBlend::Create(BlendMode::Multiply)) == u8"[feBlend mode=\"multiply\"]\n");
    }
  }

  TEST_CASE("FEComponentTransfer::ComputeLookupTable", "[Graphics][Filters][FilterEffect]")
  {
    using Function = ComponentTransferFunction;
    using Type = ComponentTransferType;

    auto identity = FEComponentTransfer::ComputeLookupTable({});
    CHECK(identity[0] == 0u);
    CHECK(identity[200] == 200u);

    auto table = FEComponentTransfer::ComputeLookupTable(
      Function {.Type = Type::FECOMPONENTTRANSFER_TYPE_TABLE, .TableValues = {1, 0}});
    CHECK(table[0] == 255u);
    CHECK(table[255] == 0u);

    auto discrete = FEComponentTransfer::ComputeLookupTable(
      Function {.Type = Type::FECOMPONENTTRANSFER_TYPE_DISCRETE, .TableValues = {0, 1}});
    CHECK(discrete[127] == 0u);
    CHECK(discrete[128] == 255u);

    auto linear = FEComponentTransfer::ComputeLookupTable(
      Function {.Type = Type::FECOMPONENTTRANSFER_TYPE_LINEAR, .Slope = 0.5f, .Intercept = 0.25f});
    CHECK(linear[100] == 113u);
    CHECK(linear[255] == 191u);

    auto gamma = FEComponentTransfer::ComputeLookupTable(
      Function {.Type = Type::FECOMPONENTTRANSFER_TYPE_GAMMA, .Amplitude = 1, .Exponent = 2, .Offset = 0});
    CHECK(gamma[128] == 64u);
    CHECK(gamma[255] == 255u);
  }

  TEST_CASE("FilterResults", "[Graphics][Filters][FilterEffect]")
  {
    FilterResults results;
    auto image = []
    {
      return FilterImage::Create({}, {}, {0, 0, 1, 1}, false, true, DestinationColorSpace::SRGB());
    };
    Ref<FilterEffect> a = FEOffset::Create(1, 0);
    Ref<FilterEffect> b = FEOffset::Create(2, 0);
    auto source = image();
    auto resultA = image();
    auto resultB = image();

    results.SetEffectResult(*a, Span<const Ref<FilterImage>>(&source, 1u), Ref(resultA));
    results.SetEffectResult(*b, Span<const Ref<FilterImage>>(&resultA, 1u), Ref(resultB));
    CHECK(results.EffectResult(*a) == resultA.get());
    CHECK(results.EffectResult(*b) == resultB.get());

    // Clearing a result clears the results that read it.
    results.ClearEffectResult(*a);
    CHECK(results.EffectResult(*a) == nullptr);
    CHECK(results.EffectResult(*b) == nullptr);
  }

  TEST_CASE("LightSource", "[Graphics][Filters][FilterEffect]")
  {
    auto filter = FilterOf({}, {2, 2});
    auto result = FilterImage::Create(FilterRegion, FilterRegion, {10, 20, 64, 64}, false, true,
                                      DestinationColorSpace::SRGB());
    LightSource::PaintingData data;

    SECTION("A distant light points along its azimuth and elevation")
    {
      DistantLightSource::Create(0, 90)->InitPaintingData(*filter, *result, data);
      CHECK(data.InitialLightingData.LightVector.X() == Approx(0).margin(1e-6));
      CHECK(data.InitialLightingData.LightVector.Z() == Approx(1));
    }

    SECTION("A point light is placed in the result's pixels, its height scaled as across")
    {
      auto light = PointLightSource::Create({15, 15, 5});
      light->InitPaintingData(*filter, *result, data);
      auto position = light->ComputePixelLightingData(data, 0, 0, 0).LightVector;
      CHECK(position == FloatPoint3D {20, 10, 10});
    }

    SECTION("A spot light's cone")
    {
      SpotLightSource::Create({0, 0, 10}, {0, 0, 0}, 1, 30)->InitPaintingData(*filter, *result, data);
      CHECK(data.ConeCutOffLimit == Approx(-0.866f).margin(1e-3));
      CHECK(data.ConeFullLight == Approx(data.ConeCutOffLimit - 0.016f));
      CHECK(data.DirectionVector == FloatPoint3D {0, 0, -1});

      SpotLightSource::Create({0, 0, 10}, {0, 0, 0}, 1, 0)->InitPaintingData(*filter, *result, data);
      CHECK(data.ConeCutOffLimit == 0.f);
    }
  }
}
