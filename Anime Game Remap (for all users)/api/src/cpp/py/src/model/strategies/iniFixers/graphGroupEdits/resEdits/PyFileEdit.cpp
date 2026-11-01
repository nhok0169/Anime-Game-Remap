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

#include "PyFileEdit.h"

#include <memory>
#include <utility>

#include <pybind11/stl.h>

#include "AGRemapCore/model/strategies/texEditors/BaseTexEditor.h"


namespace {

using FileAdd = AGRC::FileAddCreate<std::string, std::string>;
using FileDownload = AGRC::FileDownloadCreate<std::string, std::string>;
using Bytes = std::vector<std::uint8_t>;


Bytes toBytes(const py::bytes &bytes) {
    const std::string raw = bytes;
    return Bytes(raw.begin(), raw.end());
}


py::bytes fromBytes(const Bytes &bytes) {
    return py::bytes(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}


// A Python callable answering bytes, as the core's makeBytes. The callable is held under the GIL, since
// the function is copied and destroyed by C++ code that does not hold it
std::function<Bytes()> toMakeBytes(const py::object &makeBytes) {
    if (makeBytes.is_none()) {
        return {};
    }

    std::shared_ptr<py::object> held(new py::object(makeBytes), [](py::object* obj) {
        if (!Py_IsInitialized()) {
            return;
        }
        py::gil_scoped_acquire gil;
        delete obj;
    });
    return [held]() {
        py::gil_scoped_acquire gil;
        return toBytes((*held)().cast<py::bytes>());
    };
}


std::unordered_map<std::string, Bytes> toFileBytes(const py::dict &files) {
    std::unordered_map<std::string, Bytes> result;
    for (const auto &entry : files) {
        result.emplace(py::str(entry.first).cast<std::string>(), toBytes(entry.second.cast<py::bytes>()));
    }
    return result;
}


py::dict fromFileBytes(const std::unordered_map<std::string, Bytes> &files) {
    py::dict result;
    for (const auto &[file, bytes] : files) {
        result[py::str(file)] = fromBytes(bytes);
    }
    return result;
}

}


PyFileAddCreate::PyFileAddCreate(py::object resModObj, std::unordered_map<std::string, File> files, std::string resType):
    PyResEditMixin<FileAdd>(PyBaseResEditCore::GraphId(), std::move(files), makeResEditConfig(), std::move(resType)) {
    resModObjObj = std::move(resModObj);
    graphReplaceModeObj = py::none();
    refresh();
}


py::object PyFileAddCreate::pySelf() const {
    return py::cast(this, py::return_value_policy::reference);
}


PyFileRefAdd::PyFileRefAdd(std::unordered_map<std::string, std::vector<std::uint8_t>> files, std::string resType):
    PyResEditMixin<AGRC::FileRefAdd<std::string, std::string>>(std::move(files), makeResEditConfig(), std::move(resType)) {
    resModObjObj = py::none();
    graphReplaceModeObj = py::none();
    refresh();
}


py::object PyFileRefAdd::pySelf() const {
    return py::cast(this, py::return_value_policy::reference);
}


PyFileDownloadCreate::PyFileDownloadCreate(py::object resModObj, std::unordered_map<std::string, File> files,
                                           std::string resType):
    PyResEditMixin<FileDownload>(PyBaseResEditCore::GraphId(), std::move(files), makeResEditConfig(), std::move(resType)) {
    resModObjObj = std::move(resModObj);
    graphReplaceModeObj = py::none();
    refresh();
}


py::object PyFileDownloadCreate::pySelf() const {
    return py::cast(this, py::return_value_policy::reference);
}


void initCppFileEdits(pybind11::module_ &m) {
    // ---------- FileAddCreate ----------
    auto addCls = py::class_<PyFileAddCreate, PyBaseResEditCore, py::smart_holder>(m, "FileAddCreate", R"doc(
This class inherits from :class:`BaseResEdit`

Creates a brand new file of contents the fix already knows -- eg. the constant buffer a ``CustomShader``
reads -- together with the ``[Resource...]`` `section`_ that names it

Use it through :class:`ResRegCollect`: the collect finds the register naming the resource (eg.
``ps-cb4 = ResourceShaderCB4``), and this edit builds the `section`_ of that name from :attr:`files` and a
:class:`RemapFileAddResource` that writes the file. The `section`_ keeps the name the register gives it,
and a name :attr:`files` does not list builds no `section`_

Parameters
----------
resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The mod object to hold the newly created :class:`IniSectionGraph` for the resource :raw-html:`<br />` :raw-html:`<br />`

    The tuple contains:

    #. The index for the .ini file
    #. The name of the component
    #. The name of the object

files: Dict[:class:`str`, :class:`FileAddCreate.File`]
    The files to make, as ``section name -> file``

resType: :class:`str`
    The name of the type of resource :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``"resourceRemapFileAdd"``
    )doc");

    py::class_<FileAdd::File>(addCls, "File", R"doc(
One file a :class:`FileAddCreate` makes, and the `section`_ naming it

Parameters
----------
keys: List[Tuple[:class:`str`, :class:`str`]]
    The `section`_'s keys before its ``filename =``, eg. ``type``, ``format``, ``stride``

file: :class:`str`
    The file, relative to the ``.ini`` file

bytes: :class:`bytes`
    The contents

makeBytes: Optional[Callable[[], :class:`bytes`]]
    The contents computed when the file is written, in place of ``bytes`` -- eg. a buffer made from files
    the run downloads first. **Default**: ``None``
    )doc")
        .def(py::init([](std::vector<std::pair<std::string, std::string>> keys, std::string file, const py::bytes &bytes,
                         const py::object &makeBytes) {
                 FileAdd::File result;
                 result.keys = std::move(keys);
                 result.file = std::move(file);
                 result.bytes = toBytes(bytes);
                 result.makeBytes = toMakeBytes(makeBytes);
                 return result;
             }),
             py::arg("keys") = std::vector<std::pair<std::string, std::string>>{}, py::arg("file") = "",
             py::arg("bytes") = py::bytes(), py::arg("makeBytes") = py::none())
        .def_readwrite("keys", &FileAdd::File::keys, py::doc(R"doc(
List[Tuple[:class:`str`, :class:`str`]]: The `section`_'s keys before its ``filename =``
        )doc"))
        .def_readwrite("file", &FileAdd::File::file, py::doc(R"doc(
:class:`str`: The file, relative to the ``.ini`` file
        )doc"))
        .def_property("bytes", [](const FileAdd::File &self) { return fromBytes(self.bytes); },
                      [](FileAdd::File &self, const py::bytes &bytes) { self.bytes = toBytes(bytes); }, py::doc(R"doc(
:class:`bytes`: The contents
        )doc"))
        .def_property("makeBytes", [](const FileAdd::File &self) -> py::object {
                          if (!self.makeBytes) {
                              return py::none();
                          }
                          return py::cpp_function([makeBytes = self.makeBytes]() { return fromBytes(makeBytes()); });
                      },
                      [](FileAdd::File &self, const py::object &makeBytes) { self.makeBytes = toMakeBytes(makeBytes); },
                      py::doc(R"doc(
Optional[Callable[[], :class:`bytes`]]: The contents computed when the file is written, in place of :attr:`bytes`
        )doc"));

    addCls.def(py::init([](py::object resModObj, std::unordered_map<std::string, FileAdd::File> files, std::string resType) {
                   return std::make_unique<PyFileAddCreate>(std::move(resModObj), std::move(files), std::move(resType));
               }),
               py::arg("resModObj"), py::arg("files") = std::unordered_map<std::string, FileAdd::File>{},
               py::arg("resType") = "resourceRemapFileAdd");

    addCls.def_readwrite("files", &PyFileAddCreate::files, py::doc(R"doc(
Dict[:class:`str`, :class:`FileAddCreate.File`]: The files to make, as ``section name -> file`` -- a copy is
handed back, so assign a whole new dictionary to change it
    )doc"));

    // ---------- FileRefAdd ----------
    auto refCls = py::class_<PyFileRefAdd, PyBaseResEditCore, py::smart_holder>(m, "FileRefAdd", R"doc(
This class inherits from :class:`BaseResEdit`

Creates a brand new file that a register names DIRECTLY by its path -- 3DMigoto's ``ps = <file>.hlsl`` in a
``CustomShader``, which takes no ``[Resource...]`` `section`_

Use it through :class:`ResRegCollect` like any other resource edit: the collect hands over each value of the
register, which here is the file itself, and this edit builds a :class:`RemapFileAddResource` for every value
:attr:`files` lists. It builds no graph and changes no register

Parameters
----------
files: Dict[:class:`str`, :class:`bytes`]
    The files to make, as ``file -> contents``, each file as the register names it

resType: :class:`str`
    The name of the type of resource :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``"resourceRemapFileAdd"``
    )doc");

    refCls.def(py::init([](const py::dict &files, std::string resType) {
                   return std::make_unique<PyFileRefAdd>(toFileBytes(files), std::move(resType));
               }),
               py::arg("files") = py::dict(), py::arg("resType") = "resourceRemapFileAdd");

    refCls.def_property("files", [](const PyFileRefAdd &self) { return fromFileBytes(self.files); },
                        [](PyFileRefAdd &self, const py::dict &files) { self.files = toFileBytes(files); }, py::doc(R"doc(
Dict[:class:`str`, :class:`bytes`]: The files to make, as ``file -> contents`` -- a copy is handed back, so assign
a whole new dictionary to change it
    )doc"));

    // ---------- FileDownloadCreate ----------
    auto downloadCls = py::class_<PyFileDownloadCreate, PyBaseResEditCore, py::smart_holder>(m, "FileDownloadCreate", R"doc(
This class inherits from :class:`BaseResEdit`

Downloads a file the fix needs -- eg. the source character's own game texture for a role a mod ships no file
for -- together with the ``[Resource...]`` `section`_ that names it

The download counterpart of :class:`FileAddCreate`, and used the same way, through :class:`ResRegCollect`.
The `section`_ keeps the name the register gives it, and a file the name :attr:`files` gives it: name a
download ``...RemapDL...``, the naming an undo recognises a download by. A name :attr:`files` does not list
builds no `section`_

Parameters
----------
resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The mod object to hold the newly created :class:`IniSectionGraph` for the resource :raw-html:`<br />` :raw-html:`<br />`

    The tuple contains:

    #. The index for the .ini file
    #. The name of the component
    #. The name of the object

files: Dict[:class:`str`, :class:`FileDownloadCreate.File`]
    The files to download, as ``section name -> file``

resType: :class:`str`
    The name of the type of resource :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``"download"``
    )doc");

    py::class_<FileDownload::File>(downloadCls, "File", R"doc(
One file a :class:`FileDownloadCreate` downloads, and the `section`_ naming it

Parameters
----------
keys: List[Tuple[:class:`str`, :class:`str`]]
    The `section`_'s keys before its ``filename =``

file: :class:`str`
    The file the `section`_ names, relative to the ``.ini`` file

url: :class:`str`
    Where the file is downloaded from

texEdit: Optional[:class:`CppBaseTexEditor`]
    An edit of the downloaded texture -- eg. a :class:`TexEditor` moving a light map's bands. Set, the
    download goes to ``rawFile`` and the edit writes ``file`` from it, so the `section`_ names the EDITED
    file. Held, not copied, so a subclass's own :meth:`~CppBaseTexEditor.fix` is the one that runs.
    **Default**: ``None``, the download is ``file`` itself

rawFile: :class:`str`
    Where the download goes when there is a ``texEdit``, relative to the ``.ini`` file. Name it
    ``...RemapDL...`` too: no `section`_ names it, and the undo recognises it by that name. **Default**: ``""``
    )doc")
        .def(py::init([](std::vector<std::pair<std::string, std::string>> keys, std::string file, std::string url,
                         std::shared_ptr<AGRC::BaseTexEditor> texEdit, std::string rawFile) {
                 FileDownload::File result;
                 result.keys = std::move(keys);
                 result.file = std::move(file);
                 result.url = std::move(url);
                 result.texEdit = std::move(texEdit);
                 result.rawFile = std::move(rawFile);
                 return result;
             }),
             py::arg("keys") = std::vector<std::pair<std::string, std::string>>{}, py::arg("file") = "",
             py::arg("url") = "", py::arg("texEdit") = py::none(), py::arg("rawFile") = "")
        .def_readwrite("keys", &FileDownload::File::keys, py::doc(R"doc(
List[Tuple[:class:`str`, :class:`str`]]: The `section`_'s keys before its ``filename =``
        )doc"))
        .def_readwrite("file", &FileDownload::File::file, py::doc(R"doc(
:class:`str`: The file the `section`_ names, relative to the ``.ini`` file
        )doc"))
        .def_readwrite("url", &FileDownload::File::url, py::doc(R"doc(
:class:`str`: Where the file is downloaded from
        )doc"))
        .def_readwrite("texEdit", &FileDownload::File::texEdit, py::doc(R"doc(
Optional[:class:`CppBaseTexEditor`]: An edit of the downloaded texture, or ``None`` -- held, not copied
        )doc"))
        .def_readwrite("rawFile", &FileDownload::File::rawFile, py::doc(R"doc(
:class:`str`: Where the download goes when there is a :attr:`texEdit`, relative to the ``.ini`` file
        )doc"));

    downloadCls.def(py::init([](py::object resModObj, std::unordered_map<std::string, FileDownload::File> files,
                                std::string resType) {
                        return std::make_unique<PyFileDownloadCreate>(std::move(resModObj), std::move(files), std::move(resType));
                    }),
                    py::arg("resModObj"), py::arg("files") = std::unordered_map<std::string, FileDownload::File>{},
                    py::arg("resType") = "download");

    downloadCls.def_readwrite("files", &PyFileDownloadCreate::files, py::doc(R"doc(
Dict[:class:`str`, :class:`FileDownloadCreate.File`]: The files to download, as ``section name -> file`` -- a copy
is handed back, so assign a whole new dictionary to change it
    )doc"));
}
