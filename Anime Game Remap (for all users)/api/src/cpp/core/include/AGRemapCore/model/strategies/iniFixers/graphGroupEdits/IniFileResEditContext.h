#ifndef AGRemapCore_IniFileResEditContext_H
#define AGRemapCore_IniFileResEditContext_H

#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/IniResEditContext.h"


namespace AGRemapCore {

    class IniFile;

    /**
     * @brief
     @rst
     This class implements :cpp:class:`IniResEditContext` :raw-html:`<br />` :raw-html:`<br />`

     The plain-C++ :cpp:class:`IniResEditContext` -- an :cpp:class:`AGRemapCore::IniFile` behind that
     interface, so a caller with a real C++ ``.ini`` file needs no context of its own. The resource
     counterpart of :cpp:class:`IniFileRemoveContext`, and built to the same shape
     :raw-html:`<br />` :raw-html:`<br />`

     A built model goes to exactly one of three places, checked in this order -- the same three the
     pure-Python original chooses between:

     #. into #takeCollectedResources' buffer, while a #beginCollectingResources pass is open
     #. into a caller-supplied :cpp:func:`collected` map, keyed by file key, when one was given --
        the ``Dict[str, Deque[IniResource]]`` the original's ``ResRegCollect`` collects into
     #. onto the ``.ini`` file's own :cpp:func:`IniFile::getResources`, otherwise -- the original's
        ``ini.resources.append(...)``, where the file key goes unused

     :raw-html:`<br />`

     .. danger::
        Every model this stores stays owned by whoever it was stored into, and
        :cpp:func:`takeCollectedResources` hands back **raw pointers** to models it keeps owning.
        Two ways to dangle one: clearing the ``.ini`` file (see
        :cpp:func:`IniFile::getResources`), or destroying this context while a
        :cpp:class:`ResGroupCollect` still holds what it took

     .. note::
        The `pybind11`_ layer has its own implementation instead (``PyIniResEditContext``), which
        forwards to a `Python`_ ``IniFile`` through real attribute lookup. Neither derives from the
        other -- they are two implementations of one interface
     @endrst
     */
    class IniFileResEditContext: public IniResEditContext<std::string, std::string> {
        public:

            /**
             * @brief
             @rst
             Where a caller-supplied collect map keeps its models -- the C++ spelling of the
             original's ``Dict[str, Deque[IniResource]]``
             @endrst
             */
            using Collected = std::unordered_map<std::string, std::deque<std::unique_ptr<IniResource>>>;

            /**
             * @brief Wraps one .ini file
             *
             * @param iniFile
             @rst
             The ``.ini`` file resources are being built for -- non-owning, and it must outlive this
             context. ``nullptr`` makes every method here answer as if there were no ``.ini`` file
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param collected
             @rst
             Where to collect built models instead of handing them to the ``.ini`` file, or
             ``nullptr`` to hand them over. Non-owning, and it must outlive this context too
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             */
            explicit IniFileResEditContext(IniFile* iniFile = nullptr, Collected* collected = nullptr);

            /**
             * @brief The .ini file this wraps, or ``nullptr``
             */
            IniFile* getIniFile() const;

            /**
             * @brief The caller-supplied collect map, or ``nullptr`` -- see the constructor
             */
            Collected* getCollected() const;

            /**
             * @brief Whether a #beginCollectingResources pass is currently open
             */
            bool isCollecting() const;

            std::string iniFolder() const override;
            void storeResource(const std::string& fileKey, std::unique_ptr<IniResource> resource) override;
            void beginCollectingResources() override;
            std::vector<std::pair<std::string, IniResource*>> takeCollectedResources() override;
            void endCollectingResources() override;
            bool hasIni() const override;
            std::unordered_map<std::string, Section*> sectionIfTemplates() const override;
            Z3Context* z3Ctx() const override;

        private:
            IniFile* iniFile_;
            Collected* collected_;

            bool collecting_ = false;

            // What a capture pass has taken so far, and what owns it. Kept apart from 'buffer_'
            // because takeCollectedResources hands back bare pointers while promising the models
            // stay alive -- so the ownership has to outlive the hand-off, and every capture this
            // context ever made stays here until the context itself goes.
            std::vector<std::unique_ptr<IniResource>> captureKeepAlive_;
            std::vector<std::pair<std::string, IniResource*>> buffer_;
    };
}

#endif
