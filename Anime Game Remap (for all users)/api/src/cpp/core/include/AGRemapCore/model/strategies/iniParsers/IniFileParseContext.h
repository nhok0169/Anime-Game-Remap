#ifndef AGRemapCore_IniFileParseContext_H
#define AGRemapCore_IniFileParseContext_H

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "AGRemapCore/model/IniGraphGroup.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/IIniGraphGroups.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseContext.h"


namespace AGRemapCore {

    class IniFile;
    class ModType;

    /**
     * @brief
     @rst
     This class implements :cpp:class:`IniParseContext` :raw-html:`<br />` :raw-html:`<br />`

     The plain-C++ :cpp:class:`IniParseContext` -- an :cpp:class:`AGRemapCore::IniFile` behind that
     interface, so a caller with a real C++ ``.ini`` file needs no context of its own. The parsing
     counterpart of :cpp:class:`IniFileRemoveContext`, and built to the same shape
     :raw-html:`<br />` :raw-html:`<br />`

     This is what lets :cpp:func:`IniParseBuilder::defaultFactory` build a real
     :cpp:class:`GIMIParser` rather than the do-nothing :cpp:class:`BaseIniParser`

     :raw-html:`<br />`

     .. note::
        It owns the group storage every graph a parser builds lives in, and hands it out through
        #graphGroups. That is the whole reason a parser needs a context to build graphs at all --
        see :cpp:class:`IniParseContext`'s own note. #takeGroups is how a caller gets them out
        afterwards

     .. note::
        The `pybind11`_ layer has its own implementation instead (``PyIniParseContext``), which
        forwards to a `Python`_ ``IniFile`` through real attribute lookup. Neither derives from the
        other -- they are two implementations of one interface
     @endrst
     */
    class IniFileParseContext: public IniParseContext<std::string, std::string> {
        public:

            /**
             * @brief The kind of group storage this owns
             */
            using Groups = IniGraphGroupsVec<std::string, std::string>;

            /**
             * @brief Wraps one .ini file
             *
             * @param iniFile
             @rst
             The ``.ini`` file being parsed -- non-owning, and it must outlive this context.
             ``nullptr`` makes every method here answer as if there were no ``.ini`` file
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param modTypeId
             @rst
             Which of the ``.ini`` file's mod types is being parsed for, or ``std::nullopt`` for
             none -- what #modTypeName, #modTypeHashes and #modTypeIndices answer from
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             */
            explicit IniFileParseContext(IniFile* iniFile = nullptr, std::optional<int> modTypeId = std::nullopt);

            /**
             * @brief The .ini file this wraps, or ``nullptr``
             */
            IniFile* getIniFile() const;

            /**
             * @brief Which mod type this is parsing for -- see the constructor
             */
            const std::optional<int>& getModTypeId() const;

            /**
             * @copydoc getModTypeId() const
             *
             * @param modTypeId The mod type to parse for, or ``std::nullopt`` for none
             */
            void setModTypeId(std::optional<int> modTypeId);

            /**
             * @brief
             @rst
             Hands back every group built so far and leaves this context empty, so it can be reused
             for another parse :raw-html:`<br />` :raw-html:`<br />`

             The groups own their graphs, so what comes back stays valid after this context goes
             @endrst
             */
            std::vector<IniGraphGroup<std::string, std::string>> takeGroups();

            bool hasIni() const override;
            std::string iniFolder() const override;
            std::optional<Version> version() const override;
            DownloadMode downloadMode() const override;
            Z3Context* z3Ctx() const override;
            std::unordered_map<std::string, Section*> sectionIfTemplates() const override;
            std::vector<std::string> sectionNames() const override;
            Section* getSection(const std::string& name) const override;
            Section* addSection(const std::string& name, std::unique_ptr<Section> section) override;
            void removeSection(const std::string& name) override;
            void addFileDownload(std::unique_ptr<IniResource> download) override;
            bool hasModType() const override;
            std::string modTypeName() const override;
            void log(const std::string& message) override;
            Assets* modTypeHashes() const override;
            Assets* modTypeIndices() const override;
            GraphGroups& graphGroups() override;

        private:
            // The .ini file's own ModType for #getModTypeId, or nullptr when there is none.
            const ModType* modType() const;

            IniFile* iniFile_;
            std::optional<int> modTypeId_;

            // The storage IniGraphGroupsVec is only a *view* over -- it has to outlive the view, so
            // both live here and in this order.
            std::vector<IniGraphGroup<std::string, std::string>> groupStorage_;
            std::unique_ptr<Groups> groups_;

            void rebuildGroups();
    };
}

#endif
