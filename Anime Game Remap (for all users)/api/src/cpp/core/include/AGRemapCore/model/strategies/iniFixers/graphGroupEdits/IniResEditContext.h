#ifndef AGRemapCore_IniResEditContext_H
#define AGRemapCore_IniResEditContext_H

#include <functional>
#include <string>
#include <unordered_map>

#include <memory>
#include <utility>
#include <vector>

#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/iniresources/IniResource.h"
#include "AGRemapCore/tools/z3/Z3Context.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The ``.ini`` file a resource edit is building resources *for*, behind an interface
     :raw-html:`<br />` :raw-html:`<br />`

     **Why this isn't just an** :cpp:class:`IniFile` **pointer.** The ``.ini`` file every real
     caller of this subsystem passes is the *`Python`_* ``IniFile`` (``model/files/IniFile.py``),
     which is an unrelated class to :cpp:class:`AGRemapCore::IniFile` and has no C++ counterpart to
     cast to -- so a plain ``IniFile*`` parameter would always be ``nullptr`` here, and every
     resource edit would be inert (compare the ``regEdits/`` family, where that *is* the right
     answer, because those edits genuinely never read the ``.ini`` file). Resource edits do the
     opposite: reading the ``.ini`` file's `sections`_ is most of what they do. This interface is
     the narrowest thing that makes them work against either implementation.

     :raw-html:`<br />`

     .. note::
        Building and storing the resource *models* themselves is deliberately **not** here -- it
        lives on :cpp:func:`BaseResEdit::buildResModel`, since which kind of resource gets built is
        per-edit-class behaviour rather than a property of the ``.ini`` file
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class IniResEditContext {
        public:

            /**
             * @brief The type of `section`_ a ``.ini`` file is made of
             */
            using Section = IfTemplate<K, V, KeyHash, KeyEqual>;

            virtual ~IniResEditContext() = default;

            /**
             * @brief
             @rst
             The folder the ``.ini`` file lives in (the equivalent of the pure-Python original's
             ``ini.folder``), or an empty string when there is no ``.ini`` file
             @endrst
             */
            virtual std::string iniFolder() const = 0;

            /**
             * @brief
             @rst
             Stores one built resource model under 'fileKey' :raw-html:`<br />` :raw-html:`<br />`

             Where it goes is the environment's business: the pure-Python original either appends to
             ``ini.resources`` or collects into a caller-supplied ``Dict[str, Deque[IniResource]]``,
             and a resource edit itself never reads a built model back
             @endrst
             *
             * @param fileKey The assigned id for the source file
             * @param resource The built resource model
             */
            virtual void storeResource(const std::string& fileKey, std::unique_ptr<IniResource> resource) = 0;

            /**
             * @brief
             @rst
             Starts capturing built models by file key instead of handing them to the ``.ini`` file
             :raw-html:`<br />` :raw-html:`<br />`

             :cpp:class:`ResGroupCollect` builds each resource's models many times over (once per
             replicated copy of its graph) and has to see *which* models came out before deciding
             which ones a group actually keeps -- it cannot simply let them flow into the ``.ini``
             file. Everything captured stays owned by the implementation
             @endrst
             */
            virtual void beginCollectingResources() = 0;

            /**
             * @brief
             @rst
             Hands back everything captured since the last call, as ``(file key, model)`` pairs in
             the order they were built, and clears the capture buffer
             @endrst
             */
            virtual std::vector<std::pair<std::string, IniResource*>> takeCollectedResources() = 0;

            /**
             * @brief Stops capturing -- later models go to the ``.ini`` file again
             */
            virtual void endCollectingResources() = 0;

            /**
             * @brief
             @rst
             Whether there is a real ``.ini`` file behind this context :raw-html:`<br />`
             :raw-html:`<br />`

             ``false`` stands in for the pure-Python original's ``ini = None``, which several
             resource-edit paths check before doing anything at all
             @endrst
             */
            virtual bool hasIni() const = 0;

            /**
             * @brief
             @rst
             Every `section`_ parsed out of the ``.ini`` file, keyed by name -- the equivalent of
             the pure-Python original's ``ini.sectionIfTemplates``. Borrowed, not owned
             @endrst
             */
            virtual std::unordered_map<std::string, Section*> sectionIfTemplates() const = 0;

            /**
             * @brief
             @rst
             The one `Z3`_ context this ``.ini`` file owns (the equivalent of the pure-Python
             original's ``ini._z3Ctx``), or ``nullptr`` :raw-html:`<br />` :raw-html:`<br />`

             There is deliberately one context per ``.ini`` file, not one per
             :cpp:class:`IniSectionGraph` -- every graph built here borrows this one
             @endrst
             */
            virtual Z3Context* z3Ctx() const = 0;
    };
}

#endif
