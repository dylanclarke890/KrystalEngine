#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"

namespace krys::boo::layout
{
  Rect BoxGeometry::MarginBox() const noexcept
  {
    auto borderBox = BorderBox();

    Rect marginBox;
    marginBox.SetTop(borderBox.Top() - MarginBefore());
    marginBox.SetLeft(borderBox.Left() - MarginStart());
    marginBox.SetHeight(borderBox.Height() + MarginBefore() + MarginAfter());
    marginBox.SetWidth(borderBox.Width() + MarginStart() + MarginEnd());
    return marginBox;
  }

  Rect BoxGeometry::BorderBox() const noexcept
  {
    Rect borderBox;
    borderBox.SetTopLeft({});
    borderBox.SetSize({BorderBoxWidth(), BorderBoxHeight()});
    return borderBox;
  }

  Rect BoxGeometry::PaddingBox() const noexcept
  {
    auto borderBox = BorderBox();

    Rect paddingBox;
    paddingBox.SetTop(borderBox.Top() + BorderBefore());
    paddingBox.SetLeft(borderBox.Left() + BorderStart());
    paddingBox.SetHeight(borderBox.Bottom() - VerticalSpaceForScrollbar() - BorderAfter() - BorderBefore());
    paddingBox.SetWidth(borderBox.Width() - BorderEnd() - HorizontalSpaceForScrollbar() - BorderStart());
    return paddingBox;
  }

  Rect BoxGeometry::ContentBox() const noexcept
  {
    Rect contentBox;
    contentBox.SetTop(ContentBoxTop());
    contentBox.SetLeft(ContentBoxLeft());
    contentBox.SetWidth(ContentBoxWidth());
    contentBox.SetHeight(ContentBoxHeight());
    return contentBox;
  }

  void BoxGeometry::Reset() noexcept
  {
    SetTopLeft({});
    SetHorizontalMargin({});
    SetVerticalMargin({});
    SetBorder({});
    SetPadding({});
    SetContentBoxSize({});
    SetVerticalSpaceForScrollbar({});
    SetHorizontalSpaceForScrollbar({});
  }
}
