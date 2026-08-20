#ifndef AGRemapCore_Version_H
#define AGRemapCore_Version_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Class for representing a version string, following the same `PEP 440`_ grammar and ordering
     rules as Python's `packaging.version.Version`_ -- this is a from-scratch C++ port of that
     class's parsing/normalization/comparison algorithm (``epoch!release.preN.postN.devN+local``),
     not an approximation of it :raw-html:`<br />` :raw-html:`<br />`

     Supports:

     * an optional leading ``v``
     * an optional epoch (``1!2.0``)
     * a release segment of arbitrary length (``2``, ``2.0``, ``2.0.1``, ...)
     * a pre-release segment (``a``/``alpha``, ``b``/``beta``, ``rc``/``c``/``pre``/``preview``,
       each with an optional number)
     * a post-release segment (``.post1``, or the implicit ``-1`` shorthand)
     * a dev-release segment (``.dev1``)
     * a local version segment (``+abc.1.twelve``)
     * surrounding whitespace and case-insensitive letters, both trimmed/normalized away

     Ordering follows `PEP 440`_'s comparison key algorithm exactly: trailing zeros are stripped
     from the release segment before comparing (so ``"1.0"`` and ``"1.0.0"`` compare equal); a
     missing pre-release sorts after any real pre-release *unless* a dev-release is present with
     no post-release, in which case it sorts before (so ``1.0.dev1 < 1.0a1 < 1.0 < 1.0.post1``);
     local version segments sort alphanumeric parts before numeric parts, and a version with no
     local segment always sorts below one that has one
     @endrst
     */
    class Version {
        public:

            /**
             * @brief
             @rst
             Parses a raw version string according to the `PEP 440`_ grammar
             @endrst
             *
             * @param raw The raw version string to parse
             *
             * @return The parsed version, or ``std::nullopt`` if 'raw' does not conform to
             *      `PEP 440`_ in any way
             */
            static std::optional<Version> parse(const std::string& raw);

            /**
             * @brief
             @rst
             Constructs a simple version from an already-parsed release segment, with no epoch,
             pre-release, post-release, dev-release, or local segment -- a convenience for the
             common case (equivalent to ``Version::parse(...)`` on the same dotted-numeric string)
             @endrst
             *
             * @param release The numeric components of the release segment, in order
             */
            explicit Version(std::vector<std::uint64_t> release);

            /**
             * @brief Compares this version against another
             *
             * @param other The other version to compare against
             *
             * @return A negative number if this version is less than 'other', a positive number if
             *      this version is greater than 'other', and zero if they are equal
             */
            int compare(const Version& other) const;

            bool operator<(const Version& other) const;
            bool operator<=(const Version& other) const;
            bool operator>(const Version& other) const;
            bool operator>=(const Version& other) const;
            bool operator==(const Version& other) const;
            bool operator!=(const Version& other) const;

            /**
             * @brief
             @rst
             Converts the version back into its normalized, round-trippable string form (matching
             `packaging.version.Version`_'s own ``__str__``)
             @endrst
             */
            std::string toString() const;

            /**
             * @brief The epoch of the version (``0`` if none was specified)
             */
            std::uint64_t getEpoch() const;

            /**
             * @brief
             @rst
             The numeric components of the release segment, in order, including any trailing
             zeros (e.g. ``Version::parse("2.0.0")->getRelease() == {2, 0, 0}``) -- use
             :cpp:func:`compare` /the comparison operators for a trailing-zero-insensitive
             comparison rather than comparing this directly
             @endrst
             */
            const std::vector<std::uint64_t>& getRelease() const;

            /**
             * @brief The pre-release segment (normalized letter -- ``"a"``, ``"b"``, or ``"rc"``
             *      -- and its number), or ``std::nullopt`` if there is none
             */
            const std::optional<std::pair<std::string, std::uint64_t>>& getPre() const;

            /**
             * @brief The post-release number, or ``std::nullopt`` if there is none
             */
            std::optional<std::uint64_t> getPost() const;

            /**
             * @brief The dev-release number, or ``std::nullopt`` if there is none
             */
            std::optional<std::uint64_t> getDev() const;

            /**
             * @brief The local version segment, dot-joined (e.g. ``"abc.1.twelve"``), or
             *      ``std::nullopt`` if there is none
             */
            std::optional<std::string> getLocal() const;

            /**
             * @brief Whether this is a pre-release (has a pre-release or dev-release segment)
             */
            bool isPrerelease() const;

            /**
             * @brief Whether this is a post-release (has a post-release segment)
             */
            bool isPostrelease() const;

            /**
             * @brief Whether this is a dev-release (has a dev-release segment)
             */
            bool isDevrelease() const;

            /**
             * @brief The first component of #getRelease, or ``0`` if the release segment is empty
             */
            std::uint64_t getMajor() const;

            /**
             * @brief The second component of #getRelease, or ``0`` if unavailable
             */
            std::uint64_t getMinor() const;

            /**
             * @brief The third component of #getRelease, or ``0`` if unavailable
             */
            std::uint64_t getMicro() const;

            /**
             * @brief
             @rst
             The public portion of the version -- #toString without the local segment (mirrors
             `packaging.version.Version`_'s ``.public``)
             @endrst
             */
            std::string getPublic() const;

            /**
             * @brief
             @rst
             The "base version" -- epoch and release segment only, with no pre/post/dev/local
             segment (mirrors `packaging.version.Version`_'s ``.base_version``)
             @endrst
             */
            std::string getBaseVersion() const;

        private:

            /**
             * @brief
             @rst
             A tri-state bound used to compare the pre/post/dev segments the same way `PEP 440`_'s
             own comparison key does: a real ``(letter, number)`` value sorts between the
             ``NegInf``/``PosInf`` extremes used to represent "this segment is absent, and absence
             sorts before/after a real value here" -- see :cpp:func:`Version::compare`'s callers
             for which extreme each segment uses when absent
             @endrst
             */
            struct PrePostDevBound {
                enum class Kind : std::uint8_t { NegInf, Value, PosInf };

                Kind kind = Kind::NegInf;
                std::string letter;
                std::uint64_t number = 0;

                int compare(const PrePostDevBound& other) const;
                bool operator==(const PrePostDevBound& other) const;
            };

            /**
             * @brief One dot-separated segment of a local version, already classified as numeric
             *      or alphanumeric text (lowercased) -- numeric segments always sort after
             *      alphanumeric ones, per `PEP 440`_
             */
            struct LocalSegment {
                bool isNumeric = false;
                std::uint64_t number = 0;
                std::string text;

                int compare(const LocalSegment& other) const;
                bool operator==(const LocalSegment& other) const;
                std::string toString() const;
            };

            Version() = default;

            static std::vector<std::uint64_t> stripTrailingZeros(const std::vector<std::uint64_t>& release);
            static int compareUintVectors(const std::vector<std::uint64_t>& a, const std::vector<std::uint64_t>& b);
            int compareLocal(const Version& other) const;
            std::size_t hashValue() const;

            std::uint64_t epoch_ = 0;
            std::vector<std::uint64_t> release_;
            std::optional<std::pair<std::string, std::uint64_t>> pre_;
            std::optional<std::uint64_t> post_;
            std::optional<std::uint64_t> dev_;
            bool hasLocal_ = false;
            std::vector<LocalSegment> localSegments_;

            // Precomputed once at construction, mirroring packaging.version.Version's own
            // cached `_key` -- see Version.cpp's `_cmpkey`-equivalent for how these are built.
            std::vector<std::uint64_t> strippedRelease_;
            PrePostDevBound preKey_;
            PrePostDevBound postKey_;
            PrePostDevBound devKey_;

            friend struct std::hash<Version>;
    };
}


namespace std {
    /**
     * @brief
     @rst
     Hash specialization for :cpp:class:`AGRemapCore::Version`, owned by this codebase (unlike the
     ``py::object``-keyed containers elsewhere in :cpp:class:`AGRemapCore` that thread a custom
     ``KeyHash``/``KeyEqual`` through instead of specializing ``std::hash`` globally -- see
     :cpp:class:`BaseDFA`'s ``StateHash``/``TransHash`` template params -- specializing here is
     safe since ``AGRemapCore`` fully owns the ``Version`` type) :raw-html:`<br />` :raw-html:`<br />`

     Computed from the same normalized fields :cpp:func:`AGRemapCore::Version::compare` uses (the
     trailing-zero-stripped release, not the raw one), so two versions comparing equal via
     :cpp:func:`AGRemapCore::Version::operator==` always hash equal too
     @endrst
     */
    template <>
    struct hash<AGRemapCore::Version> {
        std::size_t operator()(const AGRemapCore::Version& version) const noexcept;
    };
}

#endif
