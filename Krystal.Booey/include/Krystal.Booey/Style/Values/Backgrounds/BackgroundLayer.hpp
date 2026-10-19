#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/BackgroundSize.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/RepeatStyle.hpp"
#include "Krystal.Booey/Style/Values/Images/ImageOrNone.hpp"
#include "Krystal.Booey/Style/Values/Masking/MaskMode.hpp"
#include "Krystal.Booey/Style/Values/Primitives/CoordinatedValueListValue.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Position.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <tuple>

namespace krys::boo
{
  class RenderElement;
}

namespace krys::boo::style
{
  // macro(ownerType, property, type, lowercaseName, uppercaseName)

  // clang-format off
#define KRYS_FOR_EACH_BACKGROUND_LAYER_REFERENCE(macro)                          \
  macro(BackgroundLayer, BackgroundImage, ImageOrNone, image, Image)             \
  macro(BackgroundLayer, BackgroundPositionX, PositionX, positionX, PositionX)   \
  macro(BackgroundLayer, BackgroundPositionY, PositionY, positionY, PositionY)   \
  macro(BackgroundLayer, BackgroundSize, BackgroundSize, size, Size)             \
  macro(BackgroundLayer, BackgroundRepeat, RepeatStyle, repeat, Repeat)

#define KRYS_FOR_EACH_BACKGROUND_LAYER_ENUM(macro)                                          \
  macro(BackgroundLayer, BackgroundAttachment, FillAttachment, attachment, Attachment)      \
  macro(BackgroundLayer, BackgroundClip, FillBox, clip, Clip)                               \
  macro(BackgroundLayer, BackgroundOrigin, FillBox, origin, Origin)                         \
  macro(BackgroundLayer, BackgroundBlendMode, BlendMode, blendMode, BlendMode)

#define KRYS_FOR_EACH_BACKGROUND_LAYER_SHORTHAND(macro) \
  macro(BackgroundLayer, BackgroundPosition, Position, position, Position)

#define KRYS_FOR_EACH_BACKGROUND_LAYER_PROPERTY(macro) \
  KRYS_FOR_EACH_BACKGROUND_LAYER_REFERENCE(macro)      \
  KRYS_FOR_EACH_BACKGROUND_LAYER_ENUM(macro)
  // clang-format on

  /// @brief One layer of an element's background: the `background-*` values at one index.
  /// @note The generated `FillAttachment()` and `FillRepeat()` setters hide those types inside the class.
  /// FIXME: `hasOpaqueImage` comes with the renderer (it asks the image whether it's known to be opaque).
  struct BackgroundLayer
  {
  private:
    ImageOrNone _image;
    PositionX _positionX;
    PositionY _positionY;
    BackgroundSize _size;
    RepeatStyle _repeat;
    uint8 _attachment : 2;
    uint8 _clip : FillBoxBitWidth;
    uint8 _origin : FillBoxBitWidth;
    uint8 _blendMode : 5;
    mutable uint8 _clipMax : FillBoxBitWidth; // maximum _clip value from this to bottom layer
    KRYS_FOR_EACH_BACKGROUND_LAYER_PROPERTY(KRYS_DECLARE_COORDINATED_VALUE_LIST_IS_SET_AND_IS_FILLED_MEMBERS)

  public:
    BackgroundLayer() noexcept;
    BackgroundLayer(css::keywords::None) noexcept;
    BackgroundLayer(ImageOrNone &&) noexcept;
    BackgroundLayer(RefPtr<StyleImage> &&) noexcept;

    KRYS_NODISCARD const ImageOrNone &GetImage() const noexcept
    {
      return _image;
    }
    KRYS_NODISCARD const PositionX &GetPositionX() const noexcept
    {
      return _positionX;
    }
    KRYS_NODISCARD const PositionY &GetPositionY() const noexcept
    {
      return _positionY;
    }
    KRYS_NODISCARD const BackgroundSize &GetSize() const noexcept
    {
      return _size;
    }
    KRYS_NODISCARD const RepeatStyle &GetRepeat() const noexcept
    {
      return _repeat;
    }
    KRYS_NODISCARD FillAttachment GetAttachment() const noexcept
    {
      return static_cast<boo::FillAttachment>(_attachment);
    }
    KRYS_NODISCARD FillBox GetClip() const noexcept
    {
      return static_cast<FillBox>(_clip);
    }
    KRYS_NODISCARD FillBox GetOrigin() const noexcept
    {
      return static_cast<FillBox>(_origin);
    }
    KRYS_NODISCARD BlendMode GetBlendMode() const noexcept
    {
      return static_cast<BlendMode>(_blendMode);
    }

    KRYS_NODISCARD constexpr static CompositeOperator GetComposite() noexcept
    {
      return CompositeOperator::SourceOver;
    }
    KRYS_NODISCARD constexpr static MaskMode GetMaskMode() noexcept
    {
      return MaskMode::MatchSource;
    }

    KRYS_NODISCARD CompositeOperator CompositeForPainting(bool /* isLastLayer */) const noexcept
    {
      return GetComposite();
    }

    KRYS_NODISCARD static ImageOrNone InitialImage() noexcept
    {
      return css::keywords::None {};
    }
    KRYS_NODISCARD static PositionX InitialPositionX() noexcept
    {
      using namespace css::Literals;
      return 0_css_percentage;
    }
    KRYS_NODISCARD static PositionY InitialPositionY() noexcept
    {
      using namespace css::Literals;
      return 0_css_percentage;
    }
    KRYS_NODISCARD static BackgroundSize InitialSize() noexcept
    {
      return css::keywords::Auto {};
    }
    KRYS_NODISCARD constexpr static RepeatStyle InitialRepeat() noexcept
    {
      return {.Values {boo::FillRepeat::Repeat, boo::FillRepeat::Repeat}};
    }
    KRYS_NODISCARD constexpr static FillAttachment InitialAttachment() noexcept
    {
      return boo::FillAttachment::ScrollBackground;
    }
    KRYS_NODISCARD constexpr static FillBox InitialClip() noexcept
    {
      return FillBox::BorderBox;
    }
    KRYS_NODISCARD constexpr static FillBox InitialOrigin() noexcept
    {
      return FillBox::PaddingBox;
    }
    KRYS_NODISCARD constexpr static BlendMode InitialBlendMode() noexcept
    {
      return BlendMode::Normal;
    }

    KRYS_NODISCARD bool HasImage() const noexcept
    {
      return _image.IsImage();
    }
    /// @brief Whether the layer has an image known to cover what it paints.
    KRYS_NODISCARD bool HasOpaqueImage(const RenderElement &) const noexcept;

    KRYS_NODISCARD bool HasRepeatXY() const noexcept
    {
      return GetRepeat() == boo::FillRepeat::Repeat;
    }

    KRYS_NODISCARD bool ClipOccludesNextLayers() const noexcept
    {
      return _clip == _clipMax;
    }
    void SetClipMax(FillBox clipMax) const noexcept
    {
      _clipMax = static_cast<uint8>(clipMax);
    }

    KRYS_NODISCARD bool operator==(const BackgroundLayer &) const noexcept;

    KRYS_FOR_EACH_BACKGROUND_LAYER_REFERENCE(KRYS_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_REFERENCE)
    KRYS_FOR_EACH_BACKGROUND_LAYER_ENUM(KRYS_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_ENUM)

    // Support for the `background-position` shorthand.
    KRYS_NODISCARD static Position InitialPosition() noexcept
    {
      return {InitialPositionX(), InitialPositionY()};
    }
    KRYS_NODISCARD Position GetPosition() const noexcept
    {
      return {_positionX, _positionY};
    }
    void SetPosition(Position &&position) noexcept
    {
      SetPositionX(krys::move(position.X));
      SetPositionY(krys::move(position.Y));
    }
    void FillPosition(Position &&position) noexcept
    {
      FillPositionX(krys::move(position.X));
      FillPositionY(krys::move(position.Y));
    }
    void ClearPosition() noexcept
    {
      ClearPositionX();
      ClearPositionY();
    }
    KRYS_NODISCARD bool IsPositionUnset() const noexcept
    {
      return IsPositionXUnset() && IsPositionYUnset();
    }
    KRYS_NODISCARD bool IsPositionSet() const noexcept
    {
      return IsPositionXSet() || IsPositionYSet();
    }
    KRYS_NODISCARD bool IsPositionFilled() const noexcept
    {
      return IsPositionXFilled() || IsPositionYFilled();
    }

    // CoordinatedValueList interface.
    constexpr static auto ComputedValueUsesUsedValues = true;
    constexpr static auto BaseProperty = css::PropertyNameConstant<css::PropertyId::BackgroundImage> {};
    constexpr static auto Properties =
      std::tuple {KRYS_FOR_EACH_BACKGROUND_LAYER_PROPERTY(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY)};

    KRYS_NODISCARD static BackgroundLayer Clone(const BackgroundLayer &other) noexcept
    {
      return other;
    }

    KRYS_NODISCARD bool IsInitial() const noexcept
    {
      return _image.IsNone();
    }

  private:
    // Needed by macros to access members.
    KRYS_NODISCARD BackgroundLayer &Data() noexcept
    {
      return *this;
    }
    KRYS_NODISCARD const BackgroundLayer &Data() const noexcept
    {
      return *this;
    }
  };

  KRYS_FOR_EACH_BACKGROUND_LAYER_REFERENCE(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_REFERENCE)
  KRYS_FOR_EACH_BACKGROUND_LAYER_ENUM(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_ENUM)
  KRYS_FOR_EACH_BACKGROUND_LAYER_SHORTHAND(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_SHORTHAND)

  // MARK: - Blending

  template <>
  struct Blending<BackgroundLayer>
  {
    KRYS_NODISCARD bool CanBlend(const BackgroundLayer &, const BackgroundLayer &) noexcept;
  };

#undef KRYS_FOR_EACH_BACKGROUND_LAYER_REFERENCE
#undef KRYS_FOR_EACH_BACKGROUND_LAYER_ENUM
#undef KRYS_FOR_EACH_BACKGROUND_LAYER_SHORTHAND
#undef KRYS_FOR_EACH_BACKGROUND_LAYER_PROPERTY
}
