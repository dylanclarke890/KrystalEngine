#include "Krystal.Booey/SVG/SVGTransformList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/SVG/PropertyTestUtils.hpp"
#include "Krystal.Booey/SVG/SVGTransformable.hpp"

namespace krys::boo::svg::tests
{
  TEST_CASE("SVGTransformList: parsing and serialising", "[SVG][SVGTransformList]")
  {
    auto list = SVGTransformList::Create();
    list->Parse(u8" translate(10) scale(2, 3),rotate(45 1 2) skewX(30) skewY(-15) matrix(1 2 3 4 5 6) ");
    REQUIRE(list->NumberOfItems() == 6);
    CHECK(list->Items()[0]->Type() == SVGTransformValue::SVG_TRANSFORM_TRANSLATE);
    CHECK(list->Items()[2]->Angle() == 45);
    CHECK(list->Items()[2]->Value().RotationCenter() == FloatPoint(1, 2));
    CHECK(list->ValueAsString()
          == u8"translate(10 0) scale(2 3) rotate(45 1 2) skewX(30) skewY(-15) matrix(1 2 3 4 5 6)");

    list->Parse(u8"scale(2) rotate(90)");
    CHECK(list->ValueAsString() == u8"scale(2 2) rotate(90)");

    // A bad list leaves no transforms.
    for (auto bad : {u8"translate(10,)", u8"scale(1) ,", u8"rotate(1 2)", u8"skewX()", u8"translate 10",
                     u8"scale(1 2 3)", u8"spin(1)", u8"matrix(1 2 3 4 5)"})
    {
      list->Parse(u8"scale(2)");
      list->Parse(bad);
      INFO(std::string(bad, bad + std::char_traits<char8>::length(bad)));
      CHECK(list->IsEmpty());
    }

    // The transform types, read from the front of a string.
    CHECK(SVGTransformable::ParseTransformTypeFromString(u8"skewY(1)")
          == SVGTransformValue::SVG_TRANSFORM_SKEWY);
    CHECK(SVGTransformable::ParseTransformTypeFromString(u8"translate")
          == SVGTransformValue::SVG_TRANSFORM_TRANSLATE);
    CHECK_FALSE(SVGTransformable::ParseTransformTypeFromString(u8"skew(1)"));
    CHECK_FALSE(SVGTransformable::ParseTransformTypeFromString(u8""));
  }

  TEST_CASE("SVGTransformList: reparsing keeps the transforms of the same type", "[SVG][SVGTransformList]")
  {
    auto list = SVGTransformList::Create();
    list->Parse(u8"translate(1 2) scale(3)");
    Ref first = list->Items()[0];
    Ref second = list->Items()[1];

    list->Parse(u8"translate(4 5) rotate(6) skewX(7)");
    REQUIRE(list->NumberOfItems() == 3);
    CHECK(list->Items()[0].get() == first.get());
    CHECK(list->Items()[1].get() != second.get());
    CHECK_FALSE(second->IsAttached());
    CHECK(first->Value().ValueAsString() == u8"translate(4 5)");

    // A shorter list drops the rest.
    list->Parse(u8"translate(8 9)");
    REQUIRE(list->NumberOfItems() == 1);
    CHECK(list->Items()[0].get() == first.get());
    CHECK(list->ValueAsString() == u8"translate(8 9)");
  }

  TEST_CASE("SVGTransformList: consolidating and concatenating", "[SVG][SVGTransformList]")
  {
    RecordingPropertyOwner owner;
    auto list = SVGTransformList::Create(&owner, SVGPropertyAccess::ReadWrite);

    auto empty = list->Consolidate();
    REQUIRE_FALSE(empty.HasException());
    CHECK_FALSE(empty.Value());

    list->Parse(u8"translate(10 0) scale(2)");
    CHECK(list->Concatenate() == AffineTransform(2, 0, 0, 2, 10, 0));

    auto consolidated = list->Consolidate();
    REQUIRE_FALSE(consolidated.HasException());
    REQUIRE(consolidated.Value());
    CHECK(list->NumberOfItems() == 1);
    CHECK(list->ValueAsString() == u8"matrix(2 0 0 2 10 0)");
    CHECK(owner.Committed.size() == 1);

    // A single transform is the consolidated one.
    auto single = list->Consolidate();
    REQUIRE_FALSE(single.HasException());
    CHECK(single.Value().get() == list->Items()[0].get());

    auto fromMatrix = list->CreateSVGTransformFromMatrix(css::DOMMatrix2DInit {.A = 2, .D = 3, .F = 4});
    REQUIRE_FALSE(fromMatrix.HasException());
    CHECK(fromMatrix.Value()->Value().ValueAsString() == u8"matrix(2 0 0 3 0 4)");
    CHECK_FALSE(fromMatrix.Value()->IsAttached());

    auto readOnly = SVGTransformList::Create(*list, SVGPropertyAccess::ReadOnly);
    CHECK(readOnly->Consolidate().GetException().Code() == dom::ExceptionCode::NoModificationAllowedError);
  }

  TEST_CASE("SVGTransform: setters, matrices and owners", "[SVG][SVGTransform]")
  {
    auto transform = SVGTransform::Create();
    CHECK(transform->Type() == SVGTransformValue::SVG_TRANSFORM_MATRIX);
    CHECK(transform->Value().ValueAsString() == u8"matrix(1 0 0 1 0 0)");

    REQUIRE_FALSE(transform->SetRotate(90, 5, 5).HasException());
    CHECK(transform->Type() == SVGTransformValue::SVG_TRANSFORM_ROTATE);
    CHECK(transform->Angle() == 90);
    CHECK(transform->Value().ValueAsString() == u8"rotate(90 5 5)");

    // Changing the matrix directly makes the transform a matrix.
    REQUIRE_FALSE(transform->Matrix().SetE(1).HasException());
    CHECK(transform->Type() == SVGTransformValue::SVG_TRANSFORM_MATRIX);
    CHECK(transform->Angle() == 0);

    REQUIRE_FALSE(transform->SetTranslate(1, 2).HasException());
    CHECK(transform->Value().Translate() == FloatPoint(1, 2));
    REQUIRE_FALSE(transform->SetScale(3, 4).HasException());
    CHECK(transform->Value().Scale() == FloatSize(3, 4));
    REQUIRE_FALSE(transform->SetSkewY(10).HasException());
    CHECK(transform->Value().ValueAsString() == u8"skewY(10)");
    REQUIRE_FALSE(transform->SetMatrix(css::DOMMatrix2DInit {.B = 1, .E = 2}).HasException());
    CHECK(transform->Value().ValueAsString() == u8"matrix(1 1 0 1 2 0)");

    // A change to a transform's matrix is committed through the transform and its list.
    RecordingPropertyOwner owner;
    auto list = SVGTransformList::Create(&owner, SVGPropertyAccess::ReadWrite);
    auto appended = list->AppendItem(SVGTransform::Create(SVGTransformValue::SVG_TRANSFORM_SKEWX));
    REQUIRE_FALSE(appended.HasException());
    REQUIRE(owner.Committed.size() == 1);
    REQUIRE_FALSE(list->Items()[0]->Matrix().SetA(2).HasException());
    REQUIRE(owner.Committed.size() == 2);
    CHECK(owner.Committed[1] == list.get());
    CHECK(list->Items()[0]->Type() == SVGTransformValue::SVG_TRANSFORM_MATRIX);

    // The matrices of a read-only list's transforms are read-only.
    auto readOnly = SVGTransformList::Create(*list, SVGPropertyAccess::ReadOnly);
    auto code = dom::ExceptionCode::NoModificationAllowedError;
    CHECK(readOnly->Items()[0]->Matrix().SetA(1).GetException().Code() == code);
    CHECK(readOnly->Items()[0]->SetSkewX(1).GetException().Code() == code);

    // Removed from the list, the transform and its matrix are writable again.
    auto removed = list->RemoveItem(0);
    REQUIRE_FALSE(removed.HasException());
    CHECK_FALSE(removed.Value()->Matrix().IsReadOnly());

    auto clone = transform->Clone();
    CHECK(clone->Value().ValueAsString() == transform->Value().ValueAsString());
    CHECK(&clone->Matrix() != &transform->Matrix());
  }

  TEST_CASE("SVGMatrix: operations", "[SVG][SVGMatrix]")
  {
    auto matrix = SVGMatrix::Create(AffineTransform(1, 0, 0, 1, 10, 20));
    CHECK(matrix->Translate(5, 5)->Value() == AffineTransform(1, 0, 0, 1, 15, 25));
    CHECK(matrix->Scale(2)->Value() == AffineTransform(2, 0, 0, 2, 10, 20));
    CHECK(matrix->ScaleNonUniform(2, 3)->Value() == AffineTransform(2, 0, 0, 3, 10, 20));
    CHECK(matrix->FlipX()->Value() == AffineTransform(-1, 0, 0, 1, 10, 20));
    CHECK(matrix->FlipY()->Value() == AffineTransform(1, 0, 0, -1, 10, 20));
    CHECK(matrix->Multiply(*SVGMatrix::Create(AffineTransform(2, 0, 0, 2, 0, 0)))->Value()
          == AffineTransform(2, 0, 0, 2, 10, 20));
    CHECK(matrix->Value() == AffineTransform(1, 0, 0, 1, 10, 20));

    auto inverse = matrix->Inverse();
    REQUIRE_FALSE(inverse.HasException());
    CHECK(inverse.Value()->Value() == AffineTransform(1, 0, 0, 1, -10, -20));
    CHECK(SVGMatrix::Create(AffineTransform(0, 0, 0, 0, 0, 0))->Inverse().GetException().Code()
          == dom::ExceptionCode::InvalidStateError);

    CHECK(matrix->RotateFromVector(0, 1).GetException().Code() == dom::ExceptionCode::TypeError);
    CHECK(matrix->RotateFromVector(1, 0).GetException().Code() == dom::ExceptionCode::TypeError);
    REQUIRE_FALSE(matrix->RotateFromVector(1, 1).HasException());

    REQUIRE_FALSE(matrix->SetD(4).HasException());
    CHECK(matrix->D() == 4);
    auto readOnly = SVGMatrix::Create(nullptr, SVGPropertyAccess::ReadOnly);
    CHECK(readOnly->SetA(2).GetException().Code() == dom::ExceptionCode::NoModificationAllowedError);
  }
}
