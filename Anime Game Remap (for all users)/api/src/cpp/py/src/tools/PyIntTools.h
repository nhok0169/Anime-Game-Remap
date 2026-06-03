#ifndef PyIntTools_H
#define PyIntTools_H

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