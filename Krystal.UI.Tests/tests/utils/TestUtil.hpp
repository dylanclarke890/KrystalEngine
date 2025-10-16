#pragma once

#include "Krystal.UI/Api/Node.hpp"
#include "Krystal.UI/Event/Event.hpp"
#include <functional>
#include <string_view>

namespace Krys::Tests
{
  using namespace Krys::UI;

  struct TestUtil
  {
    static void StartCountingNodes();
    static int NodeCount();
    static int StopCountingNodes();
  };

  struct ScopedEventSubscription
  {
    explicit ScopedEventSubscription(std::function<Event::Subscriber> &&);
    ~ScopedEventSubscription();
  };

  Size IntrinsicSizeMeasure(NodeConstRef node, float width, MeasureMode widthMode, float height,
                            MeasureMode heightMode);

  float LongestWordWidth(std::string_view text, float widthPerChar);

  float CalculateHeight(std::string_view text, float measuredWidth, float widthPerChar, float heightPerChar);
}