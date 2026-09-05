#ifndef AGRemapCore_IniFileRemoveContext_H
#define AGRemapCore_IniFileRemoveContext_H

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "AGRemapCore/model/strategies/iniRemovers/IniRemoveContext.h"


namespace AGRemapCore {

    class IniFile;

    /**
     * @brief
     @rst
     This class implements :cpp:class:`IniRemoveContext` :raw-html:`<br />` :raw-html:`<br />`

     The plain-C++ :cpp:class:`IniRemoveContext` -- an :cpp:class:`AGRemapCore::IniFile` behind that
     interface, so a caller with a real C++ ``.ini`` file needs no context of its own
     :raw-html:`<br />` :raw-html:`<br />`

     This is what makes :cpp:func:`IniFile::removeFix` work: the remover
     :cpp:func:`RemapIniRemover::factory` builds wraps the ``IniFile*`` it is handed in one of these, and
     rebuilds it whenever :cpp:func:`RemapIniRemover::setIniFile` re-points it -- which the
     :cpp:class:`IniRemoveBuilder` does on every :cpp:func:`IniRemoveBuilder::build`

     :raw-html:`<br />`

     .. note::
        The `pybind11`_ layer has its own implementation instead (``PyIniRemoveContext``), which
        forwards to a `Python`_ ``IniFile`` through real attribute lookup. Neither derives from the
        other -- they are two implementations of one interface, exactly as
        :cpp:class:`IniParseContext` has
     @endrst
     */
    class IniFileRemoveContext: public IniRemoveContext<std::string, std::string> {
        public:

            /**
             * @brief Wraps one .ini file
             *
             * @param iniFile
             @rst
             The ``.ini`` file to remove the fix from -- non-owning, and it must outlive this
             context. ``nullptr`` makes every method here answer as if there were no ``.ini`` file
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             */
            explicit IniFileRemoveContext(IniFile* iniFile = nullptr);

            /**
             * @brief The .ini file this wraps, or ``nullptr``
             */
            IniFile* getIniFile() const;

            bool hasIni() const override;
            std::string iniFolder() const override;
            std::optional<Version> version() const override;
            std::vector<Assets*> modTypeHashes() const override;
            std::vector<std::string> readFileLines() override;
            std::unordered_map<std::string, Section*> sectionIfTemplates() const override;
            std::string fileTxt() const override;
            void setFileTxt(std::string txt) override;
            std::string write() override;
            void clearRead() override;
            void removeBackup() override;
            void setIsFixed(bool isFixed) override;

        private:

            IniFile* iniFile_;
    };
}

#endif
