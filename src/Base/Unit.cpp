// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   This file is part of FreeCAD.                                          *
 *                                                                          *
 *   FreeCAD is free software: you can redistribute it and/or modify it     *
 *   under the terms of the GNU Lesser General Public License as            *
 *   published by the Free Software Foundation, either version 2.1 of the   *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   FreeCAD is distributed in the hope that it will be useful, but         *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with FreeCAD. If not, see                                *
 *   <https://www.gnu.org/licenses/>.                                       *
 *                                                                          *
 ***************************************************************************/

#include "PreCompiled.h"
#ifndef _PreComp_
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <limits>
#include <vector>
#endif

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "Exception.h"
#include "Unit.h"

using namespace Base;

bool Unit::operator==(const Unit& that) const
{
    return _vals == that._vals;
}

bool Unit::operator!=(const Unit& that) const
{
    return _vals != that._vals;
}

bool Unit::operator==(const int one) const
{
//    static_assert(one == 1, "Invalid Unit dimension");
    constexpr UnitVals allzero {0, 0, 0, 0, 0, 0, 0, 0};
    return _vals == allzero;
}

bool Unit::operator!=(const int one) const
{
//    static_assert(one == 1, "Invalid Unit dimension");
    constexpr UnitVals allzero {0, 0, 0, 0, 0, 0, 0, 0};
    return _vals != allzero;
}

Unit& Unit::operator*=(const Unit& that)
{
    *this = *this * that;
    return *this;
}

Unit& Unit::operator/=(const Unit& that)
{
    *this = *this / that;
    return *this;
}

Unit Unit::operator*(const Unit& right) const
{
    auto mult = [&](auto leftVal, auto rightVal) {
        return leftVal + rightVal;
    };

    UnitVals res {};
    std::transform(_vals.begin(), _vals.end(), right._vals.begin(), res.begin(), mult);

    return Unit {res};
}

Unit Unit::operator/(const Unit& right) const
{
    auto div = [&](auto leftVal, auto rightVal) mutable {
        return leftVal - rightVal;
    };

    UnitVals res {};
    std::transform(_vals.begin(), _vals.end(), right._vals.begin(), res.begin(), div);

    return Unit {res};
}

Unit Unit::root(const uint8_t num) const
{
    auto apply = [&](auto val) {
        if (val % num != 0) {
            throw UnitsMismatchError("unit values must be divisible by root");
        }
        return static_cast<int8_t>(val / num);
    };

    if (num < 1) {
        throw UnitsMismatchError("root must be > 0");
    }

    std::array<int8_t, unitNumVals> res {};
    std::transform(_vals.begin(), _vals.end(), res.begin(), apply);

    return Unit {res};
}

Unit Unit::pow(const double exp) const
{
    auto apply = [&](const auto val) {
        const auto num {val * exp};
        if (std::fabs(std::round(num) - num) >= std::numeric_limits<double>::epsilon()) {
            throw UnitsMismatchError("pow() of unit not possible");
        }

        return static_cast<int>(val * exp);
    };

    UnitVals res {};
    std::transform(_vals.begin(), _vals.end(), res.begin(), apply);

    return Unit {res};
}

std::array<int8_t, unitNumVals> Unit::vals() const
{
    return _vals;
}

int Unit::length() const
{
    return _vals[0];
}

std::string Unit::getString() const
{
    auto buildSubStr = [&](auto index) {
        const std::string unitStrString {valNames.at(index)};
        const auto absol {abs(_vals.at(index))};

        return absol <= 1 ? unitStrString
                          : fmt::format("{}^{}", unitStrString, std::to_string(absol));
    };

    auto buildStr = [&](auto indexes) {
        std::vector<std::string> subStrings {};
        std::transform(indexes.begin(), indexes.end(), std::back_inserter(subStrings), buildSubStr);

        return format("{}", fmt::join(subStrings, "*"));
    };

    //------------------------------------------------------------------------------

    auto [posValIndexes, negValIndexes] = nonZeroValsIndexes();
    auto numeratorStr = buildStr(posValIndexes);
    if (negValIndexes.empty()) {
        return numeratorStr;
    }

    auto denominatorStr = buildStr(negValIndexes);

    return fmt::format("{}/{}",
                       numeratorStr.empty() ? "1" : numeratorStr,
                       negValIndexes.size() > 1 ? fmt::format("({})", denominatorStr)
                                                : denominatorStr);
}

std::string Unit::representation() const
{
    auto inParen = format("Unit: {} ({})", getString(), fmt::join(_vals, ","));
    return _name.empty() ? inParen : fmt::format("{} [{}]", inParen, _name);
}

std::string Unit::getTypeString() const
{
    return std::string {_name.data(), _name.size()};
}

std::pair<std::vector<size_t>, std::vector<size_t>> Unit::nonZeroValsIndexes() const
{
    std::vector<size_t> pos {};
    std::vector<size_t> neg {};

    auto posNeg = [&, index {0}](auto val) mutable {
        if (val != 0) {
            (val > 0 ? pos : neg).push_back(index);
        }
        ++index;
    };

    std::for_each(_vals.begin(), _vals.end(), posNeg);

    return {pos, neg};
}
