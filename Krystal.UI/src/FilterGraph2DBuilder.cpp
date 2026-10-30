#include "Krystal.UI/FilterGraph2DBuilder.hpp"
#include "Krystal.Booey/Graphics/Filters/DistantLightSource.hpp"
#include "Krystal.Booey/Graphics/Filters/FEBlend.hpp"
#include "Krystal.Booey/Graphics/Filters/FEColorMatrix.hpp"
#include "Krystal.Booey/Graphics/Filters/FEComponentTransfer.hpp"
#include "Krystal.Booey/Graphics/Filters/FEComposite.hpp"
#include "Krystal.Booey/Graphics/Filters/FEConvolveMatrix.hpp"
#include "Krystal.Booey/Graphics/Filters/FEDiffuseLighting.hpp"
#include "Krystal.Booey/Graphics/Filters/FEDisplacementMap.hpp"
#include "Krystal.Booey/Graphics/Filters/FEDropShadow.hpp"
#include "Krystal.Booey/Graphics/Filters/FEFlood.hpp"
#include "Krystal.Booey/Graphics/Filters/FEGaussianBlur.hpp"
#include "Krystal.Booey/Graphics/Filters/FEImage.hpp"
#include "Krystal.Booey/Graphics/Filters/FEMerge.hpp"
#include "Krystal.Booey/Graphics/Filters/FEMorphology.hpp"
#include "Krystal.Booey/Graphics/Filters/FEOffset.hpp"
#include "Krystal.Booey/Graphics/Filters/FESpecularLighting.hpp"
#include "Krystal.Booey/Graphics/Filters/FETile.hpp"
#include "Krystal.Booey/Graphics/Filters/FETurbulence.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Booey/Graphics/Filters/SpotLightSource.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.Core/Colour/ColourMatrix.hpp"
#include "Krystal.UI/ImageBufferTextures.hpp"
#include <algorithm>
#include <cmath>

namespace krys::ui
{
  namespace
  {
    /// @brief Applies one effect by appending its nodes to a builder's graph.
    class FilterGraph2DApplier final : public boo::FilterEffectApplier
    {
    private:
      FilterGraph2DBuilder &_builder;
      const boo::FilterEffect &_effect;

    public:
      FilterGraph2DApplier(FilterGraph2DBuilder &builder, const boo::FilterEffect &effect) noexcept
          : _builder(builder), _effect(effect)
      {
      }

      KRYS_NODISCARD bool Apply(const boo::Filter &filter, Span<const Ref<boo::FilterImage>> inputs,
                                boo::FilterImage &result) const noexcept override
      {
        return _builder.Apply(_effect, filter, inputs, result);
      }
    };

    KRYS_NODISCARD gfx::PixelRect2D ToPixelRect(const boo::IntRect &rect) noexcept
    {
      return {.X = rect.X(),
              .Y = rect.Y(),
              .Width = static_cast<uint32>(std::max(rect.Width(), 0)),
              .Height = static_cast<uint32>(std::max(rect.Height(), 0))};
    }

    KRYS_NODISCARD gfx::FilterColourSpace2D
      ToColourSpace(const boo::DestinationColorSpace &colorSpace) noexcept
    {
      return colorSpace == boo::DestinationColorSpace::LinearSRGB() ? gfx::FilterColourSpace2D::LinearSRGB
                                                                    : gfx::FilterColourSpace2D::SRGB;
    }

    KRYS_NODISCARD ColourSpace ToColourSpaceOf(const boo::DestinationColorSpace &colorSpace) noexcept
    {
      return colorSpace == boo::DestinationColorSpace::LinearSRGB() ? ColourSpace::LinearSRGB
                                                                    : ColourSpace::SRGB;
    }

    /// @brief @p color's straight components in @p colorSpace, from 0 to 1.
    KRYS_NODISCARD Array<float, 4> ComponentsOf(const boo::Color &color,
                                                const boo::DestinationColorSpace &colorSpace) noexcept
    {
      auto components = color.ToResolvedColorComponentsInColorSpace(ToColourSpaceOf(colorSpace));
      return {std::clamp(components[0], 0.f, 1.f), std::clamp(components[1], 0.f, 1.f),
              std::clamp(components[2], 0.f, 1.f), std::clamp(components[3], 0.f, 1.f)};
    }

    /// @brief A node over @p result's pixels, in its colour space.
    KRYS_NODISCARD gfx::FilterNode2D NodeFor(gfx::FilterNode2D::Kind kind,
                                             const boo::FilterImage &result) noexcept
    {
      return {.Type = kind,
              .Region = ToPixelRect(result.AbsoluteImageRect()),
              .ColourSpace = ToColourSpace(result.ColorSpace())};
    }

    KRYS_NODISCARD gfx::FilterEdgeMode2D ToEdgeMode(boo::EdgeModeType edgeMode) noexcept
    {
      switch (edgeMode)
      {
        case boo::EdgeModeType::Duplicate: return gfx::FilterEdgeMode2D::Duplicate;
        case boo::EdgeModeType::Wrap:      return gfx::FilterEdgeMode2D::Wrap;
        default:                           return gfx::FilterEdgeMode2D::None;
      }
    }

    KRYS_NODISCARD gfx::FilterChannel2D ToChannel(boo::ChannelSelectorType channel) noexcept
    {
      switch (channel)
      {
        case boo::ChannelSelectorType::CHANNEL_R: return gfx::FilterChannel2D::Red;
        case boo::ChannelSelectorType::CHANNEL_G: return gfx::FilterChannel2D::Green;
        case boo::ChannelSelectorType::CHANNEL_B: return gfx::FilterChannel2D::Blue;
        default:                                  return gfx::FilterChannel2D::Alpha;
      }
    }

    KRYS_NODISCARD gfx::FilterCompositeOperator2D ToOperator(boo::CompositeOperationType type) noexcept
    {
      switch (type)
      {
        case boo::CompositeOperationType::FECOMPOSITE_OPERATOR_IN: return gfx::FilterCompositeOperator2D::In;
        case boo::CompositeOperationType::FECOMPOSITE_OPERATOR_OUT:
          return gfx::FilterCompositeOperator2D::Out;
        case boo::CompositeOperationType::FECOMPOSITE_OPERATOR_ATOP:
          return gfx::FilterCompositeOperator2D::Atop;
        case boo::CompositeOperationType::FECOMPOSITE_OPERATOR_XOR:
          return gfx::FilterCompositeOperator2D::Xor;
        case boo::CompositeOperationType::FECOMPOSITE_OPERATOR_ARITHMETIC:
          return gfx::FilterCompositeOperator2D::Arithmetic;
        case boo::CompositeOperationType::FECOMPOSITE_OPERATOR_LIGHTER:
          return gfx::FilterCompositeOperator2D::Lighter;
        default: return gfx::FilterCompositeOperator2D::Over;
      }
    }

    template <size_t TColumns, size_t TRows>
    KRYS_NODISCARD Array<float, 20> MatrixOf(const ColourMatrix<TColumns, TRows> &matrix) noexcept
    {
      Array<float, 20> result {};
      const auto data = ColourMatrix<5, 4>(matrix).data();
      std::copy_n(data.begin(), 20u, result.begin());
      return result;
    }

    /// @brief The matrix of an `feColorMatrix`, rows of five.
    KRYS_NODISCARD Array<float, 20> MatrixOf(const boo::FEColorMatrix &effect) noexcept
    {
      const auto &values = effect.Values();
      switch (effect.GetType())
      {
        case boo::ColorMatrixType::FECOLORMATRIX_TYPE_MATRIX:
        {
          if (values.size() != 20u)
          {
            break;
          }
          Array<float, 20> result {};
          std::copy_n(values.begin(), 20u, result.begin());
          return result;
        }
        case boo::ColorMatrixType::FECOLORMATRIX_TYPE_SATURATE:
          return MatrixOf(SaturationColourMatrix(values.empty() ? 1.f : values[0]));
        case boo::ColorMatrixType::FECOLORMATRIX_TYPE_HUEROTATE:
          return MatrixOf(HueRotateColourMatrix(values.empty() ? 0.f : values[0]));
        case boo::ColorMatrixType::FECOLORMATRIX_TYPE_LUMINANCETOALPHA:
          return {0.f, 0.f, 0.f, 0.f, 0.f, 0.f,     0.f,     0.f,     0.f, 0.f,
                  0.f, 0.f, 0.f, 0.f, 0.f, 0.2125f, 0.7154f, 0.0721f, 0.f, 0.f};
        default: break;
      }
      // The identity.
      return gfx::FilterNode2D {}.Matrix;
    }
  }

  FilterGraph2DBuilder::FilterGraph2DBuilder(ImageBufferTextures &imageBuffers,
                                             gfx::CommandBuffer &commands) noexcept
      : _imageBuffers(imageBuffers), _commands(commands)
  {
  }

  FilterGraph2DBuilder::~FilterGraph2DBuilder() noexcept = default;

  UniquePtr<boo::FilterEffectApplier>
    FilterGraph2DBuilder::CreateApplier(const boo::FilterEffect &effect) noexcept
  {
    return CreateUnique<FilterGraph2DApplier>(*this, effect);
  }

  bool FilterGraph2DBuilder::Finish(boo::FilterImage &result) noexcept
  {
    auto input = InputFor(result);
    if (!input)
    {
      return false;
    }

    // The result is drawn in sRGB; the last node is the graph's result.
    const bool isLastNode =
      input->Type == gfx::FilterInput2D::Kind::Node && input->Index + 1u == _graph.Nodes.size();
    if (isLastNode && _graph.Nodes.back().ColourSpace == gfx::FilterColourSpace2D::SRGB)
    {
      return true;
    }
    AddNode({.Type = gfx::FilterNode2D::Kind::Offset,
             .Inputs = {*input},
             .Region = ToPixelRect(result.AbsoluteImageRect()),
             .ColourSpace = gfx::FilterColourSpace2D::SRGB});
    return true;
  }

  Maybe<gfx::FilterInput2D> FilterGraph2DBuilder::InputFor(boo::FilterImage &image) noexcept
  {
    if (auto node = image.AcceleratedImage())
    {
      return gfx::FilterInput2D::Node(*node);
    }
    if (auto *buffer = image.GetImageBuffer())
    {
      return SourceFor(*buffer, ToPixelRect(image.AbsoluteImageRect()));
    }
    return null;
  }

  Maybe<gfx::FilterInput2D> FilterGraph2DBuilder::SourceFor(boo::ImageBuffer &buffer,
                                                            const gfx::PixelRect2D &region) noexcept
  {
    const auto identifier = buffer.GetRenderingResourceIdentifier().ToUInt64();
    if (auto it = _sources.find(identifier); it != _sources.end())
    {
      return gfx::FilterInput2D::Source(it->second);
    }

    const auto texture = _imageBuffers.Render(buffer, _commands);
    if (!texture.IsValid())
    {
      return null;
    }
    const auto index = static_cast<uint32>(_graph.Sources.size());
    _graph.Sources.push_back({.Texture = texture, .Region = region});
    _sources.emplace(identifier, index);
    return gfx::FilterInput2D::Source(index);
  }

  uint32 FilterGraph2DBuilder::AddNode(gfx::FilterNode2D &&node) noexcept
  {
    _graph.Nodes.push_back(krys::move(node));
    return static_cast<uint32>(_graph.Nodes.size() - 1u);
  }

  bool FilterGraph2DBuilder::Apply(const boo::FilterEffect &effect, const boo::Filter &filter,
                                   Span<const Ref<boo::FilterImage>> inputs,
                                   boo::FilterImage &result) noexcept
  {
    List<gfx::FilterInput2D> reads;
    reads.reserve(inputs.size());
    for (const auto &input : inputs)
    {
      auto read = InputFor(*input);
      if (!read)
      {
        return false;
      }
      reads.push_back(*read);
    }
    auto readAt = [&](size_t index)
    {
      return index < reads.size() ? reads[index] : gfx::FilterInput2D {};
    };

    const auto scaledSize = [&](float width, float height)
    {
      return filter.ScaledByFilterScale(filter.ResolvedSize({width, height}));
    };

    using Kind = gfx::FilterNode2D::Kind;
    auto node = NodeFor(Kind::Offset, result);
    node.Inputs = {readAt(0u), readAt(1u)};

    switch (effect.FilterType())
    {
      case boo::FilterFunction::Type::SourceGraphic:
        // A copy of the source, in the effect's colour space.
        break;

      case boo::FilterFunction::Type::SourceAlpha:
        node.Type = Kind::ColourMatrix;
        node.Matrix = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
                       0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f};
        break;

      case boo::FilterFunction::Type::FEOffset:
      {
        const auto &offset = Downcast<boo::FEOffset>(effect);
        const auto absoluteOffset = scaledSize(offset.Dx(), offset.Dy());
        node.Offset = {absoluteOffset.Width(), absoluteOffset.Height()};
        break;
      }

      case boo::FilterFunction::Type::FEFlood:
      {
        const auto &flood = Downcast<boo::FEFlood>(effect);
        auto colour = ComponentsOf(flood.FloodColor(), result.ColorSpace());
        colour[3] *= std::clamp(flood.FloodOpacity(), 0.f, 1.f);
        node.Type = Kind::Flood;
        node.Colour = {colour[0] * colour[3], colour[1] * colour[3], colour[2] * colour[3], colour[3]};
        break;
      }

      case boo::FilterFunction::Type::FEBlend:
        node.Type = Kind::Blend;
        node.Blend = Downcast<boo::FEBlend>(effect).GetBlendMode();
        break;

      case boo::FilterFunction::Type::FEComposite:
      {
        const auto &composite = Downcast<boo::FEComposite>(effect);
        node.Type = Kind::Composite;
        node.Operator = ToOperator(composite.Operation());
        node.K = {composite.K1(), composite.K2(), composite.K3(), composite.K4()};
        break;
      }

      case boo::FilterFunction::Type::FEColorMatrix:
        node.Type = Kind::ColourMatrix;
        node.Matrix = MatrixOf(Downcast<boo::FEColorMatrix>(effect));
        break;

      case boo::FilterFunction::Type::FEComponentTransfer:
      {
        const auto &transfer = Downcast<boo::FEComponentTransfer>(effect);
        const Array<boo::FEComponentTransfer::LookupTable, 4> tables {
          boo::FEComponentTransfer::ComputeLookupTable(transfer.RedFunction()),
          boo::FEComponentTransfer::ComputeLookupTable(transfer.GreenFunction()),
          boo::FEComponentTransfer::ComputeLookupTable(transfer.BlueFunction()),
          boo::FEComponentTransfer::ComputeLookupTable(transfer.AlphaFunction())};
        node.Type = Kind::ComponentTransfer;
        node.Table.resize(256u);
        for (size_t i = 0u; i < 256u; ++i)
        {
          node.Table[i] = {tables[0][i], tables[1][i], tables[2][i], tables[3][i]};
        }
        break;
      }

      case boo::FilterFunction::Type::FEConvolveMatrix:
      {
        const auto &convolve = Downcast<boo::FEConvolveMatrix>(effect);
        const auto kernelSize = convolve.KernelSize();
        node.Type = Kind::ConvolveMatrix;
        node.Order = {static_cast<uint32>(std::max(kernelSize.Width(), 0)),
                      static_cast<uint32>(std::max(kernelSize.Height(), 0))};
        node.Kernel = convolve.Kernel();
        node.Target = {convolve.TargetOffset().X(), convolve.TargetOffset().Y()};
        node.Divisor = convolve.Divisor();
        node.Bias = convolve.Bias();
        node.PreserveAlpha = convolve.PreserveAlpha();
        node.EdgeMode = ToEdgeMode(convolve.EdgeMode());
        break;
      }

      case boo::FilterFunction::Type::FEMorphology:
      {
        const auto &morphology = Downcast<boo::FEMorphology>(effect);
        const auto radius = scaledSize(morphology.RadiusX(), morphology.RadiusY());
        node.Type = Kind::Morphology;
        node.Radius = {static_cast<int32>(std::floor(radius.Width())),
                       static_cast<int32>(std::floor(radius.Height()))};
        node.Dilate = morphology.MorphologyOperator() == boo::MorphologyOperatorType::Dilate;
        break;
      }

      case boo::FilterFunction::Type::FEGaussianBlur:
      {
        const auto &blur = Downcast<boo::FEGaussianBlur>(effect);
        const auto stdDeviation = scaledSize(blur.StdDeviationX(), blur.StdDeviationY());
        node.Type = Kind::Blur;
        node.StandardDeviation = {stdDeviation.Width(), stdDeviation.Height()};
        node.EdgeMode = ToEdgeMode(blur.EdgeMode());
        break;
      }

      case boo::FilterFunction::Type::FEDropShadow:
      {
        // The input's alpha in the shadow's colour, blurred and moved, under the input.
        const auto &shadow = Downcast<boo::FEDropShadow>(effect);
        auto colour = ComponentsOf(shadow.ShadowColor(), result.ColorSpace());
        auto alpha = node;
        alpha.Type = Kind::ColourMatrix;
        alpha.Matrix = {0.f,
                        0.f,
                        0.f,
                        0.f,
                        colour[0],
                        0.f,
                        0.f,
                        0.f,
                        0.f,
                        colour[1],
                        0.f,
                        0.f,
                        0.f,
                        0.f,
                        colour[2],
                        0.f,
                        0.f,
                        0.f,
                        colour[3] * shadow.ShadowOpacity(),
                        0.f};
        const auto alphaIndex = AddNode(krys::move(alpha));

        const auto stdDeviation = scaledSize(shadow.StdDeviationX(), shadow.StdDeviationY());
        auto blur = NodeFor(Kind::Blur, result);
        blur.Inputs = {gfx::FilterInput2D::Node(alphaIndex)};
        blur.StandardDeviation = {stdDeviation.Width(), stdDeviation.Height()};
        const auto blurIndex = AddNode(krys::move(blur));

        const auto offset = scaledSize(shadow.Dx(), shadow.Dy());
        auto moved = NodeFor(Kind::Offset, result);
        moved.Inputs = {gfx::FilterInput2D::Node(blurIndex)};
        moved.Offset = {offset.Width(), offset.Height()};
        const auto movedIndex = AddNode(krys::move(moved));

        node.Type = Kind::Composite;
        node.Inputs = {readAt(0u), gfx::FilterInput2D::Node(movedIndex)};
        break;
      }

      case boo::FilterFunction::Type::FEDisplacementMap:
      {
        const auto &displacement = Downcast<boo::FEDisplacementMap>(effect);
        const auto scale = scaledSize(displacement.Scale(), displacement.Scale());
        node.Type = Kind::DisplacementMap;
        node.Offset = {scale.Width(), scale.Height()};
        node.XChannel = ToChannel(displacement.XChannelSelector());
        node.YChannel = ToChannel(displacement.YChannelSelector());
        break;
      }

      case boo::FilterFunction::Type::FEDiffuseLighting:
      case boo::FilterFunction::Type::FESpecularLighting:
      {
        const auto &lighting = static_cast<const boo::FELighting &>(effect);
        const auto colour = ComponentsOf(lighting.LightingColor(), result.ColorSpace());
        node.Type = effect.FilterType() == boo::FilterFunction::Type::FEDiffuseLighting
                      ? Kind::DiffuseLighting
                      : Kind::SpecularLighting;
        node.SurfaceScale = lighting.SurfaceScale();
        node.DiffuseConstant = lighting.DiffuseConstant();
        node.SpecularConstant = lighting.SpecularConstant();
        node.SpecularExponent = lighting.SpecularExponent();
        node.LightingColour = {colour[0], colour[1], colour[2]};

        auto &light = lighting.GetLightSource();
        boo::LightSource::PaintingData paintingData;
        light.InitPaintingData(filter, result, paintingData);
        switch (light.Type())
        {
          case boo::LightType::LS_DISTANT:
          {
            const auto &vector = paintingData.InitialLightingData.LightVector;
            node.Light.Type = gfx::FilterLight2D::Kind::Distant;
            node.Light.Vector = {vector.X(), vector.Y(), vector.Z()};
            break;
          }
          case boo::LightType::LS_POINT:
          case boo::LightType::LS_SPOT:
          {
            // The light's position in the result's pixels: its vector from the result's origin at height 0.
            const auto position = light.ComputePixelLightingData(paintingData, 0, 0, 0.f).LightVector;
            node.Light.Vector = {position.X(), position.Y(), position.Z()};
            if (light.Type() == boo::LightType::LS_POINT)
            {
              node.Light.Type = gfx::FilterLight2D::Kind::Point;
              break;
            }
            const auto &direction = paintingData.DirectionVector;
            node.Light.Type = gfx::FilterLight2D::Kind::Spot;
            node.Light.Direction = {direction.X(), direction.Y(), direction.Z()};
            node.Light.SpecularExponent = Downcast<boo::SpotLightSource>(light).SpecularExponent();
            node.Light.ConeCutOff = paintingData.ConeCutOffLimit;
            node.Light.ConeFullLight = paintingData.ConeFullLight;
            break;
          }
        }
        break;
      }

      case boo::FilterFunction::Type::FETurbulence:
      {
        const auto &turbulence = Downcast<boo::FETurbulence>(effect);
        const auto tileSize = result.PrimitiveSubregion().Size();
        node.Type = Kind::Turbulence;
        node.FractalNoise = turbulence.GetType() == boo::TurbulenceType::FractalNoise;
        node.BaseFrequency = {turbulence.BaseFrequencyX(), turbulence.BaseFrequencyY()};
        node.Octaves = static_cast<uint32>(std::max(turbulence.NumOctaves(), 0));
        node.Seed = static_cast<int32>(turbulence.Seed());
        node.StitchTiles = turbulence.StitchTiles();
        node.StitchTileSize = {tileSize.Width(), tileSize.Height()};
        node.Scale = {filter.FilterScale().Width(), filter.FilterScale().Height()};
        break;
      }

      case boo::FilterFunction::Type::FETile:
      {
        if (inputs.empty())
        {
          return false;
        }
        auto tileRect = inputs[0]->MaxEffectRect(filter);
        tileRect.Scale(filter.FilterScale());
        node.Type = Kind::Tile;
        node.TileRect = ToPixelRect(EnclosingIntRect(tileRect));
        break;
      }

      case boo::FilterFunction::Type::FEMerge:
      {
        // The inputs drawn over one another, the first lowest.
        if (reads.empty())
        {
          node.Type = Kind::Flood;
          break;
        }
        for (size_t i = 1u; i < reads.size(); ++i)
        {
          auto over = NodeFor(Kind::Composite, result);
          over.Inputs = {reads[i], node.Inputs[0]};
          node.Inputs = {gfx::FilterInput2D::Node(AddNode(krys::move(over)))};
        }
        if (reads.size() > 1u)
        {
          result.SetAcceleratedImage(node.Inputs[0].Index);
          return true;
        }
        break;
      }

      case boo::FilterFunction::Type::FEImage:
      {
        // The image drawn into a buffer over the result, as it's drawn in software.
        const auto &image = Downcast<boo::FEImage>(effect);
        const auto absoluteImageRect = result.AbsoluteImageRect();
        auto buffer = boo::ImageBuffer::Create(boo::FloatSize(absoluteImageRect.Size()), 1);
        if (!buffer)
        {
          return false;
        }
        auto &context = buffer->Context();
        const auto &sourceImage = image.GetSourceImage();
        const auto primitiveSubregion = result.PrimitiveSubregion();
        if (auto *nativeImage = sourceImage.NativeImageIfExists())
        {
          auto imageRect = primitiveSubregion;
          auto srcRect = image.SourceImageRect();
          image.PreserveAspectRatio().TransformRect(imageRect, srcRect);
          imageRect.Scale(filter.FilterScale());
          imageRect = boo::FloatRect(boo::IntRect(imageRect) - absoluteImageRect.Location());
          context.DrawNativeImage(*nativeImage, imageRect, srcRect);
        }
        else if (auto *imageBuffer = sourceImage.ImageBufferIfExists())
        {
          auto imageRect = primitiveSubregion;
          imageRect.MoveBy(image.SourceImageRect().Location());
          imageRect.Scale(filter.FilterScale());
          imageRect = boo::FloatRect(boo::IntRect(imageRect) - absoluteImageRect.Location());
          context.DrawImageBuffer(*imageBuffer, imageRect.Location());
        }
        else
        {
          return false;
        }

        Ref<boo::ImageBuffer> drawn = ShareRef(*buffer);
        auto source = SourceFor(*drawn, ToPixelRect(absoluteImageRect));
        _images.push_back(krys::move(drawn));
        if (!source)
        {
          return false;
        }
        node.Inputs = {*source};
        break;
      }

      default: return false;
    }

    result.SetAcceleratedImage(AddNode(krys::move(node)));
    return true;
  }
}
