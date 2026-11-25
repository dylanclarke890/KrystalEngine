#include "Krystal.Booey/Geometry/DOMQuad.hpp"
#include "Krystal.Booey/Geometry/DOMRect.hpp"
#include "Krystal.Core/Move.hpp"

namespace krys::boo
{
  DOMQuad::DOMQuad(Ref<DOMPoint> point1, Ref<DOMPoint> point2, Ref<DOMPoint> point3,
                   Ref<DOMPoint> point4) noexcept
      : _point1(krys::move(point1)), _point2(krys::move(point2)), _point3(krys::move(point3)),
        _point4(krys::move(point4))
  {
  }

#pragma region DOMQuad - https://drafts.csswg.org/geometry-1/#domquad

  Ref<DOMQuad> DOMQuad::Create(const DOMPointInit &p1, const DOMPointInit &p2, const DOMPointInit &p3,
                               const DOMPointInit &p4) noexcept
  {
    // 1. Let point1 be a new DOMPoint object with its attributes set to the values of the namesake dictionary
    //    members in p1.
    auto point1 = DOMPoint::FromPoint(p1);
    // 2. Let point2 be a new DOMPoint object with its attributes set to the values of the namesake dictionary
    //    members in p2.
    auto point2 = DOMPoint::FromPoint(p2);
    // 3. Let point3 be a new DOMPoint object with its attributes set to the values of the namesake dictionary
    //    members in p3.
    auto point3 = DOMPoint::FromPoint(p3);
    // 4. Let point4 be a new DOMPoint object with its attributes set to the values of the namesake dictionary
    //    members in p4.
    auto point4 = DOMPoint::FromPoint(p4);

    // 5. Return a new DOMQuad with point 1 set to point1, point 2 set to point2, point 3 set to point3 and
    // point 4
    //    set to point4.
    return AdoptRef(
      *new DOMQuad(krys::move(point1), krys::move(point2), krys::move(point3), krys::move(point4)));
  }

  Ref<DOMQuad> DOMQuad::FromRect(const DOMRectInit &other) noexcept
  {
    // https://drafts.csswg.org/geometry-1/#create-a-domquad-from-the-domrectinit-dictionary
    // 1. Let x, y, width and height be the value of other's x, y, width and height dictionary members,
    //    respectively.
    const double x = other.X;
    const double y = other.Y;
    const double width = other.Width;
    const double height = other.Height;

    // 2. Let point1 be a new DOMPoint object with x coordinate set to x, y coordinate set to y, z coordinate
    // set
    //    to 0 and w perspective set to 1.
    auto point1 = DOMPoint::Create(x, y, 0, 1);
    // 3. Let point2 be a new DOMPoint object with x coordinate set to x + width, y coordinate set to y, z
    //    coordinate set to 0 and w perspective set to 1.
    auto point2 = DOMPoint::Create(x + width, y, 0, 1);
    // 4. Let point3 be a new DOMPoint object with x coordinate set to x + width, y coordinate set to y +
    // height,
    //    z coordinate set to 0 and w perspective set to 1.
    auto point3 = DOMPoint::Create(x + width, y + height, 0, 1);
    // 5. Let point4 be a new DOMPoint object with x coordinate set to x, y coordinate set to y + height, z
    //    coordinate set to 0 and w perspective set to 1.
    auto point4 = DOMPoint::Create(x, y + height, 0, 1);

    // 6. Return a new DOMQuad with point 1 set to point1, point 2 set to point2, point 3 set to point3 and
    // point 4
    //    set to point4.
    return AdoptRef(
      *new DOMQuad(krys::move(point1), krys::move(point2), krys::move(point3), krys::move(point4)));
  }

  Ref<DOMQuad> DOMQuad::FromQuad(const DOMQuadInit &other) noexcept
  {
    // https://drafts.csswg.org/geometry-1/#create-a-domquad-from-the-domquadinit-dictionary
    // A member that's missing is the default point, as the dictionary's members are.
    // 1. Let point1 be the result of invoking create a DOMPoint from the dictionary p1 dictionary member of
    //    other, if it exists.
    auto point1 = DOMPoint::FromPoint(other.P1);
    // 2. Let point2 be the result of invoking create a DOMPoint from the dictionary p2 dictionary member of
    //    other, if it exists.
    auto point2 = DOMPoint::FromPoint(other.P2);
    // 3. Let point3 be the result of invoking create a DOMPoint from the dictionary p3 dictionary member of
    //    other, if it exists.
    auto point3 = DOMPoint::FromPoint(other.P3);
    // 4. Let point4 be the result of invoking create a DOMPoint from the dictionary p4 dictionary member of
    //    other, if it exists.
    auto point4 = DOMPoint::FromPoint(other.P4);

    // 5. Return a new DOMQuad with point 1 set to point1, point 2 set to point2, point 3 set to point3 and
    // point 4
    //    set to point4.
    return AdoptRef(
      *new DOMQuad(krys::move(point1), krys::move(point2), krys::move(point3), krys::move(point4)));
  }

  Ref<DOMRect> DOMQuad::GetBounds() const noexcept
  {
    // 1. Let bounds be a DOMRect object.
    auto bounds = DOMRect::Create();

    // 2. Let left be the NaN-safe minimum of point 1's x coordinate, point 2's x coordinate, point 3's x
    //    coordinate and point 4's x coordinate.
    const double left =
      NaNSafeMinimum(NaNSafeMinimum(_point1->X(), _point2->X()), NaNSafeMinimum(_point3->X(), _point4->X()));
    // 3. Let top be the NaN-safe minimum of point 1's y coordinate, point 2's y coordinate, point 3's y
    //    coordinate and point 4's y coordinate.
    const double top =
      NaNSafeMinimum(NaNSafeMinimum(_point1->Y(), _point2->Y()), NaNSafeMinimum(_point3->Y(), _point4->Y()));
    // 4. Let right be the NaN-safe maximum of point 1's x coordinate, point 2's x coordinate, point 3's x
    //    coordinate and point 4's x coordinate.
    const double right =
      NaNSafeMaximum(NaNSafeMaximum(_point1->X(), _point2->X()), NaNSafeMaximum(_point3->X(), _point4->X()));
    // 5. Let bottom be the NaN-safe maximum of point 1's y coordinate, point 2's y coordinate, point 3's y
    //    coordinate and point 4's y coordinate.
    const double bottom =
      NaNSafeMaximum(NaNSafeMaximum(_point1->Y(), _point2->Y()), NaNSafeMaximum(_point3->Y(), _point4->Y()));

    // 6. Set x coordinate of bounds to left, y coordinate of bounds to top, width dimension of bounds to
    // right -
    //    left and height dimension of bounds to bottom - top.
    bounds->SetX(left);
    bounds->SetY(top);
    bounds->SetWidth(right - left);
    bounds->SetHeight(bottom - top);

    // 7. Return bounds.
    return bounds;
  }

#pragma endregion
}
