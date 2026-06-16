#pragma once

#include <vector>
#include <tuple>
#include <string>
#include <functional>

#include <TROOT.h>
#include <TVector3.h>

namespace PointGroupOperations
{
    using pointGenerator = std::function<std::vector<TVector3>(const TVector3&)>;

    pointGenerator GetPointGroupOperations(std::string groupName);
}