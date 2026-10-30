#include "Krystal.Booey/Rendering/OrderIterator.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>

namespace krys::boo
{
  OrderIterator::OrderIterator(RenderBox &containerBox) noexcept : _containerBox(&containerBox)
  {
    Reset();
  }

  RenderBox *OrderIterator::First() noexcept
  {
    Reset();
    return Next();
  }

  int32 OrderIterator::CurrentOrderValue() const noexcept
  {
    return _reversedOrder ? _orderValues[_orderValues.size() - 1 - _orderValuesIndex]
                          : _orderValues[_orderValuesIndex];
  }

  RenderBox *OrderIterator::Next() noexcept
  {
    do
    {
      if (!_currentChild)
      {
        if (_orderValuesIndex == _orderValues.size())
        {
          return nullptr;
        }

        if (!_isReset)
        {
          ++_orderValuesIndex;
          if (_orderValuesIndex == _orderValues.size())
          {
            return nullptr;
          }
        }
        else
        {
          _isReset = false;
        }
        _currentChild = _reversedOrder ? _containerBox->LastChildBox() : _containerBox->FirstChildBox();
      }
      else
      {
        _currentChild =
          _reversedOrder ? _currentChild->PreviousSiblingBox() : _currentChild->NextSiblingBox();
      }
    } while (!_currentChild || _currentChild->Style().GetOrder().Value != CurrentOrderValue());

    return _currentChild;
  }

  void OrderIterator::Reset() noexcept
  {
    _currentChild = nullptr;
    _orderValuesIndex = 0;
    _isReset = true;
  }

  bool OrderIterator::ShouldSkipChild(const RenderObject &child) const noexcept
  {
    return child.IsOutOfFlowPositioned() || child.IsExcludedFromNormalLayout();
  }

  OrderIterator OrderIterator::Reverse() const noexcept
  {
    auto reversedIterator = *this;
    reversedIterator._reversedOrder = !_reversedOrder;
    reversedIterator.Reset();
    return reversedIterator;
  }

  OrderIteratorPopulator::OrderIteratorPopulator(OrderIterator &iterator) noexcept : _iterator(iterator)
  {
    _iterator._orderValues.clear();
  }

  OrderIteratorPopulator::~OrderIteratorPopulator() noexcept
  {
    auto &orderValues = _iterator._orderValues;
    std::ranges::sort(orderValues);
    orderValues.erase(std::ranges::unique(orderValues).begin(), orderValues.end());
    _iterator.Reset();
  }

  bool OrderIteratorPopulator::CollectChild(const RenderBox &child) noexcept
  {
    _iterator._orderValues.push_back(child.Style().GetOrder().Value);
    return !_iterator.ShouldSkipChild(child);
  }
}
