#include "TestUtil.hpp"
#include "Krystal.UI/Layout/Api/Node.hpp"
#include "Krystal.UI/Layout/Api/NodeStyle.hpp"
#include "Krystal.UI/Layout/Event/Event.hpp"
#include "Krystal.UI/Layout/Node/Node.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

namespace Krys::Tests
{
  using namespace Krys::UI;

  int nodeInstanceCount = 0;

  namespace
  {
    void EventSubscriber(NodeConstRef /*node*/, Event::Type eventType, const Event::Data & /*eventData*/)
    {
      switch (eventType)
      {
        case Event::NodeAllocation:   nodeInstanceCount++; break;
        case Event::NodeDeallocation: nodeInstanceCount--; break;
        default:                      break;
      }
    }
  }

  void TestUtil::StartCountingNodes()
  {
    nodeInstanceCount = 0;
    Event::Subscribe(EventSubscriber);
  }

  int TestUtil::NodeCount()
  {
    return nodeInstanceCount;
  }

  int TestUtil::StopCountingNodes()
  {
    Event::Reset();
    auto prev = nodeInstanceCount;
    nodeInstanceCount = 0;
    return prev;
  }

  ScopedEventSubscription::ScopedEventSubscription(std::function<Event::Subscriber> &&s)
  {
    Event::Subscribe(std::move(s));
  }

  ScopedEventSubscription::~ScopedEventSubscription()
  {
    Event::Reset();
  }

  Size IntrinsicSizeMeasure(NodeConstRef node, float width, MeasureMode widthMode, float height,
                            MeasureMode heightMode)
  {
    std::string_view innerText((char *)NodeGetContext(node));
    float heightPerChar = 10;
    float widthPerChar = 10;
    float measuredWidth;
    float measuredHeight;

    if (widthMode == MeasureMode::Exactly)
    {
      measuredWidth = width;
    }
    else if (widthMode == MeasureMode::AtMost)
    {
      measuredWidth = std::min((float)innerText.size() * widthPerChar, width);
    }
    else
    {
      measuredWidth = (float)innerText.size() * widthPerChar;
    }

    if (heightMode == MeasureMode::Exactly)
    {
      measuredHeight = height;
    }
    else if (heightMode == MeasureMode::AtMost)
    {
      measuredHeight =
        std::min(CalculateHeight(innerText,
                                 NodeStyleGetFlexDirection(node) == FlexDirection::Column
                                   ? measuredWidth
                                   : std::max(LongestWordWidth(innerText, widthPerChar), measuredWidth),
                                 widthPerChar, heightPerChar),
                 height);
    }
    else
    {
      measuredHeight = CalculateHeight(innerText,
                                       NodeStyleGetFlexDirection(node) == FlexDirection::Column
                                         ? measuredWidth
                                         : std::max(LongestWordWidth(innerText, widthPerChar), measuredWidth),
                                       widthPerChar, heightPerChar);
    }

    return Size {measuredWidth, measuredHeight};
  }

  float LongestWordWidth(std::string_view text, float widthPerChar)
  {
    int maxLength = 0;
    int currentLength = 0;
    for (auto c : text)
    {
      if (c == ' ')
      {
        maxLength = std::max(currentLength, maxLength);
        currentLength = 0;
      }
      else
      {
        currentLength++;
      }
    }
    return (float)std::max(currentLength, maxLength) * widthPerChar;
  }

  float CalculateHeight(std::string_view text, float measuredWidth, float widthPerChar, float heightPerChar)
  {
    if ((float)text.size() * widthPerChar <= measuredWidth)
    {
      return heightPerChar;
    }

    std::vector<std::string> words;
    std::istringstream iss((std::string)text);
    std::string currentWord;
    while (getline(iss, currentWord, ' '))
    {
      words.push_back(currentWord);
    }

    float lines = 1;
    float currentLineLength = 0;
    for (const std::string &word : words)
    {
      float wordWidth = (float)word.length() * widthPerChar;
      if (wordWidth > measuredWidth)
      {
        if (currentLineLength > 0)
        {
          lines++;
        }
        lines++;
        currentLineLength = 0;
      }
      else if (currentLineLength + wordWidth <= measuredWidth)
      {
        currentLineLength += wordWidth + widthPerChar;
      }
      else
      {
        lines++;
        currentLineLength = wordWidth + widthPerChar;
      }
    }
    return (currentLineLength == 0 ? lines - 1 : lines) * heightPerChar;
  }

}