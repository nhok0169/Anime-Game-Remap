#ifndef AGRemapPyBind_PyBaseLogger_H
#define AGRemapPyBind_PyBaseLogger_H

#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "AGRemapCore/view/BaseLogger.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_ trampoline for a :cpp:class:`AGRemapCore::BaseLogger` subclass, letting a
 `Python`_ class override any of the logger's virtual methods and have *C++* callers reach that
 override :raw-html:`<br />` :raw-html:`<br />`

 A class template rather than one concrete trampoline because two bound classes need the exact
 same set of overrides: :cpp:class:`AGRemapCore::BaseLogger` itself (``BaseLogger``, abstract --
 a `Python`_ subclass *must* supply ``write``/``read``) and :cpp:class:`PyLogger` (``Logger``,
 concrete -- a `Python`_ subclass *may* override either to redirect the console output). The only
 difference between the two is what happens when no `Python`_ override exists for one of the two
 pure virtuals, which ``std::is_abstract_v<LoggerBase>`` decides at compile time.

 Every method here is the standard ``PYBIND11_OVERRIDE`` shape, spelled out via
 ``PYBIND11_OVERRIDE_IMPL`` only so the pure-virtual failure message names ``BaseLogger`` rather
 than the template parameter.

 @tparam LoggerBase The bound C++ class being made overridable from `Python`_
 @endrst
 */
template <typename LoggerBase>
class PyBindLoggerT: public LoggerBase, public py::trampoline_self_life_support {
    public:
        using LoggerBase::LoggerBase;

        void write(const std::string& message) override {
            PYBIND11_OVERRIDE_IMPL(void, LoggerBase, "write", message);

            if constexpr (std::is_abstract_v<LoggerBase>) {
                py::pybind11_fail("Tried to call pure virtual function \"BaseLogger::write\"");
            } else {
                return LoggerBase::write(message);
            }
        }

        std::string read(const std::string& desc) override {
            PYBIND11_OVERRIDE_IMPL(std::string, LoggerBase, "read", desc);

            if constexpr (std::is_abstract_v<LoggerBase>) {
                py::pybind11_fail("Tried to call pure virtual function \"BaseLogger::read\"");
            } else {
                return LoggerBase::read(desc);
            }
        }

        std::string getStr(const std::string& message) const override {
            PYBIND11_OVERRIDE(std::string, LoggerBase, getStr, message);
        }

        void log(const std::string& message) override {
            PYBIND11_OVERRIDE(void, LoggerBase, log, message);
        }

        void split() override {
            PYBIND11_OVERRIDE(void, LoggerBase, split);
        }

        void space() override {
            PYBIND11_OVERRIDE(void, LoggerBase, space);
        }

        void openHeading(const std::string& txt, std::size_t sideLen, const std::string& headingChar) override {
            PYBIND11_OVERRIDE(void, LoggerBase, openHeading, txt, sideLen, headingChar);
        }

        void closeHeading() override {
            PYBIND11_OVERRIDE(void, LoggerBase, closeHeading);
        }

        void bulletPoint(const std::string& txt) override {
            PYBIND11_OVERRIDE(void, LoggerBase, bulletPoint, txt);
        }

        void list(const std::vector<std::string>& lst, const AGRC::BaseLogger::Transform& transform) override {
            PYBIND11_OVERRIDE(void, LoggerBase, list, lst, transform);
        }

        void box(const std::string& message, const std::string& header) override {
            PYBIND11_OVERRIDE(void, LoggerBase, box, message, header);
        }

        void error(const std::string& message) override {
            PYBIND11_OVERRIDE(void, LoggerBase, error, message);
        }

        std::string input(const std::string& desc) override {
            PYBIND11_OVERRIDE(std::string, LoggerBase, input, desc);
        }

        void waitExit() override {
            PYBIND11_OVERRIDE(void, LoggerBase, waitExit);
        }
};


/**
 * @brief
 @rst
 The trampoline for the abstract ``BaseLogger`` itself
 @endrst
 */
using PyBindBaseLogger = PyBindLoggerT<AGRC::BaseLogger>;


void initCppBaseLogger(pybind11::module_ &m);

#endif
