#pragma once

#include "Krystal.Lib/List.hpp"
#include "Krystal.UI/Layout/Node/Node.hpp"

namespace Krys::UI
{
  struct FlexLineRunningLayout
  {
    /// @brief Total flex grow factors of flex items which are to be laid in the current line. This is
    /// decremented as free space is distributed.
    float TotalFlexGrowFactors {0.0f};

    /// @brief Total flex shrink factors of flex items which are to be laid in the current line. This is
    /// decremented as free space is distributed.
    float TotalFlexShrinkScaledFactors {0.0f};

    /// @brief The amount of available space within inner dimensions of the line which may still be
    /// distributed.
    float RemainingFreeSpace {0.0f};

    /// @brief The size of the mainDim for the row after considering size, padding, margin and border of flex
    /// items. This is used to calculate maxLineDim after going through all the rows to decide on the main
    /// axis size of owner.
    float MainDim {0.0f};

    /// @brief The size of the crossDim for the row after considering size, padding, margin and border of flex
    /// items. Used for calculating containers crossSize.
    float CrossDim {0.0f};
  };

  struct FlexLine
  {
    /// @brief List of children which are part of the line flow. This means they are not positioned
    /// absolutely, or with `display: "none"`, and do not overflow the available dimensions.
    const List<Node *> ItemsInFlow {};

    /// @brief Layout information about the line computed in steps after line-breaking
    FlexLineRunningLayout Layout {};

    /// @brief Accumulation of the dimensions and margin of all the children on the current line. This will be
    /// used in order to either set the dimensions of the node if none already exist or to compute the
    /// remaining space left for the flexible children.
    const float SizeConsumed {0.0f};

    /// @brief Number of edges along the line flow with an auto margin.
    const size_t NumberOfAutoMargins {0};
  };

  /// @brief Calculates where a line starting at a given index should break, returning information about the
  /// collective children on the liune. This function assumes that all the children of node have their
  /// ComputedFlexBasis properly computed (to do this use `ComputeFlexBasisForChildren()`).
  FlexLine CalculateFlexLine(Node *node, Direction ownerDirection, float ownerWidth,
                             float mainAxisOwnerSize, float availableInnerWidth, float availableInnerMainDim,
                             Node::LayoutableChildren::Iterator &iterator, uint16 lineCount);
}