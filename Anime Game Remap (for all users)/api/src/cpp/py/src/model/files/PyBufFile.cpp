#include "PyBufFile.h"

#include <memory>
#include <optional>
#include <string>

#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/files/BufFile.h"
#include "AGRemapCore/model/files/BufFileErrors.h"
#include "../buffers/PyBufBytes.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


std::vector<std::unique_ptr<AGRC::BufElementType>> parseBufElementTypes(const py::object &elements) {
    // Clones each incoming BufElementType (a real C++ copy, deep-cloning its own BufDataTypes in
    // turn) rather than taking ownership of the exact Python object -- see PyBufElementType.cpp's
    // parseBufDataTypes for the identical reasoning one hierarchy level down. A BufElementType is
    // just as shareable in this codebase's real usage: constants/BufElementTypes.py caches each
    // element once behind a DeferredEnum (eg. 'BufElementTypes.BlendWeightFloatRGBA.value'), and a
    // caller can hold one shared 'elements' list config (eg. RemapBlendResource.blendElements)
    // that gets passed into more than one BufFile/BlendFile construction over its lifetime.
    std::vector<std::unique_ptr<AGRC::BufElementType>> result;
    for (auto item : elements) {
        py::object element = py::reinterpret_borrow<py::object>(item);
        const AGRC::BufElementType &ref = element.cast<const AGRC::BufElementType&>();
        result.push_back(std::make_unique<AGRC::BufElementType>(ref));
    }
    return result;
}


namespace {

    // The fully-qualified dotted path to this 'core' module's own parent package -- derived once
    // (see initCppBufFile) the same way PyBaseTokenizer.cpp's syntaxErrModulePath() is, and for
    // the exact same reason (this repo's Unit Tester harness imports the package as
    // 'src.py.FixRaidenBoss2' rather than a top-level 'FixRaidenBoss2').
    std::string &bufFileErrorsParentPackage() {
        static std::string path;
        return path;
    }

    [[noreturn]] void raisePyBadBufData(const AGRC::BadBufData &e) {
        py::object cls = py::module_::import((bufFileErrorsParentPackage() + ".exceptions.BadBufData").c_str()).attr("BadBufData");
        py::object excInstance = cls(py::arg("fileType") = py::cast(e.fileType()));
        PyErr_SetObject(cls.ptr(), excInstance.ptr());
        throw py::error_already_set();
    }

    [[noreturn]] void raisePyBufFileNotRecognized(const AGRC::BufFileNotRecognized &e) {
        py::object cls = py::module_::import((bufFileErrorsParentPackage() + ".exceptions.BufFileNotRecognized").c_str()).attr("BufFileNotRecognized");
        py::object excInstance = cls(py::cast(e.filePath()), py::arg("fileType") = py::cast(e.fileType()));
        PyErr_SetObject(cls.ptr(), excInstance.ptr());
        throw py::error_already_set();
    }

    // Wraps any call that can construct/read a BufFile, translating the two plain, Python-free
    // core exceptions into the exact real Python exception classes real callers already catch by
    // name -- mirrors PyBaseTokenizer.cpp's callSimplifiedMaximalMunch.
    template <typename Func>
    auto translateBufFileErrors(Func &&func) -> decltype(func()) {
        try {
            return func();
        } catch (const AGRC::BadBufData &e) {
            raisePyBadBufData(e);
        } catch (const AGRC::BufFileNotRecognized &e) {
            raisePyBufFileNotRecognized(e);
        }
    }

    // One column -> one NumPy array of the matching dtype (int64/uint64/float64), filled straight
    // from the core's contiguous buffer. Nothing is boxed per value, which is the entire point of
    // going through decodeAll rather than calling decodeLine once per line.
    py::object columnToPy(const AGRC::BufFile::BufColumn &column) {
        return std::visit([](const auto &values) -> py::object {
            using T = typename std::decay_t<decltype(values)>::value_type;
            return py::array_t<T>(static_cast<py::ssize_t>(values.size()), values.data());
        }, column);
    }

    // The inverse: one NumPy array -> one core column, keeping the array's own dtype kind so an
    // integer column stays integral the whole way down (see BufTools.fromDataFrame's own note on
    // why a whole-frame conversion would corrupt a Blend.buf's indices). 'forcecast' materialises
    // a copy for the cases that need one -- a sliced/non-contiguous column, or an int32 one.
    AGRC::BufFile::BufColumn pyToColumn(const py::handle &values) {
        py::array array = py::reinterpret_borrow<py::object>(values).cast<py::array>();
        char kind = array.dtype().kind();

        if (kind == 'u') {
            auto typed = array.cast<py::array_t<unsigned long long, py::array::c_style | py::array::forcecast>>();
            return std::vector<unsigned long long>(typed.data(), typed.data() + typed.size());
        }

        if (kind == 'i' || kind == 'b') {
            auto typed = array.cast<py::array_t<long long, py::array::c_style | py::array::forcecast>>();
            return std::vector<long long>(typed.data(), typed.data() + typed.size());
        }

        // Floats, and anything else (most realistically an 'object' column of plain Python
        // numbers), go through double: an integer data type's own encode truncates back to an
        // integer anyway, whereas forcing to an integer here would throw a real fractional value
        // away for good.
        auto typed = array.cast<py::array_t<double, py::array::c_style | py::array::forcecast>>();
        return std::vector<double>(typed.data(), typed.data() + typed.size());
    }

}


AGRC::BufFile::Filter parseFilter(const py::function &filter) {
    return [filter](const AGRC::BufLineData &data, long long startInd, double lineInd, long long lineSize) {
        py::gil_scoped_acquire gil;
        py::object result = filter(data, startInd, lineInd, lineSize);
        return result.cast<AGRC::BufLineData>();
    };
}

std::vector<AGRC::BufFile::Filter> parseFilters(const std::vector<py::function> &filters) {
    std::vector<AGRC::BufFile::Filter> result;
    result.reserve(filters.size());
    for (const auto &filter : filters) {
        result.push_back(parseFilter(filter));
    }
    return result;
}

py::object fixResultToPy(const AGRC::BufFile::FixResult &result) {
    if (std::holds_alternative<std::string>(result)) {
        return py::cast(std::get<std::string>(result));
    }
    return vecToPyByteArray(std::get<AGRC::ByteVec>(result));
}


AGRC::BufFile::FixResult pyToFixResult(const py::object &result) {
    if (py::isinstance<py::str>(result)) {
        return AGRC::BufFile::FixResult(result.cast<std::string>());
    }

    py::buffer buf = result.cast<py::buffer>();
    py::buffer_info info = buf.request();
    const auto *data = static_cast<const std::uint8_t*>(info.ptr);
    return AGRC::BufFile::FixResult(AGRC::ByteVec(data, data + info.size));
}


void initCppBufFile(pybind11::module_ &m) {
    std::string coreModuleName = m.attr("__name__").cast<std::string>();
    std::string parentPackage = coreModuleName;
    size_t lastDot = parentPackage.rfind('.');
    if (lastDot != std::string::npos) {
        parentPackage.erase(lastDot);
    } else {
        parentPackage.clear();
    }
    bufFileErrorsParentPackage() = parentPackage;

    py::class_<AGRC::BufFile, AGRC::BinaryFile, py::smart_holder>(m, "CppBufFile", R"doc(
This class inherits from :class:`BinaryFile`

A class to handle ``.buf`` files

A ``.buf`` file is a binary file made up of a sequence of same-sized "lines" (one line per vertex),
each one composed of the same sequence of :class:`BufElementType`\s -- there is no header or
footer, just the lines themselves back-to-back
    )doc")

        .def(py::init([](py::object src, py::object elements, std::string fileType) {
            return translateBufFileErrors([&]() {
                return std::make_unique<AGRC::BufFile>(pyToBinarySrc(src), parseBufElementTypes(elements), std::move(fileType));
            });
        }), py::arg("src"), py::arg("elements"), py::arg("fileType") = "Buffer", py::doc(R"doc(
Constructs a new ``.buf`` file and immediately reads it

Parameters
----------
src: Union[:class:`str`, :class:`bytes`]
    The source file or bytes for the ``.buf`` file

elements: List[:class:`BufElementType`]
    The sequence of elements within the ``.buf`` file -- each is cloned, so the same passed-in
    instance can safely be reused for other ``.buf`` files afterward

fileType: :class:`str`
    The name for the type of ``.buf`` file. **Default**: ``"Buffer"``

Raises
------
:class:`BufFileNotRecognized`
    If 'src' holds a file path that cannot be read as a valid ``.buf`` file of this format

:class:`BadBufData`
    If 'src' holds raw bytes that are not valid for this format
        )doc"))

        .def_property("fileType", &AGRC::BufFile::getFileType, &AGRC::BufFile::setFileType, py::doc(R"doc(
:class:`str`: The name for the type of ``.buf`` file
        )doc"))

        .def_property("elements", [](AGRC::BufFile &self) {
            std::vector<AGRC::BufElementType*> result;
            for (const auto &element : self.getElements()) {
                result.push_back(element.get());
            }
            return result;
        }, [](AGRC::BufFile &self, py::object elements) {
            self.setElements(parseBufElementTypes(elements));
        }, py::return_value_policy::reference_internal, py::doc(R"doc(
List[:class:`BufElementType`]: The sequence of elements within the ``.buf`` file

Assigning a new list clones each new element the same way the constructor's own ``elements``
parameter does
        )doc"))

        .def_property_readonly("bytesPerLine", &AGRC::BufFile::getBytesPerLine, py::doc(R"doc(
:class:`int`: The number of bytes per line (per vertex)
        )doc"))

        .def("isValid", &AGRC::BufFile::isValid, py::doc(R"doc(
Whether the size of the data is divisible by the # of bytes per line

Returns
-------
:class:`bool`
    Whether the provided data for the ``.buf`` file is valid
        )doc"))

        .def("read", [](AGRC::BufFile &self) {
            return translateBufFileErrors([&]() {
                return vecToPyBytes(self.read());
            });
        }, py::doc(R"doc(
Reads the bytes in the ``.buf`` file

Returns
-------
:class:`bytes`
    The read bytes

Raises
------
:class:`BufFileNotRecognized`
    If :attr:`src` holds a file path that cannot be read as a valid ``.buf`` file of this format

:class:`BadBufData`
    If :attr:`src` holds raw bytes that are not valid for this format
        )doc"))

        .def("decodeLine", [](const AGRC::BufFile &self, const py::bytes &src) {
            return self.decodeLine(pyBytesToVec(src));
        }, py::arg("src"), py::doc(R"doc(
Decodes a line (a vertex) within the ``.buf`` file

Parameters
----------
src: :class:`bytes`
    The source bytes to decode

Returns
-------
Dict[:class:`str`, List[Any]]
    The decoded values for the line

    The keys are the names to the elements and the values are what is decoded
        )doc"))

        .def("encodeLine", [](const AGRC::BufFile &self, const AGRC::BufLineData &src) {
            return vecToPyBytes(self.encodeLine(src));
        }, py::arg("src"), py::doc(R"doc(
Encodes the data about a vertex to their corresponding bytes for the line

Parameters
----------
src: Dict[:class:`str`, List[Any]]
    The corresponding data for the vertex

    The keys are the names for the elements and the values are the data for the elements

Returns
-------
:class:`bytes`
    The encoded bytes for the line
        )doc"))

        .def("decodeAll", [](const AGRC::BufFile &self) {
            py::dict result;
            for (const auto &column : self.decodeAll()) {
                result[py::make_tuple(py::str(column.elementKey), column.valueInd)] = columnToPy(column.values);
            }
            return result;
        }, py::doc(R"doc(
Decodes the whole ``.buf`` file at once, column by column -- the bulk counterpart to
:meth:`decodeLine`

Where :meth:`decodeLine` builds a fresh dict per line, this decodes every line in C++ and hands
back one `NumPy`_ array per column, so a whole file costs a single crossing into C++ instead of one
per line. :meth:`BufTools.toDataFrame` is built on this

.. note::
    Each array's dtype follows the data type it came from -- ``int64`` for a signed integer,
    ``uint64`` for an unsigned one and ``float64`` for a `floating point`_ one -- so an integer
    element stays integral rather than being widened to a float

Returns
-------
Dict[Tuple[:class:`str`, :class:`int`], `numpy.ndarray`_]
    One entry per column, keyed by ``(elementKey, indexWithinElement)``, each holding that
    column's value for every line in line order
        )doc"))

        .def("encodeAll", [](AGRC::BufFile &self, const py::dict &columns) {
            std::vector<AGRC::BufFile::BufColumnData> parsed;
            parsed.reserve(columns.size());

            for (auto item : columns) {
                py::tuple key = py::reinterpret_borrow<py::object>(item.first).cast<py::tuple>();

                AGRC::BufFile::BufColumnData column;
                column.elementKey = py::str(key[0]).cast<std::string>();
                column.valueInd = key[1].cast<std::size_t>();
                column.values = pyToColumn(item.second);

                parsed.push_back(std::move(column));
            }

            translateBufFileErrors([&]() {
                self.encodeAll(parsed);
                return 0;
            });
        }, py::arg("columns"), py::doc(R"doc(
Encodes whole columns back into the ``.buf`` file's bytes -- the inverse of :meth:`decodeAll`, and
the bulk counterpart to :meth:`encodeLine`. :meth:`BufTools.fromDataFrame` is built on this

The columns are matched to the file's current :attr:`elements` by their key, so their order does not
matter; a column the file has no data type for is ignored, and a data type with no matching column
encodes as 0. The number of lines produced is the longest column's length

.. note::
    :attr:`data` cannot be assigned directly, so this sets :attr:`src` to the newly encoded bytes
    and re-reads from it -- a ``.buf`` file originally constructed from a file path therefore ends
    up with raw bytes as its :attr:`src`, and the file on disk is untouched

Parameters
----------
columns: Dict[Tuple[:class:`str`, :class:`int`], `numpy.ndarray`_]
    The columns to encode, as produced by :meth:`decodeAll`

Raises
------
:class:`BadBufData`
    If the encoded bytes do not divide evenly into lines for the file's current :attr:`elements`
        )doc"))

        .def("merge", [](AGRC::BufFile &self, const std::vector<AGRC::BufFile*> &bufFiles) {
            std::vector<const AGRC::BufFile*> sources(bufFiles.begin(), bufFiles.end());
            translateBufFileErrors([&]() {
                self.merge(sources);
                return 0;
            });
        }, py::arg("bufFiles"), py::doc(R"doc(
Merges several other ``.buf`` files into this one, line by line

A GI character's vertex buffer does not live in one file -- it is split across a ``Position.buf``,
a ``Blend.buf`` and a ``Texcoord.buf``, one line each per vertex. This stitches such a set back
together: line *i* of the result is line *i* of every source concatenated in the order given, and
this file's :attr:`elements` becomes every source's elements in that same order

.. note::
    The sources are left untouched -- their elements are deep-copied in, so each stays usable
    afterwards

.. note::
    The number of lines produced is the **smallest** line count among the sources, so a ragged set
    truncates rather than reading past the end of the shortest file

.. note::
    :attr:`data` cannot be assigned directly, so this sets :attr:`src` to the merged bytes and
    re-reads from it -- this file ends up with raw bytes as its :attr:`src`

Parameters
----------
bufFiles: List[:class:`CppBufFile`]
    The ``.buf`` files to merge, in the byte order their elements should appear in a line

Raises
------
:class:`BadBufData`
    If the merged bytes do not divide evenly into lines
        )doc"))

        .def("getDumpStr", [](const AGRC::BufFile &self, const std::string &prefix) {
            return self.getDumpStr(prefix);
        }, py::arg("prefix") = "vb0", py::doc(R"doc(
The **data** section of the dump text for this ``.buf`` file -- the text a 3dmigoto frame analysis
writes, which `Blender`_ can then import

One line per element per line (vertex), in the elements' declared order, shaped as
``prefix[lineInd]+byteOffset elementKey: value, value, ...``, with a blank line between lines:

.. code-block::

    vb0[0]+000 POSITION: 1.0, 2.0, 3.0
    vb0[0]+012 TEXCOORD: 0.25, 0.5

    vb0[1]+000 POSITION: 4.0, 5.0, 6.0
    vb0[1]+012 TEXCOORD: 0.5, 0.5

.. note::
    **This is deliberately only the data.** A real dump file also needs a header, and that header
    differs by the kind of buffer being dumped -- see :class:`VbFile` and :class:`IbFile`, which add
    one each

.. note::
    An entry is named by its *element key*, so a second element sharing a name is suffixed with its
    occurrence (``TEXCOORD``, then ``TEXCOORD1``), matching both :meth:`decodeLine`'s own keys and
    what 3dmigoto writes

Parameters
----------
prefix: :class:`str`
    The buffer name each entry is prefixed with -- the vertex buffer slot a real dump was taken
    from :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``"vb0"``

Returns
-------
:class:`str`
    The data section of the dump text. Empty when the file has no lines
        )doc"))

        .def("getFlatDumpStr", [](const AGRC::BufFile &self, const std::string &valueSep) {
            return self.getFlatDumpStr(valueSep);
        }, py::arg("valueSep") = " ", py::doc(R"doc(
The **data** section of the dump text in an *index buffer*'s flat form -- every one of a line's
values on one line, separated by 'valueSep', with no element name or byte offset

This is what a ``.ib`` file's dump looks like (``0 1 2`` per triangular face), as opposed to the
per-element form :meth:`getDumpStr` produces for a vertex buffer

Parameters
----------
valueSep: :class:`str`
    What to put between two values on the same line :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``" "``

Returns
-------
:class:`str`
    The data section of the dump text. Empty when the file has no lines
        )doc"))

        .def("readDumpStr", [](AGRC::BufFile &self, const std::string &text) {
            translateBufFileErrors([&]() {
                self.readDumpStr(text);
                return 0;
            });
        }, py::arg("text"), py::doc(R"doc(
Reads dump text back into this ``.buf`` file's bytes -- the inverse of :meth:`getDumpStr`

The values are encoded with this file's **current** :attr:`elements`, one text line per element and
one blank-line-separated block per line (vertex), so a round trip out through :meth:`getDumpStr` and
back in returns the bytes it started with

.. note::
    A complete dump file works too, not just the data section :meth:`getDumpStr` returns: anything
    up to and including a ``vertex-data:`` marker is skipped. Each line's values are taken from
    after its last ``:``, so the ``prefix[i]+offset elementKey:`` part is ignored rather than having
    to match

.. note::
    A block with fewer values than the elements need is zero-filled, and extra values are dropped,
    so every block always contributes exactly :attr:`bytesPerLine` bytes

.. note::
    :attr:`data` cannot be assigned directly, so this sets :attr:`src` to the parsed bytes and
    re-reads from it

Parameters
----------
text: :class:`str`
    The dump text to read

Raises
------
:class:`BadBufData`
    If the parsed bytes do not divide evenly into lines
        )doc"))

        .def("readFlatDumpStr", [](AGRC::BufFile &self, const std::string &text, const std::string &valueSep) {
            translateBufFileErrors([&]() {
                self.readFlatDumpStr(text, valueSep);
                return 0;
            });
        }, py::arg("text"), py::arg("valueSep") = " ", py::doc(R"doc(
Reads *index buffer* dump text back into this ``.buf`` file's bytes -- the inverse of
:meth:`getFlatDumpStr`

One text line per line of the file, its values separated by 'valueSep'

.. note::
    A complete dump file works too: a header line is recognised by containing a ``:`` (every line of
    a ``.ib`` dump's header does, and none of its data lines do) and skipped

Parameters
----------
text: :class:`str`
    The dump text to read

valueSep: :class:`str`
    What separates two values on the same line :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``" "``

Raises
------
:class:`BadBufData`
    If the parsed bytes do not divide evenly into lines
        )doc"))

        .def("fix", [](AGRC::BufFile &self, py::object fixedFile, py::object filters) {
            std::optional<std::string> fixedFileOpt;
            if (!fixedFile.is_none()) {
                fixedFileOpt = fixedFile.cast<std::string>();
            }

            std::vector<py::function> rawFilters;
            if (!filters.is_none()) {
                rawFilters = filters.cast<std::vector<py::function>>();
            }

            AGRC::BufFile::FixResult result = self.fix(fixedFileOpt, parseFilters(rawFilters));
            return fixResultToPy(result);
        }, py::arg("fixedFile") = py::none(), py::arg("filters") = py::none(), py::doc(R"doc(
Fixes the ``.buf`` file

Parameters
----------
fixedFile: Optional[:class:`str`]
    The file path for the fixed ``.buf`` file. **Default**: ``None``

filters: Optional[List[Callable[[Dict[:class:`str`, List[Any]], :class:`int`, :class:`int`, :class:`int`], Dict[:class:`str`, List[Any]]]]]
    The filters to process each element, applied in order to each line

    The filters take in the following arguments:

    #. The data for a particular line
    #. The starting byte index of the line that is read
    #. The line index being processed (``i / bytesPerLine`` -- a `floating point`_ value, matching
       this codebase's pure-Python original exactly)
    #. The size of each line

    The output of the filters is the resultant data that consists where the keys are the names of
    the elements within a line in the ``.buf`` file and the values are the resultant data for each
    element in the line. **Default**: ``None``

Returns
-------
Union[Optional[:class:`str`], :class:`bytearray`]
    If the argument ``fixedFile`` is ``None``, then will return an array of bytes for the fixed
    ``.buf`` file. Otherwise will return the filename to the fixed ``.buf`` file
        )doc"));
}
