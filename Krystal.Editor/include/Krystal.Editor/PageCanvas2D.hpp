#pragma once

#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/PointerEvent.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasGradient.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasPattern.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2D.hpp"
#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/ImageData.hpp"
#include "Krystal.Booey/SVG/SVGImageElement.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <algorithm>
#include <format>
#include <numbers>

namespace krys::editor
{
  /// @brief The editor page's canvas 2D sample: `#chart`, a chart of data the editor makes, redrawn each
  /// second as a new value comes in, and `#pad`, a drawing pad that keeps pointer strokes in the colour
  /// picked from `#pad-palette` (read back from its pixels), cleared by `#pad-clear`. A page without `#chart`
  /// is left alone.
  class PageCanvas2D
  {
    KRYS_NON_COPY_MOVABLE(PageCanvas2D);

    /// @brief How many values the chart shows.
    static constexpr size_t ValueCount = 12u;
    /// @brief The values' range: cups an hour.
    static constexpr float MaxValue = 100.f;
    /// @brief Seconds between new values.
    static constexpr float SecondsPerValue = 1.f;

    // The chart's layout, in CSS pixels (its bitmap is twice as big, so it's sharp).
    static constexpr float ChartWidth = 320.f;
    static constexpr float ChartHeight = 160.f;
    static constexpr float PlotLeft = 30.f;
    static constexpr float PlotTop = 12.f;
    static constexpr float PlotRight = 270.f;
    static constexpr float PlotBottom = 140.f;
    static constexpr float BarLeft = 282.f;
    static constexpr float BarWidth = 28.f;

  private:
    WeakPtr<boo::dom::Document> _document;
    List<boo::dom::EventListenerHandle> _listeners;
    RefPtr<boo::html::CanvasRenderingContext2D> _chart;
    RefPtr<boo::html::CanvasRenderingContext2D> _pad;
    RefPtr<boo::html::CanvasRenderingContext2D> _palette;
    RefPtr<boo::html::CanvasPattern> _barPattern;
    List<float> _values;
    uint32 _seed {0x25'45'F4'91u};
    float _sinceValue {0};
    int32 _drawPointer {-1};
    double _lastX {0};
    double _lastY {0};
    boo::dom::DOMString _colour {u8"#1a2240"};

  public:
    explicit PageCanvas2D(boo::dom::Document &document) noexcept
        : _document(WeakPtr<boo::dom::Document>::WithRef(document))
    {
      _chart = Context(u8"chart");
      if (!_chart)
      {
        return;
      }
      _pad = Context(u8"pad");
      _palette = Context(u8"pad-palette");

      _values.reserve(ValueCount);
      while (_values.size() < ValueCount)
      {
        _values.push_back(NextValue());
      }
      MakeBarPattern();
      DrawChart();
      DrawPalette();
      if (_pad)
      {
        ListenToPad();
      }
    }

    /// @brief Brings in a new value each second, and redraws the chart with it.
    void Update(float seconds) noexcept
    {
      if (!_chart || !_document)
      {
        return;
      }
      _sinceValue += seconds;
      if (_sinceValue < SecondsPerValue)
      {
        return;
      }
      _sinceValue = 0;
      _values.erase(_values.begin());
      _values.push_back(NextValue());
      DrawChart();
    }

    /// @brief The values the chart shows, oldest first.
    KRYS_NODISCARD const List<float> &Values() const noexcept
    {
      return _values;
    }

  private:
    KRYS_NODISCARD RefPtr<boo::html::CanvasRenderingContext2D> Context(utf8_stringview id) const noexcept
    {
      auto *canvas = DynamicDowncast<boo::html::HTMLCanvasElement>(ById(id).get());
      return canvas ? canvas->GetContext2d() : nullptr;
    }

    KRYS_NODISCARD RefPtr<boo::dom::Element> ById(utf8_stringview id) const noexcept
    {
      return _document ? _document->GetElementById(id) : nullptr;
    }

    /// @brief The next value: a walk that stays in range, from a fixed seed so the chart starts the same.
    KRYS_NODISCARD float NextValue() noexcept
    {
      // A xorshift generator.
      _seed ^= _seed << 13;
      _seed ^= _seed >> 17;
      _seed ^= _seed << 5;
      const float step = static_cast<float>(_seed % 41u) - 20.f;
      const float last = _values.empty() ? 50.f : _values.back();
      return std::clamp(last + step, 8.f, 96.f);
    }

    /// @brief The bar's pattern: diagonal stripes, drawn into a small canvas of its own.
    void MakeBarPattern() noexcept
    {
      auto tile = Context(u8"chart-tile");
      if (!tile)
      {
        return;
      }
      tile->SetFillStyle(boo::dom::DOMString(u8"#ffd166"));
      tile->FillRect(0, 0, 12, 12);
      tile->SetStrokeStyle(boo::dom::DOMString(u8"#ef476f"));
      tile->SetLineWidth(3);
      tile->BeginPath();
      tile->MoveTo(-3, 3);
      tile->LineTo(3, -3);
      tile->MoveTo(0, 12);
      tile->LineTo(12, 0);
      tile->MoveTo(9, 15);
      tile->LineTo(15, 9);
      tile->Stroke();

      auto *canvas = DynamicDowncast<boo::html::HTMLCanvasElement>(ById(u8"chart-tile").get());
      auto pattern = _chart->CreatePattern(ShareRefPtr(canvas), u8"repeat");
      if (!pattern.HasException())
      {
        _barPattern = pattern.ReleaseValue();
      }
    }

    KRYS_NODISCARD static float ValueY(float value) noexcept
    {
      return PlotBottom - (PlotBottom - PlotTop) * value / MaxValue;
    }

    KRYS_NODISCARD static float PointX(size_t index) noexcept
    {
      return PlotLeft
             + (PlotRight - PlotLeft) * static_cast<float>(index) / static_cast<float>(ValueCount - 1u);
    }

    void DrawChart() noexcept
    {
      auto &context = *_chart;
      using boo::dom::DOMString;

      context.Reset();
      context.Scale(2, 2);
      context.SetFont(u8"11px \"Liberation Sans\"");

      // Dashed grid lines, labelled.
      context.SetLineWidth(1);
      context.SetLineDash({4, 4});
      context.SetStrokeStyle(DOMString(u8"#c9d1e6"));
      context.SetFillStyle(DOMString(u8"#5b6584"));
      context.SetTextAlign(boo::html::CanvasTextAlign::Right);
      context.SetTextBaseline(boo::html::CanvasTextBaseline::Middle);
      for (int32 step = 0; step <= 4; ++step)
      {
        const float value = MaxValue * static_cast<float>(step) / 4.f;
        const float y = std::round(ValueY(value)) + 0.5f;
        context.BeginPath();
        context.MoveTo(PlotLeft, y);
        context.LineTo(PlotRight, y);
        context.Stroke();
        context.FillText(Text(std::format("{}", static_cast<int32>(value))), PlotLeft - 6, y);
      }
      context.SetLineDash({});

      // The values as a line, with a gradient filled under it.
      auto line = [&]
      {
        context.BeginPath();
        for (size_t i = 0; i < _values.size(); ++i)
        {
          if (i == 0)
          {
            context.MoveTo(PointX(i), ValueY(_values[i]));
          }
          else
          {
            context.LineTo(PointX(i), ValueY(_values[i]));
          }
        }
      };
      auto fill = context.CreateLinearGradient(0, PlotTop, 0, PlotBottom);
      if (!fill.HasException())
      {
        auto gradient = fill.ReleaseValue();
        std::ignore = gradient->AddColorStop(0, u8"rgba(79, 109, 245, 0.45)");
        std::ignore = gradient->AddColorStop(1, u8"rgba(79, 109, 245, 0)");
        line();
        context.LineTo(PointX(_values.size() - 1u), PlotBottom);
        context.LineTo(PointX(0), PlotBottom);
        context.ClosePath();
        context.SetFillStyle(krys::move(gradient));
        context.Fill();
      }
      line();
      context.SetLineWidth(2.5);
      context.SetLineJoin(boo::html::CanvasLineJoin::Round);
      context.SetStrokeStyle(DOMString(u8"#4f6df5"));
      context.Stroke();

      // The newest value as a bar, filled with the pattern, its value above it.
      const float latest = _values.back();
      const float barTop = ValueY(latest);
      if (_barPattern)
      {
        context.SetFillStyle(Ref<boo::html::CanvasPattern>(_barPattern));
      }
      else
      {
        context.SetFillStyle(DOMString(u8"#ffd166"));
      }
      context.FillRect(BarLeft, barTop, BarWidth, PlotBottom - barTop);
      context.SetFillStyle(DOMString(u8"#1a2240"));
      context.SetTextAlign(boo::html::CanvasTextAlign::Center);
      context.SetTextBaseline(boo::html::CanvasTextBaseline::Bottom);
      context.FillText(Text(std::format("{}", static_cast<int32>(latest))), BarLeft + BarWidth / 2,
                       barTop - 3);
      context.SetTextBaseline(boo::html::CanvasTextBaseline::Top);
      context.FillText(u8"now", BarLeft + BarWidth / 2, PlotBottom + 4);

      if (auto caption = ById(u8"chart-caption"))
      {
        (void)caption->TextContent(Text(std::format("{} cups this hour.", static_cast<int32>(latest))));
      }
    }

    /// @brief The palette: every hue across, fading to white above and black below.
    void DrawPalette() noexcept
    {
      if (!_palette)
      {
        return;
      }
      auto &context = *_palette;
      auto *canvas = DynamicDowncast<boo::html::HTMLCanvasElement>(ById(u8"pad-palette").get());
      const auto width = static_cast<float>(canvas->Width());
      const auto height = static_cast<float>(canvas->Height());

      auto hues = context.CreateLinearGradient(0, 0, width, 0);
      if (hues.HasException())
      {
        return;
      }
      auto gradient = hues.ReleaseValue();
      const utf8_stringview stops[] = {u8"#ff0000", u8"#ffff00", u8"#00ff00", u8"#00ffff",
                                       u8"#0000ff", u8"#ff00ff", u8"#ff0000"};
      for (size_t i = 0; i < std::size(stops); ++i)
      {
        std::ignore = gradient->AddColorStop(static_cast<double>(i) / 6.0, stops[i]);
      }
      context.SetFillStyle(krys::move(gradient));
      context.FillRect(0, 0, width, height);

      auto shade = context.CreateLinearGradient(0, 0, 0, height);
      if (shade.HasException())
      {
        return;
      }
      auto shadeGradient = shade.ReleaseValue();
      std::ignore = shadeGradient->AddColorStop(0, u8"rgba(255, 255, 255, 0.9)");
      std::ignore = shadeGradient->AddColorStop(0.5, u8"rgba(255, 255, 255, 0)");
      std::ignore = shadeGradient->AddColorStop(0.5, u8"rgba(0, 0, 0, 0)");
      std::ignore = shadeGradient->AddColorStop(1, u8"rgba(0, 0, 0, 0.9)");
      context.SetFillStyle(krys::move(shadeGradient));
      context.FillRect(0, 0, width, height);
    }

    void ListenToPad() noexcept
    {
      auto pad = ById(u8"pad");
      auto listen =
        [&](boo::dom::Element &element, utf8_stringview type, Func<void(boo::dom::PointerEvent &)> &&callback)
      {
        _listeners.push_back(element.AddEventListener(
          boo::dom::DOMStringAtom(type), [callback = krys::move(callback)](boo::dom::Event &event)
          { callback(static_cast<boo::dom::PointerEvent &>(event)); }));
      };

      // The pointer that presses the pad captures itself to it, and draws until it lets go.
      listen(*pad, u8"pointerdown",
             [this](boo::dom::PointerEvent &pointer)
             {
               if (auto element = ById(u8"pad"))
               {
                 auto [x, y] = PadPoint(*element, pointer);
                 _lastX = x;
                 _lastY = y;
                 // A press without a move leaves a dot.
                 DrawSegment(x, y);
                 (void)element->SetPointerCapture(static_cast<int32>(pointer.PointerId()));
               }
             });
      listen(*pad, u8"gotpointercapture", [this](boo::dom::PointerEvent &pointer)
             { _drawPointer = static_cast<int32>(pointer.PointerId()); });
      listen(*pad, u8"lostpointercapture", [this](boo::dom::PointerEvent &) { _drawPointer = -1; });
      listen(*pad, u8"pointermove",
             [this](boo::dom::PointerEvent &pointer)
             {
               auto element = ById(u8"pad");
               if (!element || static_cast<int32>(pointer.PointerId()) != _drawPointer)
               {
                 return;
               }
               auto [x, y] = PadPoint(*element, pointer);
               DrawSegment(x, y);
               _lastX = x;
               _lastY = y;
             });

      if (auto palette = ById(u8"pad-palette"))
      {
        listen(*palette, u8"pointerdown",
               [this](boo::dom::PointerEvent &pointer)
               {
                 if (auto element = ById(u8"pad-palette"))
                 {
                   auto [x, y] = PadPoint(*element, pointer);
                   PickColour(static_cast<int32>(x), static_cast<int32>(y));
                 }
               });
      }

      if (auto clear = ById(u8"pad-clear"))
      {
        _listeners.push_back(
          clear->AddEventListener(boo::dom::DOMStringAtom(u8"click"),
                                  [this](boo::dom::Event &)
                                  {
                                    if (_pad && _document)
                                    {
                                      auto &canvas = Downcast<boo::html::HTMLCanvasElement>(*ById(u8"pad"));
                                      _pad->ClearRect(0, 0, canvas.Width(), canvas.Height());
                                    }
                                  }));
      }
    }

    /// @brief Where @p pointer is in @p element's canvas's pixels.
    KRYS_NODISCARD static std::pair<double, double> PadPoint(boo::dom::Element &element,
                                                             const boo::dom::PointerEvent &pointer) noexcept
    {
      auto rect = element.GetBoundingClientRect();
      auto &canvas = Downcast<boo::html::HTMLCanvasElement>(element);
      const double scaleX = rect->Width() > 0 ? canvas.Width() / rect->Width() : 1.0;
      const double scaleY = rect->Height() > 0 ? canvas.Height() / rect->Height() : 1.0;
      return {(pointer.ClientX() - rect->Left()) * scaleX, (pointer.ClientY() - rect->Top()) * scaleY};
    }

    void DrawSegment(double x, double y) noexcept
    {
      if (!_pad || !_document)
      {
        return;
      }
      auto &context = *_pad;
      context.SetStrokeStyle(boo::dom::DOMString(_colour));
      context.SetLineWidth(6);
      context.SetLineCap(boo::html::CanvasLineCap::Round);
      context.BeginPath();
      context.MoveTo(static_cast<float>(_lastX), static_cast<float>(_lastY));
      context.LineTo(static_cast<float>(x), static_cast<float>(y));
      context.Stroke();
    }

    /// @brief Draws with the palette's colour at @p x, @p y, read back from its pixels.
    void PickColour(int32 x, int32 y) noexcept
    {
      if (!_palette || !_document)
      {
        return;
      }
      auto read = _palette->GetImageData(x, y, 1, 1, {});
      if (read.HasException())
      {
        return;
      }
      auto bytes = read.Value()->Data().AsUint8ClampedArray();
      if (!bytes || bytes->Item(3) == 0)
      {
        return;
      }
      auto colour = std::format("rgb({}, {}, {})", static_cast<int32>(bytes->Item(0)),
                                static_cast<int32>(bytes->Item(1)), static_cast<int32>(bytes->Item(2)));
      _colour = Text(colour);
      if (auto swatch = ById(u8"pad-colour"))
      {
        (void)swatch->SetAttribute(boo::dom::DOMStringAtom(u8"style"), Text("background: " + colour));
      }
    }

    KRYS_NODISCARD static boo::dom::DOMString Text(const std::string &text) noexcept
    {
      return boo::dom::DOMString(reinterpret_cast<const char8_t *>(text.data()), text.size());
    }
  };
}
