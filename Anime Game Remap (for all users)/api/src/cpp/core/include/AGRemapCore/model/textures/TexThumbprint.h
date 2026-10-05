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


#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Recognizing a texture by its PIXELS rather than by its hash :raw-html:`<br />`
     :raw-html:`<br />`

     A thumbprint is a tiny grayscale reduction of an image -- an ``n`` x ``n`` grid of block
     averages -- and two thumbprints correlate near 1 when they came from the same picture at any
     size, in any format, saved by any tool :raw-html:`<br />` :raw-html:`<br />`

     Why a fix needs this at all: a texture's hash drifts. On Wuthering Waves it drifts with both
     texture streaming and the game version, so a table of "this hash is the diffuse" goes stale,
     and a mod that ships a re-saved copy of a game texture has a hash matching nothing. Comparing
     pixels answers the question the hash was standing in for :raw-html:`<br />`
     :raw-html:`<br />`

     .. note::
        A full-size correlation and a 16 x 16 thumbprint made the same decision on every one of one
        mod's 19 files, which is why the small grid is enough -- and it needs no download of the
        original to compare against, only the numbers (2026-09-19)
     @endrst
     */
    class TexThumbprint {
        public:

            /**
             * @brief
             @rst
             A table of thumbprints by whatever names them -- a hash, a role, a file
             @endrst
             */
            using Table = std::unordered_map<std::string, std::vector<std::uint8_t>>;

            /**
             * @brief
             @rst
             The thumbprint of the image at 'path': an ``size`` x ``size`` grid, row-major, of the
             mean of each block's ``(r + g + b) / 3``, rounded :raw-html:`<br />`
             :raw-html:`<br />`

             Blocks are ``(width / size)`` x ``(height / size)`` and the remainder is dropped, so an
             image whose sides do not divide evenly is thumbprinted from its top-left. Alpha is not
             read at all
             @endrst
             *
             * @param path The image to read
             * @param size The grid's side -- the thumbprint holds 'size' * 'size' values
             *
             * @return The thumbprint, or ``std::nullopt`` if the file will not open, holds no image,
             *         or is smaller than 'size' on either side
             */
            static std::optional<std::vector<double>> of(const std::string& path, int size);

            /**
             * @brief
             @rst
             `Pearson correlation`_ between a thumbprint and a stored one :raw-html:`<br />`
             :raw-html:`<br />`

             Mean-centred, so it answers "is this the same PICTURE" rather than "is this the same
             brightness": a re-saved, re-exposed or re-compressed copy still correlates near 1

             .. _Pearson correlation: https://en.wikipedia.org/wiki/Pearson_correlation_coefficient
             @endrst
             *
             * @param thumb A thumbprint, as \ref of returns it
             * @param stored A thumbprint from a \ref Table
             *
             * @return The correlation in [-1, 1], or ``0`` if the two are different lengths, empty,
             *         or either is perfectly flat (a flat image correlates with nothing)
             */
            static double correlation(const std::vector<double>& thumb, const std::vector<std::uint8_t>& stored);

            /**
             * @brief
             @rst
             Which entry of 'table' 'thumb' IS, if the answer is unambiguous :raw-html:`<br />`
             :raw-html:`<br />`

             The best match has to reach 'minScore' **and** every other entry has to stay under
             'maxRunnerUp'. The second half is what makes a wrong answer rare rather than merely
             unlikely: textures of one character are often near-copies of each other, so "the best
             of these" is a much weaker claim than "this one and nothing else"
             @endrst
             *
             * @param thumb The thumbprint to identify
             * @param table The thumbprints to identify it among
             * @param minScore How well the best match must correlate
             * @param maxRunnerUp How poorly every other entry must correlate
             *
             * @return The winning entry's name, or ``std::nullopt`` if nothing won cleanly
             */
            static std::optional<std::string> identify(const std::vector<double>& thumb, const Table& table,
                                                       double minScore, double maxRunnerUp);

            /**
             * @brief
             @rst
             \ref of followed by \ref identify -- the whole question in one call
             @endrst
             *
             * @param path The image to identify
             * @param table The thumbprints to identify it among
             * @param size The grid's side, which must be the one 'table' was built at
             * @param minScore How well the best match must correlate
             * @param maxRunnerUp How poorly every other entry must correlate
             *
             * @return The winning entry's name, or ``std::nullopt`` if the file will not read or
             *         nothing won cleanly
             */
            static std::optional<std::string> identifyFile(const std::string& path, const Table& table, int size,
                                                           double minScore, double maxRunnerUp);

            /**
             * @brief
             @rst
             Whether every texel of 'channel' holds the same value -- the texture says nothing about
             WHERE anything is :raw-html:`<br />` :raw-html:`<br />`

             Worth asking of a texture whose job is to mark regions, such as a material mask, because
             a constant one is not a neutral one. A mod shipping a mask of ``R = 255`` everywhere is
             not saying "no preference", it is saying "all of this is bare skin", and a fix that
             carries it across faithfully hands the target's shader exactly that :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                A texture that will not open answers ``false``: an unreadable file is not evidence
                that its contents are constant, and the caller's fallback for "no usable file" is a
                different path from its fallback for "no file"
             @endrst
             *
             * @param path The image to read
             * @param channel Which channel to test -- 0 is red, 3 is alpha
             *
             * @return Whether that channel is the same value everywhere
             */
            static bool channelIsConstant(const std::string& path, int channel);
    };
}
