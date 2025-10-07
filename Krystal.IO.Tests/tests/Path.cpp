#include "Krystal.IO/Path.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::IO;
  using path = std::filesystem::path;

  TEST_CASE("Path Constructor - Default", "[IO]")
  {
    Krys::IO::Path p("");
    REQUIRE_FALSE(p);
    REQUIRE(p.ToString().empty());
    REQUIRE(p.ParentPath().ToString().empty());
    REQUIRE(p.Filename().ToString().empty());
    REQUIRE(p.Extension().ToString().empty());
  }

  TEST_CASE("Path Constructor - String", "[IO]")
  {
    Krys::IO::Path p("foo/bar.txt");
    REQUIRE(p);
    REQUIRE(p.ToString() == "foo/bar.txt");
  }

  TEST_CASE("Path Constructor - Copy", "[IO]")
  {
    Krys::IO::Path p1("dir/file");
    Krys::IO::Path p2(p1);
    REQUIRE(p2.ToString() == "dir/file");
  }

  TEST_CASE("Path Constructor - Move", "[IO]")
  {
    Krys::IO::Path p1("dir/file");
    Krys::IO::Path p2(std::move(p1));
    REQUIRE(p2.ToString() == "dir/file");
  }

  TEST_CASE("Path Operator - /", "[IO]")
  {
    Krys::IO::Path p1("foo");
    Krys::IO::Path p2("baz");
    auto p3 = p1 / p2;
    REQUIRE(p3.ToString() == "foo/baz");
    REQUIRE(p3 / Krys::IO::Path("bar.txt") == Krys::IO::Path("foo/baz/bar.txt"));
  }

  TEST_CASE("Path Operator - /=", "[IO]")
  {
    Krys::IO::Path p("foo");
    p /= Krys::IO::Path("bar.txt");
    REQUIRE(p.ToString() == "foo/bar.txt");
  }

  TEST_CASE("Path Operator - ==", "[IO]")
  {
    REQUIRE(Krys::IO::Path("a/b") == Krys::IO::Path("a/b"));
    REQUIRE_FALSE(Krys::IO::Path("a/b") == Krys::IO::Path("a/c"));
  }

  TEST_CASE("Path Operator - !=", "[IO]")
  {
    REQUIRE(Krys::IO::Path("a/b") != Krys::IO::Path("a/c"));
    REQUIRE_FALSE(Krys::IO::Path("a/b") != Krys::IO::Path("a/b"));
  }

  TEST_CASE("Path Method - ParentPath", "[IO]")
  {
    Krys::IO::Path p("a/b/c.txt");
    REQUIRE(p.ParentPath().ToString() == "a/b");
  }

  TEST_CASE("Path Method - Filename", "[IO]")
  {
    Krys::IO::Path p1("a/b/c.txt");
    REQUIRE(p1.Filename().ToString() == "c.txt");

    Krys::IO::Path p2("a/b/");
    REQUIRE(p2.Filename().ToString().empty());
  }

  TEST_CASE("Path Method - HasFilename", "[IO]")
  {
    REQUIRE(Krys::IO::Path("a/file.txt").HasFilename());
    REQUIRE_FALSE(Krys::IO::Path("a/").HasFilename());
  }

  TEST_CASE("Path Method - Extension", "[IO]")
  {
    Krys::IO::Path p1("file.txt");
    REQUIRE(p1.Extension().ToString() == ".txt");

    Krys::IO::Path p2("file");
    REQUIRE(p2.Extension().ToString().empty());
  }

  TEST_CASE("Path Method - HasExtension", "[IO]")
  {
    REQUIRE(Krys::IO::Path("file.txt").HasExtension());
    REQUIRE_FALSE(Krys::IO::Path("file").HasExtension());
  }

  TEST_CASE("Path Method - StartsWith", "[IO]")
  {
    REQUIRE(Krys::IO::Path("a/b/c.txt").StartsWith(Krys::IO::Path("a/b")));
    REQUIRE_FALSE(Krys::IO::Path("a/b/c.txt").StartsWith(Krys::IO::Path("x/y")));
  }

  TEST_CASE("Path Method - RelativePath", "[IO]")
  {
    using namespace Krys::IO;

    REQUIRE(Path("a/b/c.txt").RelativePath(Path("a")).ToString() == "b/c.txt");
    REQUIRE(Path("a/b/c.txt").RelativePath(Path("x/y")).ToString() == "../../a/b/c.txt");

    // Edge cases
    REQUIRE(Path("a").RelativePath(Path("a")).ToString() == ".");
    REQUIRE(Path("a").RelativePath(Path("")).ToString() == "a");
  }

  TEST_CASE("Path Method - Normalise", "[IO]")
  {
    REQUIRE(Krys::IO::Path("a/./b").Normalise().ToString() == string("a/b"));
    REQUIRE(Krys::IO::Path("a/b/../c").Normalise().ToString() == string("a/c"));
    REQUIRE(Krys::IO::Path("a//b").Normalise().ToString() == string("a/b"));
  }
}