#pragma once

#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <cmath>
#include <numbers>

namespace krys::editor
{
  /// @brief The fonts the Booey scene writes with.
  struct BooeyFonts
  {
    RefPtr<boo::Font> Regular;
    RefPtr<boo::Font> Bold;
  };

  /// @brief Draws through Booey's graphics context what painting will draw for pages: boxes, borders,
  /// shadows, text and its decorations, paths, gradients and layers. The editor records it into a display
  /// list and replays it through Krystal.UI, as it will a page's.
  class BooeyScene
  {
  private:
    boo::GraphicsContext &_context;
    const BooeyFonts &_fonts;
    float _time;

  public:
    BooeyScene(boo::GraphicsContext &context, const BooeyFonts &fonts, float time) noexcept
        : _context(context), _fonts(fonts), _time(time)
    {
    }

    void Draw() noexcept
    {
      Card(u8"Boxes and borders", 24, 80);
      Card(u8"Shadows and text", 444, 80);
      Card(u8"Paths, gradients and layers", 864, 80);
      DrawBoxes(24, 80);
      DrawShadowsAndText(444, 80);
      DrawPathsAndLayers(864, 80);
    }

  private:
    KRYS_NODISCARD static boo::Color Rgb(uint32 rgb, uint8 alpha = 255u) noexcept
    {
      return SRGBA<uint8> {static_cast<uint8>((rgb >> 16u) & 255u), static_cast<uint8>((rgb >> 8u) & 255u),
                           static_cast<uint8>(rgb & 255u), alpha};
    }

    KRYS_NODISCARD static boo::FloatRoundedRect Rounded(float x, float y, float width, float height,
                                                        float radius) noexcept
    {
      const boo::FloatSize corner {radius, radius};
      return boo::FloatRoundedRect({x, y, width, height}, corner, corner, corner, corner);
    }

    /// @brief A line of text, its glyphs from the font's glyph pages and placed by their advances.
    void Text(utf8_stringview text, float x, float baseline, const boo::Color &color,
              bool bold = false) noexcept
    {
      const auto &font = bold ? _fonts.Bold : _fonts.Regular;
      if (!font)
      {
        return;
      }
      List<boo::GlyphBufferGlyph> glyphs;
      List<boo::GlyphBufferAdvance> advances;
      for (const auto character : text)
      {
        const auto glyph = font->GlyphForCharacter(static_cast<char32>(character));
        glyphs.push_back(glyph);
        advances.push_back({font->WidthForGlyph(glyph), 0});
      }
      _context.SetFillColor(color);
      _context.DrawGlyphs(*font, glyphs, advances, {x, baseline}, boo::FontSmoothingMode::Antialiased);
    }

    KRYS_NODISCARD float TextWidth(utf8_stringview text) const noexcept
    {
      float width = 0;
      for (const auto character : text)
      {
        width +=
          _fonts.Regular->WidthForGlyph(_fonts.Regular->GlyphForCharacter(static_cast<char32>(character)));
      }
      return width;
    }

    void Card(utf8_stringview title, float x, float y) noexcept
    {
      boo::GraphicsContextStateSaver saver(_context);
      _context.SetDropShadow({.Offset = {0, 8}, .Radius = 24, .Color = Rgb(0x1a2240, 40)});
      _context.FillRoundedRect(Rounded(x, y, 392, 616, 16), Rgb(0xffffff));
      _context.ClearDropShadow();
      Text(title, x + 24, y + 34, Rgb(0x5a6378), true);
    }

    /// @brief Boxes: rounded and elliptical corners, borders as rects with rounded holes, and border lines in
    /// each style, as the border painter draws them.
    void DrawBoxes(float x, float y) noexcept
    {
      // Backgrounds: a plain box, circular corners, elliptical ones.
      _context.FillRect({x + 24, y + 56, 104, 72}, Rgb(0x4f6df5));
      _context.FillRoundedRect(Rounded(x + 144, y + 56, 104, 72, 16), Rgb(0x2bb673));
      const boo::FloatSize ellipse {40, 24};
      _context.FillRoundedRect(
        boo::FloatRoundedRect({x + 264, y + 56, 104, 72}, ellipse, ellipse, ellipse, ellipse), Rgb(0xf5a623));

      // Borders, as the border painter fills a uniform one: clipped to the border box's rounded rect, its
      // rect with the padding box as a rounded hole, the inner radii the outer less the border's width.
      for (int i = 0; i < 3; ++i)
      {
        const float left = x + 24 + static_cast<float>(i) * 120;
        const float radius = static_cast<float>(i) * 12;
        const float width = static_cast<float>(i + 2) * 2;
        boo::GraphicsContextStateSaver saver(_context);
        _context.ClipRoundedRect(Rounded(left, y + 152, 104, 72, radius));
        _context.FillRectWithRoundedHole({left, y + 152, 104, 72},
                                         Rounded(left + width, y + 152 + width, 104 - 2 * width,
                                                 72 - 2 * width, std::max(0.f, radius - width)),
                                         Rgb(0x222733));
      }
      Text(u8"Borders: rounded clips, rounded holes", x + 24, y + 248, Rgb(0x8a93a6));

      // Border lines in each style, as the border painter draws a side: between the corners of the side's
      // rect, top-left and bottom-right.
      const boo::StrokeStyle styles[] {boo::StrokeStyle::SolidStroke, boo::StrokeStyle::DashedStroke,
                                       boo::StrokeStyle::DottedStroke};
      const char8 *names[] {u8"solid", u8"dashed", u8"dotted"};
      for (int i = 0; i < 3; ++i)
      {
        const float row = y + 280 + static_cast<float>(i) * 40;
        boo::GraphicsContextStateSaver saver(_context);
        _context.SetStrokeColor(Rgb(0xe0475b));
        _context.SetStrokeThickness(4);
        _context.SetStrokeStyle(styles[i]);
        _context.DrawLine({x + 24, row - 2}, {x + 280, row + 2});
        Text(names[i], x + 296, row + 8, Rgb(0x5a6378));
      }

      // A filled, outlined rect and list bullets.
      _context.SetFillColor(Rgb(0xeef1f6));
      _context.SetStrokeColor(Rgb(0x8a93a6));
      _context.DrawRect({x + 24, y + 412, 344, 72}, 2);
      for (int i = 0; i < 3; ++i)
      {
        const float row = y + 432 + static_cast<float>(i) * 18;
        _context.SetFillColor(Rgb(0x4f6df5));
        _context.DrawEllipse({x + 40, row - 7, 6, 6});
        Text(i == 0   ? u8"Ellipses are list bullets"
             : i == 1 ? u8"DrawRect fills, then outlines"
                      : u8"in the stroke colour",
             x + 56, row, Rgb(0x222733));
      }

      // Elliptical corners clip as paths.
      boo::GraphicsContextStateSaver saver(_context);
      const boo::FloatSize clipCorner {60, 30};
      _context.ClipRoundedRect(
        boo::FloatRoundedRect({x + 24, y + 508, 344, 84}, clipCorner, clipCorner, clipCorner, clipCorner));
      for (int i = 0; i < 12; ++i)
      {
        _context.FillRect({x + 24 + static_cast<float>(i) * 29, y + 508, 29, 84},
                          i % 2 ? Rgb(0xb04ff5) : Rgb(0x4f6df5));
      }
    }

    /// @brief Shadows outset and inset, and text with decorations and a shadow.
    void DrawShadowsAndText(float x, float y) noexcept
    {
      // Outset shadows rise with the card.
      const float lift = 0.5f + 0.5f * std::sin(_time * 1.7f);
      {
        boo::GraphicsContextStateSaver saver(_context);
        _context.SetDropShadow(
          {.Offset = {0, 2 + 8 * lift}, .Radius = 6 + 18 * lift, .Color = Rgb(0x000000, 80)});
        _context.FillRoundedRect(Rounded(x + 24, y + 60 - 4 * lift, 160, 72, 12), Rgb(0xffffff));
      }
      Text(u8"Outset shadow", x + 40, y + 102 - 4 * lift, Rgb(0x222733), true);

      // An inset shadow: a rect with the box as its hole casts it, clipped to the box.
      {
        const auto well = Rounded(x + 208, y + 56, 160, 80, 12);
        _context.FillRoundedRect(well, Rgb(0xeef1f6));
        boo::GraphicsContextStateSaver saver(_context);
        _context.ClipRoundedRect(well);
        _context.SetDropShadow({.Offset = {0, 4}, .Radius = 12, .Color = Rgb(0x000000, 110)});
        _context.FillRectWithRoundedHole({x + 168, y + 16, 240, 160}, well, Rgb(0x000000));
      }
      Text(u8"Inset shadow", x + 248, y + 102, Rgb(0x5a6378));

      // Text decorations, as text painting draws them.
      const boo::StrokeStyle styles[] {boo::StrokeStyle::SolidStroke, boo::StrokeStyle::DashedStroke,
                                       boo::StrokeStyle::DottedStroke, boo::StrokeStyle::SolidStroke};
      const char8 *labels[] {u8"Underlined text", u8"Dashed underline", u8"Dotted underline",
                             u8"Double underline"};
      for (int i = 0; i < 4; ++i)
      {
        const float baseline = y + 186 + static_cast<float>(i) * 36;
        Text(labels[i], x + 24, baseline, Rgb(0x222733));
        _context.SetStrokeColor(Rgb(0x4f6df5));
        const boo::FloatSegment segment[] {{0, TextWidth(labels[i])}};
        _context.DrawLinesForText({x + 24, baseline + 3}, 1.5f, segment, false, i == 3, styles[i]);
      }

      // A line through, and text with a shadow, sharp and blurred.
      Text(u8"Struck through", x + 224, y + 186, Rgb(0x8a93a6));
      _context.SetStrokeColor(Rgb(0x8a93a6));
      _context.DrawLineForText({x + 224, y + 181, TextWidth(u8"Struck through"), 1.5f}, false);
      {
        boo::GraphicsContextStateSaver saver(_context);
        _context.SetDropShadow({.Offset = {2, 2}, .Radius = 0, .Color = Rgb(0xf5a623)});
        Text(u8"Sharp text shadow", x + 224, y + 222, Rgb(0x222733), true);
        _context.SetDropShadow({.Offset = {0, 0}, .Radius = 8, .Color = Rgb(0x4f6df5)});
        Text(u8"Glowing text", x + 224, y + 258, Rgb(0x1a2240), true);
      }

      // A dark panel, translucent text over a gradient.
      auto gradient =
        boo::Gradient::Create(boo::Gradient::LinearData {{x + 24, 0}, {x + 368, 0}},
                              {ColourInterpolationMethod::OKLab {}, AlphaPremultiplication::Premultiplied});
      gradient->AddColorStop({0, Rgb(0x1a2240)});
      gradient->AddColorStop({1, Rgb(0x4f2a7a)});
      {
        boo::GraphicsContextStateSaver saver(_context);
        _context.ClipRoundedRect(Rounded(x + 24, y + 340, 344, 120, 14));
        _context.FillRect({x + 24, y + 340, 344, 120}, *gradient);
      }
      Text(u8"Krystal UI replays Booey's", x + 44, y + 380, Rgb(0xffffff), true);
      Text(u8"display lists into Render2D", x + 44, y + 404, Rgb(0xffffff, 200));
      Text(u8"one item at a time", x + 44, y + 428, Rgb(0xffffff, 140));

      // A focus ring around a button.
      _context.FillRoundedRect(Rounded(x + 24, y + 500, 160, 48, 10), Rgb(0x4f6df5));
      Text(u8"Focused", x + 74, y + 530, Rgb(0xffffff), true);
      boo::Path ring;
      ring.AddRoundedRect(Rounded(x + 20, y + 496, 168, 56, 13));
      _context.DrawFocusRing(ring, 2, Rgb(0xf5a623));
    }

    /// @brief Paths filled and clipped, gradients of each kind, and transparency layers.
    void DrawPathsAndLayers(float x, float y) noexcept
    {
      // Three gradients.
      auto linear =
        boo::Gradient::Create(boo::Gradient::LinearData {{x + 24, 0}, {x + 128, 0}},
                              {ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Premultiplied});
      linear->AddColorStop({0, Rgb(0x4f6df5)});
      linear->AddColorStop({1, Rgb(0xb04ff5)});
      _context.FillRect({x + 24, y + 56, 104, 80}, *linear);

      auto radial =
        boo::Gradient::Create(boo::Gradient::RadialData {{x + 196, y + 96}, {x + 196, y + 96}, 0, 52, 1},
                              {ColourInterpolationMethod::OKLab {}, AlphaPremultiplication::Premultiplied});
      radial->AddColorStop({0, Rgb(0xffe08a)});
      radial->AddColorStop({1, Rgb(0xe0475b)});
      _context.FillRect({x + 144, y + 56, 104, 80}, *radial);

      auto conic =
        boo::Gradient::Create(boo::Gradient::ConicData {{x + 316, y + 96}, _time},
                              {ColourInterpolationMethod::OKLCH {}, AlphaPremultiplication::Premultiplied});
      conic->AddColorStop({0, Rgb(0xff0000)});
      conic->AddColorStop({0.5f, Rgb(0x00a0ff)});
      conic->AddColorStop({1, Rgb(0xff0000)});
      {
        boo::GraphicsContextStateSaver saver(_context);
        boo::Path circle;
        circle.AddEllipseInRect({x + 276, y + 56, 80, 80});
        _context.ClipPath(circle, boo::WindRule::NonZero);
        _context.FillRect({x + 264, y + 56, 104, 80}, *conic);
      }

      // A star, turning, clipping a gradient; its outline stroked over it.
      {
        boo::GraphicsContextStateSaver saver(_context);
        _context.Translate(x + 100, y + 236);
        _context.Rotate(_time * 0.6f);
        const auto star = Star(64);
        {
          boo::GraphicsContextStateSaver clip(_context);
          _context.ClipPath(star, boo::WindRule::NonZero);
          auto fill = boo::Gradient::Create(
            boo::Gradient::LinearData {{-64, -64}, {64, 64}},
            {ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Premultiplied});
          fill->AddColorStop({0, Rgb(0xf5a623)});
          fill->AddColorStop({1, Rgb(0xe0475b)});
          _context.FillRect({-64, -64, 128, 128}, *fill);
        }
        _context.SetStrokeColor(Rgb(0x222733));
        _context.SetStrokeThickness(3);
        _context.SetLineJoin(boo::LineJoin::Round);
        _context.StrokePath(star);
      }

      // An even-odd star, dashed outline.
      {
        boo::GraphicsContextStateSaver saver(_context);
        _context.Translate(x + 290, y + 236);
        _context.SetFillRule(boo::WindRule::EvenOdd);
        _context.SetFillColor(Rgb(0x2bb673));
        _context.FillPath(Star(60));
        _context.SetStrokeColor(Rgb(0x1a2240));
        _context.SetStrokeThickness(2);
        _context.SetLineDash({8, 4}, _time * 10);
        _context.StrokePath(Star(60));
      }

      // Overlapping circles in a translucent layer: the layer fades as one.
      auto circles = [&](float left, float top)
      {
        const uint32 colours[] {0xe0475b, 0x2bb673, 0x4f6df5};
        for (int i = 0; i < 3; ++i)
        {
          const float angle = static_cast<float>(i) * 2.0943951f - std::numbers::pi_v<float> / 2;
          _context.SetFillColor(Rgb(colours[i]));
          _context.DrawEllipse(
            {left + 58 + 16 * std::cos(angle) - 26, top + 60 + 16 * std::sin(angle) - 26, 52, 52});
        }
      };
      _context.FillRect({x + 24, y + 336, 344, 132}, Rgb(0xeef1f6));
      circles(x + 24, y + 346);
      {
        boo::TransparencyLayerScope layer(_context, 0.5f + 0.25f * std::sin(_time));
        circles(x + 144, y + 346);
      }
      {
        _context.BeginTransparencyLayer(boo::CompositeOperator::SourceOver, BlendMode::Multiply);
        circles(x + 264, y + 346);
        _context.EndTransparencyLayer();
      }
      Text(u8"Opaque", x + 50, y + 492, Rgb(0x5a6378));
      Text(u8"A layer", x + 172, y + 492, Rgb(0x5a6378));
      Text(u8"Multiplied", x + 286, y + 492, Rgb(0x5a6378));

      // A clip-out: a frame drawn around a hole.
      boo::GraphicsContextStateSaver saver(_context);
      _context.ClipOutRoundedRect(Rounded(x + 64, y + 528, 264, 44, 22));
      _context.FillRoundedRect(Rounded(x + 24, y + 516, 344, 68, 18), Rgb(0x1a2240));
    }

    KRYS_NODISCARD static boo::Path Star(float radius) noexcept
    {
      boo::Path path;
      for (int i = 0; i < 5; ++i)
      {
        // Every second point of a pentagon: the classic pentagram.
        const float angle = -std::numbers::pi_v<float> / 2 + static_cast<float>(i * 2) * 1.2566371f;
        const boo::FloatPoint point {radius * std::cos(angle), radius * std::sin(angle)};
        if (i == 0)
        {
          path.MoveTo(point);
        }
        else
        {
          path.AddLineTo(point);
        }
      }
      path.CloseSubpath();
      return path;
    }
  };
}
