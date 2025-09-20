#include "Krystal.IO/Path.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::IO;
  using path = std::filesystem::path;

  TEST_CASE("Path", "[IO]")
  {
    SECTION("Default Construction")
    {
      Krys::IO::Path p("");
      REQUIRE_FALSE(p);
      REQUIRE(p.ToString().empty());
    }

    SECTION("String Construction")
    {
      Krys::IO::Path p("foo/bar.txt");
      REQUIRE(p);
      REQUIRE(p.ToString() == path("foo/bar.txt").lexically_normal().string());
    }

    SECTION("Copy Construction")
    {
      Krys::IO::Path p1("dir/file");
      Krys::IO::Path p2(p1);
      REQUIRE(p1 == p2);
    }

    SECTION("Move Construction")
    {
      Krys::IO::Path p1("dir/file");
      Krys::IO::Path p2(std::move(p1));
      REQUIRE(p2.ToString() == path("dir/file").lexically_normal().string());
      REQUIRE_FALSE(p1); // NOLINT(bugprone-use-after-move)
    }

    SECTION("Operator/ Composition")
    {
      Krys::IO::Path p1("foo");
      Krys::IO::Path p2("bar.txt");
      auto p3 = p1 / p2;
      REQUIRE(p3.ToString() == path("foo/bar.txt").lexically_normal().string());
    }

    SECTION("Operator/= Composition")
    {
      Krys::IO::Path p("foo");
      p /= Krys::IO::Path("bar.txt");
      REQUIRE(p.ToString() == path("foo/bar.txt").lexically_normal().string());
    }

    SECTION("Multiple Composition")
    {
      Krys::IO::Path p("a");
      p = p / Path("b") / Path("c.txt");
      REQUIRE(p.ToString() == path("a/b/c.txt").lexically_normal().string());
    }

    SECTION("Parent Path")
    {
      Krys::IO::Path p("a/b/c.txt");
      REQUIRE(p.ParentPath().ToString() == path("a/b").lexically_normal().string());
    }

    SECTION("Filename")
    {
      Krys::IO::Path p("a/b/c.txt");
      REQUIRE(p.Filename().ToString() == "c.txt");
    }

    SECTION("Extension")
    {
      Krys::IO::Path p1("file.txt");
      REQUIRE(p1.Extension().ToString() == ".txt");

      Krys::IO::Path p2("file");
      REQUIRE(p2.Extension().ToString().empty());
    }

    SECTION("HasExtension")
    {
      REQUIRE(Krys::IO::Path("file.txt").HasExtension());
      REQUIRE_FALSE(Krys::IO::Path("file").HasExtension());
    }

    SECTION("Equality")
    {
      REQUIRE(Krys::IO::Path("a/b") == Krys::IO::Path("a/b"));
      REQUIRE_FALSE(Krys::IO::Path("a/b") == Krys::IO::Path("a/c"));
    }

    SECTION("Inequality")
    {
      REQUIRE(Krys::IO::Path("a/b") != Krys::IO::Path("a/c"));
      REQUIRE_FALSE(Krys::IO::Path("a/b") != Krys::IO::Path("a/b"));
    }

    SECTION("StartsWith - Positive")
    {
      REQUIRE(Krys::IO::Path("a/b/c.txt").StartsWith(Krys::IO::Path("a/b")));
    }

    SECTION("StartsWith - Negative")
    {
      REQUIRE_FALSE(Krys::IO::Path("a/b/c.txt").StartsWith(Krys::IO::Path("x/y")));
    }

    SECTION("RelativePath - Basic")
    {
      auto rel = Krys::IO::Path("a/b/c.txt").RelativePath(Krys::IO::Path("a"));
      REQUIRE(rel.ToString() == path("b/c.txt").lexically_normal().string());
    }

    SECTION("RelativePath - Not Relative")
    {
      auto rel = Krys::IO::Path("a/b/c.txt").RelativePath(Krys::IO::Path("x/y"));
      REQUIRE(rel.ToString() == path("a/b/c.txt").lexically_normal().string());
    }

    SECTION("RelativePath - Edge Cases")
    {
      REQUIRE(Krys::IO::Path("").RelativePath(Krys::IO::Path("a")).ToString().empty());
      REQUIRE(Krys::IO::Path("a").RelativePath(Krys::IO::Path("")).ToString()
              == path("a").lexically_normal().string());
    }

    SECTION("Dot Normalization")
    {
      REQUIRE(Krys::IO::Path("a/./b").ToString() == path("a/b").lexically_normal().string());
    }

    SECTION("DotDot Normalization")
    {
      REQUIRE(Krys::IO::Path("a/b/../c").ToString() == path("a/c").lexically_normal().string());
    }

    SECTION("Multiple Separators")
    {
      REQUIRE(Krys::IO::Path("a//b").ToString() == path("a/b").lexically_normal().string());
    }

    SECTION("Empty Path")
    {
      Krys::IO::Path p("");
      REQUIRE_FALSE(p);
      REQUIRE(p.ToString().empty());
      REQUIRE(p.ParentPath().ToString().empty());
      REQUIRE(p.Filename().ToString().empty());
    }

    SECTION("Root Path")
    {
      Krys::IO::Path p("/");
      REQUIRE(p);
      REQUIRE(p.ParentPath().ToString() == path("/").lexically_normal());
      REQUIRE(p.Filename().ToString().empty());
    }

    SECTION("Current Directory")
    {
      Krys::IO::Path p(".");
      REQUIRE(p.ToString() == path(".").lexically_normal().string());
    }
  }
}