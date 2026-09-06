#ifndef AGRemapCore_IniFileFixContext_H
#define AGRemapCore_IniFileFixContext_H

#include <cstddef>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "AGRemapCore/model/IniGraphGroup.h"
#include "AGRemapCore/model/strategies/iniFixers/RemapIniFixContext.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/IIniGraphGroups.h"


namespace AGRemapCore {

    class IniFile;
    class ModType;

    /**
     * @brief
     @rst
     This class implements :cpp:class:`IniFixContext` :raw-html:`<br />` :raw-html:`<br />`

     The plain-C++ :cpp:class:`IniFixContext` -- an :cpp:class:`AGRemapCore::IniFile` behind that
     interface, so a caller with a real C++ ``.ini`` file needs no context of its own. The fixing
     counterpart of :cpp:class:`IniFileRemoveContext`, and built to the same shape
     :raw-html:`<br />` :raw-html:`<br />`

     This is what lets :cpp:func:`IniFixBuilder::defaultFactory` build a real
     :cpp:class:`GIMIFixer` rather than the do-nothing :cpp:class:`BaseIniFixer`

     :raw-html:`<br />`

     .. note::
        Its base is :cpp:class:`RemapIniFixContext`, not :cpp:class:`IniFixContext` directly, so the
        two methods that need no concrete ``.ini`` file -- the boilerplate a fix is wrapped in and
        the commenting-out of the mod it replaces -- are inherited rather than written again here.
        Only #modTypeName is added on top of them

     .. note::
        The `pybind11`_ layer has its own implementation instead (``PyIniFixContext``), which
        forwards to a `Python`_ ``IniFile`` through real attribute lookup. Neither derives from the
        other -- they are two implementations of one interface
     @endrst
     */
    class IniFileFixContext: public RemapIniFixContext<std::string, std::string> {
        public:

            /**
             * @brief The kind of group storage #makeGraphGroups hands out
             */
            using Groups = IniGraphGroupsVec<std::string, std::string>;

            /**
             * @brief Wraps one .ini file
             *
             * @param iniFile
             @rst
             The ``.ini`` file being fixed -- non-owning, and it must outlive this context.
             ``nullptr`` makes every method here answer as if there were no ``.ini`` file
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param modTypeId
             @rst
             Which of the ``.ini`` file's mod types is being fixed from, or ``std::nullopt`` for
             none -- what #modTypeName answers from, and so what the boilerplate is named after
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param header See :cpp:member:`RemapIniFixContext::header`. **Default**: ``std::nullopt``
             * @param footer See :cpp:member:`RemapIniFixContext::footer`. **Default**: ``std::nullopt``
             */
            explicit IniFileFixContext(IniFile* iniFile = nullptr, std::optional<int> modTypeId = std::nullopt,
                                        std::optional<std::string> header = std::nullopt,
                                        std::optional<std::string> footer = std::nullopt);

            /**
             * @brief The .ini file this wraps, or ``nullptr``
             */
            IniFile* getIniFile() const;

            /**
             * @brief Which mod type this is fixing from -- see the constructor
             */
            const std::optional<int>& getModTypeId() const;

            /**
             * @copydoc getModTypeId() const
             *
             * @param modTypeId The mod type to fix from, or ``std::nullopt`` for none
             */
            void setModTypeId(std::optional<int> modTypeId);

            /**
             * @brief
             @rst
             Every line #log was handed, in order :raw-html:`<br />` :raw-html:`<br />`

             An :cpp:class:`AGRemapCore::IniFile` has no logger of its own -- the pure-Python
             ``ini.print("log", ...)`` writes to one this side has no counterpart for -- so the
             lines are kept here for a caller to do something with instead of being dropped
             @endrst
             */
            const std::vector<std::string>& getLogs() const;

            std::optional<std::string> modTypeName() const override;

            /**
             * @brief
             @rst
             The ``hash`` assets of the ``.ini`` file's own :cpp:class:`ModType` -- **borrowed**,
             and ``nullptr`` when the file has no mod type :raw-html:`<br />` :raw-html:`<br />`

             The fix-side counterpart of :cpp:func:`IniFileParseContext::modTypeHashes`, and public
             for the same reason that one is: a strategy legitimately needs the assets, while the
             :cpp:class:`ModType` itself stays private :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                This sits on the **concrete** context rather than on the
                :cpp:class:`IniFixContext` seam, unlike the parse side's equivalent. Only a fixer
                that owns its own :cpp:class:`IniFileFixContext` (the way
                :cpp:func:`IniFixBuilderFuncs::raiden6_1`'s does) can reach it. Promote it to the
                seam if one reached *through* the seam ever needs it -- and remember that means
                implementing it on the `pybind11`_ side too
             @endrst
             */
            Hashes* modTypeHashes() const;

            /**
             * @brief
             @rst
             The ``match_first_index`` assets of the ``.ini`` file's own :cpp:class:`ModType` --
             **borrowed**, and ``nullptr`` when the file has no mod type. The companion of
             ef modTypeHashes, and on the concrete class for the same reason
             @endrst
             */
            Indices* modTypeIndices() const;

            /**
             * @brief
             @rst
             The version of the ``.ini`` file being fixed, or ``std::nullopt`` when it has none --
             the fix-side counterpart of :cpp:func:`IniFileParseContext::version`, and on the
             concrete class for the same reason \ref modTypeHashes is
             @endrst
             */
            std::optional<Version> version() const;

            bool hasIni() const override;
            std::vector<std::string> modsToFix() const override;
            std::optional<std::string> fixedFilePath(std::size_t groupInd) const override;
            bool fixedFileExists() const override;
            std::string fileTxt() const override;
            void setFileTxt(std::string txt) override;
            void disableIni() override;
            void log(const std::string& message) override;
            void writeFixedFile(const std::string& path, const std::string& content) override;
            void setIsFixed(bool isFixed) override;
            std::unique_ptr<GraphGroups> makeGraphGroups() override;

            /**
             * @brief
             @rst
             The ``.ini`` file's own :cpp:class:`ModType`, or ``nullptr`` when it has none --
             **borrowed** :raw-html:`<br />` :raw-html:`<br />`

             Public where :cpp:func:`IniFileParseContext::modType` is private, because a fixer has
             a use the parse side never had: :cpp:func:`ModType::getVGRemap` is the only source of
             a ``Blend.buf``'s vertex-group remap, and no narrower accessor covers it the way
             ef modTypeHashes covers the hash table. Those two are derived from this anyway, so
             exposing it widens nothing they did not already
             @endrst
             */
            const ModType* modType() const;

        private:

            IniFile* iniFile_;
            std::optional<int> modTypeId_;

            std::vector<std::string> logs_;

            // One storage vector per makeGraphGroups call, kept alive for this context's lifetime --
            // an IniGraphGroupsVec is only a *view* over a caller-owned vector, and a fixer holds
            // the view it was handed for as long as it holds its groups. A deque rather than a
            // vector because the views point into these, and growing a vector would move them.
            std::deque<std::unique_ptr<std::vector<IniGraphGroup<std::string, std::string>>>> groupStorages_;
    };
}

#endif
