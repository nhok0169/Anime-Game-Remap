#ifndef AGRemapCore_Hashes_H
#define AGRemapCore_Hashes_H

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

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/model/assets/ModMappedAssets.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The hashes related to a mod and its fix -- a :cpp:class:`ModMappedAssets` pre-populated with
     this project's real hash data :raw-html:`<br />` :raw-html:`<br />`

     Unlike :cpp:class:`ModMappedAssets` (a generic, reusable engine), this is one *specific*
     instance of it: its rows (:cpp:func:`Data::getHashDataRows`) and its 3-index
     ``(version, name, type)`` shape are baked in at construction, so a default-constructed
     ``Hashes`` already knows every hash the software ships with (the `Python`_ ``Hashes`` behaves
     the same way: a bare ``Hashes()`` is fully populated), which is why :cpp:member:`ModType::hashes` can fall back to one and still be useful
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The index columns are ``version`` (the version index, at position 0), ``name`` and ``type``.
        :cpp:class:`ModMappedAssets`'s query methods take the two non-version values positionally in
        that order -- there is no name-keyed argument form here, unlike the Python side, where the
        ``ModAssets`` wrapper adds one

     .. note::
        This is the core-side counterpart of the pybind layer's ``PyHashes``, which binds the same
        data as Python's ``Hashes``. The two are deliberately separate classes rather than one
        deriving from the other: the bound one is keyed by ``py::object`` (so Python callers can use
        any hashable), while this one is plain ``std::string``-keyed, which is all
        :cpp:class:`AGRemapCore` needs and keeps the core free of any Python dependency. They share
        the underlying data table (:cpp:func:`Data::getHashDataRows`) and nothing else
     @endrst
     */
    class Hashes: public ModMappedAssets<std::string, std::string> {
        public:

            /**
             * @brief Constructs a new, fully-populated hash lookup table
             *
             * @param map
             @rst
             The initial `adjacency list`_ mapping the hashes to fix **from** to the hashes to fix
             **to** -- see :cpp:func:`ModMappedAssets::getMap` :raw-html:`<br />` :raw-html:`<br />`

             **Default**: empty
             @endrst
             */
            explicit Hashes(std::unordered_map<std::string, std::vector<std::string>> map = {});

            /**
             * @brief A hash type's later generation of the same role's hash: ``<type>Live``, the hash bound once a
             *        texture's mips have streamed in
             */
            static inline const std::string LiveSuffix = "Live";

            /**
             * @brief A hash type's older generations: ``<type>Old``, ``<type>Old2``, ...
             */
            static inline const std::string OldSuffix = "Old";

            /**
             * @brief
             @rst
             How old 'hash' is among 'name''s hashes, lower newer: the versions this table files 'name' at that are newer
             than the one filing 'hash' (100 each), then the generation its hash type names -- ``0`` for a type's current
             hash, ``1`` for its :cpp:member:`LiveSuffix`, ``2``, ``3``, ... for its :cpp:member:`OldSuffix`, ``Old2``,
             ... The lowest over every row filing 'hash' (case-insensitively); ``1 << 20`` when none does
             @endrst
             *
             * @param name The mod name the hashes are filed under
             * @param hash The hash
             *
             * @return The age
             */
            int age(const std::string& name, const std::string& hash) const;

            /**
             * @brief
             @rst
             The hash type a generation of it is filed as, and which generation: ``("bangsDiffuse", 0)`` for
             ``"bangsDiffuse"``, ``1`` for its :cpp:member:`LiveSuffix` (``"bangsDiffuseLive"``), ``2``, ``3``, ... for
             its :cpp:member:`OldSuffix`, ``Old2``, ... -- so every hash a texture has had can be read as the one type
             @endrst
             *
             * @param hashType The hash type, as the table files it
             * @return The base type and the generation
             */
            static std::pair<std::string, int> generationOf(const std::string& hashType);
    };
}

#endif
