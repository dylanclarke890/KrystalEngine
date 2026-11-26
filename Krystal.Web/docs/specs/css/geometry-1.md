# geometry-1 review checklist

Generated from <https://drafts.csswg.org/geometry-1/> on 2026-11-25 by `Krystal.Web/tools/spec_checklist.py`: 67 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Web/tools/spec_status.py geometry-1` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[geometry-1:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Web/tools/spec_status.py geometry-1 --tick <section anchor>` ticks a section's algorithms and sets its status.

## [Abstract](https://drafts.csswg.org/geometry-1/#abstract)

Status: n/a (front matter)

## [Status of this document](https://drafts.csswg.org/geometry-1/#sotd)

Status: n/a (front matter)

## [1. Introduction](https://drafts.csswg.org/geometry-1/#intro)

Status: n/a (introduction)

## [2. The DOMPoint interfaces](https://drafts.csswg.org/geometry-1/#DOMPoint)

Status: reviewed (2026-11-25)
> Code: Css/Geometry/DOMPointReadOnly.hpp and DOMPoint.hpp; the coordinates are doubles that may be NaN or infinite. toJSON() doesn't apply (see DIFFERENCES.md).
<!-- definitions: point point-x-coordinate point-y-coordinate point-z-coordinate point-w-perspective dom-dompoint-dompoint create-a-dompoint-from-the-dictionary dom-dompointreadonly-x -->

- [x] [DOMPointReadOnly.DOMPointReadOnly(x, y, z, w)](https://drafts.csswg.org/geometry-1/#dom-dompointreadonly-dompointreadonly) — The DOMPointReadOnly(x, y, z, w) and DOMPoint(x, y, z, w) constructors, when invoked, must run the following steps:
  - 1. Let point be a new DOMPointReadOnly or DOMPoint object as appropriate.
  - 2. Set point’s variables x coordinate to x, y coordinate to y, z coordinate to z and w perspective to w.
  - 3. Return point.

- [x] [DOMPointReadOnly.fromPoint(other)](https://drafts.csswg.org/geometry-1/#dom-dompointreadonly-frompoint) — The fromPoint(other) static method on DOMPointReadOnly must create a DOMPointReadOnly from the dictionary other.

- [x] [DOMPoint.fromPoint(other)](https://drafts.csswg.org/geometry-1/#dom-dompoint-frompoint) — The fromPoint(other) static method on DOMPoint must create a DOMPoint from the dictionary other.

- [x] [create a DOMPointReadOnly from a dictionary](https://drafts.csswg.org/geometry-1/#create-a-dompointreadonly-from-the-dictionary) — To create a DOMPointReadOnly from a dictionary other, or to create a DOMPoint from a dictionary other, follow these steps:
  - 1. Let point be a new DOMPointReadOnly or DOMPoint as appropriate.
  - 2. Set point’s variables x coordinate to other’s x dictionary member, y coordinate to other’s y dictionary member, z coordinate to other’s z dictionary member and w perspective to other’s w dictionary member.
  - 3. Return point.

- [x] [y (DOMPointReadOnly, DOMPoint)](https://drafts.csswg.org/geometry-1/#dom-dompointreadonly-y) — The y attribute, on getting, must return the y coordinate value. For the DOMPoint interface, setting the y attribute must set the y coordinate to the new value.

- [x] [z (DOMPointReadOnly, DOMPoint)](https://drafts.csswg.org/geometry-1/#dom-dompointreadonly-z) — The z attribute, on getting, must return the z coordinate value. For the DOMPoint interface, setting the z attribute must set the z coordinate to the new value.

- [x] [w (DOMPointReadOnly, DOMPoint)](https://drafts.csswg.org/geometry-1/#dom-dompointreadonly-w) — The w attribute, on getting, must return the w perspective value. For the DOMPoint interface, setting the w attribute must set the w perspective to the new value.

- [x] [DOMPointReadOnly.matrixTransform(matrix)](https://drafts.csswg.org/geometry-1/#dom-dompointreadonly-matrixtransform) — The matrixTransform(matrix) method, when invoked, must run the following steps:
  - 1. Let matrixObject be the result of invoking create a DOMMatrix from the dictionary matrix.
  - 2. Return the result of invoking transform a point with a matrix, given the current point and matrixObject. The current point does not get modified.

### [2.1. Transforming a point with a matrix](https://drafts.csswg.org/geometry-1/#transforming-a-point-with-a-matrix)

Status: reviewed (2026-11-25)
> Code: TransformPointWithMatrix in DOMPointReadOnly.cpp, shared by matrixTransform() and DOMMatrixReadOnly::TransformPoint.

- [x] [transform a point with a matrix](https://drafts.csswg.org/geometry-1/#transform-a-point-with-a-matrix) — To transform a point with a matrix, given point and matrix:
  - 1. Let x be point’s x coordinate.
  - 2. Let y be point’s y coordinate.
  - 3. Let z be point’s z coordinate.
  - 4. Let w be point’s w perspective.
  - 5. Let pointVector be a new column vector with the elements being x, y, z, and w, respectively. xyzw
  - 6. Set pointVector to pointVector pre-multiplied by matrix.
  - 7. Let transformedPoint be a new DOMPoint object.
  - 8. Set transformedPoint’s x coordinate to pointVector’s first element.
  - 9. Set transformedPoint’s y coordinate to pointVector’s second element.
  - 10. Set transformedPoint’s z coordinate to pointVector’s third element.
  - 11. Set transformedPoint’s w perspective to pointVector’s fourth element.
  - 12. Return transformedPoint.

## [3. The DOMRect interfaces](https://drafts.csswg.org/geometry-1/#DOMRect)

Status: reviewed (2026-11-25)
> Code: Css/Geometry/DOMRectReadOnly.hpp and DOMRect.hpp; NaNSafeMinimum and NaNSafeMaximum are the NaN-safe minimum and maximum. toJSON() doesn't apply.
<!-- definitions: rectangle rectangle-origin rectangle-x-coordinate rectangle-y-coordinate rectangle-width-dimension rectangle-height-dimension dom-domrect-domrect create-a-domrect-from-the-dictionary dom-domrectreadonly-domrect-x -->

- [x] [DOMRectReadOnly.DOMRectReadOnly(x, y, width, height)](https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrectreadonly) — The DOMRectReadOnly(x, y, width, height) and DOMRect(x, y, width, height) constructors, when invoked, must run the following steps:
  - 1. Let rect be a new DOMRectReadOnly or DOMRect object as appropriate.
  - 2. Set rect’s variables x coordinate to x, y coordinate to y, width dimension to width and height dimension to height.
  - 3. Return rect.

- [x] [DOMRectReadOnly.fromRect(other)](https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-fromrect) — The fromRect(other) static method on DOMRectReadOnly must create a DOMRectReadOnly from the dictionary other.

- [x] [DOMRect.fromRect(other)](https://drafts.csswg.org/geometry-1/#dom-domrect-fromrect) — The fromRect(other) static method on DOMRect must create a DOMRect from the dictionary other.

- [x] [create a DOMRectReadOnly from a dictionary](https://drafts.csswg.org/geometry-1/#create-a-domrectreadonly-from-the-dictionary) — To create a DOMRectReadOnly from a dictionary other, or to create a DOMRect from a dictionary other, follow these steps:
  - 1. Let rect be a new DOMRectReadOnly or DOMRect as appropriate.
  - 2. Set rect’s variables x coordinate to other’s x dictionary member, y coordinate to other’s y dictionary member, width dimension to other’s width dictionary member and height dimension to other’s height dictionary member.
  - 3. Return rect.

- [x] [y (DOMRectReadOnly DOMRect)](https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-y) — The y attribute, on getting, it must return the y coordinate value. For the DOMRect interface, setting the y attribute must set the y coordinate to the new value.

- [x] [width (DOMRectReadOnly DOMRect)](https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-width) — The width attribute, on getting, must return the width dimension value. For the DOMRect interface, setting the width attribute must set the width dimension to the new value.

- [x] [height (DOMRectReadOnly DOMRect)](https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-height) — The height attribute, on getting, must return the height dimension value. For the DOMRect interface, setting the height attribute must set the height dimension value to the new value.

- [x] [top (DOMRectReadOnly DOMRect)](https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-top) — The top attribute, on getting, must return the NaN-safe minimum of the y coordinate and the sum of the y coordinate and the height dimension.

- [x] [right (DOMRectReadOnly DOMRect)](https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-right) — The right attribute, on getting, must return the NaN-safe maximum of the x coordinate and the sum of the x coordinate and the width dimension.

- [x] [bottom (DOMRectReadOnly DOMRect)](https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-bottom) — The bottom attribute, on getting, must return the NaN-safe maximum of the y coordinate and the sum of the y coordinate and the height dimension.

- [x] [left (DOMRectReadOnly DOMRect)](https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-left) — The left attribute, on getting, must return the NaN-safe minimum of the x coordinate and the sum of the x coordinate and the width dimension.

## [4. The DOMRectList interface](https://drafts.csswg.org/geometry-1/#DOMRectList)

Status: reviewed (2026-11-25)
> Code: Css/Geometry/DOMRectList.hpp.

- [x] [DOMRectList.length](https://drafts.csswg.org/geometry-1/#dom-domrectlist-length) — The length attribute must return the total number of DOMRect objects associated with the object.

- [x] [DOMRectList.item(index)](https://drafts.csswg.org/geometry-1/#dom-domrectlist-item) — The item(index) method, when invoked, must return null when index is greater than or equal to the number of DOMRect objects associated with the DOMRectList. Otherwise, the DOMRect object at index must be returned. Indices are zero-based.

## [5. The DOMQuad interface](https://drafts.csswg.org/geometry-1/#DOMQuad)

Status: reviewed (2026-11-25)
> Code: Css/Geometry/DOMQuad.hpp, added in this review. DOMQuadInit's members default to the default point, so a missing one makes the same point as an empty dictionary. toJSON() doesn't apply.
<!-- definitions: quadrilateral quadrilateral-point-1 quadrilateral-point-2 quadrilateral-point-3 quadrilateral-point-4 dom-domquad-p1 -->

- [x] [DOMQuad.DOMQuad(p1, p2, p3, p4)](https://drafts.csswg.org/geometry-1/#dom-domquad-domquad) — The DOMQuad(p1, p2, p3, p4) constructor, when invoked, must run the following steps:
  - 1. Let point1 be a new DOMPoint object with its attributes set to the values of the namesake dictionary members in p1.
  - 2. Let point2 be a new DOMPoint object with its attributes set to the values of the namesake dictionary members in p2.
  - 3. Let point3 be a new DOMPoint object with its attributes set to the values of the namesake dictionary members in p3.
  - 4. Let point4 be a new DOMPoint object with its attributes set to the values of the namesake dictionary members in p4.
  - 5. Return a new DOMQuad with point 1 set to point1, point 2 set to point2, point 3 set to point3 and point 4 set to point4.

- [x] [DOMQuad.fromRect(other)](https://drafts.csswg.org/geometry-1/#dom-domquad-fromrect) — The fromRect(other) static method on DOMQuad must create a DOMQuad from the DOMRectInit dictionary other.

- [x] [create a DOMQuad from a DOMRectInit dictionary](https://drafts.csswg.org/geometry-1/#create-a-domquad-from-the-domrectinit-dictionary) — To create a DOMQuad from a DOMRectInit dictionary other, follow these steps:
  - 1. Let x, y, width and height be the value of other’s x, y, width and height dictionary members, respectively.
  - 2. Let point1 be a new DOMPoint object with x coordinate set to x, y coordinate set to y, z coordinate set to 0 and w perspective set to 1.
  - 3. Let point2 be a new DOMPoint object with x coordinate set to x + width, y coordinate set to y, z coordinate set to 0 and w perspective set to 1.
  - 4. Let point3 be a new DOMPoint object with x coordinate set to x + width, y coordinate set to y + height, z coordinate set to 0 and w perspective set to 1.
  - 5. Let point4 be a new DOMPoint object with x coordinate set to x, y coordinate set to y + height, z coordinate set to 0 and w perspective set to 1.
  - 6. Return a new DOMQuad with point 1 set to point1, point 2 set to point2, point 3 set to point3 and point 4 set to point4.

- [x] [DOMQuad.fromQuad(other)](https://drafts.csswg.org/geometry-1/#dom-domquad-fromquad) — The fromQuad(other) static method on DOMQuad must create a DOMQuad from the DOMQuadInit dictionary other.

- [x] [create a DOMQuad from a DOMQuadInit dictionary](https://drafts.csswg.org/geometry-1/#create-a-domquad-from-the-domquadinit-dictionary) — To create a DOMQuad from a DOMQuadInit dictionary other, follow these steps:
  - 1. Let point1 be the result of invoking create a DOMPoint from the dictionary p1 dictionary member of other, if it exists.
  - 2. Let point2 be the result of invoking create a DOMPoint from the dictionary p2 dictionary member of other, if it exists.
  - 3. Let point3 be the result of invoking create a DOMPoint from the dictionary p3 dictionary member of other, if it exists.
  - 4. Let point4 be the result of invoking create a DOMPoint from the dictionary p4 dictionary member of other, if it exists.
  - 5. Return a new DOMQuad with point 1 set to point1, point 2 set to point2, point 3 set to point3 and point 4 set to point4.

- [x] [DOMQuad.p2](https://drafts.csswg.org/geometry-1/#dom-domquad-p2) — The p2 attribute must return point 2.

- [x] [DOMQuad.p3](https://drafts.csswg.org/geometry-1/#dom-domquad-p3) — The p3 attribute must return point 3.

- [x] [DOMQuad.p4](https://drafts.csswg.org/geometry-1/#dom-domquad-p4) — The p4 attribute must return point 4.

- [x] [DOMQuad.getBounds()](https://drafts.csswg.org/geometry-1/#dom-domquad-getbounds) — The getBounds() method, when invoked, must run the following algorithm:
  - 1. Let bounds be a DOMRect object.
  - 2. Let left be the NaN-safe minimum of point 1’s x coordinate, point 2’s x coordinate, point 3’s x coordinate and point 4’s x coordinate.
  - 3. Let top be the NaN-safe minimum of point 1’s y coordinate, point 2’s y coordinate, point 3’s y coordinate and point 4’s y coordinate.
  - 4. Let right be the NaN-safe maximum of point 1’s x coordinate, point 2’s x coordinate, point 3’s x coordinate and point 4’s x coordinate.
  - 5. Let bottom be the NaN-safe maximum of point 1’s y coordinate, point 2’s y coordinate, point 3’s y coordinate and point 4’s y coordinate.
  - 6. Set x coordinate of bounds to left, y coordinate of bounds to top, width dimension of bounds to right - left and height dimension of bounds to bottom - top.
  - 7. Return bounds.

## [6. The DOMMatrix interfaces](https://drafts.csswg.org/geometry-1/#DOMMatrix)

Status: reviewed (2026-11-25)
> Code: Css/Geometry/DOMMatrixReadOnly.hpp and DOMMatrix.hpp. The matrix is a TransformationMatrix, whose Multiply post-multiplies; is 2D is _is2D.
<!-- definitions: matrix post-multiply pre-multiply multiply matrix-m11-element matrix-m12-element matrix-m13-element matrix-m14-element matrix-m21-element matrix-m22-element matrix-m23-element matrix-m24-element matrix-m31-element matrix-m32-element matrix-m33-element matrix-m34-element matrix-m41-element matrix-m42-element matrix-m43-element matrix-m44-element -->

### [6.1. DOMMatrix2DInit and DOMMatrixInit dictionaries](https://drafts.csswg.org/geometry-1/#dommatrixinit-dictionary)

Status: reviewed (2026-11-25)
> Code: DOMMatrixReadOnly::ValidateAndFixup (both). DOMMatrixInit's 3D members have their defaults, so they're always present.

- [x] [validate and fixup (2D) (matrix)](https://drafts.csswg.org/geometry-1/#matrix-validate-and-fixup-2d) — To validate and fixup (2D) a DOMMatrix2DInit or DOMMatrixInit dictionary dict, run the following steps:
  - 1. If if at least one of the following conditions are true for dict, then throw a TypeError exception and abort these steps. a and m11 are both present and SameValueZero(a, m11) is false.; b and m12 are both present and SameValueZero(b, m12) is false.; c and m21 are both present and SameValueZero(c, m21) is false.; d and m22 are both present and SameValueZero(d, m22) is false.; e and m41 are both pr…
  - 2. If m11 is not present then set it to the value of member a, or value 1 if a is also not present.
  - 3. If m12 is not present then set it to the value of member b, or value 0 if b is also not present.
  - 4. If m21 is not present then set it to the value of member c, or value 0 if c is also not present.
  - 5. If m22 is not present then set it to the value of member d, or value 1 if d is also not present.
  - 6. If m41 is not present then set it to the value of member e, or value 0 if e is also not present.
  - 7. If m42 is not present then set it to the value of member f, or value 0 if f is also not present.

- [x] [validate and fixup (matrix)](https://drafts.csswg.org/geometry-1/#matrix-validate-and-fixup) — To validate and fixup a DOMMatrixInit dictionary dict, run the following steps:
  - 1. Validate and fixup (2D) dict.
  - 2. If is2D is true and: at least one of m13, m14, m23, m24, m31, m32, m34, m43 are present with a value other than 0 or -0, or at least one of m33, m44 are present with a value other than 1, then throw a TypeError exception and abort these steps.
  - 3. If is2D is not present and at least one of m13, m14, m23, m24, m31, m32, m34, m43 are present with a value other than 0 or -0, or at least one of m33, m44 are present with a value other than 1, set is2D to false.
  - 4. If is2D is still not present, set it to true.

### [6.2. Parsing a string into an abstract matrix](https://drafts.csswg.org/geometry-1/#dommatrix-parse)

Status: reviewed (2026-11-25)
> Code: DOMMatrixReadOnly::ParseStringIntoAbstractMatrix. Relative lengths and percentages fail to parse or are found by ComputeSizeDependencies.

- [x] [parse a string into an abstract matrix](https://drafts.csswg.org/geometry-1/#parse-a-string-into-an-abstract-matrix) — To parse a string into an abstract matrix, given a string transformList, means to run the following steps. It will either return a 4x4 abstract matrix and a boolean 2dTransform, or failure.
  - 1. If transformList is the empty string, set it to the string "matrix(1, 0, 0, 1, 0, 0)".
  - 2. Parse transformList into parsedValue given the grammar for the CSS transform property. The result will be a <transform-list>, the keyword none, or failure. If parsedValue is failure, or any <transform-function> has <length> values without absolute length units, or any keyword other than none is used, then return failure. [CSS3-SYNTAX] [CSS3-TRANSFORMS]
  - 3. If parsedValue is none, set parsedValue to a <transform-list> containing a single identity matrix.
  - 4. Let 2dTransform track the 2D/3D dimension status of parsedValue. If parsedValue consists of any three-dimensional transform functions Set 2dTransform to false. Otherwise Set 2dTransform to true.
  - 5. Transform all <transform-function>s to 4x4 abstract matrices by following the “Mathematical Description of Transform Functions”. [CSS3-TRANSFORMS]
  - 6. Let matrix be a 4x4 abstract matrix as shown in the initial figure of this section. Post-multiply all matrices from left to right and set matrix to this product.
  - 7. Return matrix and 2dTransform.

### [6.3. Creating DOMMatrixReadOnly and DOMMatrix objects](https://drafts.csswg.org/geometry-1/#dommatrix-create)

Status: reviewed (2026-11-25)
> Code: DOMMatrixReadOnly::CreateHelper, FromArrayHelper (create a 2d or 3d matrix) and FromMatrixHelper. Creating from a 2D dictionary is done by its callers (canvas setTransform(), CanvasPattern::SetTransform, Path2D::AddPath): ValidateAndFixup, then the fixed-up members. The string constructor's Window check is skipped (DIFFERENCES.md).
<!-- definitions: dom-dommatrix-dommatrix create-a-dommatrix-from-the-2d-dictionary create-a-dommatrix-from-the-dictionary dom-dommatrix-fromfloat32array dom-dommatrix-fromfloat64array -->

- [x] [create a 2d matrix](https://drafts.csswg.org/geometry-1/#create-a-2d-matrix) — To create a 2d matrix of type type being either DOMMatrixReadOnly or DOMMatrix, with a sequence init of 6 elements, follow these steps:
  - 1. Let matrix be a new instance of type.
  - 2. Set m11 element, m12 element, m21 element, m22 element, m41 element and m42 element to the values of init in order starting with the first value.
  - 3. Set m13 element, m14 element, m23 element, m24 element, m31 element, m32 element, m34 element, and m43 element to 0.
  - 4. Set m33 element and m44 element to 1.
  - 5. Set is 2D to true.
  - 6. Return matrix

- [x] [create a 3d matrix](https://drafts.csswg.org/geometry-1/#create-a-3d-matrix) — To create a 3d matrix with type being either DOMMatrixReadOnly or DOMMatrix, with a sequence init of 16 elements, follow these steps:
  - 1. Let matrix be a new instance of type.
  - 2. Set m11 element to m44 element to the values of init in column-major order.
  - 3. Set is 2D to false.
  - 4. Return matrix

- [x] [DOMMatrixReadOnly.DOMMatrixReadOnly(init)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-dommatrixreadonly) — The DOMMatrixReadOnly(init) and the DOMMatrix(init) constructors must follow these steps:
  - • If init is omitted: Return the result of invoking create a 2d matrix of type DOMMatrixReadOnly or DOMMatrix as appropriate, with the sequence [1, 0, 0, 1, 0, 0].
  - • If init is a DOMString:
    - 1. If current global object is not a Window object, then throw a TypeError exception.
    - 2. Parse init into an abstract matrix, and let matrix and 2dTransform be the result. If the result is failure, then throw a "SyntaxError" DOMException.
    - 3. If 2dTransform is true Return the result of invoking create a 2d matrix of type DOMMatrixReadOnly or DOMMatrix as appropriate, with a sequence of numbers, the values being the elements m11, m12, m21, m22, m41 and m42 of matrix. Otherwise Return the result of invoking create a 3d matrix of type DOMMatrixReadOnly or DOMMatrix as appropriate, with a sequence of numbers, the values being the 16 eleme…
  - • If init is a sequence with 6 elements: Return the result of invoking create a 2d matrix of type DOMMatrixReadOnly or DOMMatrix as appropriate, with the sequence init.
  - • If init is a sequence with 16 elements: Return the result of invoking create a 3d matrix of type DOMMatrixReadOnly or DOMMatrix as appropriate, with the sequence init.
  - • Otherwise: Throw a TypeError exception.

- [x] [DOMMatrixReadOnly.fromMatrix(other)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-frommatrix) — The fromMatrix(other) static method on DOMMatrixReadOnly must create a DOMMatrixReadOnly from the dictionary other.

- [x] [DOMMatrix.fromMatrix(other)](https://drafts.csswg.org/geometry-1/#dom-dommatrix-frommatrix) — The fromMatrix(other) static method on DOMMatrix must create a DOMMatrix from the dictionary other.

- [x] [create a DOMMatrixReadOnly from a 2D dictionary](https://drafts.csswg.org/geometry-1/#create-a-dommatrixreadonly-from-the-2d-dictionary) — To create a DOMMatrixReadOnly from a 2D dictionary other or to create a DOMMatrix from a 2D dictionary other, follow these steps:
  - 1. Validate and fixup (2D) other.
  - 2. Return the result of invoking create a 2d matrix of type DOMMatrixReadOnly or DOMMatrix as appropriate, with a sequence of numbers, the values being the 6 elements m11, m12, m21, m22, m41 and m42 of other in the given order.

- [x] [create a DOMMatrixReadOnly from a dictionary](https://drafts.csswg.org/geometry-1/#create-a-dommatrixreadonly-from-the-dictionary) — To create a DOMMatrixReadOnly from a dictionary other or to create a DOMMatrix from a dictionary other, follow these steps:
  - 1. Validate and fixup other.
  - 2. If the is2D dictionary member of other is true Return the result of invoking create a 2d matrix of type DOMMatrixReadOnly or DOMMatrix as appropriate, with a sequence of numbers, the values being the 6 elements m11, m12, m21, m22, m41 and m42 of other in the given order. Otherwise Return the result of invoking create a 3d matrix of type DOMMatrixReadOnly or DOMMatrix as appropriate, with a sequen…

- [x] [DOMMatrixReadOnly.fromFloat32Array(array32)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-fromfloat32array) — The fromFloat32Array(array32) static method on DOMMatrixReadOnly and the fromFloat32Array(array32) static method on DOMMatrix must follow these steps:
  - • If array32 has 6 elements: Return the result of invoking create a 2d matrix of type DOMMatrixReadOnly or DOMMatrix as appropriate, with a sequence of numbers taking the values from array32 in the provided order.
  - • If array32 has 16 elements: Return the result of invoking create a 3d matrix of type DOMMatrixReadOnly or DOMMatrix as appropriate, with a sequence of numbers taking the values from array32 in the provided order.
  - • Otherwise: Throw a TypeError exception.

- [x] [DOMMatrixReadOnly.fromFloat64Array(array64)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-fromfloat64array) — The fromFloat64Array(array64) static method on DOMMatrixReadOnly and the fromFloat64Array(array64) static method on DOMMatrix must follow these steps:
  - • If array64 has 6 elements: Return the result of invoking create a 2d matrix of type DOMMatrixReadOnly or DOMMatrix as appropriate, with a sequence of numbers taking the values from array64 in the provided order.
  - • If array64 has 16 elements: Return the result of invoking create a 3d matrix of type DOMMatrixReadOnly or DOMMatrix as appropriate, with a sequence of numbers taking the values from array64 in the provided order.
  - • Otherwise: Throw a TypeError exception.

### [6.4. DOMMatrix attributes](https://drafts.csswg.org/geometry-1/#dommatrix-attributes)

Status: reviewed (2026-11-25)
> Code: the accessors in DOMMatrixReadOnly.hpp and setters in DOMMatrix.hpp (DOMMatrix::ClearIs2DUnless).
<!-- definitions: dom-dommatrixreadonly-m11 dom-dommatrixreadonly-a dom-dommatrixreadonly-b dom-dommatrixreadonly-c dom-dommatrixreadonly-d dom-dommatrixreadonly-e dom-dommatrixreadonly-f dom-dommatrixreadonly-is2d -->

- [x] [m12 (DOMMatrixReadOnly, DOMMatrix)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m12) — The m12 and b attributes, on getting, must return the m12 element value. For the DOMMatrix interface, setting the m12 or the b attribute must set the m12 element to the new value.

- [x] [m13 (DOMMatrixReadOnly, DOMMatrix)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m13) — The m13 attribute, on getting, must return the m13 element value. For the DOMMatrix interface, setting the m13 attribute must set the m13 element to the new value and, if the new value is not 0 or -0, set is 2D to false.

- [x] [m14 (DOMMatrixReadOnly, DOMMatrix)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m14) — The m14 attribute, on getting, must return the m14 element value. For the DOMMatrix interface, setting the m14 attribute must set the m14 element to the new value and, if the new value is not 0 or -0, set is 2D to false.

- [x] [m21 (DOMMatrixReadOnly, DOMMatrix)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m21) — The m21 and c attributes, on getting, must return the m21 element value. For the DOMMatrix interface, setting the m21 or the c attribute must set the m21 element to the new value.

- [x] [m22 (DOMMatrixReadOnly, DOMMatrix)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m22) — The m22 and d attributes, on getting, must return the m22 element value. For the DOMMatrix interface, setting the m22 or the d attribute must set the m22 element to the new value.

- [x] [m23 (DOMMatrixReadOnly, DOMMatrix)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m23) — The m23 attribute, on getting, must return the m23 element value. For the DOMMatrix interface, setting the m23 attribute must set the m23 element to the new value and, if the new value is not 0 or -0, set is 2D to false.

- [x] [m24 (DOMMatrixReadOnly, DOMMatrix)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m24) — The m24 attribute, on getting, must return the m24 element value. For the DOMMatrix interface, setting the m24 attribute must set the m24 element to the new value and, if the new value is not 0 or -0, set is 2D to false.

- [x] [m31 (DOMMatrixReadOnly, DOMMatrix)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m31) — The m31 attribute, on getting, must return the m31 element value. For the DOMMatrix interface, setting the m31 attribute must set the m31 element to the new value and, if the new value is not 0 or -0, set is 2D to false.

- [x] [m32 (DOMMatrixReadOnly, DOMMatrix)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m32) — The m32 attribute, on getting, must return the m32 element value. For the DOMMatrix interface, setting the m32 attribute must set the m32 element to the new value and, if the new value is not 0 or -0, set is 2D to false.

- [x] [m33 (DOMMatrixReadOnly, DOMMatrix)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m33) — The m33 attribute, on getting, must return the m33 element value. For the DOMMatrix interface, setting the m33 attribute must set the m33 element to the new value and, if the new value is not 1, set is 2D to false.

- [x] [m34 (DOMMatrixReadOnly, DOMMatrix)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m34) — The m34 attribute, on getting, must return the m34 element value. For the DOMMatrix interface, setting the m34 attribute must set the m34 element to the new value and, if the new value is not 0 or -0, set is 2D to false.

- [x] [m41 (DOMMatrixReadOnly, DOMMatrix)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m41) — The m41 and e attributes, on getting, must return the m41 element value. For the DOMMatrix interface, setting the m41 or the e attribute must set the m41 element to the new value.

- [x] [m42 (DOMMatrixReadOnly, DOMMatrix)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m42) — The m42 and f attributes, on getting, must return the m42 element value. For the DOMMatrix interface, setting the m42 or the f attribute must set the m42 element to the new value.

- [x] [m43 (DOMMatrixReadOnly, DOMMatrix)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m43) — The m43 attribute, on getting, must return the m43 element value. For the DOMMatrix interface, setting the m43 attribute must set the m43 element to the new value and, if the new value is not 0 or -0, set is 2D to false.

- [x] [m44 (DOMMatrixReadOnly, DOMMatrix)](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m44) — The m44 attribute, on getting, must return the m44 element value. For the DOMMatrix interface, setting the m44 attribute must set the m44 element to the new value and, if the new value is not 1, set is 2D to false.

- [x] [DOMMatrixReadOnly.isIdentity](https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-isidentity) — The isIdentity attribute must return true if m12 element, m13 element, m14 element, m21 element, m23 element, m24 element, m31 element, m32 element, m34 element, m41 element, m42 element, m43 element are 0 or -0 and m11 element, m22 element, m33 element, m44 element are 1. Otherwise it must return false.

- [x] [is 2D (matrix)](https://drafts.csswg.org/geometry-1/#matrix-is-2d) — Every DOMMatrixReadOnly object must be flagged with a boolean is 2D. This flag indicates that:
  - 1. The current matrix was initialized as a 2D matrix. See individual creators for more details.
  - 2. Only 2D transformation operations were applied. Each mutable or immutable transformation method defines if is 2D must be set to false.

### [6.5. Immutable transformation methods](https://drafts.csswg.org/geometry-1/#immutable-transformation-methods)

Status: reviewed (2026-11-25)
> Code: DOMMatrixReadOnly.cpp, each on a clone (CloneAsDOMMatrix). Fixed in this review: setMatrixValue() was on DOMMatrixReadOnly, so a read-only matrix could be changed; it's now DOMMatrix's, returning the matrix. The stringifier's link was stale.
<!-- definitions: dom-dommatrixreadonly-translate dom-dommatrixreadonly-scale dom-dommatrixreadonly-scalenonuniform dom-dommatrixreadonly-scale3d dom-dommatrixreadonly-rotate dom-dommatrixreadonly-rotatefromvector dom-dommatrixreadonly-rotateaxisangle dom-dommatrixreadonly-skewx dom-dommatrixreadonly-skewy dom-dommatrixreadonly-multiply dom-dommatrixreadonly-flipx dom-dommatrixreadonly-flipy dom-dommatrixreadonly-inverse dom-dommatrixreadonly-transformpoint dom-dommatrixreadonly-tofloat32array dom-dommatrixreadonly-tofloat64array dommatrixreadonly-stringification-behavior -->

### [6.6. Mutable transformation methods](https://drafts.csswg.org/geometry-1/#mutable-transformation-methods)

Status: reviewed (2026-11-25)
> Code: DOMMatrix.cpp. scaleSelf() and scale3dSelf() translate with translateSelf(), so a z origin makes the matrix 3D as the spec's steps do.
<!-- definitions: dom-dommatrix-multiplyself dom-dommatrix-premultiplyself dom-dommatrix-translateself dom-dommatrix-scaleself dom-dommatrix-scale3dself dom-dommatrix-rotateself dom-dommatrix-rotatefromvectorself dom-dommatrix-rotateaxisangleself dom-dommatrix-skewxself dom-dommatrix-skewyself dom-dommatrix-invertself dom-dommatrix-setmatrixvalue -->

## [7. Structured serialization](https://drafts.csswg.org/geometry-1/#structured-serialization)

Status: n/a (no structured serialization; see DIFFERENCES.md)

- [x] [The serialization steps for DOMPointReadOnly and DOMPoint, given value and seri…](https://drafts.csswg.org/geometry-1/#structured-serialization) — The serialization steps for DOMPointReadOnly and DOMPoint, given value and serialized, are:
  - 1. Set serialized.[[X]] to value’s x coordinate.
  - 2. Set serialized.[[Y]] to value’s y coordinate.
  - 3. Set serialized.[[Z]] to value’s z coordinate.
  - 4. Set serialized.[[W]] to value’s w perspective.

- [x] [Their deserialization steps, given serialized and value,](https://drafts.csswg.org/geometry-1/#structured-serialization) — Their deserialization steps, given serialized and value, are:
  - 1. Set value’s x coordinate to serialized.[[X]].
  - 2. Set value’s y coordinate to serialized.[[Y]].
  - 3. Set value’s z coordinate to serialized.[[Z]].
  - 4. Set value’s w perspective to serialized.[[W]].

- [x] [The serialization steps for DOMRectReadOnly and DOMRect, given value and serial…](https://drafts.csswg.org/geometry-1/#structured-serialization) — The serialization steps for DOMRectReadOnly and DOMRect, given value and serialized, are:
  - 1. Set serialized.[[X]] to value’s x coordinate.
  - 2. Set serialized.[[Y]] to value’s y coordinate.
  - 3. Set serialized.[[Width]] to value’s width dimension.
  - 4. Set serialized.[[Height]] to value’s height dimension.

- [x] [Their deserialization steps, given serialized and value,](https://drafts.csswg.org/geometry-1/#structured-serialization) — Their deserialization steps, given serialized and value, are:
  - 1. Set value’s x coordinate to serialized.[[X]].
  - 2. Set value’s y coordinate to serialized.[[Y]].
  - 3. Set value’s width dimension to serialized.[[Width]].
  - 4. Set value’s height dimension to serialized.[[Height]].

- [x] [The serialization steps for DOMQuad, given value and serialized,](https://drafts.csswg.org/geometry-1/#structured-serialization) — The serialization steps for DOMQuad, given value and serialized, are:
  - 1. Set serialized.[[P1]] to the sub-serialization of value’s point 1.
  - 2. Set serialized.[[P2]] to the sub-serialization of value’s point 2.
  - 3. Set serialized.[[P3]] to the sub-serialization of value’s point 3.
  - 4. Set serialized.[[P4]] to the sub-serialization of value’s point 4.

- [x] [Their deserialization steps, given serialized and value,](https://drafts.csswg.org/geometry-1/#structured-serialization) — Their deserialization steps, given serialized and value, are:
  - 1. Set value’s point 1 to the sub-deserialization of serialized.[[P1]].
  - 2. Set value’s point 2 to the sub-deserialization of serialized.[[P2]].
  - 3. Set value’s point 3 to the sub-deserialization of serialized.[[P3]].
  - 4. Set value’s point 4 to the sub-deserialization of serialized.[[P4]].

- [x] [The serialization steps for DOMMatrixReadOnly and DOMMatrix, given value and se…](https://drafts.csswg.org/geometry-1/#structured-serialization) — The serialization steps for DOMMatrixReadOnly and DOMMatrix, given value and serialized, are:
  - 1. If value’s is 2D is true:
    - 1a. Set serialized.[[M11]] to value’s m11 element.
    - 1b. Set serialized.[[M12]] to value’s m12 element.
    - 1c. Set serialized.[[M21]] to value’s m21 element.
    - 1d. Set serialized.[[M22]] to value’s m22 element.
    - 1e. Set serialized.[[M41]] to value’s m41 element.
    - 1f. Set serialized.[[M42]] to value’s m42 element.
    - 1g. Set serialized.[[Is2D]] to true.
  - 2. Otherwise: Their deserialization steps, given serialized and value, are:
    - 2a. Set serialized.[[M11]] to value’s m11 element.
    - 2b. Set serialized.[[M12]] to value’s m12 element.
    - 2c. Set serialized.[[M13]] to value’s m13 element.
    - 2d. Set serialized.[[M14]] to value’s m14 element.
    - 2e. Set serialized.[[M21]] to value’s m21 element.
    - 2f. Set serialized.[[M22]] to value’s m22 element.
    - 2g. Set serialized.[[M23]] to value’s m23 element.
    - 2h. Set serialized.[[M24]] to value’s m24 element.
    - 2i. Set serialized.[[M31]] to value’s m31 element.
    - 2j. Set serialized.[[M32]] to value’s m32 element.
    - 2k. Set serialized.[[M33]] to value’s m33 element.
    - 2l. Set serialized.[[M34]] to value’s m34 element.
    - 2m. Set serialized.[[M41]] to value’s m41 element.
    - 2n. Set serialized.[[M42]] to value’s m42 element.
    - 2o. Set serialized.[[M43]] to value’s m43 element.
    - 2p. Set serialized.[[M44]] to value’s m44 element.
    - 2q. Set serialized.[[Is2D]] to false.
    - 2a. If serialized.[[Is2D]] is true:
      - 2a.i. Set value’s m11 element to serialized.[[M11]].
      - 2a.ii. Set value’s m12 element to serialized.[[M12]].
      - 2a.iii. Set value’s m13 element to 0.
      - 2a.iv. Set value’s m14 element to 0.
      - 2a.v. Set value’s m21 element to serialized.[[M21]].
      - 2a.vi. Set value’s m22 element to serialized.[[M22]].
      - 2a.vii. Set value’s m23 element to 0.
      - 2a.viii. Set value’s m24 element to 0.
      - 2a.ix. Set value’s m31 element to 0.
      - 2a.x. Set value’s m32 element to 0.
      - 2a.xi. Set value’s m33 element to 1.
      - 2a.xii. Set value’s m34 element to 0.
      - 2a.xiii. Set value’s m41 element to serialized.[[M41]].
      - 2a.xiv. Set value’s m42 element to serialized.[[M42]].
      - 2a.xv. Set value’s m43 element to 0.
      - 2a.xvi. Set value’s m44 element to 1.
      - 2a.xvii. Set value’s is 2D to true.
    - 2b. Otherwise:
      - 2b.i. Set value’s m11 element to serialized.[[M11]].
      - 2b.ii. Set value’s m12 element to serialized.[[M12]].
      - 2b.iii. Set value’s m13 element to serialized.[[M13]].
      - 2b.iv. Set value’s m14 element to serialized.[[M14]].
      - 2b.v. Set value’s m21 element to serialized.[[M21]].
      - 2b.vi. Set value’s m22 element to serialized.[[M22]].
      - 2b.vii. Set value’s m23 element to serialized.[[M23]].
      - 2b.viii. Set value’s m24 element to serialized.[[M24]].
      - 2b.ix. Set value’s m31 element to serialized.[[M31]].
      - 2b.x. Set value’s m32 element to serialized.[[M32]].
      - 2b.xi. Set value’s m33 element to serialized.[[M33]].
      - 2b.xii. Set value’s m34 element to serialized.[[M34]].
      - 2b.xiii. Set value’s m41 element to serialized.[[M41]].
      - 2b.xiv. Set value’s m42 element to serialized.[[M42]].
      - 2b.xv. Set value’s m43 element to serialized.[[M43]].
      - 2b.xvi. Set value’s m44 element to serialized.[[M44]].
      - 2b.xvii. Set value’s is 2D to false.

## [8. Security Considerations](https://drafts.csswg.org/geometry-1/#security)

Status: n/a (no requirements)

## [9. Privacy Considerations](https://drafts.csswg.org/geometry-1/#priv-sec)

Status: n/a (no requirements)

## [10. Historical](https://drafts.csswg.org/geometry-1/#historical)

Status: n/a (informative)

### [10.1. CSSOM View](https://drafts.csswg.org/geometry-1/#historical-cssom-view)

Status: n/a (informative)

### [10.2. SVG](https://drafts.csswg.org/geometry-1/#historical-svg)

Status: n/a (informative)

### [10.3. Non-standard](https://drafts.csswg.org/geometry-1/#historical-non-standard)

Status: n/a (WebKitCSSMatrix is a legacy window alias, and there are no global objects; WebKitPoint and MSCSSMatrix are unsupported, as required)

## [Document conventions](https://drafts.csswg.org/geometry-1/#conventions)

Status: reviewed (2026-11-25)
> Code: NaNSafeMinimum and NaNSafeMaximum in Css/Geometry/DOMRectReadOnly.hpp.
<!-- definitions: nan-safe-minimum nan-safe-maximum -->

## [Changes since last publication](https://drafts.csswg.org/geometry-1/#changes)

Status: n/a (back matter)

## [Acknowledgments](https://drafts.csswg.org/geometry-1/#acknowledgments)

Status: n/a (back matter)

## [Conformance](https://drafts.csswg.org/geometry-1/#w3c-conformance)

Status: n/a (back matter)

### [Document conventions](https://drafts.csswg.org/geometry-1/#w3c-conventions)

Status: n/a (back matter)

### [Conformance classes](https://drafts.csswg.org/geometry-1/#w3c-conformance-classes)

Status: n/a (back matter)

### [Partial implementations](https://drafts.csswg.org/geometry-1/#w3c-partial)

Status: n/a (back matter)

#### [Implementations of Unstable and Proprietary Features](https://drafts.csswg.org/geometry-1/#w3c-conform-future-proofing)

Status: n/a (back matter)

### [Non-experimental implementations](https://drafts.csswg.org/geometry-1/#w3c-testing)

Status: n/a (back matter)

### [CR exit criteria](https://drafts.csswg.org/geometry-1/#w3c-cr-exit-criteria)

Status: n/a (back matter)

## [Index](https://drafts.csswg.org/geometry-1/#index)

Status: n/a (back matter)

### [Terms defined by this specification](https://drafts.csswg.org/geometry-1/#index-defined-here)

Status: n/a (back matter)

### [Terms defined by reference](https://drafts.csswg.org/geometry-1/#index-defined-elsewhere)

Status: n/a (back matter)

## [References](https://drafts.csswg.org/geometry-1/#references)

Status: n/a (back matter)

### [Normative References](https://drafts.csswg.org/geometry-1/#normative)

Status: n/a (back matter)

### [Non-Normative References](https://drafts.csswg.org/geometry-1/#informative)

Status: n/a (back matter)

## [IDL Index](https://drafts.csswg.org/geometry-1/#idl-index)

Status: n/a (back matter)
