#pragma once

namespace Krys::HTML
{
  // A span in a single direction (either rows or columns). Note that |startLine|
  // and |endLine| are grid lines' indexes.
  // Despite line numbers in the spec start in "1", the indexes here start in "0".
  class GridSpan
  {
  public:
    static GridSpan untranslatedDefiniteGridSpan(int startLine, int endLine);
    static GridSpan translatedDefiniteGridSpan(unsigned startLine, unsigned endLine);
    static GridSpan indefiniteGridSpan();

    static GridSpan masonryAxisTranslatedDefiniteGridSpan();

    friend bool operator==(const GridSpan &, const GridSpan &) = default;

    unsigned integerSpan() const;

    int untranslatedStartLine() const;
    int untranslatedEndLine() const;

    unsigned startLine() const;
    unsigned endLine() const;

    struct GridSpanIterator
    {
      GridSpanIterator(unsigned value) : value(value)
      {
      }

      operator unsigned &()
      {
        return value;
      }
      unsigned operator*() const
      {
        return value;
      }

      unsigned value;
    };

    GridSpanIterator begin() const;

    GridSpanIterator end() const;

    bool isTranslatedDefinite() const;
    bool isIndefinite() const;

    void translate(unsigned offset);

    // Moves this span to be in the same coordinate space as |parent|.
    // If reverse is specified, then swaps the direction to handle RTL/LTR changes.
    void translateTo(const GridSpan &parent, bool reverse);

    void clamp(int max);

    bool clamp(int min, int max);

  private:
    enum GridSpanType
    {
      UntranslatedDefinite,
      TranslatedDefinite,
      Indefinite
    };

    GridSpan(int startLine, int endLine, GridSpanType);

    int m_startLine;
    int m_endLine;
    GridSpanType m_type;
  };
}