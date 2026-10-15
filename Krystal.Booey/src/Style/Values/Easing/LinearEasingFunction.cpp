#include "Krystal.Booey/Style/Values/Easing/LinearEasingFunction.hpp"

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include <algorithm>
#include <limits>

namespace krys::boo::style
{
  css::LinearEasingFunction ToCSSLinearEasingFunction(const LinearTimingFunction &function,
                                                      const RenderStyle &style) noexcept
  {
    css::CommaSeparatedList<css::LinearEasingParameters::Stop> stops;
    for (const auto &point : function.Points())
    {
      stops.Value.push_back(css::LinearEasingParameters::Stop {
        .Output = ConvertToCSS(Number<> {point.Value}, style),
        .Input = css::LinearEasingParameters::Stop::Length {
          .Input = ConvertToCSS(Percentage<> {point.Progress * 100}, style), .Extra = std::nullopt}});
    }
    return css::LinearEasingFunction {{krys::move(stops)}};
  }

  template <typename Resolver>
  KRYS_NODISCARD static Ref<TimingFunction>
    CreateTimingFunctionWithResolver(const css::LinearEasingFunction &function, Resolver &&resolver) noexcept
  {
    // https://drafts.csswg.org/css-easing-2/#create-a-linear-easing-function

    // `PendingPoint` is used for the first look of the conversion algorithm to gather the outputs, but not
    // necessarily all the inputs.
    struct PendingPoint
    {
      double Output;
      Maybe<double> Input {};
    };

    List<PendingPoint> points;
    points.reserve(function->Stops.size());

    // 1. Let function be a new linear easing function.
    // NOTE: This is implicit.

    // 2. Let `largestInput` be negative infinity.
    auto largestInput = -std::numeric_limits<double>::infinity();

    // 3. If there are less than two items in stopList, then return failure.
    krys_debug_assert(function->Stops.size() >= 2);

    // 4. For each stop in stopList:
    for (size_t i = 0; i < function->Stops.size(); ++i)
    {
      const auto &stop = function->Stops[i];
      // 4.1 Let `point` be a new linear easing point with its output set to stop’s <number> as a number.
      auto point = PendingPoint {.Output = resolver(stop.Output)};

      // 4.2 Append point to function’s points.
      points.push_back(point);

      if (stop.Input)
      {
        // 4.3 If stop has a <linear-stop-length>, then:

        // 4.3.1 Set point’s input to whichever is greater: stop’s <linear-stop-length>’s first <percentage>
        // as a number, or largestInput.
        auto firstPercentage = resolver(stop.Input->Input);
        auto input = std::max(firstPercentage / 100.0, largestInput);
        points.back().Input = input;

        // 4.3.2 Set largestInput to point’s input.
        largestInput = input;

        if (stop.Input->Extra)
        {
          // 4.3.3 If stop’s <linear-stop-length> has a second <percentage>, then:

          // 4.3.3.1 Let extraPoint be a new linear easing point with its output set to stop’s <number> as a
          // number.
          auto extraPoint = PendingPoint {.Output = point.Output};

          // 4.3.3.2 Append extraPoint to function’s points.
          points.push_back(extraPoint);

          // 4.3.3.3 Set extraPoint’s input to whichever is greater: stop’s <linear-stop-length>’s second
          // <percentage> as a number, or largestInput.
          auto secondPercentage = resolver(*stop.Input->Extra);
          auto extraInput = std::max(secondPercentage / 100.0, largestInput);
          points.back().Input = extraInput;

          // 4.3.3.4 Set largestInput to extraPoint’s input.
          largestInput = extraInput;
        }
      }
      else if (!i)
      {
        // 4.4 Otherwise, if stop is the first item in stopList, then:

        // 4.4.1 Set point’s input to 0.
        points.back().Input = 0;

        // 4.4.2 Set largestInput to 0.
        largestInput = 0;
      }
      else if (i == (function->Stops.size() - 1))
      {
        // 4.5 Otherwise, if stop is the last item in stopList, then set point’s input to whichever is
        // greater: 1 or largestInput.
        points.back().Input = std::max(1.0, largestInput);
      }
    }

    // 5. For runs of items in function’s points that have a null input, assign a number to the input by
    // linearly interpolating between the closest previous and next points that have a non-null input.
    List<LinearTimingFunction::Point> resolvedPoints;
    resolvedPoints.reserve(points.size());

    Maybe<size_t> missingInputRunStart;
    for (size_t i = 0; i <= points.size(); ++i)
    {
      if (i < points.size() && !points[i].Input)
      {
        if (!missingInputRunStart)
        {
          missingInputRunStart = i;
        }
        continue;
      }

      if (missingInputRunStart)
      {
        auto startInput = *points[*missingInputRunStart - 1].Input;
        auto endInput = *points[i].Input;
        auto numberOfMissingInputs = i - *missingInputRunStart + 1;
        auto increment = (endInput - startInput) / static_cast<double>(numberOfMissingInputs);
        for (auto j = *missingInputRunStart; j < i; ++j)
        {
          resolvedPoints.push_back(
            {.Value = points[j].Output,
             .Progress = startInput + increment * static_cast<double>(j - *missingInputRunStart + 1)});
        }
        missingInputRunStart = std::nullopt;
      }

      if (i < points.size() && points[i].Input)
      {
        resolvedPoints.push_back({.Value = points[i].Output, .Progress = *points[i].Input});
      }
    }
    krys_debug_assert(!missingInputRunStart);
    krys_debug_assert(resolvedPoints.size() == points.size());

    // 6. Return function.
    return LinearTimingFunction::Create(krys::move(resolvedPoints));
  }

  Ref<TimingFunction> CreateTimingFunction(const BuilderState &state,
                                           const css::LinearEasingFunction &function) noexcept
  {
    return CreateTimingFunctionWithResolver(function, [&](const auto &value) -> double
                                            { return ConvertToStyle(value, state).Value; });
  }

  Ref<TimingFunction> CreateTimingFunctionDeprecated(const css::LinearEasingFunction &function) noexcept
  {
    if (!css::CollectComputedStyleDependencies(function).CanResolveDependenciesWithConversionData({}))
    {
      return LinearTimingFunction::Create();
    }

    return CreateTimingFunctionWithResolver(function, [&](const auto &value) -> double
                                            { return ConvertToStyleNoConversionDataRequired(value).Value; });
  }
}
