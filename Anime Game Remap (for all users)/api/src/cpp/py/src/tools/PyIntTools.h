#ifndef AGRemapPyBind_PyIntTools_H
#define AGRemapPyBind_PyIntTools_H

// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include <tuple>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "AGRemapCore/tools/IntTools.h"


namespace AGRC = AGRemapCore;


class PyIntTools: public AGRC::IntTools {
    public:
        static std::tuple<std::vector<unsigned int>, bool> pyInttoBase(long long num, unsigned int base);
        static std::string pyInttoStrBase(long long num, unsigned int base, const std::vector<std::string>& getDigit, const std::string& negativeChar);
        static std::string pyInttoBase64(long long num, const std::optional<std::vector<std::string>>& getDigit = std::nullopt, const std::string& negativeChar = NEGATIVE_STR);
};


void initCppIntTools(pybind11::module_ &m);

#endif