//
// Created by arden on 8/29/24.
//

#include "CodecController.h"

int CodecController::getClosest (const int target, const std::vector<int>& options)
{
    auto lower = options[0];
    auto upper = options[options.size() - 1];
    for (const auto option : options)
    {
        if (option < target)
        {
            lower = option;
        }
        else
        {
            upper = option;
            return (target - lower) < (upper - target) ? lower : upper;
        }
    }
    return lower;
}
