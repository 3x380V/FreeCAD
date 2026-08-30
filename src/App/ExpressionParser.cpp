// SPDX-License-Identifier: LGPL-2.1-or-later

/***************************************************************************
 *   Copyright (c) 2026 FreeCAD project contributors                       *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

/* Handwritten parser for the FreeCAD expression language.
 *
 * Replaces the former flex/bison (GLR) parser.  Value expressions are
 * parsed by precedence climbing; unit expressions by a small recursive
 * descent parser.  The tokenizer (ExpressionLexer.h) reports word-like
 * tokens as generic names, and this parser interprets them as units or
 * identifiers according to context, following the disambiguation rules
 * the GLR grammar expressed with %dprec:
 *
 *  - An input that parses completely as a unit expression is a unit
 *    expression ("mm", "mm^2/s"); otherwise it is a value expression,
 *    in which a bare name is an identifier, so document objects named
 *    like units ("mm", "in", "t") can be referenced.
 *
 *  - After a numeric literal, a maximal unit expression is attached as
 *    a quantity ("10 mm * kg" is 10 (mm*kg)); an operand that does not
 *    continue the unit expression ends it ("1mm * 3" is (1 mm) * 3).
 *
 *  - The right-hand side of '/' prefers a unit expression ("5 / s"),
 *    falling back to a value operand ("24 V / (2 A)").
 */

#include <cerrno>
#include <cstdlib>
#include <limits>
#include <map>
#include <memory>
#include <numbers>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "DocumentObject.h"
#include "Expression.h"
#include "ExpressionParser.h"
#include "ObjectIdentifier.h"

#include <Base/Exception.h>
#include <Base/Quantity.h>

// Defined in Expression.cpp: strips <<...>> and resolves backslash escapes.
std::string unquote(const std::string& input);

namespace App::ExpressionParser
{

namespace
{

using Base::Quantity;

// ---------------------------------------------------------------------
// Unit table.  Generated from the unit rules of the former Expression.l;
// sorted by name for binary search.

struct UnitEntry
{
    std::string_view name;
    const Quantity* quantity;
};

// clang-format off
const UnitEntry unitTable[] = {
    {"A", &Quantity::Ampere},
    {"AS", &Quantity::AngSecond},
    {"C", &Quantity::Coulomb},
    {"CV", &Quantity::WattSecond},
    {"F", &Quantity::Farad},
    {"G", &Quantity::Gauss},
    {"GHz", &Quantity::GigaHertz},
    {"GPa", &Quantity::GigaPascal},
    {"H", &Quantity::Henry},
    {"Hz", &Quantity::Hertz},
    {"J", &Quantity::Joule},
    {"K", &Quantity::Kelvin},
    {"M", &Quantity::AngMinute},
    {"MA", &Quantity::MegaAmpere},
    {"MHz", &Quantity::MegaHertz},
    {"MN", &Quantity::MegaNewton},
    {"MOhm", &Quantity::MegaOhm},
    {"MPa", &Quantity::MegaPascal},
    {"MS", &Quantity::MegaSiemens},
    {"MeV", &Quantity::MegaElectronVolt},
    {"Mpsi", &Quantity::MPSI},
    {"N", &Quantity::Newton},
    {"Nm", &Quantity::NewtonMeter},
    {"Ohm", &Quantity::Ohm},
    {"Pa", &Quantity::Pascal},
    {"S", &Quantity::Siemens},
    {"T", &Quantity::Tesla},
    {"THz", &Quantity::TeraHertz},
    {"Torr", &Quantity::Torr},
    {"V", &Quantity::Volt},
    {"VA", &Quantity::VoltAmpere},
    {"VAs", &Quantity::VoltAmpereSecond},
    {"W", &Quantity::Watt},
    {"Wb", &Quantity::Weber},
    {"Ws", &Quantity::WattSecond},
    {"bar", &Quantity::Bar},
    {"cal", &Quantity::Calorie},
    {"cd", &Quantity::Candela},
    {"cft", &Quantity::CubicFoot},
    {"cm", &Quantity::CentiMetre},
    {"cwt", &Quantity::Hundredweights},
    {"deg", &Quantity::Degree},
    {"dm", &Quantity::DeciMetre},
    {"eV", &Quantity::ElectronVolt},
    {"ft", &Quantity::Foot},
    {"g", &Quantity::Gram},
    {"gon", &Quantity::Gon},
    {"h", &Quantity::Hour},
    {"in", &Quantity::Inch},
    {"kA", &Quantity::KiloAmpere},
    {"kHz", &Quantity::KiloHertz},
    {"kJ", &Quantity::KiloJoule},
    {"kN", &Quantity::KiloNewton},
    {"kOhm", &Quantity::KiloOhm},
    {"kPa", &Quantity::KiloPascal},
    {"kS", &Quantity::KiloSiemens},
    {"kV", &Quantity::KiloVolt},
    {"kW", &Quantity::KiloWatt},
    {"kWh", &Quantity::KiloWattHour},
    {"kcal", &Quantity::KiloCalorie},
    {"keV", &Quantity::KiloElectronVolt},
    {"kg", &Quantity::KiloGram},
    {"km", &Quantity::KiloMetre},
    {"ksi", &Quantity::KSI},
    {"l", &Quantity::Liter},
    {"lb", &Quantity::Pound},
    {"lbf", &Quantity::PoundForce},
    {"lbm", &Quantity::Pound},
    {"m", &Quantity::Metre},
    {"mA", &Quantity::MilliAmpere},
    {"mF", &Quantity::MilliFarad},
    {"mH", &Quantity::MilliHenry},
    {"mJ", &Quantity::MilliJoule},
    {"mK", &Quantity::MilliKelvin},
    {"mN", &Quantity::MilliNewton},
    {"mS", &Quantity::MilliSiemens},
    {"mT", &Quantity::MilliTesla},
    {"mTorr", &Quantity::mTorr},
    {"mV", &Quantity::MilliVolt},
    {"mW", &Quantity::MilliWatt},
    {"mbar", &Quantity::MilliBar},
    {"mg", &Quantity::MilliGram},
    {"mi", &Quantity::Mile},
    {"mil", &Quantity::Thou},
    {"min", &Quantity::Minute},
    {"ml", &Quantity::MilliLiter},
    {"mm", &Quantity::MilliMetre},
    {"mmol", &Quantity::MilliMole},
    {"mol", &Quantity::Mole},
    {"mph", &Quantity::MilePerHour},
    {"nA", &Quantity::NanoAmpere},
    {"nF", &Quantity::NanoFarad},
    {"nH", &Quantity::NanoHenry},
    {"nW", &Quantity::NanoWatt},
    {"nm", &Quantity::NanoMetre},
    {"nmol", &Quantity::NanoMole},
    {"oz", &Quantity::Ounce},
    {"pF", &Quantity::PicoFarad},
    {"psi", &Quantity::PSI},
    {"rad", &Quantity::Radian},
    {"s", &Quantity::Second},
    {"sqft", &Quantity::SquareFoot},
    {"st", &Quantity::Stone},
    {"t", &Quantity::Ton},
    {"thou", &Quantity::Thou},
    {"uA", &Quantity::MicroAmpere},
    {"uF", &Quantity::MicroFarad},
    {"uH", &Quantity::MicroHenry},
    {"uK", &Quantity::MicroKelvin},
    {"uS", &Quantity::MicroSiemens},
    {"uTorr", &Quantity::yTorr},
    {"uW", &Quantity::MicroWatt},
    {"ug", &Quantity::MicroGram},
    {"um", &Quantity::MicroMetre},
    {"umol", &Quantity::MicroMole},
    {"yd", &Quantity::Yard},
    {"\xc2\xb0", &Quantity::Degree},             // degree sign
    {"\xc2\xb5""A", &Quantity::MicroAmpere},     // micro sign variants
    {"\xc2\xb5""F", &Quantity::MicroFarad},
    {"\xc2\xb5""H", &Quantity::MicroHenry},
    {"\xc2\xb5""K", &Quantity::MicroKelvin},
    {"\xc2\xb5S", &Quantity::MicroSiemens},
    {"\xc2\xb5Torr", &Quantity::yTorr},
    {"\xc2\xb5W", &Quantity::MicroWatt},
    {"\xc2\xb5g", &Quantity::MicroGram},
    {"\xc2\xb5m", &Quantity::MicroMetre},
    {"\xc2\xb5mol", &Quantity::MicroMole},
    {"\xe2\x80\xb2", &Quantity::AngMinute},      // prime
    {"\xe2\x80\xb3", &Quantity::AngSecond},      // double prime
};
// clang-format on

const Quantity* lookupUnit(std::string_view name)
{
    std::size_t lo = 0;
    std::size_t hi = std::size(unitTable);
    while (lo < hi) {
        const std::size_t mid = (lo + hi) / 2;
        if (unitTable[mid].name < name) {
            lo = mid + 1;
        }
        else {
            hi = mid;
        }
    }
    if (lo < std::size(unitTable) && unitTable[lo].name == name) {
        return unitTable[lo].quantity;
    }
    return nullptr;
}

// ---------------------------------------------------------------------
// Constants and functions.

struct ConstantEntry
{
    std::string_view spelling;
    const char* name;  // stored in ConstantExpression, must be static
    double value;
};

const ConstantEntry constantTable[] = {
    {"False", "False", 0},
    {"None", "None", 0},
    {"True", "True", 1},
    {"e", "e", std::numbers::e},
    {"false", "False", 0},
    {"pi", "pi", std::numbers::pi},
    {"true", "True", 1},
};

const ConstantEntry* lookupConstant(std::string_view spelling)
{
    for (const auto& entry : constantTable) {
        if (entry.spelling == spelling) {
            return &entry;
        }
    }
    return nullptr;
}

const std::map<std::string, FunctionExpression::Function, std::less<>>& registeredFunctions()
{
    // clang-format off
    static const std::map<std::string, FunctionExpression::Function, std::less<>> functions = {
        {"abs", FunctionExpression::ABS},
        {"acos", FunctionExpression::ACOS},
        {"asin", FunctionExpression::ASIN},
        {"atan", FunctionExpression::ATAN},
        {"atan2", FunctionExpression::ATAN2},
        {"cath", FunctionExpression::CATH},
        {"cbrt", FunctionExpression::CBRT},
        {"ceil", FunctionExpression::CEIL},
        {"cos", FunctionExpression::COS},
        {"cosh", FunctionExpression::COSH},
        {"exp", FunctionExpression::EXP},
        {"floor", FunctionExpression::FLOOR},
        {"hypot", FunctionExpression::HYPOT},
        {"log", FunctionExpression::LOG},
        {"log10", FunctionExpression::LOG10},
        {"mod", FunctionExpression::MOD},
        {"pow", FunctionExpression::POW},
        {"round", FunctionExpression::ROUND},
        {"sin", FunctionExpression::SIN},
        {"sinh", FunctionExpression::SINH},
        {"sqrt", FunctionExpression::SQRT},
        {"tan", FunctionExpression::TAN},
        {"tanh", FunctionExpression::TANH},
        {"trunc", FunctionExpression::TRUNC},
        {"vangle", FunctionExpression::VANGLE},
        {"vcross", FunctionExpression::VCROSS},
        {"vdot", FunctionExpression::VDOT},
        {"vlinedist", FunctionExpression::VLINEDIST},
        {"vlinesegdist", FunctionExpression::VLINESEGDIST},
        {"vlineproj", FunctionExpression::VLINEPROJ},
        {"vnormalize", FunctionExpression::VNORMALIZE},
        {"vplanedist", FunctionExpression::VPLANEDIST},
        {"vplaneproj", FunctionExpression::VPLANEPROJ},
        {"vscale", FunctionExpression::VSCALE},
        {"vscalex", FunctionExpression::VSCALEX},
        {"vscaley", FunctionExpression::VSCALEY},
        {"vscalez", FunctionExpression::VSCALEZ},
        {"minvert", FunctionExpression::MINVERT},
        {"mrotate", FunctionExpression::MROTATE},
        {"mrotatex", FunctionExpression::MROTATEX},
        {"mrotatey", FunctionExpression::MROTATEY},
        {"mrotatez", FunctionExpression::MROTATEZ},
        {"mscale", FunctionExpression::MSCALE},
        {"mtranslate", FunctionExpression::MTRANSLATE},
        {"create", FunctionExpression::CREATE},
        {"list", FunctionExpression::LIST},
        {"matrix", FunctionExpression::MATRIX},
        {"placement", FunctionExpression::PLACEMENT},
        {"rotation", FunctionExpression::ROTATION},
        {"rotationx", FunctionExpression::ROTATIONX},
        {"rotationy", FunctionExpression::ROTATIONY},
        {"rotationz", FunctionExpression::ROTATIONZ},
        {"str", FunctionExpression::STR},
        {"parsequant", FunctionExpression::PARSEQUANT},
        {"translationm", FunctionExpression::TRANSLATIONM},
        {"tuple", FunctionExpression::TUPLE},
        {"vector", FunctionExpression::VECTOR},
        {"address", FunctionExpression::ADDRESS},
        {"hiddenref", FunctionExpression::HIDDENREF},
        {"href", FunctionExpression::HREF},
        {"not", FunctionExpression::NOT},
        {"average", FunctionExpression::AVERAGE},
        {"count", FunctionExpression::COUNT},
        {"max", FunctionExpression::MAX},
        {"min", FunctionExpression::MIN},
        {"stddev", FunctionExpression::STDDEV},
        {"sum", FunctionExpression::SUM},
        {"and", FunctionExpression::AND},
        {"or", FunctionExpression::OR},
    };
    // clang-format on
    return functions;
}

// ---------------------------------------------------------------------
// Numeric value conversion, faithful to the former lexer actions.

double numberValue(std::string_view text)
{
    // Locale-agnostic: one of '.' or ',' is the decimal delimiter, the
    // other is skipped as a group delimiter.
    const bool commaDecimal = text.find(',') != std::string_view::npos;
    const char decimal = commaDecimal ? ',' : '.';
    const char group = commaDecimal ? '.' : ',';

    char temp[40];
    int i = 0;
    for (const char c : text) {
        if (c == group) {
            continue;
        }
        temp[i++] = (c == decimal) ? '.' : c;
        if (i > 39) {
            return 0.0;
        }
    }
    temp[i] = '\0';

    errno = 0;
    const double value = std::strtod(temp, nullptr);
    if (value == 0 && errno == ERANGE) {
        throw Base::UnderflowError("Number underflow.");
    }
    if (value == HUGE_VAL || value == -HUGE_VAL) {
        throw Base::OverflowError("Number overflow.");
    }
    return value;
}

long long integerValue(std::string_view text)
{
    const long long value = std::strtoll(std::string(text).c_str(), nullptr, 10);
    if (value == std::numeric_limits<long long>::min()) {
        throw Base::UnderflowError("Integer underflow");
    }
    if (value == std::numeric_limits<long long>::max()) {
        throw Base::OverflowError("Integer overflow");
    }
    return value;
}

// ---------------------------------------------------------------------
// The parser.

/// Internal control-flow exception for syntax errors; converted to
/// Base::ParserError by the public entry points.
struct SyntaxError
{
};

class Parser
{
public:
    Parser(const App::DocumentObject* owner, std::string_view source)
        : owner(owner)
        , src(source)
    {
        std::size_t pos = 0;
        for (;;) {
            const LexToken t = nextToken(src, pos);
            toks.push_back(t);
            if (t.kind == END) {
                break;
            }
            pos = t.start + t.len;
        }
    }

    /// Parse the whole input.  A complete unit expression is preferred
    /// over a value expression, mirroring the old input rule.
    ExpressionPtr parseInput(bool& unitExpression, bool& valueExpression)
    {
        unitExpression = valueExpression = false;
        const std::size_t save = ip;
        try {
            if (ExpressionPtr unit = tryUnitExpr(); unit && kind() == END) {
                unitExpression = true;
                return unit;
            }
        }
        catch (const SyntaxError&) {
        }
        ip = save;
        ExpressionPtr result = parseExpr();
        if (kind() != END) {
            throw SyntaxError {};
        }
        valueExpression = true;
        return result;
    }

private:
    // -- token access -------------------------------------------------

    const LexToken& tok(std::size_t ahead = 0) const
    {
        const std::size_t i = ip + ahead;
        return i < toks.size() ? toks[i] : toks.back();
    }

    int kind(std::size_t ahead = 0) const
    {
        return tok(ahead).kind;
    }

    std::string_view text(std::size_t ahead = 0) const
    {
        const LexToken& t = tok(ahead);
        return src.substr(t.start, t.len);
    }

    LexToken advance()
    {
        LexToken t = tok();
        if (t.kind != END) {
            ++ip;
        }
        return t;
    }

    void expect(int k)
    {
        if (kind() != k) {
            throw SyntaxError {};
        }
        advance();
    }

    bool accept(int k)
    {
        if (kind() != k) {
            return false;
        }
        advance();
        return true;
    }

    // -- number helpers -----------------------------------------------

    bool isNumberToken(int k) const
    {
        return k == NUM || k == INTEGER;
    }

    /// num: a literal or constant, as NumberExpression/ConstantExpression.
    ExpressionPtr parseNum()
    {
        if (kind() == NUM) {
            const double v = numberValue(text());
            advance();
            return std::make_unique<NumberExpression>(owner, Quantity(v));
        }
        if (kind() == INTEGER) {
            const long long v = integerValue(text());
            advance();
            return std::make_unique<NumberExpression>(owner, Quantity(static_cast<double>(v)));
        }
        if (kind() == CONSTANT) {
            const ConstantEntry* c = lookupConstant(text());
            advance();
            return std::make_unique<ConstantExpression>(owner, c->name, Quantity(c->value));
        }
        throw SyntaxError {};
    }

    /// integer: an integral literal for unit exponents.
    long long parseInteger()
    {
        if (kind() != INTEGER) {
            throw SyntaxError {};
        }
        const long long v = integerValue(text());
        advance();
        return v;
    }

    // -- unit expressions (recursive descent) -------------------------

    ExpressionPtr unitChunk()
    {
        if (kind() == NAME) {
            if (const Quantity* q = lookupUnit(text())) {
                std::string spelled(text());
                advance();
                return std::make_unique<UnitExpression>(owner, *q, spelled);
            }
            throw SyntaxError {};
        }
        if (kind() == UNIT || kind() == USUNIT) {
            const Quantity* q = lookupUnit(text());
            if (!q && kind() == USUNIT) {
                q = text() == "\"" ? &Quantity::Inch : &Quantity::Foot;
            }
            std::string spelled(text());
            advance();
            return std::make_unique<UnitExpression>(owner, *q, spelled);
        }
        if (accept('(')) {
            ExpressionPtr inner = parseUnitExpr();
            expect(')');
            return inner;
        }
        throw SyntaxError {};
    }

    /// unit factor: chunk ('^' [-] integer)*, '^' binding tightest.
    ExpressionPtr unitFactor()
    {
        ExpressionPtr e = unitChunk();
        while (kind() == '^') {
            const std::size_t save = ip;
            advance();
            const bool negative = accept(MINUSSIGN);
            if (kind() != INTEGER && !isIntegerOne()) {
                ip = save;
                break;
            }
            const double v = static_cast<double>(parseIntegerOrOne());
            ExpressionPtr exponent = std::make_unique<NumberExpression>(owner, Quantity(v));
            if (negative) {
                exponent = std::make_unique<OperatorExpression>(
                    owner,
                    exponent.release(),
                    OperatorExpression::NEG,
                    new NumberExpression(owner, Quantity(-1)));
            }
            e = std::make_unique<OperatorExpression>(owner,
                                                     e.release(),
                                                     OperatorExpression::POW,
                                                     exponent.release());
        }
        return e;
    }

    bool isIntegerOne() const
    {
        // ONE is a runtime refinement; here an INTEGER token is enough.
        return kind() == INTEGER;
    }

    long long parseIntegerOrOne()
    {
        return parseInteger();
    }

    /// unit_exp: factor (('*'|'/') factor)*, extending only while the
    /// next operand parses as a unit (otherwise the operator is left
    /// for the surrounding value expression).
    ExpressionPtr parseUnitExpr()
    {
        ExpressionPtr e = unitFactor();
        for (;;) {
            const int op = kind();
            if (op != '*' && op != '/') {
                break;
            }
            const std::size_t save = ip;
            advance();
            ExpressionPtr rhs;
            try {
                rhs = unitFactor();
            }
            catch (const SyntaxError&) {
                ip = save;
                break;
            }
            e = std::make_unique<OperatorExpression>(
                owner,
                e.release(),
                op == '*' ? OperatorExpression::MUL : OperatorExpression::DIV,
                rhs.release());
        }
        return e;
    }

    /// Attempt a unit expression; restores the position and returns
    /// nullptr if the current token cannot start one.
    ExpressionPtr tryUnitExpr()
    {
        const std::size_t save = ip;
        try {
            return parseUnitExpr();
        }
        catch (const SyntaxError&) {
            ip = save;
            return nullptr;
        }
    }

    // -- value expressions (precedence climbing) ----------------------

    ExpressionPtr parseExpr()
    {
        return parseTernary();
    }

    ExpressionPtr parseTernary()
    {
        ExpressionPtr cond = parseComparison();
        if (!accept('?')) {
            return cond;
        }
        ExpressionPtr onTrue = parseExpr();
        expect(':');
        ExpressionPtr onFalse = parseTernary();  // right associative
        return std::make_unique<ConditionalExpression>(owner,
                                                       cond.release(),
                                                       onTrue.release(),
                                                       onFalse.release());
    }

    ExpressionPtr parseComparison()
    {
        ExpressionPtr e = parseAdditive();
        for (;;) {
            OperatorExpression::Operator op {};
            switch (kind()) {
                case EQ:
                    op = OperatorExpression::EQ;
                    break;
                case NEQ:
                    op = OperatorExpression::NEQ;
                    break;
                case LT:
                    op = OperatorExpression::LT;
                    break;
                case GT:
                    op = OperatorExpression::GT;
                    break;
                case GTE:
                    op = OperatorExpression::GTE;
                    break;
                case LTE:
                    op = OperatorExpression::LTE;
                    break;
                default:
                    return e;
            }
            advance();
            ExpressionPtr rhs = parseAdditive();
            e = std::make_unique<OperatorExpression>(owner, e.release(), op, rhs.release());
        }
    }

    ExpressionPtr parseAdditive()
    {
        ExpressionPtr e = parseMultiplicative();
        for (;;) {
            OperatorExpression::Operator op {};
            if (kind() == '+') {
                op = OperatorExpression::ADD;
            }
            else if (kind() == MINUSSIGN) {
                op = OperatorExpression::SUB;
            }
            else {
                return e;
            }
            advance();
            ExpressionPtr rhs = parseMultiplicative();
            e = std::make_unique<OperatorExpression>(owner, e.release(), op, rhs.release());
        }
    }

    ExpressionPtr parseMultiplicative()
    {
        ExpressionPtr e = parsePower();
        for (;;) {
            const int k = kind();
            if (k != '*' && k != '/' && k != '%') {
                return e;
            }
            advance();
            ExpressionPtr rhs;
            if (k == '/') {
                // exp '/' unit_exp is preferred over exp '/' exp.
                rhs = tryUnitExpr();
            }
            if (!rhs) {
                rhs = parsePower();
            }
            const OperatorExpression::Operator op = (k == '*') ? OperatorExpression::MUL
                : (k == '/')                                   ? OperatorExpression::DIV
                                                               : OperatorExpression::MOD;
            e = std::make_unique<OperatorExpression>(owner, e.release(), op, rhs.release());
        }
    }

    ExpressionPtr parsePower()
    {
        ExpressionPtr e = parseUnary();
        while (accept('^')) {
            ExpressionPtr rhs = parseUnary();  // left associative: (a^b)^c
            e = std::make_unique<OperatorExpression>(owner,
                                                     e.release(),
                                                     OperatorExpression::POW,
                                                     rhs.release());
        }
        return e;
    }

    ExpressionPtr parseUnary()
    {
        if (accept(MINUSSIGN)) {
            ExpressionPtr operand = parseUnary();
            return std::make_unique<OperatorExpression>(owner,
                                                        operand.release(),
                                                        OperatorExpression::NEG,
                                                        new NumberExpression(owner, Quantity(-1)));
        }
        if (accept('+')) {
            ExpressionPtr operand = parseUnary();
            return std::make_unique<OperatorExpression>(owner,
                                                        operand.release(),
                                                        OperatorExpression::POS,
                                                        new NumberExpression(owner, Quantity(1)));
        }
        return parsePrimary();
    }

    // -- primaries ----------------------------------------------------

    ExpressionPtr parsePrimary()
    {
        switch (kind()) {
            case NUM:
            case INTEGER:
            case CONSTANT:
                return parseQuantityOrNumber();
            case FUNC:
                return parseFunctionCall();
            case STRING:
                if (kind(1) == '#' || kind(1) == '.') {
                    return parseIdentifierExpression();
                }
                {
                    std::string s = unquote(std::string(text()));
                    advance();
                    return std::make_unique<StringExpression>(owner, s);
                }
            case NAME:
            case CELLADDRESS:
            case '.':
                return parseIdentifierExpression();
            case '(': {
                advance();
                ExpressionPtr inner = parseExpr();
                expect(')');
                return inner;
            }
            default:
                throw SyntaxError {};
        }
    }

    /// A numeric literal, optionally with a juxtaposed unit expression:
    /// unit_num: num unit_exp | num us_unit num us_unit
    ExpressionPtr parseQuantityOrNumber()
    {
        ExpressionPtr number = parseNum();

        // Imperial pair: 1' 2"
        if (kind() == USUNIT && isNumberToken(kind(1)) && kind(2) == USUNIT) {
            ExpressionPtr unit1 = unitChunk();
            ExpressionPtr number2 = parseNum();
            ExpressionPtr unit2 = unitChunk();
            auto* left = new OperatorExpression(owner,
                                                number.release(),
                                                OperatorExpression::UNIT,
                                                unit1.release());
            auto* right = new OperatorExpression(owner,
                                                 number2.release(),
                                                 OperatorExpression::UNIT,
                                                 unit2.release());
            return std::make_unique<OperatorExpression>(owner,
                                                        left,
                                                        OperatorExpression::ADD,
                                                        right);
        }

        if (ExpressionPtr unit = tryUnitExpr()) {
            return std::make_unique<OperatorExpression>(owner,
                                                        number.release(),
                                                        OperatorExpression::UNIT,
                                                        unit.release());
        }
        return number;
    }

    /// FUNC args ')'
    ExpressionPtr parseFunctionCall()
    {
        // The FUNC token spells "name [\t]* (".
        std::string_view spelled = text();
        advance();
        spelled.remove_suffix(1);
        while (!spelled.empty()
               && (spelled.back() == '\t' || spelled.back() == '\x10' || spelled.back() == ' ')) {
            spelled.remove_suffix(1);
        }
        FunctionExpression::Function func = FunctionExpression::NONE;
        std::string name;
        const auto& functions = registeredFunctions();
        if (auto found = functions.find(spelled); found != functions.end()) {
            func = found->second;
        }
        else {
            name = std::string(spelled);
        }

        std::vector<Expression*> args;
        try {
            args.push_back(parseArgument().release());
            while (accept(',') || accept(';')) {
                args.push_back(parseArgument().release());
            }
            expect(')');
        }
        catch (...) {
            for (Expression* arg : args) {
                delete arg;
            }
            throw;
        }
        return std::make_unique<FunctionExpression>(owner, func, std::move(name), std::move(args));
    }

    /// args element: range | exp
    ExpressionPtr parseArgument()
    {
        if ((kind() == NAME || kind() == CELLADDRESS) && kind(1) == ':') {
            std::string begin(text());
            advance();
            advance();
            if (kind() != NAME && kind() != CELLADDRESS) {
                throw SyntaxError {};
            }
            std::string end(text());
            advance();
            return std::make_unique<RangeExpression>(owner, begin, end);
        }
        return parseExpr();
    }

    // -- identifiers and paths ----------------------------------------

    bool isIdOrCell(int k) const
    {
        return k == NAME || k == CELLADDRESS;
    }

    std::string parseIdOrCell()
    {
        if (!isIdOrCell(kind())) {
            throw SyntaxError {};
        }
        std::string s(text());
        advance();
        return s;
    }

    std::string parseStringLiteral()
    {
        if (kind() != STRING) {
            throw SyntaxError {};
        }
        std::string s = unquote(std::string(text()));
        advance();
        return s;
    }

    /// identifier / iden: an object path, ported rule by rule from the
    /// former grammar actions.
    ObjectIdentifier parsePath()
    {
        // '.' STRING '.' id_or_cell  |  '.' id_or_cell
        if (accept('.')) {
            if (kind() == STRING) {
                ObjectIdentifier path(owner, true);
                std::string sub = parseStringLiteral();
                path.setDocumentObjectName(owner,
                                           false,
                                           ObjectIdentifier::String(std::move(sub), true),
                                           true);
                expect('.');
                path.addComponent(ObjectIdentifier::SimpleComponent(parseIdOrCell()));
                return finishPath(std::move(path));
            }
            ObjectIdentifier path(owner, true);
            path.setDocumentObjectName(owner);
            path.addComponent(ObjectIdentifier::SimpleComponent(parseIdOrCell()));
            return finishPath(std::move(path));
        }

        // document '#' ...
        if ((kind() == NAME || kind() == STRING) && kind(1) == '#') {
            ObjectIdentifier::String document = (kind() == STRING)
                ? ObjectIdentifier::String(parseStringLiteral(), true)
                : ObjectIdentifier::String(parseIdOrCell(), false, true);
            expect('#');
            ObjectIdentifier::String object = parseObjectName();
            expect('.');
            ObjectIdentifier path(owner);
            path.setDocumentName(std::move(document), true);
            if (kind() == STRING) {
                std::string sub = parseStringLiteral();
                path.setDocumentObjectName(std::move(object),
                                           true,
                                           ObjectIdentifier::String(std::move(sub), true));
                expect('.');
            }
            else {
                path.setDocumentObjectName(std::move(object), true);
            }
            path.addComponent(ObjectIdentifier::SimpleComponent(parseIdOrCell()));
            path.resolveAmbiguity();
            return finishPath(std::move(path));
        }

        // object '.' ...
        if ((isIdOrCell(kind()) || kind() == STRING) && kind(1) == '.') {
            ObjectIdentifier::String object = parseObjectName();
            expect('.');
            ObjectIdentifier path(owner);
            if (kind() == STRING) {
                // object '.' STRING '.' id_or_cell
                std::string sub = parseStringLiteral();
                path.setDocumentObjectName(std::move(object),
                                           true,
                                           ObjectIdentifier::String(std::move(sub), true),
                                           true);
                expect('.');
                path.addComponent(ObjectIdentifier::SimpleComponent(parseIdOrCell()));
            }
            else {
                // object '.' id_or_cell
                object.checkImport(owner);
                path.addComponent(ObjectIdentifier::SimpleComponent(std::move(object)));
                path.addComponent(ObjectIdentifier::SimpleComponent(parseIdOrCell()));
            }
            path.resolveAmbiguity();
            return finishPath(std::move(path));
        }

        // plain id_or_cell
        ObjectIdentifier path(owner);
        path << ObjectIdentifier::SimpleComponent(parseIdOrCell());
        return path;
    }

    ObjectIdentifier::String parseObjectName()
    {
        if (kind() == STRING) {
            return ObjectIdentifier::String(parseStringLiteral(), true);
        }
        return ObjectIdentifier::String(parseIdOrCell(), false);
    }

    /// iden '.' IDENTIFIER chain.
    ObjectIdentifier finishPath(ObjectIdentifier path)
    {
        while (kind() == '.' && kind(1) == NAME) {
            advance();
            path.addComponent(ObjectIdentifier::SimpleComponent(parseIdOrCell()));
        }
        return path;
    }

    /// identifier as an expression, with the indexable postfix chain:
    /// indexable: identifier indexer | indexable indexer | indexable '.' IDENTIFIER
    ExpressionPtr parseIdentifierExpression()
    {
        ObjectIdentifier path = parsePath();
        auto var = std::make_unique<VariableExpression>(owner, std::move(path));
        if (kind() != '[') {
            return var;
        }
        var->addComponent(parseIndexer());
        for (;;) {
            if (kind() == '[') {
                var->addComponent(parseIndexer());
            }
            else if (kind() == '.' && kind(1) == NAME) {
                advance();
                var->addComponent(Expression::createComponent(parseIdOrCell()));
            }
            else {
                break;
            }
        }
        return var;
    }

    /// indexer: '[' ... ']', ported form by form from the old grammar:
    ///   [e]      createComponent(e)         [e:]     createComponent(e,0,0,true)
    ///   [:e]     createComponent(0,e)       [::e]    createComponent(0,0,e)
    ///   [e:e]    createComponent(e,e2)      [e::e]   createComponent(e,0,e3)
    ///   [:e:e]   createComponent(0,e2,e3)   [e:e:e]  createComponent(e,e2,e3)
    Expression::Component* parseIndexer()
    {
        expect('[');
        ExpressionPtr e1;
        ExpressionPtr e2;
        ExpressionPtr e3;
        if (kind() != ':') {
            e1 = parseExpr();
        }
        const bool colon1 = accept(':');
        if (colon1 && kind() != ':' && kind() != ']') {
            e2 = parseExpr();
        }
        const bool colon2 = colon1 && accept(':');
        if (colon2 && kind() != ']') {
            e3 = parseExpr();
        }
        expect(']');
        if (!colon1) {
            if (!e1) {
                throw SyntaxError {};
            }
            return Expression::createComponent(e1.release());
        }
        if (colon2 && !e3) {
            throw SyntaxError {};  // no [e:e:] or [::] forms
        }
        if (!colon2 && !e1 && !e2) {
            throw SyntaxError {};  // no [:] form
        }
        if (!colon2 && e1 && !e2) {
            return Expression::createComponent(e1.release(), nullptr, nullptr, true);
        }
        return Expression::createComponent(e1.release(), e2.release(), e3.release());
    }

    const App::DocumentObject* owner;
    std::string_view src;
    std::vector<LexToken> toks;
    std::size_t ip {0};
};

}  // namespace

// ---------------------------------------------------------------------
// Entry points.

ExpressionPtr parseImpl(const App::DocumentObject* owner,
                        const char* buffer,
                        bool& unitExpression,
                        bool& valueExpression)
{
    Parser parser(owner, buffer);
    try {
        return parser.parseInput(unitExpression, valueExpression);
    }
    catch (const SyntaxError&) {
        return nullptr;
    }
}

std::vector<std::tuple<int, int, std::string>> tokenize(const std::string& str)
{
    std::vector<std::tuple<int, int, std::string>> result;
    std::size_t pos = 0;
    try {
        for (;;) {
            const LexToken t = nextToken(str, pos);
            if (t.kind == END) {
                break;
            }
            int kind = t.kind;
            const std::string_view text = std::string_view(str).substr(t.start, t.len);
            if (kind == NUM) {
                kind = refineOne(kind, numberValue(text));
            }
            else if (kind == INTEGER) {
                kind = refineOne(kind, static_cast<double>(integerValue(text)));
            }
            else if (kind == NAME && lookupUnit(text)) {
                // The parser decides unit vs. identifier by context, but
                // tokenize() consumers (highlighting, completion) keep
                // the historical lexical classification.
                kind = UNIT;
            }
            result.emplace_back(kind, static_cast<int>(t.start), std::string(text));
            pos = t.start + t.len;
        }
    }
    catch (...) {
        // Ignore all exceptions; return the tokens scanned so far.
    }
    return result;
}

namespace
{

/// Scan str as exactly one token; kind END if empty or more than one.
LexToken singleToken(const std::string& str)
{
    const LexToken t = nextToken(str, 0);
    if (t.kind == END || nextToken(str, t.start + t.len).kind != END) {
        return {END, 0, 0};
    }
    return t;
}

}  // namespace

bool isTokenAnIndentifier(const std::string& str)
{
    const LexToken t = singleToken(str);
    if (t.kind == CELLADDRESS) {
        return true;
    }
    // The old lexer classified unit names and constants as their own
    // token kinds, so they were not identifiers.
    return t.kind == NAME && !lookupUnit(str) && !lookupConstant(str);
}

bool isTokenAConstant(const std::string& str)
{
    return singleToken(str).kind == CONSTANT;
}

bool isTokenAUnit(const std::string& str)
{
    const LexToken t = singleToken(str);
    if (t.kind == UNIT) {
        return true;
    }
    return t.kind == NAME && lookupUnit(str) != nullptr;
}

}  // namespace App::ExpressionParser
