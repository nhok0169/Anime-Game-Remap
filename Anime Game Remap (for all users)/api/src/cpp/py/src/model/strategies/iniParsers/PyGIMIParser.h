#ifndef AGRemapPyBind_PyGIMIParser_H
#define AGRemapPyBind_PyGIMIParser_H

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <pybind11/pybind11.h>

#include "PyBaseIniParser.h"
#include "../iniFixers/graphGroupEdits/PyIniGraphGroups.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseContext.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseDownloadData.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `Python`_-backed :cpp:class:`AGRemapCore::IniParseContext` -- the still-pure-Python ``IniFile``
 a parser is parsing, plus the one :cpp:class:`AGRemapCore::IIniGraphGroups` that owns every graph
 it builds :raw-html:`<br />` :raw-html:`<br />`

 The graph groups are a real `Python`_ ``List[IniGraphGroup]`` (via ``PyIniGraphGroups``) rather
 than a C++ container, for the same reason the ``graphGroupEdits/`` family needs one: the graphs
 inside are real ``IniSectionGraph`` `Python`_ objects carrying their own keep-alive bookkeeping,
 and ``GIMIParser``'s own ``commandGraphs`` *is* group ``0``'s ``graphs`` dict -- the very dict
 ``editCommands`` hands to an edit and reads back out
 @endrst
 */
class PyIniParseContext: public AGRC::IniParseContext<py::object, py::object, PyObjectHash, PyObjectEqual> {
    public:
        using Base = AGRC::IniParseContext<py::object, py::object, PyObjectHash, PyObjectEqual>;
        using Section = Base::Section;
        using Graph = Base::Graph;
        using GraphGroups = Base::GraphGroups;
        using Assets = Base::Assets;

        /**
         * @brief Constructs a context over one Python ``IniFile``
         *
         * @param ini The Python ``IniFile``, or ``None``
         */
        explicit PyIniParseContext(py::object ini = py::none());

        /**
         * @brief The Python ``IniFile``, or ``None``
         */
        py::object ini;

        bool hasIni() const override;
        std::string iniFolder() const override;
        std::optional<AGRC::Version> version() const override;
        AGRC::DownloadMode downloadMode() const override;
        AGRC::Z3Context* z3Ctx() const override;
        std::unordered_map<std::string, Section*> sectionIfTemplates() const override;
        std::vector<std::string> sectionNames() const override;
        Section* getSection(const std::string &name) const override;
        Section* addSection(const std::string &name, std::unique_ptr<Section> section) override;
        void removeSection(const std::string &name) override;
        void addFileDownload(std::unique_ptr<AGRC::IniResource> download) override;
        bool hasModType() const override;
        std::string modTypeName() const override;
        Assets* modTypeHashes() const override;
        Assets* modTypeIndices() const override;
        GraphGroups& graphGroups() override;

        /**
         * @brief The Python ``ModType`` the .ini file was classified as, or ``None``
         */
        py::object modType() const;

        /**
         * @brief
         @rst
         Stores an already-built `Python`_ ``IfTemplate`` into ``ini.sectionIfTemplates`` -- the
         path a `Python`_ ``DownloadData.createResSection`` result takes, since that object is
         already a `Python`_ one and must not be rebuilt from C++
         @endrst
         *
         * @param name The name to store it under
         * @param section The Python ``IfTemplate``
         *
         * @return The stored section
         */
        Section* addSectionObj(const std::string &name, py::object section);

        /**
         * @brief Appends an already-built Python download resource to ``ini.fileDownloads``
         *
         * @param download The Python ``RemapIniDownload``
         */
        void addFileDownloadObj(py::object download);

        /**
         * @brief The ``List[IniGraphGroup]`` every graph this parser builds lives in
         */
        py::list groupsList() const;

        /**
         * @brief Replaces the group at index ``0`` with a fresh one wrapping 'graphs'
         *
         * @param graphs The Python ``Dict[Tuple[str, str], IniSectionGraph]`` to wrap
         */
        void setCommandGraphs(py::object graphs);

        /**
         * @brief The ``graphs`` dict of the group at index ``0`` -- ``GIMIParser.commandGraphs``
         */
        py::object commandGraphs() const;

        /**
         * @brief The view over #groupsList the core algorithms run against
         */
        PyIniGraphGroups groups;
};


/**
 * @brief
 @rst
 The `Python`_-backed :cpp:class:`AGRemapCore::IniParseDownloadData` -- calls straight back into a
 real `Python`_ ``DownloadData`` object :raw-html:`<br />` :raw-html:`<br />`

 Every method here forwards through genuine `Python`_ attribute lookup rather than reimplementing
 what ``DownloadData`` does, because that class is polymorphic on the `Python`_ side
 (``BlendDownloadData`` overrides ``addToPart``, and a user may subclass it further) -- see
 :cpp:class:`AGRemapCore::IniParseDownloadData`'s own note
 @endrst
 */
class PyIniParseDownloadData: public AGRC::IniParseDownloadData<py::object, py::object, PyObjectHash, PyObjectEqual> {
    public:
        using Base = AGRC::IniParseDownloadData<py::object, py::object, PyObjectHash, PyObjectEqual>;
        using Section = Base::Section;
        using ContentPart = Base::ContentPart;
        using Context = Base::Context;

        /**
         * @brief Wraps one Python ``DownloadData``
         *
         * @param downloadData The Python ``DownloadData``
         */
        explicit PyIniParseDownloadData(py::object downloadData);

        /**
         * @brief The Python ``DownloadData`` this wraps
         */
        py::object downloadData;

        std::string name() const override;
        bool refToSection() const override;
        void addToPart(ContentPart &part, const py::object &key, const py::object &val) override;
        void addToSection(Section &section, const py::object &key, const py::object &val) override;
        Section* createResSection(const std::string &sectionName, Context &ctx) override;
        void addFileDownload(Context &ctx, const std::string &iniFolder) override;
};


/**
 * @brief The core :cpp:class:`AGRemapCore::GIMIParser` specialization this binds
 */
using PyGIMIParserCore = AGRC::GIMIParser<py::object, py::object, PyObjectHash, PyObjectEqual, PyBaseIniParser>;


/**
 * @brief
 @rst
 The `pybind11`_-facing ``GIMIParser`` :raw-html:`<br />` :raw-html:`<br />`

 Keeps the caller's own `Python`_ objects for ``modObjs``/``objTargetFuncs``/``downloads``/
 ``commandEdits``/``keysToTrack`` and re-derives the core members from them at the start of every
 operation (see #refresh) -- the same identity/in-place-mutation contract every other ported class
 in this codebase keeps, and a hard requirement here: ``GIMIParser``'s own test suite constructs a
 parser and then assigns ``objTargetFuncs``/``trackKeys``/``keysToTrack`` afterwards

 :raw-html:`<br />`

 .. note::
    #editCommands is overridden to go back through `Python`_ (``self.commandEdits.editFromIni(...)``)
    rather than using the core's own default. That is not just faithfulness: the core default has
    no `Python`_ ``IniFile``/``ModType`` to pass down (see
    :cpp:func:`AGRemapCore::GIMIParser::editCommands`'s own comment), and a ``GraphGroupEdit``
    holding pure-`Python`_ sub-edits genuinely needs both
 @endrst
 */
class PyGIMIParser: public PyGIMIParserCore {
    public:
        using Core = PyGIMIParserCore;
        using ModObj = Core::ModObj;

        /**
         * @brief Constructs a new parser
         *
         * @param iniFile The Python ``IniFile`` to parse
         * @param modObjs The mod objects to parse, or ``None``
         * @param objTargetFuncs The custom classification functions, or ``None``
         * @param downloads The files to download, or ``None``
         * @param commandEdits Further edits for the parsed command graphs, or ``None``
         * @param makeGlobalGraph Whether to make the graph for the entire .ini file
         * @param disjointModObjs Whether each `section`_ belongs to at most one mod object
         * @param trackKeys Whether to track the `KVPs`_ in the .ini file
         * @param keysToTrack Specific keys to track, or ``None`` for all of them
         */
        PyGIMIParser(py::object iniFile, py::object modObjs, py::object objTargetFuncs, py::object downloads,
                      py::object commandEdits, bool makeGlobalGraph, bool disjointModObjs, bool trackKeys,
                      py::object keysToTrack);

        /**
         * @brief The .ini file being parsed, and the graphs built from it
         */
        PyIniParseContext ctxImpl;

        /**
         * @brief The exact Python object given for ``modObjs``
         */
        py::object modObjsObj;

        /**
         * @brief The exact Python object given for ``objTargetFuncs``
         */
        py::object objTargetFuncsObj;

        /**
         * @brief The exact Python object given for ``downloads``
         */
        py::object downloadsObj;

        /**
         * @brief The exact Python object given for ``commandEdits``
         */
        py::object commandEditsObj;

        /**
         * @brief The exact Python object given for ``keysToTrack``
         */
        py::object keysToTrackObj;

        /**
         * @brief
         @rst
         Temporary user-defined keyword variables for the user to use -- only cleared by
         :meth:`clear` :raw-html:`<br />` :raw-html:`<br />`

         .. note::
            The pure-Python original also used this to cache its ``TextureOverride`` name
            classifier. That cache is a real typed member of
            :cpp:class:`AGRemapCore::GIMIParser` now, so nothing but the user puts anything here
         @endrst
         */
        py::dict tempKwargs;

        /**
         * @brief Re-derives every core member from the Python objects above -- see this class's own note
         */
        void refresh();

        std::vector<Core::GraphGroup> parse() override;

        /**
         * @brief
         @rst
         Deliberately returns nothing :raw-html:`<br />` :raw-html:`<br />`

         The core's own implementation deep-copies every graph into a C++-owned
         :cpp:class:`AGRemapCore::IniGraphGroup`; #parseToPy builds the equivalent `Python`_ group
         out of the live graph objects instead, so doing both would mean copying every graph of
         every ``.ini`` file for a result nothing reads. See
         :cpp:func:`AGRemapCore::GIMIParser::collectParseResult`'s own note
         @endrst
         */
        std::vector<Core::GraphGroup> collectParseResult() const override;

        /**
         * @brief
         @rst
         Parses, then collects the result as a `Python`_ ``[IniGraphGroup]`` :raw-html:`<br />`
         :raw-html:`<br />`

         Same shape :cpp:func:`AGRemapCore::GIMIParser::collectParseResult` documents -- every
         command graph under its own ``(component, mod object)`` key, then every download resource
         graph under ``("download", <the download's name>)`` -- except that the graphs are the
         parser's own live objects, not copies. A `Python`_ ``IniGraphGroup`` holds a real ``dict``
         of references, so there is nothing to own and nothing to copy; a caller that wants an
         independent group calls ``deepcopy`` on the graphs itself, exactly as ``GIMIFixer`` does
         @endrst
         */
        py::object parseToPy() override;

        /**
         * @brief
         @rst
         The `Python`_ ``[IniGraphGroup]`` for whatever the *last* parse produced, without parsing
         again -- what #parseToPy returns, minus the parse :raw-html:`<br />` :raw-html:`<br />`

         Exists because ``GIMIFixer`` needs exactly this: ``IniFile.parse()`` has already run the
         parser by the time a fixer is asked to fix, so re-running it would duplicate every
         synthesized download resource
         @endrst
         */
        py::object collectToPy() const;

        void editCommands() override;
        void clear() override;

    private:
        // The IniParseDownloadData adapters the core holds borrowed pointers to, rebuilt by every
        // refresh(). They live exactly as long as the derived 'downloads' map that points at them.
        std::vector<std::unique_ptr<PyIniParseDownloadData>> downloadAdapters_;
};


void initCppGIMIParser(pybind11::module_ &m);

#endif
