#pragma once

namespace krys::boo
{
  class NestingLevelIncrementer
  {
  private:
    size_t *_nestingLevel;
  public:
    explicit NestingLevelIncrementer(size_t &nestingLevel) : _nestingLevel(&nestingLevel)
    {
      ++(*_nestingLevel);
    }

    ~NestingLevelIncrementer()
    {
      --(*_nestingLevel);
    }
  };
}