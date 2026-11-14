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

#ifndef AGRemapCore_IniSharedState_H
#define AGRemapCore_IniSharedState_H

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>


namespace AGRemapCore {
    class IniFile;

    /**
     * @brief
     @rst
     What the ``.ini`` files of one run share: each file as it reads with no fix in it, and the links its own
     text states to the others :raw-html:`<br />` :raw-html:`<br />`

     Two files are **linked** when either one's text names the other:

     * one references a namespace the other declares (``namespace = X`` in the other; ``$\X\var``,
       ``CommandList\X\...`` or ``Resource\X\...`` in this one), or
     * both name one resource file (``filename =``, resolved against each file's own folder)

     Nothing else links them -- not the folders they sit in, and not the game hashes they share: a mod's files
     can be anywhere, and two mods of one character share every hash. :cpp:class:`RemapService` fills one of
     these with every ``.ini`` file a run reaches, ``DISABLED`` ones included, before it fixes any of them,
     and hands it to each file it fixes (:cpp:member:`IniFile::sharedState`)
     @endrst
     */
    class IniSharedState {
        public:
            IniSharedState();
            ~IniSharedState();

            IniSharedState(const IniSharedState&) = delete;
            IniSharedState& operator=(const IniSharedState&) = delete;

            /**
             * @brief
             @rst
             Reads the ``.ini`` file at 'path' as it is now, and files it with its links. A path already added is
             read again
             @endrst
             *
             * @param path The ``.ini`` file
             */
            void add(const std::string& path);

            /**
             * @brief Forgets every file
             */
            void clear();

            /**
             * @brief The files, in the order they were added
             */
            const std::vector<std::string>& files() const;

            /**
             * @brief
             @rst
             The file at 'path' as it read when it was added -- its own :cpp:class:`IniFile`, never the one a fix
             changes -- or ``nullptr`` for a path not added
             @endrst
             *
             * @param path The ``.ini`` file
             */
            IniFile* file(const std::string& path) const;

            /**
             * @brief
             @rst
             The namespace 'path' declares (``namespace = X``, before its first `section`_), or ``std::nullopt``
             @endrst
             *
             * @param path The ``.ini`` file
             */
            std::optional<std::string> namespaceOf(const std::string& path) const;

            /**
             * @brief The resource files 'path' names, each resolved against its folder, in the order named, each once
             *
             * @param path The ``.ini`` file
             */
            const std::vector<std::string>& resourceFiles(const std::string& path) const;

            /**
             * @brief
             @rst
             The other files linked to 'path' -- see this class's own description -- in the order they were added
             @endrst
             *
             * @param path The ``.ini`` file
             */
            std::vector<std::string> linked(const std::string& path) const;

        private:
            struct Entry;

            static std::string keyOf(const std::string& path);
            const Entry* entryOf(const std::string& path) const;

            std::vector<std::string> files_;
            std::unordered_map<std::string, std::unique_ptr<Entry>> entries_;
    };
}

#endif
