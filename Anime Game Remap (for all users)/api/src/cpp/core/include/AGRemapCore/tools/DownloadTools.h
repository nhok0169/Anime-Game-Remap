#ifndef AGRemapCore_DownloadTools_H
#define AGRemapCore_DownloadTools_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniParsers/IniParseDownloadData.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The parts of a character's downloads that are the same for **every** character
     :raw-html:`<br />` :raw-html:`<br />`

     Almost every character needs downloads -- the default `blend`_/position/texcoord buffers and
     head/body textures a mod may be missing -- and before this existed, all of it was spelled out
     again per parser: the GitHub base URL, the :cpp:class:`DownloadData::DownloadConfig`, the
     ``RemapDL`` naming, the resource `KVPs`_ and the ``std::unique_ptr`` bookkeeping. That is a lot
     of identical code to get subtly wrong forty times over :raw-html:`<br />` :raw-html:`<br />`

     What is deliberately **not** here is anything a character actually chooses: which mod objects
     have downloads, which register each hangs off, the strides, and the file names. Those live in
     the character's own ``<Name>Parser.cpp``, which is where somebody adding a character will be
     looking
     @endrst
     */
    class DownloadTools {
        public:
            /**
             * @brief
             @rst
             The **concrete** download class. :cpp:class:`IniParseDownloadData` is the abstract base
             and has no ``DownloadConfig`` of its own, which is a trap worth spelling out once here
             rather than rediscovering per parser
             @endrst
             */
            using Download = DownloadData<>;

            /**
             * @brief The mod object a download is registered against
             */
            using ModObj = GIMIParser<>::ModObj;

            /**
             * @brief The per-mod-object, per-register download map \ref DownloadStore fills in
             */
            using Downloads = GIMIParser<>::Downloads;

            /**
             * @brief A resource `section`_'s `KVPs`_ -- ``type``/``stride``/``format`` and friends
             */
            using KVPs = std::vector<std::pair<std::string, std::string>>;

            DownloadTools() = delete;

            /**
             * @brief
             @rst
             Where every downloadable default part is published -- the pure-Python original's own
             ``GithubDownloadFolder`` (``data/FileDownloadData.py``)
             @endrst
             */
            static const std::string& downloadFolder();

            /**
             * @brief
             @rst
             One row out of :cpp:class:`VertexCountData`, by its ``(version, mod name, component)``
             key :raw-html:`<br />` :raw-html:`<br />`

             ``VertexCountData.h`` exposes only the raw row list -- there is no lookup helper -- so
             this is the small scan the pure-Python original gets for free from a nested dict
             @endrst
             *
             * @param version The game version the row is keyed under, eg. ``"4.0"``
             * @param modName The name of the mod type, eg. ``"Amber"``
             * @param component The component the count belongs to, usually ``""``
             *
             * @return The vertex count, or ``0`` when there is no such row -- which produces a
             *      visibly wrong ``draw = 0,0`` rather than a silently plausible one
             */
            static long long vertexCountOf(const std::string& version, const std::string& modName,
                                            const std::string& component);

            /**
             * @brief
             @rst
             The ``.ini``-domain customization points every download shares -- the plain-``std::string``
             counterpart of what the `pybind11`_ layer supplies for its own ``py::object``
             instantiation. ``filename`` is the register naming a resource's file, and both
             conversions are the identity because ``K`` and ``V`` already *are* ``std::string`` here
             @endrst
             */
            static Download::DownloadConfig makeConfig();

            /**
             * @brief
             @rst
             One downloadable part :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                'fixedName' is what the file is saved as locally, and must carry
                :cpp:member:`IniKeywords::RemapDL` -- that is how the remover later recognises a
                file this software downloaded rather than one the modder shipped. Build it with
                \ref fixedFileName rather than by hand
             @endrst
             *
             * @param name The name for the kind of download, folded into the resource `section`_'s name
             * @param urlPath The path under \ref downloadFolder, eg. ``"GI/Amber/4_0/AmberBlend.buf"``
             * @param fixedName What the downloaded file is saved as locally
             * @param resourceKVPs The `KVPs`_ of the resource `section`_ this creates. **Default**: none
             * @param downloadRefKVPs The `KVPs`_ written after the reference to this download. **Default**: none
             */
            static std::unique_ptr<Download> make(const std::string& name, const std::string& urlPath,
                                                   const std::string& fixedName, KVPs resourceKVPs = {},
                                                   KVPs downloadRefKVPs = {});

            /**
             * @brief
             @rst
             ``<prefix><kind><RemapDL><ext>`` -- eg. ``"AmberBlendRemapDL.buf"``
             @endrst
             *
             * @param prefix The character's own file prefix, eg. ``"Amber"``
             * @param kind What the file holds, eg. ``"Blend"``
             * @param ext The file extension, **including** the dot
             */
            static std::string fixedFileName(const std::string& prefix, const std::string& kind,
                                              const std::string& ext);

            /**
             * @brief
             @rst
             ``GI/<charFolder>/<versionFolder>/<prefix><kind><ext>`` -- the path under
             \ref downloadFolder a download is fetched from :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                'charFolder' and 'prefix' are **separate arguments on purpose**, and composing one
                from the other is the mistake to avoid: ``Raiden/`` holds ``RaidenShogun``-prefixed
                files, and ``AyakaSpringbloom/4_0`` uses a different prefix from the same
                character's ``5_4``. See CreatingRemaps' "The download assets"
             @endrst
             */
            static std::string urlPath(const std::string& charFolder, const std::string& versionFolder,
                                        const std::string& prefix, const std::string& kind,
                                        const std::string& ext);

            /**
             * @brief A plain buffer resource's `KVPs`_ -- ``type = Buffer`` and the given stride
             *
             * @param stride The byte size of one vertex in the buffer
             */
            static KVPs bufResourceKVPs(int stride);

            /**
             * @brief An index buffer resource's `KVPs`_ -- ``type = Buffer`` and a 32-bit uint format
             */
            static KVPs ibResourceKVPs();

            /**
             * @brief
             @rst
             The two `KVPs`_ that must follow a **vb1** blend reference: ``handling = skip`` and
             ``draw = <vertexCount>,0`` :raw-html:`<br />` :raw-html:`<br />`

             Without them the downloaded blend is bound but never drawn from
             @endrst
             *
             * @param vertexCount How many vertices the drawn model has -- see \ref vertexCountOf
             */
            static KVPs blendRefKVPs(long long vertexCount);
    };


    /**
     * @brief
     @rst
     Owns the :cpp:class:`DownloadData` objects a parser hands out by pointer
     :raw-html:`<br />` :raw-html:`<br />`

     :cpp:member:`GIMIParser::downloads` holds **borrowed** pointers, so something has to keep the
     objects alive for exactly as long as the parser. Hold one of these as a parser member,
     declared **before** anything that reads ``downloads``, and add through it
     @endrst
     */
    class DownloadStore {
        public:
            using Download = DownloadTools::Download;
            using ModObj = DownloadTools::ModObj;
            using Downloads = DownloadTools::Downloads;

            /**
             * @brief Registers 'download' under ('modObj', 'reg') in 'downloads', and keeps it alive here
             *
             * @param downloads The parser's own borrowed-pointer map
             * @param modObj The mod object whose graph references this download
             * @param reg The register the download is referenced from, eg. ``"ps-t0"``
             * @param download The download to register -- ownership moves here
             */
            void add(Downloads& downloads, const ModObj& modObj, const std::string& reg,
                      std::unique_ptr<Download> download);

        private:
            std::vector<std::unique_ptr<Download>> owned_;
    };
}

#endif
