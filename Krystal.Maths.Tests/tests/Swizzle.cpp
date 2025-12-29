#include "Krystal.Maths/Swizzle.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Maths;

  static Maths::Vec1 v1 = Vec1 {1};
  static Maths::Vec2 v2 = Vec2 {1, 2};
  static Maths::Vec3 v3 = Vec3 {1, 2, 3};
  static Maths::Vec4 v4 = Vec4 {1, 2, 3, 4};

#pragma region Two Components

  TEST_CASE("Swizzle(xx)", "[Swizzle]")
  {
    REQUIRE(xx(v1) == Vec2(1, 1));
    REQUIRE(xx(v2) == Vec2(1, 1));
    REQUIRE(xx(v3) == Vec2(1, 1));
    REQUIRE(xx(v4) == Vec2(1, 1));
  }

  TEST_CASE("Swizzle(xy)", "[Swizzle]")
  {
    REQUIRE(xy(v2) == Vec2(1, 2));
    REQUIRE(xy(v3) == Vec2(1, 2));
    REQUIRE(xy(v4) == Vec2(1, 2));
  }

  TEST_CASE("Swizzle(xz)", "[Swizzle]")
  {
    REQUIRE(xz(v3) == Vec2(1, 3));
    REQUIRE(xz(v4) == Vec2(1, 3));
  }

  TEST_CASE("Swizzle(xw)", "[Swizzle]")
  {
    REQUIRE(xw(v4) == Vec2(1, 4));
  }

  TEST_CASE("Swizzle(yx)", "[Swizzle]")
  {
    REQUIRE(yx(v2) == Vec2(2, 1));
    REQUIRE(yx(v3) == Vec2(2, 1));
    REQUIRE(yx(v4) == Vec2(2, 1));
  }

  TEST_CASE("Swizzle(yy)", "[Swizzle]")
  {
    REQUIRE(yy(v2) == Vec2(2, 2));
    REQUIRE(yy(v3) == Vec2(2, 2));
    REQUIRE(yy(v4) == Vec2(2, 2));
  }

  TEST_CASE("Swizzle(yz)", "[Swizzle]")
  {
    REQUIRE(yz(v3) == Vec2(2, 3));
    REQUIRE(yz(v4) == Vec2(2, 3));
  }

  TEST_CASE("Swizzle(yw)", "[Swizzle]")
  {
    REQUIRE(yw(v4) == Vec2(2, 4));
  }

  TEST_CASE("Swizzle(zx)", "[Swizzle]")
  {
    REQUIRE(zx(v3) == Vec2(3, 1));
    REQUIRE(zx(v4) == Vec2(3, 1));
  }

  TEST_CASE("Swizzle(zy)", "[Swizzle]")
  {
    REQUIRE(zy(v3) == Vec2(3, 2));
    REQUIRE(zy(v4) == Vec2(3, 2));
  }

  TEST_CASE("Swizzle(zz)", "[Swizzle]")
  {
    REQUIRE(zz(v3) == Vec2(3, 3));
    REQUIRE(zz(v4) == Vec2(3, 3));
  }

  TEST_CASE("Swizzle(zw)", "[Swizzle]")
  {
    REQUIRE(zw(v4) == Vec2(3, 4));
  }

  TEST_CASE("Swizzle(wx)", "[Swizzle]")
  {
    REQUIRE(wx(v4) == Vec2(4, 1));
  }

  TEST_CASE("Swizzle(wy)", "[Swizzle]")
  {
    REQUIRE(wy(v4) == Vec2(4, 2));
  }

  TEST_CASE("Swizzle(wz)", "[Swizzle]")
  {
    REQUIRE(wz(v4) == Vec2(4, 3));
  }

  TEST_CASE("Swizzle(ww)", "[Swizzle]")
  {
    REQUIRE(ww(v4) == Vec2(4, 4));
  }

#pragma endregion Two Components

#pragma region Three Components

  TEST_CASE("Swizzle(xxx)", "[Swizzle]")
  {
    REQUIRE(xxx(v1) == Vec3(1, 1, 1));
    REQUIRE(xxx(v2) == Vec3(1, 1, 1));
    REQUIRE(xxx(v3) == Vec3(1, 1, 1));
    REQUIRE(xxx(v4) == Vec3(1, 1, 1));
  }

  TEST_CASE("Swizzle(xxy)", "[Swizzle]")
  {
    REQUIRE(xxy(v2) == Vec3(1, 1, 2));
    REQUIRE(xxy(v3) == Vec3(1, 1, 2));
    REQUIRE(xxy(v4) == Vec3(1, 1, 2));
  }

  TEST_CASE("Swizzle(xxz)", "[Swizzle]")
  {
    REQUIRE(xxz(v3) == Vec3(1, 1, 3));
    REQUIRE(xxz(v4) == Vec3(1, 1, 3));
  }

  TEST_CASE("Swizzle(xxw)", "[Swizzle]")
  {
    REQUIRE(xxw(v4) == Vec3(1, 1, 4));
  }

  TEST_CASE("Swizzle(xyx)", "[Swizzle]")
  {
    REQUIRE(xyx(v2) == Vec3(1, 2, 1));
    REQUIRE(xyx(v3) == Vec3(1, 2, 1));
    REQUIRE(xyx(v4) == Vec3(1, 2, 1));
  }

  TEST_CASE("Swizzle(xyy)", "[Swizzle]")
  {
    REQUIRE(xyy(v2) == Vec3(1, 2, 2));
    REQUIRE(xyy(v3) == Vec3(1, 2, 2));
    REQUIRE(xyy(v4) == Vec3(1, 2, 2));
  }

  TEST_CASE("Swizzle(xyz)", "[Swizzle]")
  {
    REQUIRE(xyz(v3) == Vec3(1, 2, 3));
    REQUIRE(xyz(v4) == Vec3(1, 2, 3));
  }

  TEST_CASE("Swizzle(xyw)", "[Swizzle]")
  {
    REQUIRE(xyw(v4) == Vec3(1, 2, 4));
  }

  TEST_CASE("Swizzle(xzx)", "[Swizzle]")
  {
    REQUIRE(xzx(v3) == Vec3(1, 3, 1));
    REQUIRE(xzx(v4) == Vec3(1, 3, 1));
  }

  TEST_CASE("Swizzle(xzy)", "[Swizzle]")
  {
    REQUIRE(xzy(v3) == Vec3(1, 3, 2));
    REQUIRE(xzy(v4) == Vec3(1, 3, 2));
  }

  TEST_CASE("Swizzle(xzz)", "[Swizzle]")
  {
    REQUIRE(xzz(v3) == Vec3(1, 3, 3));
    REQUIRE(xzz(v4) == Vec3(1, 3, 3));
  }

  TEST_CASE("Swizzle(xzw)", "[Swizzle]")
  {
    REQUIRE(xzw(v4) == Vec3(1, 3, 4));
  }

  TEST_CASE("Swizzle(xwx)", "[Swizzle]")
  {
    REQUIRE(xwx(v4) == Vec3(1, 4, 1));
  }

  TEST_CASE("Swizzle(xwy)", "[Swizzle]")
  {
    REQUIRE(xwy(v4) == Vec3(1, 4, 2));
  }

  TEST_CASE("Swizzle(xwz)", "[Swizzle]")
  {
    REQUIRE(xwz(v4) == Vec3(1, 4, 3));
  }

  TEST_CASE("Swizzle(xww)", "[Swizzle]")
  {
    REQUIRE(xww(v4) == Vec3(1, 4, 4));
  }

  TEST_CASE("Swizzle(yxx)", "[Swizzle]")
  {
    REQUIRE(yxx(v2) == Vec3(2, 1, 1));
    REQUIRE(yxx(v3) == Vec3(2, 1, 1));
    REQUIRE(yxx(v4) == Vec3(2, 1, 1));
  }

  TEST_CASE("Swizzle(yxy)", "[Swizzle]")
  {
    REQUIRE(yxy(v2) == Vec3(2, 1, 2));
    REQUIRE(yxy(v3) == Vec3(2, 1, 2));
    REQUIRE(yxy(v4) == Vec3(2, 1, 2));
  }

  TEST_CASE("Swizzle(yxz)", "[Swizzle]")
  {
    REQUIRE(yxz(v3) == Vec3(2, 1, 3));
    REQUIRE(yxz(v4) == Vec3(2, 1, 3));
  }

  TEST_CASE("Swizzle(yxw)", "[Swizzle]")
  {
    REQUIRE(yxw(v4) == Vec3(2, 1, 4));
  }

  TEST_CASE("Swizzle(yyx)", "[Swizzle]")
  {
    REQUIRE(yyx(v2) == Vec3(2, 2, 1));
    REQUIRE(yyx(v3) == Vec3(2, 2, 1));
    REQUIRE(yyx(v4) == Vec3(2, 2, 1));
  }

  TEST_CASE("Swizzle(yyy)", "[Swizzle]")
  {
    REQUIRE(yyy(v2) == Vec3(2, 2, 2));
    REQUIRE(yyy(v3) == Vec3(2, 2, 2));
    REQUIRE(yyy(v4) == Vec3(2, 2, 2));
  }

  TEST_CASE("Swizzle(yyz)", "[Swizzle]")
  {
    REQUIRE(yyz(v3) == Vec3(2, 2, 3));
    REQUIRE(yyz(v4) == Vec3(2, 2, 3));
  }

  TEST_CASE("Swizzle(yyw)", "[Swizzle]")
  {
    REQUIRE(yyw(v4) == Vec3(2, 2, 4));
  }

  TEST_CASE("Swizzle(yzx)", "[Swizzle]")
  {
    REQUIRE(yzx(v3) == Vec3(2, 3, 1));
    REQUIRE(yzx(v4) == Vec3(2, 3, 1));
  }

  TEST_CASE("Swizzle(yzy)", "[Swizzle]")
  {
    REQUIRE(yzy(v3) == Vec3(2, 3, 2));
    REQUIRE(yzy(v4) == Vec3(2, 3, 2));
  }

  TEST_CASE("Swizzle(yzz)", "[Swizzle]")
  {
    REQUIRE(yzz(v3) == Vec3(2, 3, 3));
    REQUIRE(yzz(v4) == Vec3(2, 3, 3));
  }

  TEST_CASE("Swizzle(yzw)", "[Swizzle]")
  {
    REQUIRE(yzw(v4) == Vec3(2, 3, 4));
  }

  TEST_CASE("Swizzle(ywx)", "[Swizzle]")
  {
    REQUIRE(ywx(v4) == Vec3(2, 4, 1));
  }

  TEST_CASE("Swizzle(ywy)", "[Swizzle]")
  {
    REQUIRE(ywy(v4) == Vec3(2, 4, 2));
  }

  TEST_CASE("Swizzle(ywz)", "[Swizzle]")
  {
    REQUIRE(ywz(v4) == Vec3(2, 4, 3));
  }

  TEST_CASE("Swizzle(yww)", "[Swizzle]")
  {
    REQUIRE(yww(v4) == Vec3(2, 4, 4));
  }

  TEST_CASE("Swizzle(zxx)", "[Swizzle]")
  {
    REQUIRE(zxx(v3) == Vec3(3, 1, 1));
    REQUIRE(zxx(v4) == Vec3(3, 1, 1));
  }

  TEST_CASE("Swizzle(zxy)", "[Swizzle]")
  {
    REQUIRE(zxy(v3) == Vec3(3, 1, 2));
    REQUIRE(zxy(v4) == Vec3(3, 1, 2));
  }

  TEST_CASE("Swizzle(zxz)", "[Swizzle]")
  {
    REQUIRE(zxz(v3) == Vec3(3, 1, 3));
    REQUIRE(zxz(v4) == Vec3(3, 1, 3));
  }

  TEST_CASE("Swizzle(zxw)", "[Swizzle]")
  {
    REQUIRE(zxw(v4) == Vec3(3, 1, 4));
  }

  TEST_CASE("Swizzle(zyx)", "[Swizzle]")
  {
    REQUIRE(zyx(v3) == Vec3(3, 2, 1));
    REQUIRE(zyx(v4) == Vec3(3, 2, 1));
  }

  TEST_CASE("Swizzle(zyy)", "[Swizzle]")
  {
    REQUIRE(zyy(v3) == Vec3(3, 2, 2));
    REQUIRE(zyy(v4) == Vec3(3, 2, 2));
  }

  TEST_CASE("Swizzle(zyz)", "[Swizzle]")
  {
    REQUIRE(zyz(v3) == Vec3(3, 2, 3));
    REQUIRE(zyz(v4) == Vec3(3, 2, 3));
  }

  TEST_CASE("Swizzle(zyw)", "[Swizzle]")
  {
    REQUIRE(zyw(v4) == Vec3(3, 2, 4));
  }

  TEST_CASE("Swizzle(zzx)", "[Swizzle]")
  {
    REQUIRE(zzx(v3) == Vec3(3, 3, 1));
    REQUIRE(zzx(v4) == Vec3(3, 3, 1));
  }

  TEST_CASE("Swizzle(zzy)", "[Swizzle]")
  {
    REQUIRE(zzy(v3) == Vec3(3, 3, 2));
    REQUIRE(zzy(v4) == Vec3(3, 3, 2));
  }

  TEST_CASE("Swizzle(zzz)", "[Swizzle]")
  {
    REQUIRE(zzz(v3) == Vec3(3, 3, 3));
    REQUIRE(zzz(v4) == Vec3(3, 3, 3));
  }

  TEST_CASE("Swizzle(zzw)", "[Swizzle]")
  {
    REQUIRE(zzw(v4) == Vec3(3, 3, 4));
  }

  TEST_CASE("Swizzle(zwx)", "[Swizzle]")
  {
    REQUIRE(zwx(v4) == Vec3(3, 4, 1));
  }

  TEST_CASE("Swizzle(zwy)", "[Swizzle]")
  {
    REQUIRE(zwy(v4) == Vec3(3, 4, 2));
  }

  TEST_CASE("Swizzle(zwz)", "[Swizzle]")
  {
    REQUIRE(zwz(v4) == Vec3(3, 4, 3));
  }

  TEST_CASE("Swizzle(zww)", "[Swizzle]")
  {
    REQUIRE(zww(v4) == Vec3(3, 4, 4));
  }

  TEST_CASE("Swizzle(wxx)", "[Swizzle]")
  {
    REQUIRE(wxx(v4) == Vec3(4, 1, 1));
  }

  TEST_CASE("Swizzle(wxy)", "[Swizzle]")
  {
    REQUIRE(wxy(v4) == Vec3(4, 1, 2));
  }

  TEST_CASE("Swizzle(wxz)", "[Swizzle]")
  {
    REQUIRE(wxz(v4) == Vec3(4, 1, 3));
  }

  TEST_CASE("Swizzle(wxw)", "[Swizzle]")
  {
    REQUIRE(wxw(v4) == Vec3(4, 1, 4));
  }

  TEST_CASE("Swizzle(wyx)", "[Swizzle]")
  {
    REQUIRE(wyx(v4) == Vec3(4, 2, 1));
  }

  TEST_CASE("Swizzle(wyy)", "[Swizzle]")
  {
    REQUIRE(wyy(v4) == Vec3(4, 2, 2));
  }

  TEST_CASE("Swizzle(wyz)", "[Swizzle]")
  {
    REQUIRE(wyz(v4) == Vec3(4, 2, 3));
  }

  TEST_CASE("Swizzle(wyw)", "[Swizzle]")
  {
    REQUIRE(wyw(v4) == Vec3(4, 2, 4));
  }

  TEST_CASE("Swizzle(wzx)", "[Swizzle]")
  {
    REQUIRE(wzx(v4) == Vec3(4, 3, 1));
  }

  TEST_CASE("Swizzle(wzy)", "[Swizzle]")
  {
    REQUIRE(wzy(v4) == Vec3(4, 3, 2));
  }

  TEST_CASE("Swizzle(wzz)", "[Swizzle]")
  {
    REQUIRE(wzz(v4) == Vec3(4, 3, 3));
  }

  TEST_CASE("Swizzle(wzw)", "[Swizzle]")
  {
    REQUIRE(wzw(v4) == Vec3(4, 3, 4));
  }

  TEST_CASE("Swizzle(wwx)", "[Swizzle]")
  {
    REQUIRE(wwx(v4) == Vec3(4, 4, 1));
  }

  TEST_CASE("Swizzle(wwy)", "[Swizzle]")
  {
    REQUIRE(wwy(v4) == Vec3(4, 4, 2));
  }

  TEST_CASE("Swizzle(wwz)", "[Swizzle]")
  {
    REQUIRE(wwz(v4) == Vec3(4, 4, 3));
  }

  TEST_CASE("Swizzle(www)", "[Swizzle]")
  {
    REQUIRE(www(v4) == Vec3(4, 4, 4));
  }

#pragma endregion Three Components

#pragma region Four Components

  TEST_CASE("Swizzle(xxxx)", "[Swizzle]")
  {
    REQUIRE(xxxx(v1) == Vec4(1, 1, 1, 1));
    REQUIRE(xxxx(v2) == Vec4(1, 1, 1, 1));
    REQUIRE(xxxx(v3) == Vec4(1, 1, 1, 1));
    REQUIRE(xxxx(v4) == Vec4(1, 1, 1, 1));
  }

  TEST_CASE("Swizzle(xxxy)", "[Swizzle]")
  {
    REQUIRE(xxxy(v2) == Vec4(1, 1, 1, 2));
    REQUIRE(xxxy(v3) == Vec4(1, 1, 1, 2));
    REQUIRE(xxxy(v4) == Vec4(1, 1, 1, 2));
  }

  TEST_CASE("Swizzle(xxxz)", "[Swizzle]")
  {
    REQUIRE(xxxz(v3) == Vec4(1, 1, 1, 3));
    REQUIRE(xxxz(v4) == Vec4(1, 1, 1, 3));
  }

  TEST_CASE("Swizzle(xxxw)", "[Swizzle]")
  {
    REQUIRE(xxxw(v4) == Vec4(1, 1, 1, 4));
  }

  TEST_CASE("Swizzle(xxyx)", "[Swizzle]")
  {
    REQUIRE(xxyx(v2) == Vec4(1, 1, 2, 1));
    REQUIRE(xxyx(v3) == Vec4(1, 1, 2, 1));
    REQUIRE(xxyx(v4) == Vec4(1, 1, 2, 1));
  }

  TEST_CASE("Swizzle(xxyy)", "[Swizzle]")
  {
    REQUIRE(xxyy(v2) == Vec4(1, 1, 2, 2));
    REQUIRE(xxyy(v3) == Vec4(1, 1, 2, 2));
    REQUIRE(xxyy(v4) == Vec4(1, 1, 2, 2));
  }

  TEST_CASE("Swizzle(xxyz)", "[Swizzle]")
  {
    REQUIRE(xxyz(v3) == Vec4(1, 1, 2, 3));
    REQUIRE(xxyz(v4) == Vec4(1, 1, 2, 3));
  }

  TEST_CASE("Swizzle(xxyw)", "[Swizzle]")
  {
    REQUIRE(xxyw(v4) == Vec4(1, 1, 2, 4));
  }

  TEST_CASE("Swizzle(xxzx)", "[Swizzle]")
  {
    REQUIRE(xxzx(v3) == Vec4(1, 1, 3, 1));
    REQUIRE(xxzx(v4) == Vec4(1, 1, 3, 1));
  }

  TEST_CASE("Swizzle(xxzy)", "[Swizzle]")
  {
    REQUIRE(xxzy(v3) == Vec4(1, 1, 3, 2));
    REQUIRE(xxzy(v4) == Vec4(1, 1, 3, 2));
  }

  TEST_CASE("Swizzle(xxzz)", "[Swizzle]")
  {
    REQUIRE(xxzz(v3) == Vec4(1, 1, 3, 3));
    REQUIRE(xxzz(v4) == Vec4(1, 1, 3, 3));
  }

  TEST_CASE("Swizzle(xxzw)", "[Swizzle]")
  {
    REQUIRE(xxzw(v4) == Vec4(1, 1, 3, 4));
  }

  TEST_CASE("Swizzle(xxwx)", "[Swizzle]")
  {
    REQUIRE(xxwx(v4) == Vec4(1, 1, 4, 1));
  }

  TEST_CASE("Swizzle(xxwy)", "[Swizzle]")
  {
    REQUIRE(xxwy(v4) == Vec4(1, 1, 4, 2));
  }

  TEST_CASE("Swizzle(xxwz)", "[Swizzle]")
  {
    REQUIRE(xxwz(v4) == Vec4(1, 1, 4, 3));
  }

  TEST_CASE("Swizzle(xxww)", "[Swizzle]")
  {
    REQUIRE(xxww(v4) == Vec4(1, 1, 4, 4));
  }

  TEST_CASE("Swizzle(xyxx)", "[Swizzle]")
  {
    REQUIRE(xyxx(v2) == Vec4(1, 2, 1, 1));
    REQUIRE(xyxx(v3) == Vec4(1, 2, 1, 1));
    REQUIRE(xyxx(v4) == Vec4(1, 2, 1, 1));
  }

  TEST_CASE("Swizzle(xyxy)", "[Swizzle]")
  {
    REQUIRE(xyxy(v2) == Vec4(1, 2, 1, 2));
    REQUIRE(xyxy(v3) == Vec4(1, 2, 1, 2));
    REQUIRE(xyxy(v4) == Vec4(1, 2, 1, 2));
  }

  TEST_CASE("Swizzle(xyxz)", "[Swizzle]")
  {
    REQUIRE(xyxz(v3) == Vec4(1, 2, 1, 3));
    REQUIRE(xyxz(v4) == Vec4(1, 2, 1, 3));
  }

  TEST_CASE("Swizzle(xyxw)", "[Swizzle]")
  {
    REQUIRE(xyxw(v4) == Vec4(1, 2, 1, 4));
  }

  TEST_CASE("Swizzle(xyyx)", "[Swizzle]")
  {
    REQUIRE(xyyx(v2) == Vec4(1, 2, 2, 1));
    REQUIRE(xyyx(v3) == Vec4(1, 2, 2, 1));
    REQUIRE(xyyx(v4) == Vec4(1, 2, 2, 1));
  }

  TEST_CASE("Swizzle(xyyy)", "[Swizzle]")
  {
    REQUIRE(xyyy(v2) == Vec4(1, 2, 2, 2));
    REQUIRE(xyyy(v3) == Vec4(1, 2, 2, 2));
    REQUIRE(xyyy(v4) == Vec4(1, 2, 2, 2));
  }

  TEST_CASE("Swizzle(xyyz)", "[Swizzle]")
  {
    REQUIRE(xyyz(v3) == Vec4(1, 2, 2, 3));
    REQUIRE(xyyz(v4) == Vec4(1, 2, 2, 3));
  }

  TEST_CASE("Swizzle(xyyw)", "[Swizzle]")
  {
    REQUIRE(xyyw(v4) == Vec4(1, 2, 2, 4));
  }

  TEST_CASE("Swizzle(xyzx)", "[Swizzle]")
  {
    REQUIRE(xyzx(v3) == Vec4(1, 2, 3, 1));
    REQUIRE(xyzx(v4) == Vec4(1, 2, 3, 1));
  }

  TEST_CASE("Swizzle(xyzy)", "[Swizzle]")
  {
    REQUIRE(xyzy(v3) == Vec4(1, 2, 3, 2));
    REQUIRE(xyzy(v4) == Vec4(1, 2, 3, 2));
  }

  TEST_CASE("Swizzle(xyzz)", "[Swizzle]")
  {
    REQUIRE(xyzz(v3) == Vec4(1, 2, 3, 3));
    REQUIRE(xyzz(v4) == Vec4(1, 2, 3, 3));
  }

  TEST_CASE("Swizzle(xyzw)", "[Swizzle]")
  {
    REQUIRE(xyzw(v4) == Vec4(1, 2, 3, 4));
  }

  TEST_CASE("Swizzle(xywx)", "[Swizzle]")
  {
    REQUIRE(xywx(v4) == Vec4(1, 2, 4, 1));
  }

  TEST_CASE("Swizzle(xywy)", "[Swizzle]")
  {
    REQUIRE(xywy(v4) == Vec4(1, 2, 4, 2));
  }

  TEST_CASE("Swizzle(xywz)", "[Swizzle]")
  {
    REQUIRE(xywz(v4) == Vec4(1, 2, 4, 3));
  }

  TEST_CASE("Swizzle(xyww)", "[Swizzle]")
  {
    REQUIRE(xyww(v4) == Vec4(1, 2, 4, 4));
  }

  TEST_CASE("Swizzle(xzxx)", "[Swizzle]")
  {
    REQUIRE(xzxx(v3) == Vec4(1, 3, 1, 1));
    REQUIRE(xzxx(v4) == Vec4(1, 3, 1, 1));
  }

  TEST_CASE("Swizzle(xzxy)", "[Swizzle]")
  {
    REQUIRE(xzxy(v3) == Vec4(1, 3, 1, 2));
    REQUIRE(xzxy(v4) == Vec4(1, 3, 1, 2));
  }

  TEST_CASE("Swizzle(xzxz)", "[Swizzle]")
  {
    REQUIRE(xzxz(v3) == Vec4(1, 3, 1, 3));
    REQUIRE(xzxz(v4) == Vec4(1, 3, 1, 3));
  }

  TEST_CASE("Swizzle(xzxw)", "[Swizzle]")
  {
    REQUIRE(xzxw(v4) == Vec4(1, 3, 1, 4));
  }

  TEST_CASE("Swizzle(xzyx)", "[Swizzle]")
  {
    REQUIRE(xzyx(v3) == Vec4(1, 3, 2, 1));
    REQUIRE(xzyx(v4) == Vec4(1, 3, 2, 1));
  }

  TEST_CASE("Swizzle(xzyy)", "[Swizzle]")
  {
    REQUIRE(xzyy(v3) == Vec4(1, 3, 2, 2));
    REQUIRE(xzyy(v4) == Vec4(1, 3, 2, 2));
  }

  TEST_CASE("Swizzle(xzyz)", "[Swizzle]")
  {
    REQUIRE(xzyz(v3) == Vec4(1, 3, 2, 3));
    REQUIRE(xzyz(v4) == Vec4(1, 3, 2, 3));
  }

  TEST_CASE("Swizzle(xzyw)", "[Swizzle]")
  {
    REQUIRE(xzyw(v4) == Vec4(1, 3, 2, 4));
  }

  TEST_CASE("Swizzle(xzzx)", "[Swizzle]")
  {
    REQUIRE(xzzx(v3) == Vec4(1, 3, 3, 1));
    REQUIRE(xzzx(v4) == Vec4(1, 3, 3, 1));
  }

  TEST_CASE("Swizzle(xzzy)", "[Swizzle]")
  {
    REQUIRE(xzzy(v3) == Vec4(1, 3, 3, 2));
    REQUIRE(xzzy(v4) == Vec4(1, 3, 3, 2));
  }

  TEST_CASE("Swizzle(xzzz)", "[Swizzle]")
  {
    REQUIRE(xzzz(v3) == Vec4(1, 3, 3, 3));
    REQUIRE(xzzz(v4) == Vec4(1, 3, 3, 3));
  }

  TEST_CASE("Swizzle(xzzw)", "[Swizzle]")
  {
    REQUIRE(xzzw(v4) == Vec4(1, 3, 3, 4));
  }

  TEST_CASE("Swizzle(xzwx)", "[Swizzle]")
  {
    REQUIRE(xzwx(v4) == Vec4(1, 3, 4, 1));
  }

  TEST_CASE("Swizzle(xzwy)", "[Swizzle]")
  {
    REQUIRE(xzwy(v4) == Vec4(1, 3, 4, 2));
  }

  TEST_CASE("Swizzle(xzwz)", "[Swizzle]")
  {
    REQUIRE(xzwz(v4) == Vec4(1, 3, 4, 3));
  }

  TEST_CASE("Swizzle(xzww)", "[Swizzle]")
  {
    REQUIRE(xzww(v4) == Vec4(1, 3, 4, 4));
  }

  TEST_CASE("Swizzle(xwxx)", "[Swizzle]")
  {
    REQUIRE(xwxx(v4) == Vec4(1, 4, 1, 1));
  }

  TEST_CASE("Swizzle(xwxy)", "[Swizzle]")
  {
    REQUIRE(xwxy(v4) == Vec4(1, 4, 1, 2));
  }

  TEST_CASE("Swizzle(xwxz)", "[Swizzle]")
  {
    REQUIRE(xwxz(v4) == Vec4(1, 4, 1, 3));
  }

  TEST_CASE("Swizzle(xwxw)", "[Swizzle]")
  {
    REQUIRE(xwxw(v4) == Vec4(1, 4, 1, 4));
  }

  TEST_CASE("Swizzle(xwyx)", "[Swizzle]")
  {
    REQUIRE(xwyx(v4) == Vec4(1, 4, 2, 1));
  }

  TEST_CASE("Swizzle(xwyy)", "[Swizzle]")
  {
    REQUIRE(xwyy(v4) == Vec4(1, 4, 2, 2));
  }

  TEST_CASE("Swizzle(xwyz)", "[Swizzle]")
  {
    REQUIRE(xwyz(v4) == Vec4(1, 4, 2, 3));
  }

  TEST_CASE("Swizzle(xwyw)", "[Swizzle]")
  {
    REQUIRE(xwyw(v4) == Vec4(1, 4, 2, 4));
  }

  TEST_CASE("Swizzle(xwzx)", "[Swizzle]")
  {
    REQUIRE(xwzx(v4) == Vec4(1, 4, 3, 1));
  }

  TEST_CASE("Swizzle(xwzy)", "[Swizzle]")
  {
    REQUIRE(xwzy(v4) == Vec4(1, 4, 3, 2));
  }

  TEST_CASE("Swizzle(xwzz)", "[Swizzle]")
  {
    REQUIRE(xwzz(v4) == Vec4(1, 4, 3, 3));
  }

  TEST_CASE("Swizzle(xwzw)", "[Swizzle]")
  {
    REQUIRE(xwzw(v4) == Vec4(1, 4, 3, 4));
  }

  TEST_CASE("Swizzle(xwwx)", "[Swizzle]")
  {
    REQUIRE(xwwx(v4) == Vec4(1, 4, 4, 1));
  }

  TEST_CASE("Swizzle(xwwy)", "[Swizzle]")
  {
    REQUIRE(xwwy(v4) == Vec4(1, 4, 4, 2));
  }

  TEST_CASE("Swizzle(xwwz)", "[Swizzle]")
  {
    REQUIRE(xwwz(v4) == Vec4(1, 4, 4, 3));
  }

  TEST_CASE("Swizzle(xwww)", "[Swizzle]")
  {
    REQUIRE(xwww(v4) == Vec4(1, 4, 4, 4));
  }

  TEST_CASE("Swizzle(yxxx)", "[Swizzle]")
  {
    REQUIRE(yxxx(v2) == Vec4(2, 1, 1, 1));
    REQUIRE(yxxx(v3) == Vec4(2, 1, 1, 1));
    REQUIRE(yxxx(v4) == Vec4(2, 1, 1, 1));
  }

  TEST_CASE("Swizzle(yxxy)", "[Swizzle]")
  {
    REQUIRE(yxxy(v2) == Vec4(2, 1, 1, 2));
    REQUIRE(yxxy(v3) == Vec4(2, 1, 1, 2));
    REQUIRE(yxxy(v4) == Vec4(2, 1, 1, 2));
  }

  TEST_CASE("Swizzle(yxxz)", "[Swizzle]")
  {
    REQUIRE(yxxz(v3) == Vec4(2, 1, 1, 3));
    REQUIRE(yxxz(v4) == Vec4(2, 1, 1, 3));
  }

  TEST_CASE("Swizzle(yxxw)", "[Swizzle]")
  {
    REQUIRE(yxxw(v4) == Vec4(2, 1, 1, 4));
  }

  TEST_CASE("Swizzle(yxyx)", "[Swizzle]")
  {
    REQUIRE(yxyx(v2) == Vec4(2, 1, 2, 1));
    REQUIRE(yxyx(v3) == Vec4(2, 1, 2, 1));
    REQUIRE(yxyx(v4) == Vec4(2, 1, 2, 1));
  }

  TEST_CASE("Swizzle(yxyy)", "[Swizzle]")
  {
    REQUIRE(yxyy(v2) == Vec4(2, 1, 2, 2));
    REQUIRE(yxyy(v3) == Vec4(2, 1, 2, 2));
    REQUIRE(yxyy(v4) == Vec4(2, 1, 2, 2));
  }

  TEST_CASE("Swizzle(yxyz)", "[Swizzle]")
  {
    REQUIRE(yxyz(v3) == Vec4(2, 1, 2, 3));
    REQUIRE(yxyz(v4) == Vec4(2, 1, 2, 3));
  }

  TEST_CASE("Swizzle(yxyw)", "[Swizzle]")
  {
    REQUIRE(yxyw(v4) == Vec4(2, 1, 2, 4));
  }

  TEST_CASE("Swizzle(yxzx)", "[Swizzle]")
  {
    REQUIRE(yxzx(v3) == Vec4(2, 1, 3, 1));
    REQUIRE(yxzx(v4) == Vec4(2, 1, 3, 1));
  }

  TEST_CASE("Swizzle(yxzy)", "[Swizzle]")
  {
    REQUIRE(yxzy(v3) == Vec4(2, 1, 3, 2));
    REQUIRE(yxzy(v4) == Vec4(2, 1, 3, 2));
  }

  TEST_CASE("Swizzle(yxzz)", "[Swizzle]")
  {
    REQUIRE(yxzz(v3) == Vec4(2, 1, 3, 3));
    REQUIRE(yxzz(v4) == Vec4(2, 1, 3, 3));
  }

  TEST_CASE("Swizzle(yxzw)", "[Swizzle]")
  {
    REQUIRE(yxzw(v4) == Vec4(2, 1, 3, 4));
  }

  TEST_CASE("Swizzle(yxwx)", "[Swizzle]")
  {
    REQUIRE(yxwx(v4) == Vec4(2, 1, 4, 1));
  }

  TEST_CASE("Swizzle(yxwy)", "[Swizzle]")
  {
    REQUIRE(yxwy(v4) == Vec4(2, 1, 4, 2));
  }

  TEST_CASE("Swizzle(yxwz)", "[Swizzle]")
  {
    REQUIRE(yxwz(v4) == Vec4(2, 1, 4, 3));
  }

  TEST_CASE("Swizzle(yxww)", "[Swizzle]")
  {
    REQUIRE(yxww(v4) == Vec4(2, 1, 4, 4));
  }

  TEST_CASE("Swizzle(yyxx)", "[Swizzle]")
  {
    REQUIRE(yyxx(v2) == Vec4(2, 2, 1, 1));
    REQUIRE(yyxx(v3) == Vec4(2, 2, 1, 1));
    REQUIRE(yyxx(v4) == Vec4(2, 2, 1, 1));
  }

  TEST_CASE("Swizzle(yyxy)", "[Swizzle]")
  {
    REQUIRE(yyxy(v2) == Vec4(2, 2, 1, 2));
    REQUIRE(yyxy(v3) == Vec4(2, 2, 1, 2));
    REQUIRE(yyxy(v4) == Vec4(2, 2, 1, 2));
  }

  TEST_CASE("Swizzle(yyxz)", "[Swizzle]")
  {
    REQUIRE(yyxz(v3) == Vec4(2, 2, 1, 3));
    REQUIRE(yyxz(v4) == Vec4(2, 2, 1, 3));
  }

  TEST_CASE("Swizzle(yyxw)", "[Swizzle]")
  {
    REQUIRE(yyxw(v4) == Vec4(2, 2, 1, 4));
  }

  TEST_CASE("Swizzle(yyyx)", "[Swizzle]")
  {
    REQUIRE(yyyx(v2) == Vec4(2, 2, 2, 1));
    REQUIRE(yyyx(v3) == Vec4(2, 2, 2, 1));
    REQUIRE(yyyx(v4) == Vec4(2, 2, 2, 1));
  }

  TEST_CASE("Swizzle(yyyy)", "[Swizzle]")
  {
    REQUIRE(yyyy(v2) == Vec4(2, 2, 2, 2));
    REQUIRE(yyyy(v3) == Vec4(2, 2, 2, 2));
    REQUIRE(yyyy(v4) == Vec4(2, 2, 2, 2));
  }

  TEST_CASE("Swizzle(yyyz)", "[Swizzle]")
  {
    REQUIRE(yyyz(v3) == Vec4(2, 2, 2, 3));
    REQUIRE(yyyz(v4) == Vec4(2, 2, 2, 3));
  }

  TEST_CASE("Swizzle(yyyw)", "[Swizzle]")
  {
    REQUIRE(yyyw(v4) == Vec4(2, 2, 2, 4));
  }

  TEST_CASE("Swizzle(yyzx)", "[Swizzle]")
  {
    REQUIRE(yyzx(v3) == Vec4(2, 2, 3, 1));
    REQUIRE(yyzx(v4) == Vec4(2, 2, 3, 1));
  }

  TEST_CASE("Swizzle(yyzy)", "[Swizzle]")
  {
    REQUIRE(yyzy(v3) == Vec4(2, 2, 3, 2));
    REQUIRE(yyzy(v4) == Vec4(2, 2, 3, 2));
  }

  TEST_CASE("Swizzle(yyzz)", "[Swizzle]")
  {
    REQUIRE(yyzz(v3) == Vec4(2, 2, 3, 3));
    REQUIRE(yyzz(v4) == Vec4(2, 2, 3, 3));
  }

  TEST_CASE("Swizzle(yyzw)", "[Swizzle]")
  {
    REQUIRE(yyzw(v4) == Vec4(2, 2, 3, 4));
  }

  TEST_CASE("Swizzle(yywx)", "[Swizzle]")
  {
    REQUIRE(yywx(v4) == Vec4(2, 2, 4, 1));
  }

  TEST_CASE("Swizzle(yywy)", "[Swizzle]")
  {
    REQUIRE(yywy(v4) == Vec4(2, 2, 4, 2));
  }

  TEST_CASE("Swizzle(yywz)", "[Swizzle]")
  {
    REQUIRE(yywz(v4) == Vec4(2, 2, 4, 3));
  }

  TEST_CASE("Swizzle(yyww)", "[Swizzle]")
  {
    REQUIRE(yyww(v4) == Vec4(2, 2, 4, 4));
  }

  TEST_CASE("Swizzle(yzxx)", "[Swizzle]")
  {
    REQUIRE(yzxx(v3) == Vec4(2, 3, 1, 1));
    REQUIRE(yzxx(v4) == Vec4(2, 3, 1, 1));
  }

  TEST_CASE("Swizzle(yzxy)", "[Swizzle]")
  {
    REQUIRE(yzxy(v3) == Vec4(2, 3, 1, 2));
    REQUIRE(yzxy(v4) == Vec4(2, 3, 1, 2));
  }

  TEST_CASE("Swizzle(yzxz)", "[Swizzle]")
  {
    REQUIRE(yzxz(v3) == Vec4(2, 3, 1, 3));
    REQUIRE(yzxz(v4) == Vec4(2, 3, 1, 3));
  }

  TEST_CASE("Swizzle(yzxw)", "[Swizzle]")
  {
    REQUIRE(yzxw(v4) == Vec4(2, 3, 1, 4));
  }

  TEST_CASE("Swizzle(yzyx)", "[Swizzle]")
  {
    REQUIRE(yzyx(v3) == Vec4(2, 3, 2, 1));
    REQUIRE(yzyx(v4) == Vec4(2, 3, 2, 1));
  }

  TEST_CASE("Swizzle(yzyy)", "[Swizzle]")
  {
    REQUIRE(yzyy(v3) == Vec4(2, 3, 2, 2));
    REQUIRE(yzyy(v4) == Vec4(2, 3, 2, 2));
  }

  TEST_CASE("Swizzle(yzyz)", "[Swizzle]")
  {
    REQUIRE(yzyz(v3) == Vec4(2, 3, 2, 3));
    REQUIRE(yzyz(v4) == Vec4(2, 3, 2, 3));
  }

  TEST_CASE("Swizzle(yzyw)", "[Swizzle]")
  {
    REQUIRE(yzyw(v4) == Vec4(2, 3, 2, 4));
  }

  TEST_CASE("Swizzle(yzzx)", "[Swizzle]")
  {
    REQUIRE(yzzx(v3) == Vec4(2, 3, 3, 1));
    REQUIRE(yzzx(v4) == Vec4(2, 3, 3, 1));
  }

  TEST_CASE("Swizzle(yzzy)", "[Swizzle]")
  {
    REQUIRE(yzzy(v3) == Vec4(2, 3, 3, 2));
    REQUIRE(yzzy(v4) == Vec4(2, 3, 3, 2));
  }

  TEST_CASE("Swizzle(yzzz)", "[Swizzle]")
  {
    REQUIRE(yzzz(v3) == Vec4(2, 3, 3, 3));
    REQUIRE(yzzz(v4) == Vec4(2, 3, 3, 3));
  }

  TEST_CASE("Swizzle(yzzw)", "[Swizzle]")
  {
    REQUIRE(yzzw(v4) == Vec4(2, 3, 3, 4));
  }

  TEST_CASE("Swizzle(yzwx)", "[Swizzle]")
  {
    REQUIRE(yzwx(v4) == Vec4(2, 3, 4, 1));
  }

  TEST_CASE("Swizzle(yzwy)", "[Swizzle]")
  {
    REQUIRE(yzwy(v4) == Vec4(2, 3, 4, 2));
  }

  TEST_CASE("Swizzle(yzwz)", "[Swizzle]")
  {
    REQUIRE(yzwz(v4) == Vec4(2, 3, 4, 3));
  }

  TEST_CASE("Swizzle(yzww)", "[Swizzle]")
  {
    REQUIRE(yzww(v4) == Vec4(2, 3, 4, 4));
  }

  TEST_CASE("Swizzle(ywxx)", "[Swizzle]")
  {
    REQUIRE(ywxx(v4) == Vec4(2, 4, 1, 1));
  }

  TEST_CASE("Swizzle(ywxy)", "[Swizzle]")
  {
    REQUIRE(ywxy(v4) == Vec4(2, 4, 1, 2));
  }

  TEST_CASE("Swizzle(ywxz)", "[Swizzle]")
  {
    REQUIRE(ywxz(v4) == Vec4(2, 4, 1, 3));
  }

  TEST_CASE("Swizzle(ywxw)", "[Swizzle]")
  {
    REQUIRE(ywxw(v4) == Vec4(2, 4, 1, 4));
  }

  TEST_CASE("Swizzle(ywyx)", "[Swizzle]")
  {
    REQUIRE(ywyx(v4) == Vec4(2, 4, 2, 1));
  }

  TEST_CASE("Swizzle(ywyy)", "[Swizzle]")
  {
    REQUIRE(ywyy(v4) == Vec4(2, 4, 2, 2));
  }

  TEST_CASE("Swizzle(ywyz)", "[Swizzle]")
  {
    REQUIRE(ywyz(v4) == Vec4(2, 4, 2, 3));
  }

  TEST_CASE("Swizzle(ywyw)", "[Swizzle]")
  {
    REQUIRE(ywyw(v4) == Vec4(2, 4, 2, 4));
  }

  TEST_CASE("Swizzle(ywzx)", "[Swizzle]")
  {
    REQUIRE(ywzx(v4) == Vec4(2, 4, 3, 1));
  }

  TEST_CASE("Swizzle(ywzy)", "[Swizzle]")
  {
    REQUIRE(ywzy(v4) == Vec4(2, 4, 3, 2));
  }

  TEST_CASE("Swizzle(ywzz)", "[Swizzle]")
  {
    REQUIRE(ywzz(v4) == Vec4(2, 4, 3, 3));
  }

  TEST_CASE("Swizzle(ywzw)", "[Swizzle]")
  {
    REQUIRE(ywzw(v4) == Vec4(2, 4, 3, 4));
  }

  TEST_CASE("Swizzle(ywwx)", "[Swizzle]")
  {
    REQUIRE(ywwx(v4) == Vec4(2, 4, 4, 1));
  }

  TEST_CASE("Swizzle(ywwy)", "[Swizzle]")
  {
    REQUIRE(ywwy(v4) == Vec4(2, 4, 4, 2));
  }

  TEST_CASE("Swizzle(ywwz)", "[Swizzle]")
  {
    REQUIRE(ywwz(v4) == Vec4(2, 4, 4, 3));
  }

  TEST_CASE("Swizzle(ywww)", "[Swizzle]")
  {
    REQUIRE(ywww(v4) == Vec4(2, 4, 4, 4));
  }

  TEST_CASE("Swizzle(zxxx)", "[Swizzle]")
  {
    REQUIRE(zxxx(v3) == Vec4(3, 1, 1, 1));
    REQUIRE(zxxx(v4) == Vec4(3, 1, 1, 1));
  }

  TEST_CASE("Swizzle(zxxy)", "[Swizzle]")
  {
    REQUIRE(zxxy(v3) == Vec4(3, 1, 1, 2));
    REQUIRE(zxxy(v4) == Vec4(3, 1, 1, 2));
  }

  TEST_CASE("Swizzle(zxxz)", "[Swizzle]")
  {
    REQUIRE(zxxz(v3) == Vec4(3, 1, 1, 3));
    REQUIRE(zxxz(v4) == Vec4(3, 1, 1, 3));
  }

  TEST_CASE("Swizzle(zxxw)", "[Swizzle]")
  {
    REQUIRE(zxxw(v4) == Vec4(3, 1, 1, 4));
  }

  TEST_CASE("Swizzle(zxyx)", "[Swizzle]")
  {
    REQUIRE(zxyx(v3) == Vec4(3, 1, 2, 1));
    REQUIRE(zxyx(v4) == Vec4(3, 1, 2, 1));
  }

  TEST_CASE("Swizzle(zxyy)", "[Swizzle]")
  {
    REQUIRE(zxyy(v3) == Vec4(3, 1, 2, 2));
    REQUIRE(zxyy(v4) == Vec4(3, 1, 2, 2));
  }

  TEST_CASE("Swizzle(zxyz)", "[Swizzle]")
  {
    REQUIRE(zxyz(v3) == Vec4(3, 1, 2, 3));
    REQUIRE(zxyz(v4) == Vec4(3, 1, 2, 3));
  }

  TEST_CASE("Swizzle(zxyw)", "[Swizzle]")
  {
    REQUIRE(zxyw(v4) == Vec4(3, 1, 2, 4));
  }

  TEST_CASE("Swizzle(zxzx)", "[Swizzle]")
  {
    REQUIRE(zxzx(v3) == Vec4(3, 1, 3, 1));
    REQUIRE(zxzx(v4) == Vec4(3, 1, 3, 1));
  }

  TEST_CASE("Swizzle(zxzy)", "[Swizzle]")
  {
    REQUIRE(zxzy(v3) == Vec4(3, 1, 3, 2));
    REQUIRE(zxzy(v4) == Vec4(3, 1, 3, 2));
  }

  TEST_CASE("Swizzle(zxzz)", "[Swizzle]")
  {
    REQUIRE(zxzz(v3) == Vec4(3, 1, 3, 3));
    REQUIRE(zxzz(v4) == Vec4(3, 1, 3, 3));
  }

  TEST_CASE("Swizzle(zxzw)", "[Swizzle]")
  {
    REQUIRE(zxzw(v4) == Vec4(3, 1, 3, 4));
  }

  TEST_CASE("Swizzle(zxwx)", "[Swizzle]")
  {
    REQUIRE(zxwx(v4) == Vec4(3, 1, 4, 1));
  }

  TEST_CASE("Swizzle(zxwy)", "[Swizzle]")
  {
    REQUIRE(zxwy(v4) == Vec4(3, 1, 4, 2));
  }

  TEST_CASE("Swizzle(zxwz)", "[Swizzle]")
  {
    REQUIRE(zxwz(v4) == Vec4(3, 1, 4, 3));
  }

  TEST_CASE("Swizzle(zxww)", "[Swizzle]")
  {
    REQUIRE(zxww(v4) == Vec4(3, 1, 4, 4));
  }

  TEST_CASE("Swizzle(zyxx)", "[Swizzle]")
  {
    REQUIRE(zyxx(v3) == Vec4(3, 2, 1, 1));
    REQUIRE(zyxx(v4) == Vec4(3, 2, 1, 1));
  }

  TEST_CASE("Swizzle(zyxy)", "[Swizzle]")
  {
    REQUIRE(zyxy(v3) == Vec4(3, 2, 1, 2));
    REQUIRE(zyxy(v4) == Vec4(3, 2, 1, 2));
  }

  TEST_CASE("Swizzle(zyxz)", "[Swizzle]")
  {
    REQUIRE(zyxz(v3) == Vec4(3, 2, 1, 3));
    REQUIRE(zyxz(v4) == Vec4(3, 2, 1, 3));
  }

  TEST_CASE("Swizzle(zyxw)", "[Swizzle]")
  {
    REQUIRE(zyxw(v4) == Vec4(3, 2, 1, 4));
  }

  TEST_CASE("Swizzle(zyyx)", "[Swizzle]")
  {
    REQUIRE(zyyx(v3) == Vec4(3, 2, 2, 1));
    REQUIRE(zyyx(v4) == Vec4(3, 2, 2, 1));
  }

  TEST_CASE("Swizzle(zyyy)", "[Swizzle]")
  {
    REQUIRE(zyyy(v3) == Vec4(3, 2, 2, 2));
    REQUIRE(zyyy(v4) == Vec4(3, 2, 2, 2));
  }

  TEST_CASE("Swizzle(zyyz)", "[Swizzle]")
  {
    REQUIRE(zyyz(v3) == Vec4(3, 2, 2, 3));
    REQUIRE(zyyz(v4) == Vec4(3, 2, 2, 3));
  }

  TEST_CASE("Swizzle(zyyw)", "[Swizzle]")
  {
    REQUIRE(zyyw(v4) == Vec4(3, 2, 2, 4));
  }

  TEST_CASE("Swizzle(zyzx)", "[Swizzle]")
  {
    REQUIRE(zyzx(v3) == Vec4(3, 2, 3, 1));
    REQUIRE(zyzx(v4) == Vec4(3, 2, 3, 1));
  }

  TEST_CASE("Swizzle(zyzy)", "[Swizzle]")
  {
    REQUIRE(zyzy(v3) == Vec4(3, 2, 3, 2));
    REQUIRE(zyzy(v4) == Vec4(3, 2, 3, 2));
  }

  TEST_CASE("Swizzle(zyzz)", "[Swizzle]")
  {
    REQUIRE(zyzz(v3) == Vec4(3, 2, 3, 3));
    REQUIRE(zyzz(v4) == Vec4(3, 2, 3, 3));
  }

  TEST_CASE("Swizzle(zyzw)", "[Swizzle]")
  {
    REQUIRE(zyzw(v4) == Vec4(3, 2, 3, 4));
  }

  TEST_CASE("Swizzle(zywx)", "[Swizzle]")
  {
    REQUIRE(zywx(v4) == Vec4(3, 2, 4, 1));
  }

  TEST_CASE("Swizzle(zywy)", "[Swizzle]")
  {
    REQUIRE(zywy(v4) == Vec4(3, 2, 4, 2));
  }

  TEST_CASE("Swizzle(zywz)", "[Swizzle]")
  {
    REQUIRE(zywz(v4) == Vec4(3, 2, 4, 3));
  }

  TEST_CASE("Swizzle(zyww)", "[Swizzle]")
  {
    REQUIRE(zyww(v4) == Vec4(3, 2, 4, 4));
  }

  TEST_CASE("Swizzle(zzxx)", "[Swizzle]")
  {
    REQUIRE(zzxx(v3) == Vec4(3, 3, 1, 1));
    REQUIRE(zzxx(v4) == Vec4(3, 3, 1, 1));
  }

  TEST_CASE("Swizzle(zzxy)", "[Swizzle]")
  {
    REQUIRE(zzxy(v3) == Vec4(3, 3, 1, 2));
    REQUIRE(zzxy(v4) == Vec4(3, 3, 1, 2));
  }

  TEST_CASE("Swizzle(zzxz)", "[Swizzle]")
  {
    REQUIRE(zzxz(v3) == Vec4(3, 3, 1, 3));
    REQUIRE(zzxz(v4) == Vec4(3, 3, 1, 3));
  }

  TEST_CASE("Swizzle(zzxw)", "[Swizzle]")
  {
    REQUIRE(zzxw(v4) == Vec4(3, 3, 1, 4));
  }

  TEST_CASE("Swizzle(zzyx)", "[Swizzle]")
  {
    REQUIRE(zzyx(v3) == Vec4(3, 3, 2, 1));
    REQUIRE(zzyx(v4) == Vec4(3, 3, 2, 1));
  }

  TEST_CASE("Swizzle(zzyy)", "[Swizzle]")
  {
    REQUIRE(zzyy(v3) == Vec4(3, 3, 2, 2));
    REQUIRE(zzyy(v4) == Vec4(3, 3, 2, 2));
  }

  TEST_CASE("Swizzle(zzyz)", "[Swizzle]")
  {
    REQUIRE(zzyz(v3) == Vec4(3, 3, 2, 3));
    REQUIRE(zzyz(v4) == Vec4(3, 3, 2, 3));
  }

  TEST_CASE("Swizzle(zzyw)", "[Swizzle]")
  {
    REQUIRE(zzyw(v4) == Vec4(3, 3, 2, 4));
  }

  TEST_CASE("Swizzle(zzzx)", "[Swizzle]")
  {
    REQUIRE(zzzx(v3) == Vec4(3, 3, 3, 1));
    REQUIRE(zzzx(v4) == Vec4(3, 3, 3, 1));
  }

  TEST_CASE("Swizzle(zzzy)", "[Swizzle]")
  {
    REQUIRE(zzzy(v3) == Vec4(3, 3, 3, 2));
    REQUIRE(zzzy(v4) == Vec4(3, 3, 3, 2));
  }

  TEST_CASE("Swizzle(zzzz)", "[Swizzle]")
  {
    REQUIRE(zzzz(v3) == Vec4(3, 3, 3, 3));
    REQUIRE(zzzz(v4) == Vec4(3, 3, 3, 3));
  }

  TEST_CASE("Swizzle(zzzw)", "[Swizzle]")
  {
    REQUIRE(zzzw(v4) == Vec4(3, 3, 3, 4));
  }

  TEST_CASE("Swizzle(zzwx)", "[Swizzle]")
  {
    REQUIRE(zzwx(v4) == Vec4(3, 3, 4, 1));
  }

  TEST_CASE("Swizzle(zzwy)", "[Swizzle]")
  {
    REQUIRE(zzwy(v4) == Vec4(3, 3, 4, 2));
  }

  TEST_CASE("Swizzle(zzwz)", "[Swizzle]")
  {
    REQUIRE(zzwz(v4) == Vec4(3, 3, 4, 3));
  }

  TEST_CASE("Swizzle(zzww)", "[Swizzle]")
  {
    REQUIRE(zzww(v4) == Vec4(3, 3, 4, 4));
  }

  TEST_CASE("Swizzle(zwxx)", "[Swizzle]")
  {
    REQUIRE(zwxx(v4) == Vec4(3, 4, 1, 1));
  }

  TEST_CASE("Swizzle(zwxy)", "[Swizzle]")
  {
    REQUIRE(zwxy(v4) == Vec4(3, 4, 1, 2));
  }

  TEST_CASE("Swizzle(zwxz)", "[Swizzle]")
  {
    REQUIRE(zwxz(v4) == Vec4(3, 4, 1, 3));
  }

  TEST_CASE("Swizzle(zwxw)", "[Swizzle]")
  {
    REQUIRE(zwxw(v4) == Vec4(3, 4, 1, 4));
  }

  TEST_CASE("Swizzle(zwyx)", "[Swizzle]")
  {
    REQUIRE(zwyx(v4) == Vec4(3, 4, 2, 1));
  }

  TEST_CASE("Swizzle(zwyy)", "[Swizzle]")
  {
    REQUIRE(zwyy(v4) == Vec4(3, 4, 2, 2));
  }

  TEST_CASE("Swizzle(zwyz)", "[Swizzle]")
  {
    REQUIRE(zwyz(v4) == Vec4(3, 4, 2, 3));
  }

  TEST_CASE("Swizzle(zwyw)", "[Swizzle]")
  {
    REQUIRE(zwyw(v4) == Vec4(3, 4, 2, 4));
  }

  TEST_CASE("Swizzle(zwzx)", "[Swizzle]")
  {
    REQUIRE(zwzx(v4) == Vec4(3, 4, 3, 1));
  }

  TEST_CASE("Swizzle(zwzy)", "[Swizzle]")
  {
    REQUIRE(zwzy(v4) == Vec4(3, 4, 3, 2));
  }

  TEST_CASE("Swizzle(zwzz)", "[Swizzle]")
  {
    REQUIRE(zwzz(v4) == Vec4(3, 4, 3, 3));
  }

  TEST_CASE("Swizzle(zwzw)", "[Swizzle]")
  {
    REQUIRE(zwzw(v4) == Vec4(3, 4, 3, 4));
  }

  TEST_CASE("Swizzle(zwwx)", "[Swizzle]")
  {
    REQUIRE(zwwx(v4) == Vec4(3, 4, 4, 1));
  }

  TEST_CASE("Swizzle(zwwy)", "[Swizzle]")
  {
    REQUIRE(zwwy(v4) == Vec4(3, 4, 4, 2));
  }

  TEST_CASE("Swizzle(zwwz)", "[Swizzle]")
  {
    REQUIRE(zwwz(v4) == Vec4(3, 4, 4, 3));
  }

  TEST_CASE("Swizzle(zwww)", "[Swizzle]")
  {
    REQUIRE(zwww(v4) == Vec4(3, 4, 4, 4));
  }

  TEST_CASE("Swizzle(wxxx)", "[Swizzle]")
  {
    REQUIRE(wxxx(v4) == Vec4(4, 1, 1, 1));
  }

  TEST_CASE("Swizzle(wxxy)", "[Swizzle]")
  {
    REQUIRE(wxxy(v4) == Vec4(4, 1, 1, 2));
  }

  TEST_CASE("Swizzle(wxxz)", "[Swizzle]")
  {
    REQUIRE(wxxz(v4) == Vec4(4, 1, 1, 3));
  }

  TEST_CASE("Swizzle(wxxw)", "[Swizzle]")
  {
    REQUIRE(wxxw(v4) == Vec4(4, 1, 1, 4));
  }

  TEST_CASE("Swizzle(wxyx)", "[Swizzle]")
  {
    REQUIRE(wxyx(v4) == Vec4(4, 1, 2, 1));
  }

  TEST_CASE("Swizzle(wxyy)", "[Swizzle]")
  {
    REQUIRE(wxyy(v4) == Vec4(4, 1, 2, 2));
  }

  TEST_CASE("Swizzle(wxyz)", "[Swizzle]")
  {
    REQUIRE(wxyz(v4) == Vec4(4, 1, 2, 3));
  }

  TEST_CASE("Swizzle(wxyw)", "[Swizzle]")
  {
    REQUIRE(wxyw(v4) == Vec4(4, 1, 2, 4));
  }

  TEST_CASE("Swizzle(wxzx)", "[Swizzle]")
  {
    REQUIRE(wxzx(v4) == Vec4(4, 1, 3, 1));
  }

  TEST_CASE("Swizzle(wxzy)", "[Swizzle]")
  {
    REQUIRE(wxzy(v4) == Vec4(4, 1, 3, 2));
  }

  TEST_CASE("Swizzle(wxzz)", "[Swizzle]")
  {
    REQUIRE(wxzz(v4) == Vec4(4, 1, 3, 3));
  }

  TEST_CASE("Swizzle(wxzw)", "[Swizzle]")
  {
    REQUIRE(wxzw(v4) == Vec4(4, 1, 3, 4));
  }

  TEST_CASE("Swizzle(wxwx)", "[Swizzle]")
  {
    REQUIRE(wxwx(v4) == Vec4(4, 1, 4, 1));
  }

  TEST_CASE("Swizzle(wxwy)", "[Swizzle]")
  {
    REQUIRE(wxwy(v4) == Vec4(4, 1, 4, 2));
  }

  TEST_CASE("Swizzle(wxwz)", "[Swizzle]")
  {
    REQUIRE(wxwz(v4) == Vec4(4, 1, 4, 3));
  }

  TEST_CASE("Swizzle(wxww)", "[Swizzle]")
  {
    REQUIRE(wxww(v4) == Vec4(4, 1, 4, 4));
  }

  TEST_CASE("Swizzle(wyxx)", "[Swizzle]")
  {
    REQUIRE(wyxx(v4) == Vec4(4, 2, 1, 1));
  }

  TEST_CASE("Swizzle(wyxy)", "[Swizzle]")
  {
    REQUIRE(wyxy(v4) == Vec4(4, 2, 1, 2));
  }

  TEST_CASE("Swizzle(wyxz)", "[Swizzle]")
  {
    REQUIRE(wyxz(v4) == Vec4(4, 2, 1, 3));
  }

  TEST_CASE("Swizzle(wyxw)", "[Swizzle]")
  {
    REQUIRE(wyxw(v4) == Vec4(4, 2, 1, 4));
  }

  TEST_CASE("Swizzle(wyyx)", "[Swizzle]")
  {
    REQUIRE(wyyx(v4) == Vec4(4, 2, 2, 1));
  }

  TEST_CASE("Swizzle(wyyy)", "[Swizzle]")
  {
    REQUIRE(wyyy(v4) == Vec4(4, 2, 2, 2));
  }

  TEST_CASE("Swizzle(wyyz)", "[Swizzle]")
  {
    REQUIRE(wyyz(v4) == Vec4(4, 2, 2, 3));
  }

  TEST_CASE("Swizzle(wyyw)", "[Swizzle]")
  {
    REQUIRE(wyyw(v4) == Vec4(4, 2, 2, 4));
  }

  TEST_CASE("Swizzle(wyzx)", "[Swizzle]")
  {
    REQUIRE(wyzx(v4) == Vec4(4, 2, 3, 1));
  }

  TEST_CASE("Swizzle(wyzy)", "[Swizzle]")
  {
    REQUIRE(wyzy(v4) == Vec4(4, 2, 3, 2));
  }

  TEST_CASE("Swizzle(wyzz)", "[Swizzle]")
  {
    REQUIRE(wyzz(v4) == Vec4(4, 2, 3, 3));
  }

  TEST_CASE("Swizzle(wyzw)", "[Swizzle]")
  {
    REQUIRE(wyzw(v4) == Vec4(4, 2, 3, 4));
  }

  TEST_CASE("Swizzle(wywx)", "[Swizzle]")
  {
    REQUIRE(wywx(v4) == Vec4(4, 2, 4, 1));
  }

  TEST_CASE("Swizzle(wywy)", "[Swizzle]")
  {
    REQUIRE(wywy(v4) == Vec4(4, 2, 4, 2));
  }

  TEST_CASE("Swizzle(wywz)", "[Swizzle]")
  {
    REQUIRE(wywz(v4) == Vec4(4, 2, 4, 3));
  }

  TEST_CASE("Swizzle(wyww)", "[Swizzle]")
  {
    REQUIRE(wyww(v4) == Vec4(4, 2, 4, 4));
  }

  TEST_CASE("Swizzle(wzxx)", "[Swizzle]")
  {
    REQUIRE(wzxx(v4) == Vec4(4, 3, 1, 1));
  }

  TEST_CASE("Swizzle(wzxy)", "[Swizzle]")
  {
    REQUIRE(wzxy(v4) == Vec4(4, 3, 1, 2));
  }

  TEST_CASE("Swizzle(wzxz)", "[Swizzle]")
  {
    REQUIRE(wzxz(v4) == Vec4(4, 3, 1, 3));
  }

  TEST_CASE("Swizzle(wzxw)", "[Swizzle]")
  {
    REQUIRE(wzxw(v4) == Vec4(4, 3, 1, 4));
  }

  TEST_CASE("Swizzle(wzyx)", "[Swizzle]")
  {
    REQUIRE(wzyx(v4) == Vec4(4, 3, 2, 1));
  }

  TEST_CASE("Swizzle(wzyy)", "[Swizzle]")
  {
    REQUIRE(wzyy(v4) == Vec4(4, 3, 2, 2));
  }

  TEST_CASE("Swizzle(wzyz)", "[Swizzle]")
  {
    REQUIRE(wzyz(v4) == Vec4(4, 3, 2, 3));
  }

  TEST_CASE("Swizzle(wzyw)", "[Swizzle]")
  {
    REQUIRE(wzyw(v4) == Vec4(4, 3, 2, 4));
  }

  TEST_CASE("Swizzle(wzzx)", "[Swizzle]")
  {
    REQUIRE(wzzx(v4) == Vec4(4, 3, 3, 1));
  }

  TEST_CASE("Swizzle(wzzy)", "[Swizzle]")
  {
    REQUIRE(wzzy(v4) == Vec4(4, 3, 3, 2));
  }

  TEST_CASE("Swizzle(wzzz)", "[Swizzle]")
  {
    REQUIRE(wzzz(v4) == Vec4(4, 3, 3, 3));
  }

  TEST_CASE("Swizzle(wzzw)", "[Swizzle]")
  {
    REQUIRE(wzzw(v4) == Vec4(4, 3, 3, 4));
  }

  TEST_CASE("Swizzle(wzwx)", "[Swizzle]")
  {
    REQUIRE(wzwx(v4) == Vec4(4, 3, 4, 1));
  }

  TEST_CASE("Swizzle(wzwy)", "[Swizzle]")
  {
    REQUIRE(wzwy(v4) == Vec4(4, 3, 4, 2));
  }

  TEST_CASE("Swizzle(wzwz)", "[Swizzle]")
  {
    REQUIRE(wzwz(v4) == Vec4(4, 3, 4, 3));
  }

  TEST_CASE("Swizzle(wzww)", "[Swizzle]")
  {
    REQUIRE(wzww(v4) == Vec4(4, 3, 4, 4));
  }

  TEST_CASE("Swizzle(wwxx)", "[Swizzle]")
  {
    REQUIRE(wwxx(v4) == Vec4(4, 4, 1, 1));
  }

  TEST_CASE("Swizzle(wwxy)", "[Swizzle]")
  {
    REQUIRE(wwxy(v4) == Vec4(4, 4, 1, 2));
  }

  TEST_CASE("Swizzle(wwxz)", "[Swizzle]")
  {
    REQUIRE(wwxz(v4) == Vec4(4, 4, 1, 3));
  }

  TEST_CASE("Swizzle(wwxw)", "[Swizzle]")
  {
    REQUIRE(wwxw(v4) == Vec4(4, 4, 1, 4));
  }

  TEST_CASE("Swizzle(wwyx)", "[Swizzle]")
  {
    REQUIRE(wwyx(v4) == Vec4(4, 4, 2, 1));
  }

  TEST_CASE("Swizzle(wwyy)", "[Swizzle]")
  {
    REQUIRE(wwyy(v4) == Vec4(4, 4, 2, 2));
  }

  TEST_CASE("Swizzle(wwyz)", "[Swizzle]")
  {
    REQUIRE(wwyz(v4) == Vec4(4, 4, 2, 3));
  }

  TEST_CASE("Swizzle(wwyw)", "[Swizzle]")
  {
    REQUIRE(wwyw(v4) == Vec4(4, 4, 2, 4));
  }

  TEST_CASE("Swizzle(wwzx)", "[Swizzle]")
  {
    REQUIRE(wwzx(v4) == Vec4(4, 4, 3, 1));
  }

  TEST_CASE("Swizzle(wwzy)", "[Swizzle]")
  {
    REQUIRE(wwzy(v4) == Vec4(4, 4, 3, 2));
  }

  TEST_CASE("Swizzle(wwzz)", "[Swizzle]")
  {
    REQUIRE(wwzz(v4) == Vec4(4, 4, 3, 3));
  }

  TEST_CASE("Swizzle(wwzw)", "[Swizzle]")
  {
    REQUIRE(wwzw(v4) == Vec4(4, 4, 3, 4));
  }

  TEST_CASE("Swizzle(wwwx)", "[Swizzle]")
  {
    REQUIRE(wwwx(v4) == Vec4(4, 4, 4, 1));
  }

  TEST_CASE("Swizzle(wwwy)", "[Swizzle]")
  {
    REQUIRE(wwwy(v4) == Vec4(4, 4, 4, 2));
  }

  TEST_CASE("Swizzle(wwwz)", "[Swizzle]")
  {
    REQUIRE(wwwz(v4) == Vec4(4, 4, 4, 3));
  }

  TEST_CASE("Swizzle(wwww)", "[Swizzle]")
  {
    REQUIRE(wwww(v4) == Vec4(4, 4, 4, 4));
  }

#pragma endregion Four Components
}