#ifndef AGRemapPyBind_PyRemapStats_H
#define AGRemapPyBind_PyRemapStats_H

#include <pybind11/pybind11.h>

#include "PyCachedFileStats.h"
#include "PyFileStats.h"

// Python-facing counterpart to AGRemapCore::RemapStats -- a plain aggregate, but with every member
// typed as PyFileStats/PyCachedFileStats (NOT the raw AGRemapCore::FileStats/CachedFileStats),
// since those are the ones with usable Python-facing exception storage -- see PyFileStats.h's own
// note. This is therefore its own standalone class here, not a binding of AGRemapCore::RemapStats
// itself.
class PyRemapStats {
    public:
        PyFileStats blend;
        PyFileStats position;
        PyFileStats texcoord;
        PyFileStats buf;
        PyFileStats other;
        PyFileStats ini;
        PyFileStats texEdit;
        PyFileStats texAdd;
        PyCachedFileStats download;

        void clear();
};

void initCppRemapStats(pybind11::module_ &m);

#endif
