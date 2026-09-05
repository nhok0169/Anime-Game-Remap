#ifndef AGRemapCore_IniResource_H
#define AGRemapCore_IniResource_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "AGRemapCore/view/BaseLogger.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Base class for a resource in the .ini file :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the pure-Python ``IniResource`` class (``model/iniresources/IniResource.py``)
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The Python original also carries a generic, per-instance ``fixFunc``/``fix()``/``_fix()``
        override mechanism at this level, built on ``*args``/``**kwargs`` -- that pattern doesn't
        translate to a single, uniform C++ virtual signature (every real subclass in this codebase's
        port -- ``RemapBlendResource``, ``RemapTexAddResource``, ``RemapIniDownload`` -- needs a
        genuinely different signature for its own fix logic), so this class stays a plain data
        holder: each concrete leaf class defines its own, concretely-typed ``fix()`` (and its own
        ``fixFunc`` member, typed for that leaf specifically) instead of inheriting a generic one
        from here. :cpp:class:`IniGroupedResource` is the one exception in this hierarchy that keeps
        the generic pattern, since (unlike every other class here) it's actually used directly/bare,
        with no natural leaf subclass of its own to hold concrete fix logic
     @endrst
     */
    class IniResource {
        public:

            /**
             * @brief Constructs a new resource
             *
             * @param type The name for the type of resource
             * @param iniFolderPath The path to the folder of the .ini file
             * @param srcPath The file path to the resource (resolved to an absolute path against 'iniFolderPath')
             */
            IniResource(std::string type, const std::string& iniFolderPath, const std::string& srcPath);

            virtual ~IniResource() = default;

            /**
             * @brief The name for the type of resource
             */
            std::string type;

            /**
             * @brief The full file path to the resource
             */
            std::string srcPath;

            /**
             * @brief
             @rst
             Where this resource reports progress and problems, or ``nullptr`` for nowhere
             :raw-html:`<br />` :raw-html:`<br />`

             Set by whoever registers the resource (see
             :cpp:func:`IniResEditContext::logger`), rather than passed to a ``fix``. It lives here,
             on the base, for two reasons :raw-html:`<br />` :raw-html:`<br />`

             * every ``fix`` in this hierarchy is declared per concrete leaf with its own signature
               (see this class's own note on why there is no generic one), and each leaf also carries
               a ``fixFunc`` letting a caller replace that fix wholesale. Threading a view through as
               a *parameter* would mean either denying it to those custom fixes -- the case where
               narration matters most -- or reshaping every one of those ``std::function`` typedefs,
               each of which is pinned by a `pybind11`_ binding
             * a plain :cpp:class:`IniResource` is what actually gets registered today, not one of
               the ``Remap`` subclasses, so a view attached only to those would reach nothing

             :raw-html:`<br />`

             A ``shared_ptr``, so a :cpp:class:`BaseLogger` subclass defined in `Python`_ stays alive
             for exactly as long as this resource holds it

             .. note::
                :cpp:class:`IniGroupedResource` is **not** an :cpp:class:`IniResource` -- it is its
                own root -- so it carries its own copy of this member rather than inheriting one
             @endrst
             */
            std::shared_ptr<BaseLogger> logger;
    };

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`IniResource`

     Base class for a resource to be fixed in the .ini file -- mirrors the pure-Python
     ``IniFixResource`` class (``model/iniresources/IniResource.py``); see :cpp:class:`IniResource`'s
     own doc comment for why the generic ``fixFunc`` mechanism isn't carried over to this level
     @endrst
     */
    class IniFixResource: public IniResource {
        public:

            /**
             * @brief Constructs a new resource to be fixed
             *
             * @param type The name for the type of resource
             * @param iniFolderPath The path to the folder of the .ini file
             * @param srcPath The file path to the resource (resolved to an absolute path against 'iniFolderPath')
             * @param fixedPath The file path to the fixed resource (resolved to an absolute path against 'iniFolderPath')
             */
            IniFixResource(std::string type, const std::string& iniFolderPath, const std::string& srcPath, const std::string& fixedPath);

            /**
             * @brief The full file path to the fixed resource
             */
            std::string fixedPath;
    };

    /**
     * @brief
     @rst
     Base class for a group of resources -- mirrors the pure-Python ``IniGroupedResource`` class
     (``model/iniresources/IniResource.py``) :raw-html:`<br />` :raw-html:`<br />`

     Unlike :cpp:class:`IniResource`/:cpp:class:`IniFixResource`, this class keeps a generic
     #fixFunc/#fix/#_fix override mechanism, since (per :cpp:class:`IniResource`'s own doc comment)
     it's actually used directly/bare rather than through a dedicated leaf subclass
     @endrst
     */
    class IniGroupedResource {
        public:

            /**
             * @brief Constructs a new group of resources
             *
             * @param name The name of the group of resources
             * @param resources
             @rst
             The group of resources, keyed by the type of the resource :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                No default value (unlike 'fixFunc'/'isBuilt' below) -- MSVC has a known quirk where
                a by-value parameter of a move-only container type (here, a value type holding
                ``unique_ptr``) with a ``= {}`` default argument fails to compile
                (``std::construct_at``/copy-constructor errors from deep inside ``<xmemory>``, at
                every call site that omits the argument) under some optimization settings, even
                though the empty case is never actually copied. Pass an explicit empty map instead
                of relying on a default here
             @endrst
             * @param fixFunc Custom function for fixing the resource, overriding #_fix if given
             * @param isBuilt Whether the grouped resource is ready to be fixed
             */
            explicit IniGroupedResource(std::string name, std::unordered_map<std::string, std::unique_ptr<IniResource>> resources,
                                         std::function<bool(IniGroupedResource&)> fixFunc = nullptr, bool isBuilt = true);

            virtual ~IniGroupedResource() = default;

            /**
             * @brief
             @rst
             .. note::
                Explicitly declared (rather than left implicit) to work around an MSVC quirk: with
                #resources being a ``std::unordered_map`` of ``unique_ptr``, some MSVC/STL versions
                eagerly instantiate (rather than merely SFINAE-check) the container's own copy
                constructor body while determining whether this class's *implicit* copy constructor
                should be deleted -- and that eager instantiation is itself a hard compile error deep
                inside ``<unordered_map>``/``<xhash>``, even though the deletion determination itself
                is completely valid. Declaring the special members explicitly here sidesteps that
                determination entirely
             @endrst
             */
            IniGroupedResource(const IniGroupedResource&) = delete;
            IniGroupedResource& operator=(const IniGroupedResource&) = delete;
            IniGroupedResource(IniGroupedResource&&) = default;
            IniGroupedResource& operator=(IniGroupedResource&&) = default;

            /**
             * @brief The name of the group of resources
             */
            std::string name;

            /**
             * @brief The group of resources, keyed by the type of the resource
             */
            std::unordered_map<std::string, std::unique_ptr<IniResource>> resources;

            /**
             * @brief Custom function for fixing the resource, overriding #_fix if set
             */
            std::function<bool(IniGroupedResource&)> fixFunc;

            /**
             * @brief Whether the grouped resource is ready to be fixed
             */
            bool isBuilt;

            /**
             * @brief
             @rst
             Where this grouped resource reports progress and problems, or ``nullptr`` for nowhere
             :raw-html:`<br />` :raw-html:`<br />`

             Its own member rather than an inherited one: unlike every other resource here, this
             class is **not** an :cpp:class:`IniResource` -- it is a separate root that merely
             *holds* them. See :cpp:member:`IniResource::logger` for why this is an attribute rather
             than a ``fix`` parameter; the reasoning is the same, and #fixFunc is the same obstacle

             .. note::
                Deliberately independent of the loggers on the resources inside #resources. A group
                and its members are registered separately, and nothing here propagates one to the
                other
             @endrst
             */
            std::shared_ptr<BaseLogger> logger;

            /**
             * @brief Fixes the resource -- calls #fixFunc if set, otherwise #_fix
             *
             * @return Whether the resource was fixed
             */
            bool fix();

            /**
             * @brief
             @rst
             Given a subset of names of the collected resources so far, is this grouped resource
             missing some type of resource from the given subset
             @endrst
             *
             * @param collected The subset of the names of the collected resources so far
             *
             * @return Whether this grouped resource is missing some resource from the specified subset
             */
            bool isMissing(const std::unordered_set<std::string>& collected) const;

            /**
             * @brief Adds an individual resource to the resource group
             *
             * @param resType The name for the type of resource
             * @param resource The resource to add -- ownership is transferred into this group
             */
            void addResource(const std::string& resType, std::unique_ptr<IniResource> resource);

        protected:

            /**
             * @brief Fixes the resource. No-op by default -- see #fixFunc for the real, per-instance behavior
             *
             * @return Whether the resource was fixed
             */
            virtual bool _fix();
    };
}

#endif
