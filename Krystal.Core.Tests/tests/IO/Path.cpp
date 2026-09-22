#include "Krystal.Core/IO/Path.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::io::tests
{
  using path = std::filesystem::path;

  TEST_CASE("Path Constructor - Default", "[IO]")
  {
    krys::io::Path p("");
    REQUIRE_FALSE(p);
    REQUIRE(p.ToString().empty());
    REQUIRE(p.ParentPath().ToString().empty());
    REQUIRE(p.Filename().ToString().empty());
    REQUIRE(p.Extension().ToString().empty());
  }

  TEST_CASE("Path Constructor - String", "[IO]")
  {
    krys::io::Path p("foo/bar.txt");
    REQUIRE(p);
    REQUIRE(p.ToString() == "foo/bar.txt");
  }

  TEST_CASE("Path Constructor - Copy", "[IO]")
  {
    krys::io::Path p1("dir/file");
    krys::io::Path p2(p1);
    REQUIRE(p2.ToString() == "dir/file");
  }

  TEST_CASE("Path Constructor - Move", "[IO]")
  {
    krys::io::Path p1("dir/file");
    krys::io::Path p2(std::move(p1));
    REQUIRE(p2.ToString() == "dir/file");
  }

  TEST_CASE("Path Operator - /", "[IO]")
  {
    krys::io::Path p1("foo");
    krys::io::Path p2("baz");
    auto p3 = p1 / p2;
    REQUIRE(p3.ToString() == "foo/baz");
    REQUIRE(p3 / krys::io::Path("bar.txt") == krys::io::Path("foo/baz/bar.txt"));
  }

  TEST_CASE("Path Operator - /=", "[IO]")
  {
    krys::io::Path p("foo");
    p /= krys::io::Path("bar.txt");
    REQUIRE(p.ToString() == "foo/bar.txt");
  }

  TEST_CASE("Path Operator - ==", "[IO]")
  {
    REQUIRE(krys::io::Path("a/b") == krys::io::Path("a/b"));
    REQUIRE_FALSE(krys::io::Path("a/b") == krys::io::Path("a/c"));
  }

  TEST_CASE("Path Operator - !=", "[IO]")
  {
    REQUIRE(krys::io::Path("a/b") != krys::io::Path("a/c"));
    REQUIRE_FALSE(krys::io::Path("a/b") != krys::io::Path("a/b"));
  }

  TEST_CASE("Path Method - ParentPath", "[IO]")
  {
    krys::io::Path p("a/b/c.txt");
    REQUIRE(p.ParentPath().ToString() == "a/b");
  }

  TEST_CASE("Path Method - Filename", "[IO]")
  {
    krys::io::Path p1("a/b/c.txt");
    REQUIRE(p1.Filename().ToString() == "c.txt");

    krys::io::Path p2("a/b/");
    REQUIRE(p2.Filename().ToString().empty());
  }

  TEST_CASE("Path Method - HasFilename", "[IO]")
  {
    REQUIRE(krys::io::Path("a/file.txt").HasFilename());
    REQUIRE_FALSE(krys::io::Path("a/").HasFilename());
  }

  TEST_CASE("Path Method - Extension", "[IO]")
  {
    krys::io::Path p1("file.txt");
    REQUIRE(p1.Extension().ToString() == ".txt");

    krys::io::Path p2("file");
    REQUIRE(p2.Extension().ToString().empty());
  }

  TEST_CASE("Path Method - HasExtension", "[IO]")
  {
    REQUIRE(krys::io::Path("file.txt").HasExtension());
    REQUIRE_FALSE(krys::io::Path("file").HasExtension());
  }

  TEST_CASE("Path Method - StartsWith", "[IO]")
  {
    REQUIRE(krys::io::Path("a/b/c.txt").StartsWith(krys::io::Path("a/b")));
    REQUIRE_FALSE(krys::io::Path("a/b/c.txt").StartsWith(krys::io::Path("x/y")));
  }

  TEST_CASE("Path Method - RelativePath", "[IO]")
  {
    using namespace krys::io;

    REQUIRE(Path("a/b/c.txt").RelativePath(Path("a")).ToString() == "b/c.txt");
    REQUIRE(Path("a/b/c.txt").RelativePath(Path("x/y")).ToString() == "../../a/b/c.txt");

    // Edge cases
    REQUIRE(Path("a").RelativePath(Path("a")).ToString() == ".");
    REQUIRE(Path("a").RelativePath(Path("")).ToString() == "a");
  }

  TEST_CASE("Path Method - Normalise", "[IO]")
  {
    REQUIRE(krys::io::Path("a/./b").Normalise().ToString() == string("a/b"));
    REQUIRE(krys::io::Path("a/b/../c").Normalise().ToString() == string("a/c"));
    REQUIRE(krys::io::Path("a//b").Normalise().ToString() == string("a/b"));
  }
}