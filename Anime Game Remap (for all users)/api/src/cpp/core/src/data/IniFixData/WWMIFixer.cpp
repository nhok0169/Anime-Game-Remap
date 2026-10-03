#include "AGRemapCore/data/IniFixData/WWMIFixer.h"

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

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixData/ModBranches.h"
#include "AGRemapCore/data/WWMITextureFacts.h"
#include "AGRemapCore/data/WWMITextureRoles.h"
#include "AGRemapCore/data/IniParseData/WWMIParser.h"
#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/iftemplate/IfTemplateRender.h"
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/IniSectionText.h"
#include "AGRemapCore/model/assets/Hashes.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/buffers/BufFloat.h"
#include "AGRemapCore/model/buffers/BufInt.h"
#include "AGRemapCore/model/files/IbFile.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/files/IniScan.h"
#include "AGRemapCore/model/textures/TexThumbprint.h"
#include "AGRemapCore/model/iftemplate/IfTemplateRender.h"
#include "AGRemapCore/model/iniresources/RemapBlendResource.h"
#include "AGRemapCore/model/iniresources/RemapIniResource.h"
#include "AGRemapCore/model/iniresources/RemapTexResource.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIFixer.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFileFixContext.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/GraphRename.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegSurroundedAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupPartEdits.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemap.h"
#include "AGRemapCore/tools/DownloadTools.h"
#include "AGRemapCore/tools/files/FileDownload.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemove.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphRemove.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResRegCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/BlendEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAssetRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemove.h"

#include <stdexcept>
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include "AGRemapCore/tools/NumTools.h"
#include "AGRemapCore/tools/ListTools.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/TextTools.h"
#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {
    namespace {
        using Fixer = GIMIFixer<>;
        using ObjGroupEdit = GraphGroupEdit<>;
        using ModObj = ObjGroupEdit::ModObj;
        using GraphId = GraphRemove<>::GraphId;
        using Collector = ResRegCollect<>;
        using PartEdit = ObjGroupEdit::PartEdit;

        // Hash-data keys and .ini registers spelled literally, as the rest of data/ spells them.
        const std::string Vb0HashKey = "vb0";
        const std::string ShapeKeyChecksumKey = "$\\WWMIv1\\shapekey_checksum";
        const std::string ShapeKeyType = "shapekeys";
        const std::string VgOffsetKey = "$\\WWMIv1\\vg_offset";
        const std::string VgCountKey = "$\\WWMIv1\\vg_count";
        const std::string MeshVertexCountKey = "global $mesh_vertex_count";
        const std::string ConstantsSection = "Constants";
        const std::string BlendBufferResource = "ResourceBlendBuffer";

        // What a ResGroupCollect files the blend's edit under -- this library's vocabulary, not an
        // `.ini` key, so it does not belong in IniKeywords
        const std::string BlendResGroupMember = "blend";
        const std::string IndexBufferResource = "ResourceIndexBuffer";
        const std::string PositionBufferResource = "ResourcePositionBuffer";
        const std::string TexcoordBufferResource = "ResourceTexcoordBuffer";
        const std::string VectorBufferResource = "ResourceVectorBuffer";



        // The one element a texcoord line decodes into: `stride / 2` halves, named so
        // the BufFile filters can find it in the decoded line.
        const std::string TexcoordElement = "Texcoord";
        const std::string Cb4HashKey = "cb4";
        constexpr std::size_t WWMIBlendStride = 8;      // four R8 bone indices then four R8 weights

        // THE HALF CODEC THAT USED TO LIVE HERE IS GONE (2026-09-29). It was a second
        // implementation of `model/buffers/BufFloat.cpp`'s, kept apart because that one TRUNCATES
        // where a folded UV needs round-half-to-even -- and both files carried a comment telling
        // the next reader not to merge them.
        //
        // That was a missing PARAMETER, not two different jobs: `BufFloat16::Rounding::NearestEven`
        // is the mode, and under it decode-then-encode is an exact identity over all 65536 half bit
        // patterns, which is what lets `BufFile::fix` -- it re-encodes every line, touched or not --
        // own this buffer. See `core/tests/BufFloat16_Rounding_test.cpp`.
        const std::string ShapeKeyZero = "ShapeKeyZero";
        const std::string ChecksumNotFound = "ChecksumNotFound";

        // How a file's name may say what type of texture it is, when nothing else does.

        // How far above the .ini file's own folder the mod may reach for its textures: each level
        // climbed has to hold a .ini file of its own (a LOD folder's parent holding the file that
        // declares the textures), so a library of many mods is never indexed as one.


        // The WWMI blend line: N R8 bone indices then N R8 weights (Metadata.json's export_format
        // 'Blend'); the library's default BlendFile layout is GIMI's 32-byte one.
        //
        // N IS PER CHARACTER AND HAS TO BE DERIVED. Sanhua's line is 8 bytes (four influences) and
        // Chisa, ChisaParfait, Augusta, Iuno and Galbrena carry EIGHT (16 bytes). Fixed at four,
        // BufFile reads a 16-byte line as TWO lines -- so BlendFile::remapIndices remaps ids 0-3
        // gated on ids 4-7 read as weights, and then remaps the real WEIGHTS 0-3 through the vertex
        // group table as if they were bone ids. The result is a blend with corrupted weights,
        // written with no error anywhere. Every ChisaParfait mod is 8-influence and none carries a
        // blend remap, so ChisaParfait -> Chisa meets it on every mod.
        //
        // The format name is 3dmigoto's label, read back only when a buffer is written as dump text
        // (VbFile). WWMI Tools' own `.fmt` calls the 8-wide element `R8_UINT`, so that is what an
        // 8-influence line is called here; four keeps the name it has always had, so the shipped
        // characters' output cannot move.
        // A Position.buf is three floats a vertex, and dividing by that is how every lift below
        // learns the vertex count the rest of its work is built on. It was the bare digits at seven
        // sites across four functions until 2026-09-29.
        constexpr std::uintmax_t WWMIPositionStride = 12;

        // What a Texcoord.buf's bytes per vertex are taken to be when the resource declares
        // none of its own.
        constexpr std::size_t DefaultTexcoordStride = 16;

        // The highest bone an 8-bit blend index can name, and the entries in one WWMI blend remap.
        // Past the first, a character needs the remap; past the second, one remap cannot hold the
        // row (see readMod and writeBlendRemap, which check the same fact).
        constexpr long long WWMIMaxByteBone = 255;
        constexpr std::size_t WWMIBlendRemapSize = 512;

        // The D3D semantic names every line of these buffers is keyed by. A filter that asks for
        // the wrong one finds nothing, leaves every vertex alone and reports success, so they are
        // written once.

        // A BLEND LIFT THAT GIVES UP HAS TO SAY WHY (2026-09-29).
        //
        // `RemapService::_fixResource` returning false is recorded as neither fixed nor skipped and
        // logs nothing -- only a thrown exception carries a reason into the summary. So a bare
        // `return false` here is a `.buf` the fix decided it could not write, with no line anywhere
        // saying so, and the mod renders with a part missing.
        //
        // Every one of these is a shape check on the mod's own buffers, so the reason IS the
        // diagnosis. Returns false exactly as before; this only adds the line.
        bool bail(const RemapBlendResource& resource, const std::string& why) {
            // THROWN, not returned. `RemapService::_fixResource` returning false is a bare
            // `continue` -- the file is counted as neither fixed nor skipped and nothing is printed
            // -- where a thrown exception is recorded against the resource and its message appears
            // in the summary's per-file list, which is where "see log above" sends the user.
            // Measured by truncating a mod's Blend.buf: `fixed 0 ... and skipped 0`, for a file the
            // fix had just refused to write (2026-09-29).
            throw std::runtime_error(
                "cannot fix " + FileService::pathToStr(FileService::strToPath(resource.srcPath).filename())
                + ": " + why);
        }


        /**
         * @brief
         @rst
         What one weighted influence of a `blend`_ line becomes -- ``std::nullopt`` to leave the id
         alone. Handed the vertex, which slot of that vertex this is, and the id the buffer holds
         @endrst
         */
        using InfluenceRemap = std::function<std::optional<long long>(std::size_t vertex, std::size_t slot,
                                                                     long long id)>;


        /**
         * @brief
         @rst
         A :cpp:type:`BufFile::Filter` that rewrites the ids of every WEIGHTED influence of each
         `blend`_ line, leaving a weight-zero slot alone as the library's own
         :cpp:func:`BlendFile::remapIndices` does :raw-html:`<br />` :raw-html:`<br />`

         Both of this file's blend passes -- the ordinary remap and the 8-bit lift onto a target past
         256 merged bones -- were this same twenty-line walk with four lines of their own in the
         middle
         @endrst
         *
         * @param vertices How many vertices the mesh has, from its ``Position.buf``
         * @param influences How many (id, weight) slots each vertex carries
         * @param remap What each weighted influence becomes
         *
         * @return The filter to hand :cpp:func:`BufFile::fix`
         */
        BufFile::Filter blendIndexFilter(std::size_t vertices, std::size_t influences, InfluenceRemap remap) {
            return [vertices, influences, remap = std::move(remap)](const BufLineData& line, long long,
                                                                    double index, long long) {
                BufLineData out = line;
                const auto vertex = static_cast<std::size_t>(index);
                if (vertex >= vertices) {
                    return out;
                }

                const auto ids = out.find(BlendFile::BlendIndicesKey);
                const auto weights = out.find(BlendFile::BlendWeightKey);
                if (ids == out.end() || weights == out.end()) {
                    return out;
                }

                for (std::size_t b = 0; b < influences && b < ids->second.size()
                                        && b < weights->second.size(); ++b) {
                    if (bufValueAsFloat(weights->second[b]) == 0) {
                        continue;                                 // a weight-zero slot
                    }

                    const std::optional<long long> to =
                        remap(vertex, b, bufValueAsInt(ids->second[b]));
                    if (to.has_value()) {
                        ids->second[b] = static_cast<unsigned long long>(*to);
                    }
                }

                return out;
            };
        }


        /**
         * @brief
         @rst
         Whether one of an ``.ini``'s resource lists already names 'path' as its source
         @endrst
         *
         * @tparam Lst The list type -- ``IniFile::getResources()`` or ``getFileDownloads()``
         * @param lst The list to search
         * @param path The absolute source path asked about
         *
         * @return Whether the list already holds it
         */
        /**
         * @brief
         @rst
         Every line's values for one element of a ``.buf`` file, in line order -- ``out[line][i]`` is
         the element's i-th data type :raw-html:`<br />` :raw-html:`<br />`

         :cpp:func:`BufFile::decodeAll` is the library's bulk read, documented as "one entry per
         column, each holding one value per line" and walking the elements once into contiguous
         buffers. The alternative this replaces -- a :cpp:type:`BufFile::Filter` over
         ``fix(std::nullopt, ...)`` that pushes each line into a vector -- allocates and hashes a
         :cpp:type:`BufLineData` per line and then picks one element back out of it :raw-html:`<br />`
         :raw-html:`<br />`

         This is :cpp:func:`VGComponentSplit::readBlend` generalised over 'width': that one is fixed
         at FOUR influences and a Chisa mod carries eight, so it would drop half of every vertex
         @endrst
         *
         * @param file The buffer to read
         * @param elementKey The element to take, eg. :cpp:member:`BlendFile::BlendIndicesKey`
         * @param width How many data types of it to keep per line, zero-padded when the file has fewer
         *
         * @return One row per line
         */
        std::vector<std::vector<long long>> bufRows(BufFile& file, const std::string& elementKey,
                                                    std::size_t width) {
            file.read();
            const std::vector<BufFile::BufColumnData> columns = file.decodeAll();

            std::size_t lines = 0;
            for (const BufFile::BufColumnData& column : columns) {
                if (column.elementKey == elementKey) {
                    lines = std::max(lines, std::visit([](auto&& values) { return values.size(); },
                                                       column.values));
                }
            }

            std::vector<std::vector<long long>> rows(lines, std::vector<long long>(width, 0));
            for (const BufFile::BufColumnData& column : columns) {
                if (column.elementKey != elementKey || column.valueInd >= width) {
                    continue;
                }

                std::visit([&rows, &column, lines](auto&& values) {
                    for (std::size_t i = 0; i < values.size() && i < lines; ++i) {
                        rows[i][column.valueInd] = static_cast<long long>(values[i]);
                    }
                }, column.values);
            }

            return rows;
        }


        /**
         * @brief One column of a ``.buf`` file as numbers, in line order -- see \ref bufRows
         *
         * @param file The buffer to read
         * @param elementKey The element to take
         * @param valueInd Which of its data types
         *
         * @return One value per line
         */
        std::vector<double> bufColumn(BufFile& file, const std::string& elementKey, std::size_t valueInd) {
            file.read();
            const std::vector<BufFile::BufColumnData> columns = file.decodeAll();
            for (const BufFile::BufColumnData& column : columns) {
                if (column.elementKey != elementKey || column.valueInd != valueInd) {
                    continue;
                }

                return std::visit([](auto&& values) {
                    std::vector<double> out;
                    out.reserve(values.size());
                    for (const auto& value : values) {
                        out.push_back(static_cast<double>(value));
                    }

                    return out;
                }, column.values);
            }

            return {};
        }


        template <typename Lst>
        bool listsSrcPath(const Lst& lst, const std::string& path) {
            for (const auto& resource : lst) {
                if (resource != nullptr && resource->srcPath == path) {
                    return true;
                }
            }

            return false;
        }


        std::vector<std::unique_ptr<BufElementType>> wwmiBlendElements(std::size_t influences) {
            // `influences` ids then `influences` weights, one unsigned byte each. The four-wide name
            // is what a dump of a four-influence character writes; any other width is named after the
            // type and the count, which is why this no longer carries a `== 4 ?` conditional.
            const auto byte = [] { return std::make_unique<BufUnSignedInt>("UnsignedInt8", 1, false); };
            const std::string format = influences == 4 ? "R8G8B8A8_UINT" : "R8_UINT";

            std::vector<std::unique_ptr<BufElementType>> elements;
            elements.push_back(BufElementType::repeated(BlendFile::BlendIndicesKey, influences, byte, format));
            elements.push_back(BufElementType::repeated(BlendFile::BlendWeightKey, influences, byte, format));
            return elements;
        }

        // The SAME ids as 16-bit, which is what a character past 256 bones keeps in
        // BlendRemapVertexVG.buf -- one element of `influences` of them, no weights. Built here
        // rather than inline at each use: two functions need it, and the pair's whole risk is the
        // two disagreeing about the width.
        /**
         * @brief
         @rst
         The zero shape-key stream's one element -- ``stride`` bytes a vertex, all of them zero
         :raw-html:`<br />` :raw-html:`<br />`

         Declared rather than written as a byte count so the file goes out through
         :cpp:func:`BufFile::fix` like every other buffer the fix writes
         @endrst
         *
         * @param stride How many bytes each vertex carries
         *
         * @return The elements
         */
        std::vector<std::unique_ptr<BufElementType>> zeroStreamElements(int stride) {
            std::vector<std::unique_ptr<BufElementType>> elements;
            elements.push_back(BufElementType::repeated(
                ShapeKeyZero, static_cast<std::size_t>(stride),
                [] { return std::make_unique<BufUnSignedInt>("UnsignedInt8", 1, false); }, "R8_UINT"));

            return elements;
        }


        /**
         * @brief
         @rst
         Puts a back-face twin after EVERY ``drawindexed`` of a part -- see
         :cpp:member:`WWMIFixerConfig::mirroredComponents`
         @endrst
         *
         * Once per path is not enough: ChisaParfait2 draws its skirt as five ranges, each inside its
         * own ``if $Variable... == 1``, and a twin at the end of the section would be drawn whatever
         * the toggles say. Beside the draw it mirrors, it is drawn exactly when that draw is.
         *
         * `ib` and the vector register are restored after each twin, so the next toggle's draw is
         * not left reading the mirrored buffers -- the mod's own names, which the shared-resource
         * list binds and which the graph remap does not rename.
         */
        class MirrorTwin : public BaseRegEdit<> {
            public:
                MirrorTwin(std::string mirrorIndex, std::string mirrorVector, std::string ownIndex,
                           std::string ownVector, std::string vectorReg, long long base)
                    : mirrorIndex_(std::move(mirrorIndex)), mirrorVector_(std::move(mirrorVector)),
                      ownIndex_(std::move(ownIndex)), ownVector_(std::move(ownVector)),
                      vectorReg_(std::move(vectorReg)), base_(base) {}

                ContentPart& edit(ContentPart& part, const std::string& sectionName, const ModType* modType = nullptr,
                                  const std::string& modName = "", const OrderRanges* partRanges = nullptr) override {
                    (void)sectionName;
                    (void)modType;
                    (void)modName;

                    const auto ranges = toRangeSpec(partRanges);
                    const std::vector<std::pair<long long, std::string>> draws =
                        part.getValsWithInds(IniKeywords::DrawIndexed, true, ranges);

                    // BACKWARDS, so an insertion does not move the index of one not yet reached
                    for (auto entry = draws.rbegin(); entry != draws.rend(); ++entry) {
                        const std::optional<std::string> twin = twinDraw(entry->second);
                        if (!twin.has_value()) {
                            continue;                     // `auto`, or a range this cannot read
                        }

                        long long at = entry->first + 1;
                        part.addKVPAt(at++, IniKeywords::Ib, mirrorIndex_);
                        part.addKVPAt(at++, vectorReg_, mirrorVector_);
                        part.addKVPAt(at++, IniKeywords::DrawIndexed, *twin);
                        part.addKVPAt(at++, IniKeywords::Ib, ownIndex_);
                        part.addKVPAt(at++, vectorReg_, ownVector_);
                    }

                    return part;
                }

            private:
                std::string mirrorIndex_;
                std::string mirrorVector_;
                std::string ownIndex_;
                std::string ownVector_;
                std::string vectorReg_;
                long long base_;

                // `count, first, 0` -> the same count at `first - base`, which is where that triangle
                // sits in the mirrored copy of the component's span
                std::optional<std::string> twinDraw(const std::string& value) const {
                    const std::size_t comma = value.find(',');
                    const std::size_t second = value.find(',', comma + 1);
                    if (comma == std::string::npos || second == std::string::npos) {
                        return std::nullopt;
                    }

                    try {
                        const long long count = std::stoll(std::string(StringTools::strip(value.substr(0, comma))));
                        const long long first = std::stoll(
                            std::string(StringTools::strip(value.substr(comma + 1, second - comma - 1))));
                        return std::to_string(count) + ", " + std::to_string(first - base_) + ", 0";
                    } catch (const std::exception&) {
                        return std::nullopt;
                    }
                }
        };


        /**
         * @brief
         @rst
         Re-keys the lines the MOD itself writes to bind a texture inside a component section, from
         the source's register layout to the target's -- see :cpp:member:`WWMIFixerConfig::plan`
         @endrst
         *
         * A mod may bind its textures per DRAW rather than once per section, to give different
         * parts different art. Those lines are copied into the remapped section and land after the
         * fix's own `run = CommandList<Char>Component<N>Textures`, so they win -- in the source's
         * order, which is a role rotation on the target's shader.
         *
         * The role is asked of the RESOURCE, which the texture scan has already decided, and the
         * register is the one the target reads for that role. A resource whose role is unknown is
         * DROPPED rather than guessed: the fix's list has already bound every role the target's
         * draw reads, so dropping leaves a correct binding standing, while keeping one leaves a
         * texture of unknown meaning at a register the target reads for something else.
         */
        /**
         * @brief What a config's role name says the texture IS, with the part it belongs to stripped
         *
         * `upperDiffuse`, `lowerDiffuse` and `panelDiffuse` all have the kind `Diffuse`. The names
         * are the config's own, so the convention is this repo's -- see CarriedTexRegs.
         */
        std::string roleKind(const std::string& role) {
            for (std::size_t i = 0; i < role.size(); ++i) {
                if (std::isupper(static_cast<unsigned char>(role[i])) != 0) {
                    return role.substr(i);
                }
            }

            return "";
        }


        class CarriedTexRegs : public BaseRegEdit<> {
            public:
                /** @brief resource (lowered) -> the role it plays, if the scan knows one */
                using RoleOf = std::function<std::optional<std::string>(const std::string&)>;

                /** @brief role -> the register the TARGET's draw reads it at */
                using RegOf = std::function<std::optional<std::string>(const std::string&)>;

                /** @brief resource + role -> the fix's edited copy of it, where one was written */
                using EditedOf = std::function<std::string(const std::string&, const std::string&)>;

                CarriedTexRegs(std::string regPrefix, RoleOf roleOf, RegOf regOf, EditedOf editedOf)
                    : regPrefix_(StringTools::toLower(regPrefix)), roleOf_(std::move(roleOf)),
                      regOf_(std::move(regOf)), editedOf_(std::move(editedOf)) {}

                ContentPart& edit(ContentPart& part, const std::string& sectionName, const ModType* modType = nullptr,
                                  const std::string& modName = "", const OrderRanges* partRanges = nullptr) override {
                    (void)sectionName;
                    (void)modType;
                    (void)modName;
                    (void)partRanges;

                    // BACKWARDS, so a removal does not move the position of one not yet reached
                    const std::vector<ContentPart::Item> items = part.items();
                    for (auto item = items.rbegin(); item != items.rend(); ++item) {
                        if (!StringTools::startsWith(StringTools::toLower(item->key), regPrefix_)) {
                            continue;
                        }

                        // `ps-t1 = ref ResourceFoo` names the same resource as `ps-t1 = ResourceFoo`
                        std::string value(StringTools::strip(item->value));
                        const std::string ref = StringTools::toLower(IniKeywords::Ref);
                        if (StringTools::startsWith(StringTools::toLower(value), ref + " ")) {
                            value = std::string(StringTools::lstrip(value.substr(ref.size())));
                        }

                        const std::optional<std::string> role = roleOf_(StringTools::toLower(value));
                        const std::optional<std::string> reg = role.has_value() ? regOf_(*role) : std::nullopt;

                        const auto at = static_cast<size_t>(item->orderIndex);
                        part.removeKVPAt(at);
                        if (reg.has_value()) {
                            part.addKVPAt(static_cast<long long>(at), *reg, editedOf_(value, *role));
                        }
                    }

                    return part;
                }

            private:
                std::string regPrefix_;
                RoleOf roleOf_;
                RegOf regOf_;
                EditedOf editedOf_;
        };


        std::vector<std::unique_ptr<BufElementType>> wwmiVertexVGElements(std::size_t influences) {
            std::vector<std::unique_ptr<BufElementType>> elements;
            elements.push_back(BufElementType::repeated(
                BlendFile::BlendIndicesKey, influences,
                [] { return std::make_unique<BufUnSignedInt>("UnsignedInt16", 2, false); }, "R16_UINT"));

            return elements;
        }


        // How many bone influences a vertex the mod's OWN blend carries: its byte stride over the
        // vertex count, halved. 'vertices' is the mod's declared `global $mesh_vertex_count`, and the
        // Position.buf stands in when it has none (a mod-manager-packaged mod declares [Constants]
        // more than once, and the count can come back 0).
        //
        // Falls back to FOUR whenever it cannot be derived, which is exactly the behaviour before
        // this existed -- so a derivation that fails cannot move a shipped character's output.
        /**
         * @brief The pass names of one ``{pass -> bindings}`` table, in its own order
         *
         * @tparam ByPass The table type
         * @param byPass The table
         *
         * @return Its keys
         */
        template <typename ByPass>
        std::vector<std::string> passNamesOf(const ByPass& byPass) {
            std::vector<std::string> names;
            names.reserve(byPass.size());
            for (const auto& [pass, binds] : byPass) {
                (void)binds;
                names.push_back(pass);
            }

            return names;
        }


        /**
         * @brief What a mod's ``Position.buf`` and ``Blend.buf`` sizes say its blend layout is
         */
        struct BlendLayout {
            std::size_t vertices = 0;

            /**
             * @brief How many (id, weight) slots each vertex carries
             */
            std::size_t influences = 0;
        };


        /**
         * @brief
         @rst
         The vertex count and influences per vertex the two buffers' SIZES imply, or ``std::nullopt``
         when they do not imply one :raw-html:`<br />` :raw-html:`<br />`

         Derived rather than assumed: hardcoding the layout is what made the legacy 8-bit lift
         silently do nothing on these same mods, and what read an 8-influence line as two. The
         arithmetic lives here and nowhere else -- the three callers differ only in what they do when
         it fails (bail with a reason, or fall back to four)
         @endrst
         *
         * @param blendPath The mod's own ``Blend.buf``
         * @param positionPath Its ``Position.buf``, which is what gives the vertex count
         *
         * @return The layout, or ``std::nullopt``
         */
        std::optional<BlendLayout> blendLayout(const std::string& blendPath, const std::string& positionPath) {
            const std::optional<std::uintmax_t> positionSize = FileService::fileSize(positionPath);
            if (!positionSize.has_value() || *positionSize < WWMIPositionStride) {
                return std::nullopt;
            }

            const auto vertices = static_cast<std::uintmax_t>(*positionSize / WWMIPositionStride);
            const std::optional<std::uintmax_t> blendSize = FileService::fileSize(blendPath);
            if (vertices == 0 || !blendSize.has_value() || *blendSize == 0 || *blendSize % vertices != 0) {
                return std::nullopt;
            }

            const std::uintmax_t stride = *blendSize / vertices;
            if (stride < 2 || stride % 2 != 0) {
                return std::nullopt;
            }

            return BlendLayout{static_cast<std::size_t>(vertices), static_cast<std::size_t>(stride / 2)};
        }


        std::size_t wwmiBlendInfluences(const std::string& blendPath, long long vertices,
                                        const std::string& positionPath) {
            // The FALLBACK is this caller's own policy, and the one thing it does not share: it
            // exists so a failed derivation cannot move a shipped character's output.
            constexpr std::size_t Fallback = 4;
            const std::optional<BlendLayout> layout = blendLayout(blendPath, positionPath);
            if (layout.has_value() && (vertices <= 0
                                       || static_cast<std::size_t>(vertices) == layout->vertices)) {
                return layout->influences;
            }

            if (vertices <= 0) {
                return Fallback;
            }

            // A caller that already knows the vertex count and whose Position.buf did not answer.
            const std::optional<std::uintmax_t> blendSize = FileService::fileSize(blendPath);
            if (!blendSize.has_value() || *blendSize == 0
                    || *blendSize % static_cast<std::uintmax_t>(vertices) != 0) {
                return Fallback;
            }

            const std::uintmax_t stride = *blendSize / static_cast<std::uintmax_t>(vertices);
            return (stride < 2 || stride % 2 != 0) ? Fallback : static_cast<std::size_t>(stride / 2);
        }

        BaseResEdit<>::ResEditConfig makeResEditConfig() {
            return BaseResEdit<>::ResEditConfig{
                IniKeywords::Filename,
                [](const std::string& value) { return value; },
                [](const std::string& file) { return file; }
            };
        }


        // ---- the blend: the library's vertex-group row, over the WWMI layout ----

        /**
         * RemapBlendReplace's buildResModel builds a straight copy; VGRemapBlendReplace's the GIMI
         * layout. This one is the WuWa layout over the same vertex-group lookup.
         */
        class WWMIBlendReplace: public RemapBlendReplace<> {
            public:
                using Base = RemapBlendReplace<>;
                using GraphId = Base::GraphId;
                using ResEditConfig = Base::ResEditConfig;
                using Context = Base::Context;

                WWMIBlendReplace(GraphId resModObj, ResEditConfig config, const ModType* modType,
                                 std::optional<Version> fromVersion, std::optional<Version> toVersion,
                                 std::function<bool(RemapBlendResource&)> fixFunc = {},
                                 std::map<long long, std::vector<long long>> anchorChains = {},
                                 long long vertices = 0, std::string positionPath = {}):
                    Base(std::move(resModObj), std::move(config), "blend"),
                    modType_(modType), fromVersion_(std::move(fromVersion)), toVersion_(std::move(toVersion)),
                    fixFunc_(std::move(fixFunc)), anchorChains_(std::move(anchorChains)),
                    vertices_(vertices), positionPath_(std::move(positionPath)) {}

            protected:
                void buildResModel(const std::string& resType, const std::string& srcPath, const std::string& fixedPath,
                                   const std::string& modName, const std::string& fileKey, Context& ctx) override {
                    (void)resType;
                    std::optional<VGRemap> vgRemap;
                    if (modType_ != nullptr) {
                        vgRemap = modType_->getVGRemap(modName, fromVersion_, toVersion_);
                    }

                    if (!vgRemap.has_value()) {
                        Base::buildResModel(this->resType, srcPath, fixedPath, modName, fileKey, ctx);
                        return;
                    }

                    // Pin each chain to whatever its ROOT maps to -- see
                    // WWMIFixerConfig::anchorChains. A root the row has no entry for is left alone
                    // rather than guessed at: that would write a target of 0, which is a real bone.
                    if (!anchorChains_.empty()) {
                        std::unordered_map<long long, long long> row = vgRemap->getRemap();
                        for (const auto& [root, members] : anchorChains_) {
                            const auto at = row.find(root);
                            if (at == row.end()) {
                                continue;
                            }

                            for (long long member : members) {
                                row[member] = at->second;
                            }
                        }

                        vgRemap->setRemap(std::move(row));
                    }

                    auto resource = std::make_unique<RemapBlendResource>(
                        ctx.iniFolder(), srcPath, fixedPath, std::move(*vgRemap), this->resType,
                        fixFunc_,
                        wwmiBlendElements(wwmiBlendInfluences(srcPath, vertices_, positionPath_)));
                    resource->logger = ctx.logger();
                    ctx.storeResource(fileKey, std::move(resource));
                }

            private:
                const ModType* modType_;
                std::optional<Version> fromVersion_;
                std::optional<Version> toVersion_;
                std::function<bool(RemapBlendResource&)> fixFunc_;
                std::map<long long, std::vector<long long>> anchorChains_;
                long long vertices_ = 0;
                std::string positionPath_;
        };


        /**
         * The blend of a mod from before WWMI's merged skeleton, remapped.
         *
         * Such a mod's `Blend.buf` holds each component's OWN bone indices (a draw could then only
         * address the bones the game hands it for that component), so each vertex is read as local to
         * the component that DRAWS it, lifted into the source's merged skeleton through that
         * component's `vg_map`, and only then sent through the library's row. Read as merged indices
         * instead, every bone of the body goes somewhere else -- the mod a noodle mess in game.
         *
         * 'drawRanges' is each component's (index count, first index) draws, over 'indexPath'.
         */
        // The remapped blend of a mod whose merged skeleton passes 256 bones: its true ids are
        // the 16-bit ones in BlendRemapVertexVG, and Blend.buf holds them TRUNCATED. The weights
        // come from Blend.buf unchanged; VertexVG holds every vertex, not only the remapped
        // components'.
        /**
         * @brief
         @rst
         The vertex count and influences per vertex the two buffers' SIZES imply, throwing through
         \ref bail when they disagree :raw-html:`<br />` :raw-html:`<br />`

         Derived rather than assumed: hardcoding the layout is what made the legacy 8-bit lift
         silently do nothing on these same mods, and what read an 8-influence line as two. Two
         functions had these eleven lines each
         @endrst
         *
         * @param resource The `blend`_ being fixed, for the failure message
         * @param positionSize Its ``Position.buf``'s size in bytes
         * @param blendSize Its ``Blend.buf``'s size in bytes
         *
         * @return The layout
         */
        BlendLayout blendShape(RemapBlendResource& resource, const std::string& positionPath) {
            const std::optional<BlendLayout> layout = blendLayout(resource.srcPath, positionPath);
            if (!layout.has_value()) {
                bail(resource, "its Blend.buf and Position.buf do not agree on a vertex count and an"
                               " even number of influences per vertex");
            }

            return layout.value_or(BlendLayout{});
        }


        bool remapFromVertexVG(RemapBlendResource& resource, const std::string& vertexVGPath,
                               const std::string& positionPath) {
            // Both buffers through BufFile: integers, so decode-then-encode is exact and the
            // re-encode of the lines nothing touches cannot move them.
            //
            // The layout is derived, not assumed -- hardcoding it is what made the legacy lift
            // silently do nothing on these same mods.
            const BlendLayout shape = blendShape(resource, positionPath);
            const std::size_t vertices = shape.vertices;
            const std::size_t influences = shape.influences;

            std::vector<std::vector<long long>> trueIds;
            try {
                BufFile vgFile{vertexVGPath, wwmiVertexVGElements(influences)};
                if (!vgFile.isValid()) {
                    return bail(resource, "its BlendRemapVertexVG.buf could not be read");
                }

                trueIds = bufRows(vgFile, BlendFile::BlendIndicesKey, influences);
            } catch (const std::exception& exception) {
                return bail(resource, std::string("its BlendRemapVertexVG.buf could not be read: ") + exception.what());
            }

            if (trueIds.size() != vertices) {
                return bail(resource, "its BlendRemapVertexVG.buf holds a different number of vertices than its Position.buf");
            }

            const std::unordered_map<long long, long long>& row = resource.vgRemap.getRemap();
            // The id to remap comes from BlendRemapVertexVG.buf, not from the Blend.buf line: past
            // 256 bones the line's 8-bit id is an index into the remap, and #trueIds holds what it
            // really names.
            const BufFile::Filter remap = blendIndexFilter(
                vertices, influences,
                [&trueIds, &row](std::size_t vertex, std::size_t slot, long long) -> std::optional<long long> {
                    const auto target = row.find(static_cast<long long>(trueIds[vertex][slot]));
                    return target == row.end() ? std::nullopt : std::optional<long long>(target->second);
                });

            try {
                BufFile blend{resource.srcPath, wwmiBlendElements(influences)};
                if (!blend.isValid()) {
                    return bail(resource, "its Blend.buf could not be read");
                }

                blend.fix(resource.fixedPath, {remap});
            } catch (const std::exception& exception) {
                return bail(resource, std::string("its Blend.buf could not be read: ") + exception.what());
            }

            return true;
        }


        // ---- a target past 256 merged bones ------------------------------------------------
        //
        // Where the blend is remapped ONTO a character whose merged skeleton passes 256 slots, the
        // 8-bit ids of a WWMI Blend.buf cannot name the bones the row asks for -- so the fix has to
        // write WWMI's own blend remap, exactly as WWMI Tools writes one for such a character's own
        // mods (blender_export/data_models/data_model_wwmi.py's build_blend_remap, and this repo's
        // Tools/Misc/Prototypes/wwmiIdentityMod.py, whose output renders correctly in game).
        //
        // Four files come out of one pass, because they are four views of one computation:
        //   * the remapped blend      -- the mapped ids TRUNCATED to 8 bits, plus the mod's own
        //                                weights untouched. What a component with no remap reads.
        //   * ...BlendRemapVertexVG   -- every vertex's mapped ids at their full 16 bits
        //   * ...BlendRemapForward    -- 512 uint16 per remapped component, local -> merged
        //   * ...BlendRemapReverse    -- 512 per remap, merged -> local
        //
        // ONE remap for the whole mesh, shared by every component -- where WWMI Tools writes one
        // per component. Its scheme exists so that a mesh using more than 256 bones can still give
        // each component a set that fits; it needs each component's VERTEX SET, and the two answers
        // available here disagree on a real mod -- the section's declared window against the mod's
        // own toggled `drawindexed` ranges. Measured, on the three real ChisaParfait mods: 16433 of
        // one component's 83582 weighted slots landed on the wrong bone, and a mod with a component
        // it never draws shifted every later remap index past the end. The identity mod passed all
        // of it, being the easy case as ever.
        //
        // The union over the whole mesh has no such ambiguity, and it is bounded by the ROW rather
        // than hoped about: this pair's names 182 distinct targets against a remap's 256 entries,
        // and 173-182 are used across the four mods in hand. A union that does NOT fit is refused
        // rather than truncated -- see the check below.
        struct BlendRemapOut {
            std::string vertexVG;
            std::string forward;
            std::string reverse;
        };

        bool writeBlendRemap(RemapBlendResource& resource, const std::string& srcVertexVGPath,
                             const std::string& positionPath, const BlendRemapOut& out) {

            // The layout is derived, never assumed -- hardcoding it is what made the legacy lift
            // silently do nothing on these same mods, and what read an 8-influence line as two. It
            // comes first because it is what says how many influences a line has, which is what the
            // BufFile's elements are built from.
            const BlendLayout shape = blendShape(resource, positionPath);
            const std::size_t vertices = shape.vertices;
            const std::size_t influences = shape.influences;

            // Both buffers through BufFile: integers, so decode-then-encode is exact and the
            // re-encode of the lines nothing touches cannot move them.
            std::vector<std::vector<long long>> ids;                  // per vertex, `influences` of them
            std::vector<std::vector<long long>> weights;
            try {
                BufFile blendFile{resource.srcPath, wwmiBlendElements(influences)};
                if (!blendFile.isValid()) {
                    return bail(resource, "its Blend.buf could not be read");
                }

                ids = bufRows(blendFile, BlendFile::BlendIndicesKey, influences);
                weights = bufRows(blendFile, BlendFile::BlendWeightKey, influences);
            } catch (const std::exception& exception) {
                return bail(resource, std::string("its Blend.buf could not be read: ") + exception.what());
            }

            if (ids.size() != vertices) {
                return bail(resource, "its Blend.buf holds a different number of vertices than its Position.buf");
            }

            // The TRUE ids: the mod's own 16-bit ones when it carries a blend remap of its own,
            // otherwise Blend.buf's, which are the whole truth for a source under 256 bones.
            std::vector<std::uint16_t> trueIds(vertices * influences, 0);
            bool haveVertexVG = false;
            if (!srcVertexVGPath.empty()) {
                try {
                    BufFile vgFile{srcVertexVGPath, wwmiVertexVGElements(influences)};
                    if (vgFile.isValid()) {
                        const std::vector<std::vector<long long>> rows =
                            bufRows(vgFile, BlendFile::BlendIndicesKey, influences);
                        haveVertexVG = rows.size() * influences == trueIds.size();
                        if (haveVertexVG) {
                            for (std::size_t vertex = 0; vertex < rows.size(); ++vertex) {
                                for (std::size_t b = 0; b < influences; ++b) {
                                    trueIds[vertex * influences + b] =
                                        static_cast<std::uint16_t>(rows[vertex][b]);
                                }
                            }
                        }
                    }
                } catch (const std::exception&) {
                    haveVertexVG = false;                  // its own ids are optional; Blend.buf's stand
                }
            }

            if (!haveVertexVG) {
                for (std::size_t vertex = 0; vertex < vertices; ++vertex) {
                    for (std::size_t b = 0; b < influences; ++b) {
                        trueIds[vertex * influences + b] =
                            static_cast<std::uint16_t>(ids[vertex][b]);
                    }
                }
            }

            // ---- map every id through the library's row --------------------------------------
            const std::unordered_map<long long, long long>& row = resource.vgRemap.getRemap();
            std::vector<std::uint16_t> mapped(trueIds);
            for (std::size_t vertex = 0; vertex < vertices; ++vertex) {
                for (std::size_t b = 0; b < influences; ++b) {
                    const std::size_t at = vertex * influences + b;
                    if (weights[vertex][b] == 0) {
                        continue;                                     // a weight-zero slot
                    }

                    const auto target = row.find(static_cast<long long>(trueIds[at]));
                    if (target != row.end() && target->second >= 0
                            && static_cast<std::size_t>(target->second) < WWMIBlendRemapSize) {
                        mapped[at] = static_cast<std::uint16_t>(target->second);
                    }
                }
            }

            // ---- the one map, over the ROW's distinct targets ----------------------------------
            // Not over the bones this mod happens to weight: the .ini naming the remap's bone count
            // is written BEFORE this runs, so the two have to derive it from the same thing, and the
            // row is the only thing neither can change. An entry no vertex reaches costs two bytes.
            std::set<std::uint16_t> used;
            std::set<long long> unaddressable;
            for (const auto& [srcBone, dstBone] : row) {
                (void)srcBone;
                if (dstBone < 0) {
                    continue;
                }

                if (static_cast<std::size_t>(dstBone) < WWMIBlendRemapSize) {
                    used.insert(static_cast<std::uint16_t>(dstBone));
                } else {
                    unaddressable.insert(dstBone);
                }
            }

            if (!unaddressable.empty()) {
                // Refused rather than truncated: a bone this remap cannot name gets no reverse entry,
                // so every vertex weighted to it reads local 0 -- a limb pinned to the root, with
                // nothing in the output to say so. This is a fault in the LIBRARY's vertex group row
                // rather than in the mod, so without the message it is undiagnosable from the
                // outside: the part simply does not render.
                //
                // ASKED ABOUT THE VALUES, NOT THE COUNT (2026-09-30). This was
                // `used.size() > WWMIBlendRemapSize` over a set the loop had already filtered to
                // `[0, WWMIBlendRemapSize)`, so it could not be true -- and the filter that made it
                // unreachable is what caused the pinned limb it refuses. No pair registered today
                // reaches it either way (Chisa's 420 merged slots are the largest), so the fix moves
                // no output; it makes the refusal possible.
                return bail(resource, "the vertex group row maps to target bone "
                                      + std::to_string(*unaddressable.begin()) + ", which a WWMI blend "
                                      "remap of " + std::to_string(WWMIBlendRemapSize)
                                      + " entries cannot name ("
                                      + std::to_string(unaddressable.size()) + " such bone(s))");
            }

            if (used.size() > WWMIBlendRemapSize) {
                return bail(resource, "the vertex group row names " + std::to_string(used.size())
                                      + " distinct target bones, more than the "
                                      + std::to_string(WWMIBlendRemapSize) + " a WWMI blend remap holds");
            }

            std::vector<std::uint16_t> forward(WWMIBlendRemapSize, 0);
            std::vector<std::uint16_t> reverse(WWMIBlendRemapSize, 0);
            std::uint16_t local = 0;
            for (const std::uint16_t merged : used) {
                forward[local] = merged;
                reverse[merged] = local;
                ++local;
            }

            // ---- the blend, with the LOCAL ids the remap addresses ----------------------------
            // NOT the mapped id truncated to a byte. Blend.buf holds one byte per id, and WWMI's
            // BlendRemapper overwrites these bytes at run time with exactly `reverse[mapped]`
            // (`BlendRemapper.hlsl`: `RemappedBlend[v*16+i] = ReverseMap[FullRangeVG[v*8+i]]`), so
            // writing that value here is idempotent with the compute pass and correct without it.
            // A truncated merged id is correct under NEITHER: Chisa's `409 & 0xFF` is 153, a live
            // bone elsewhere on the body, so any frame the pass does not land draws a scrambled
            // mesh rather than nothing -- while every buffer a frame dump can show still measures
            // correct, because the wrong ids are the ones the pass was going to replace.
            // 42% of ChisaParfaitIdentity's 69411 vertices were affected.
            const BufFile::Filter writeIds =
                [&mapped, &reverse, vertices, influences](const BufLineData& line, long long, double index, long long) {
                    BufLineData out = line;
                    const auto vertex = static_cast<std::size_t>(index);
                    const auto found = out.find(BlendFile::BlendIndicesKey);
                    if (vertex >= vertices || found == out.end()) {
                        return out;
                    }

                    for (std::size_t b = 0; b < influences && b < found->second.size(); ++b) {
                        // A bone the row never names keeps the SOURCE's id here, which the
                        // reverse map answers 0 for -- the same answer the compute pass gives.
                        const std::size_t id = mapped[vertex * influences + b];
                        found->second[b] = static_cast<unsigned long long>(
                            id < reverse.size() ? reverse[id] : 0);
                    }

                    return out;
                };

            try {
                BufFile blendOut{resource.srcPath, wwmiBlendElements(influences)};
                if (!blendOut.isValid()) {
                    return bail(resource, "its Blend.buf could not be rewritten");
                }

                blendOut.fix(resource.fixedPath, {writeIds});
            } catch (const std::exception& exception) {
                return bail(resource, std::string("its Blend.buf could not be rewritten: ") + exception.what());
            }

            // Through BufFile, so the width and the byte order come from the element declaration
            // rather than from a `* 2` and a reinterpret_cast -- and so these files are written by
            // the same code that reads them (`wwmiVertexVGElements`), which is what they are read
            // back with.
            const auto write = [&resource](const std::string& path,
                                           const std::vector<std::uint16_t>& ids, std::size_t perLine) {
                FileService::makeFolderFor(path);
                ByteVec bytes;
                bytes.reserve(ids.size() * 2);
                for (const std::uint16_t id : ids) {
                    bytes.push_back(static_cast<std::uint8_t>(id & 0xFF));
                    bytes.push_back(static_cast<std::uint8_t>((id >> 8) & 0xFF));
                }

                try {
                    BufFile out{std::move(bytes), wwmiVertexVGElements(perLine)};
                    if (!out.isValid()) {
                        return bail(resource, "could not build " + path);
                    }

                    out.fix(path);
                } catch (const std::exception& exception) {
                    return bail(resource, "could not write " + path + ": " + exception.what());
                }

                return true;
            };

            if (!write(out.vertexVG, mapped, influences)) {
                return bail(resource, "its BlendRemapVertexVG.buf could not be written");
            }

            return write(out.forward, forward, 1) && write(out.reverse, reverse, 1);
        }


        bool liftLegacyBlend(RemapBlendResource& resource, const std::string& indexPath,
                             const std::string& positionPath,
                             const std::map<int, std::vector<std::pair<long long, long long>>>& drawRanges,
                             const std::map<int, std::vector<int>>& vgMaps) {
            // The index buffer through IbFile: it decodes the triangles, and a flat list of its
            // vertex ids is those triples in order -- which is what a draw range indexes into.
            std::vector<std::uint32_t> indexList;
            try {
                IbFile indices{indexPath};

                // A draw range indexes into the triangles' corners in order, so the rows are
                // flattened back to that one list.
                for (const std::vector<long long>& triangle :
                         bufRows(indices, IbFile::TriangleBufElementKey, IbFile::VerticesPerTriangle)) {
                    for (const long long corner : triangle) {
                        indexList.push_back(static_cast<std::uint32_t>(corner));
                    }
                }
            } catch (const std::exception& exception) {
                return bail(resource, std::string("its index buffer could not be read: ") + exception.what());
            }
            // THE LAYOUT IS DERIVED, NOT ASSUMED. WWMIBlendStride is four R8 ids then four R8
            // weights, which is one WWMI layout and not the only one: Chisa's mods carry EIGHT of
            // each. Reading a 16-byte vertex as two 8-byte ones gives twice the vertex count, so
            // componentOf is indexed by a vertex id that means nothing, most vertices get no
            // component at all and their ids pass through unlifted -- local id 5 of the skirt read
            // as merged bone 5, the jumbled mesh this function exists to prevent.
            // ...asked of the one derivation rather than restated. This block used to compute the
            // vertex count and the stride itself AND call `wwmiBlendInfluences`, then check the two
            // answers against each other -- which is the same arithmetic three times over.
            const BlendLayout shape = blendShape(resource, positionPath);
            const std::size_t vertices = shape.vertices;
            const std::size_t influences = shape.influences;

            const std::size_t indexCount = indexList.size();

            // which component draws each vertex
            std::vector<int> componentOf(vertices, -1);
            for (const auto& entry : drawRanges) {
                for (const auto& range : entry.second) {
                    for (long long k = range.second; k < range.second + range.first && k >= 0; ++k) {
                        if (static_cast<std::size_t>(k) >= indexCount) {
                            break;
                        }

                        const std::uint32_t vertex = indexList[static_cast<std::size_t>(k)];
                        if (vertex < vertices) {
                            componentOf[vertex] = entry.first;
                        }
                    }
                }
            }

            // A blend line is `wwmiBlendElements`' two elements -- `influences` ids then `influences`
            // weights, one unsigned byte each -- so BufFile hands the filter the ids and the weights
            // already separated, and its decode/encode of an integer is exact. That last part is
            // what makes this refactor provable where the texcoord one is not.
            BufFile blend{resource.srcPath, wwmiBlendElements(influences)};
            if (!blend.isValid()) {
                return bail(resource, "its Blend.buf could not be read for the legacy lift");
            }

            const std::unordered_map<long long, long long>& row = resource.vgRemap.getRemap();
            // Here the line's id IS a component-local vertex group, so it is read from the line and
            // carried through that component's own vg_map before the remap row.
            const BufFile::Filter lift = blendIndexFilter(
                vertices, influences,
                [&componentOf, &vgMaps, &row](std::size_t vertex, std::size_t,
                                              long long id) -> std::optional<long long> {
                    const int component = componentOf[vertex];
                    auto vgMap = vgMaps.find(component);
                    if (component < 0 || vgMap == vgMaps.end()) {
                        return std::nullopt;
                    }

                    const auto local = static_cast<std::size_t>(id);
                    if (local >= vgMap->second.size()) {
                        return std::nullopt;
                    }

                    auto target = row.find(vgMap->second[local]);
                    return target == row.end() ? std::nullopt : std::optional<long long>(target->second);
                });

            try {
                blend.fix(resource.fixedPath, {lift});
            } catch (const std::exception& exception) {
                return bail(resource, std::string("its lifted Blend.buf could not be written: ")
                                      + exception.what());
            }

            return true;
        }


        // ---- one WWMI character, out of the library's tables ----

        struct Slot {
            std::string indexOffset;
            std::string indexCount;
            std::string vgOffset;
            std::string vgCount;
        };

        struct Character {
            std::string name;
            std::string vb0Hash;
            std::string cb4Hash;         // the game's bone-data constant buffer, which a legacy mod's merge is gated on
            std::vector<Slot> slots;
        };

        // Everything the fix needs to know about one character, read out of its ModType's tables
        // (the same shape WWMI-Assets' Metadata.json has). A missing row is an error, since a zero
        // here would look like a real value.
        std::optional<Character> readCharacter(const ModType& modType, const std::optional<Version>& version,
                                               const std::string& slotPrefix, std::string& error) {
            Character out;
            out.name = modType.name;
            if (modType.hashes == nullptr || modType.indices == nullptr) {
                error = "the library has no hash or index rows for " + modType.name;
                return std::nullopt;
            }

            std::optional<std::string> hash = modType.hashes->get({modType.name, Vb0HashKey}, version, false);
            if (!hash.has_value()) {
                error = "the library has no '" + Vb0HashKey + "' hash for " + modType.name;
                return std::nullopt;
            }

            out.vb0Hash = StringTools::toLower(*hash);
            out.cb4Hash = StringTools::toLower(modType.hashes->get({modType.name, Cb4HashKey}, version, false).value_or(""));
            while (true) {
                const std::string slot = slotPrefix + std::to_string(out.slots.size());
                std::optional<std::string> first = modType.indices->get({modType.name, "", slot}, version, false);
                if (!first.has_value()) {
                    break;
                }

                std::optional<std::string> count = modType.getIndexCount(slot, "", version);
                std::optional<std::string> vgOffset = modType.getVGOffset(slot, "", version);
                std::optional<std::string> vgCount = modType.getVGCount(slot, "", version);
                if (!count.has_value() || !vgOffset.has_value() || !vgCount.has_value()) {
                    error = "the library has " + modType.name + "'s " + slot + " match_first_index but not all of its"
                            " match_index_count / vg_offset / vg_count";
                    return std::nullopt;
                }

                out.slots.push_back(Slot{*first, *count, *vgOffset, *vgCount});
            }

            if (out.slots.empty()) {
                error = "the library has no draw slots (Indices rows typed " + slotPrefix + "N) for " + modType.name;
                return std::nullopt;
            }

            return out;
        }


        // ---- the mod's textures, by role ----
        //
        // `WWMITextureRoles::Role` and `TextureIndex` MOVED to data/WWMITextureFacts.h (2026-09-29), with the
        // constants and the `Component<N>_<Type>` table they read. Sorting a mod's files into roles
        // is CLASSIFICATION -- it reads only the SOURCE's own textures and never the target, the
        // plan or the fix -- so it belongs where a parser can reach it too, which is the point of
        // the header. The fixer still constructs it here; moving WHEN it runs is the next step.


        // ---- the fixer ----

        class WWMIFixerImpl: public Fixer {
            // DECLARED FIRST, and it has to be: everything Z3 hands out belongs to this
            // context and must not outlive it, which member destruction order decides.
            // ModBranches carries the same note about its own.
            Z3Context z3_;

            public:
                WWMIFixerImpl(BaseIniParser<>* parser, const std::string& toModName, std::optional<int> modTypeId,
                              WWMIFixerConfig config):
                    Fixer(parser, nullptr, {},
                          toModName.empty() ? std::optional<std::vector<std::string>>(std::nullopt)
                                            : std::optional<std::vector<std::string>>({toModName}),
                          nullptr, makeConfig()),
                    ctx_(parser != nullptr ? parser->getIniFile() : nullptr, modTypeId), toModName_(toModName),
                    config_(std::move(config)) {
                    this->setCtx(&ctx_);
                    this->copyPreamble = config_.copyPreamble;

                    std::string error;
                    if (!readCharacters(error) || !readMod(error)) {
                        if (!error.empty()) {
                            ctx_.log(error + " -- so this .ini is left alone");
                        }

                        giveUp();
                        return;
                    }

                    readTextures();
                    planTexEdits();
                    buildEdits();
                    buildAppended();

                    this->graphGroupEdits.clear();
                    this->graphGroupEdits.push_back(slotRemap_.get());
                    this->graphGroupEdits.push_back(mainEdits_.get());
                    for (auto& collect : blendCollects_) {
                        this->graphGroupEdits.push_back(collect.get());
                    }

                    // The copies are the prototype's shape: each carries the fix's own sections (the
                    // texture command lists, the shader tags, the resources) and none of the mod's
                    // own draw sections live -- the mod's own file keeps serving the mod on its
                    // source character, and a copy exists only for one more claimant of a draw.
                    this->appendedSectionsInCopies = true;
                    for (const auto& entry : ctx_.getIniFile()->getIfTemplates()) {
                        if (StringTools::startsWith(entry.first, IniKeywords::TextureOverride)) {
                            this->copyHiddenSectionNames.insert(entry.first);
                        }
                    }
                }

            protected:
                // GIMIFixer's own hands every group edit a nullptr .ini file, which stops a collect
                // ever building anything -- see GIMICharFixerImpl. The files the fix writes outside
                // the resource system (the zero stream) and the resources it adds by hand (the
                // created textures) go in here too, at fix time, so a parse alone writes nothing.
                void applyGraphGroupEdits(const std::string& modName) override {
                    if (this->graphGroups() == nullptr) {
                        return;
                    }

                    if (!gaveUp_) {
                        writeZeroStream();
                        writeMirrorBuffers();
                        addCreatedTextures();
                        addFallbackDownloads();
                        addTexEdits();
                    }

                    for (Fixer::GroupEdit* edit : this->graphGroupEdits) {
                        if (edit != nullptr) {
                            edit->editFromIni(*this->graphGroups(), ctx_.getIniFile(), nullptr, modName);
                        }
                    }

                    rebindBlendOverride();
                }

                // A MOD OF A CHARACTER PAST 256 BONES BINDS THE BLEND REGISTER TWICE: `vb4 =
                // ResourceBlendBuffer` (a file) and `vb4 = ref ResourceBlendBufferOverride` (a buffer
                // WWMI's BlendRemapper fills at load). The collect takes only the first -- the second
                // names no file, and collecting it kills the run looking for a section called
                // `Resourceref Resource...` -- so without this the second keeps pointing at the
                // SOURCE's own override, which is an inverse of the whole remap.
                //
                // Here rather than at render time: this runs after every group edit, including the
                // GraphGroupRemap that makes the copies, so the copies are in the graphs and the
                // rewrite is one RegNewVals over them. It used to be a pass over the fix's own
                // rendered TEXT -- `getline`, find the `=`, splice lines back -- which is the section
                // model written out and read straight back in. Fires on 19 of the 58 WuWa mod folders
                // on disk, all of them this character's (2026-09-29).
                // The context's log is not const where the callers are, so this is the one place
                // that casts it away.
                void note(const std::string& message) const {
                    const_cast<IniFileFixContext&>(ctx_).log(message);
                }

                void rebindBlendOverride() {
                    if (gaveUp_ || config_.blendReg.empty() || this->graphGroups() == nullptr) {
                        return;
                    }

                    Fixer::GraphGroups& groups = *this->graphGroups();
                    for (std::size_t g = 0; g < groups.size(); ++g) {
                        // The remapped blend as the GRAPH spells it, which is what the collect wrote
                        // -- read rather than rebuilt, so the two cannot drift apart.
                        std::string remapped;
                        forEachPart(groups, g, [&](ContentPart& part) {
                            if (!remapped.empty()) {
                                return;
                            }

                            for (const std::string& value : part.getVals(config_.blendReg)) {
                                if (!IniNamingTools::hasRefPrefix(value)
                                        && value.find(IniKeywords::Remap) != std::string::npos) {
                                    remapped = std::string(StringTools::strip(value));
                                    break;
                                }
                            }
                        });

                        if (remapped.empty()) {
                            continue;                   // nothing remapped this group's blend
                        }

                        RegNewVals<> rebind({{config_.blendReg, RegNewVals<>::NewVal(
                            RegNewVals<>::OldValProducer(
                                [&remapped](const std::string& oldValue, const ModType*) {
                                    return IniNamingTools::hasRefPrefix(oldValue) ? remapped : oldValue;
                                }))}});

                        forEachPart(groups, g, [&](ContentPart& part) {
                            rebind.edit(part, "");
                        });
                    }
                }

                using ContentPart = IfContentPart<std::string, std::string>;

                // Every content part of every section of every graph in one group.
                template <typename Fn>
                void forEachPart(Fixer::GraphGroups& groups, std::size_t groupInd, Fn&& fn) const {
                    for (const ModObj& obj : groups.modObjs(groupInd)) {
                        auto* graph = groups.getGraph(groupInd, obj);
                        if (graph == nullptr) {
                            continue;
                        }

                        for (const auto& entry : graph->sections()) {
                            if (entry.second == nullptr) {
                                continue;
                            }

                            for (const std::unique_ptr<IfTemplatePart>& part : entry.second->parts()) {
                                auto* content = dynamic_cast<ContentPart*>(part.get());
                                if (content != nullptr) {
                                    fn(*content);
                                }
                            }
                        }
                    }
                }

            private:
                using FixerConfig = Fixer::FixerConfig;

                static FixerConfig makeConfig() {
                    FixerConfig config{};
                    config.sectionToStr = &renderIfTemplate;
                    return config;
                }

                // Gives up: the fixer writes NOTHING (see GraphGroupRemove and GIMIComponentFixerImpl).
                void giveUp() {
                    gaveUp_ = true;
                    this->graphGroupEdits = {&removeEveryGroup_};
                }

                std::string fixName(const std::string& name) const {
                    return IniNamingTools::getRemapFixName(name, toModName_);
                }

                ModObj slotObj(int component) const {
                    return ModObj("", config_.slotPrefix + std::to_string(component));
                }

                // Is 'hash' a vb0 the library files under the SOURCE -- at any game version, not
                // only the one `fromVersion()` resolves to?
                //
                // GlobalIniClassifiers registers every version's vb0 for a reason it states: a
                // mod's .ini carries whichever version's hashes its author dumped, and nothing can
                // tell which that was. Matching only the resolved one here made the two disagree
                // the moment a character's vb0 moved -- ChisaParfait's did at WuWa 3.7 -- so a mod
                // of the other generation classified as her and then found no section of its own.
                // The caller's slot test is what keeps this safe: the section must still carry a
                // match_first_index one of the source's slots declares AT `fromVersion()`, so this
                // can only turn a rejection into a match where the geometry agrees too.
                bool isSourceVb0(const std::string& hash) const {
                    if (hash == source_.vb0Hash) {
                        return true;
                    }

                    const ModType* source = ctx_.modType();
                    if (source == nullptr || source->hashes == nullptr) {
                        return false;
                    }

                    return source->hashes->hasFrom(hash, std::nullopt,
                                                   {std::optional<std::string>(source->name),
                                                    std::optional<std::string>(Vb0HashKey)});
                }

                std::optional<Version> fromVersion() const {
                    std::optional<Version> version = ctx_.version();
                    if (!version.has_value()) {
                        // the SOURCE's own version, which is not the target's when the pair is not
                        // filed under one -- see WWMIFixerConfig::sourceVersion for what a
                        // reverse-then-forward lookup does with the wrong one
                        version = Version::parse(config_.sourceVersion.empty() ? config_.version
                                                                               : config_.sourceVersion);
                    }

                    return version;
                }

                std::optional<Version> toVersion() const {
                    const IniFile* ini = ctx_.getIniFile();
                    if (ini != nullptr && ini->toVersion.has_value()) {
                        return ini->toVersion;
                    }

                    return Version::parse(config_.version);
                }

                // ---- the two characters ----

                bool readCharacters(std::string& error) {
                    const ModType* source = ctx_.modType();
                    if (source == nullptr) {
                        error = "the fixer has no source mod type";
                        return false;
                    }

                    targetType_ = ModTypeIdTools::getModType(static_cast<int>(config_.targetId));
                    if (!targetType_.has_value()) {
                        error = "the library has no mod type for the target";
                        return false;
                    }

                    std::optional<Character> src = readCharacter(*source, fromVersion(), config_.slotPrefix, error);
                    if (!src.has_value()) {
                        return false;
                    }

                    std::optional<Character> dst = readCharacter(*targetType_, toVersion(), config_.slotPrefix, error);
                    if (!dst.has_value()) {
                        return false;
                    }

                    source_ = std::move(*src);
                    target_ = std::move(*dst);

                    // The names a fix of this .ini could have written its sections under -- the
                    // source's and the target's. IniFileRemoveContext::modTypeNames builds the same
                    // list for the undo, from every type the .ini classified as and everything each
                    // remaps onto; a fixer is built for ONE pair, which is this one.
                    remapNames_ = {source_.name, target_.name};
                    return true;
                }

                // ---- what the mod's .ini has: its slot sections, its vertex count, its folders ----

                bool readMod(std::string& error) {
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        error = "the fixer has no .ini file";
                        return false;
                    }

                    // The fixer is built BEFORE the parser parses, so the sections are found by hash
                    // over IniFile::getIfTemplates rather than through the parser's graphs.
                    const auto& templates = ini->getIfTemplates();
                    for (const auto& entry : templates) {
                        if (entry.second == nullptr) {
                            continue;
                        }

                        const IfTemplate<std::string, std::string>& tpl = *entry.second;
                        std::optional<std::string> hash = ModBranches::firstVal(tpl, IniKeywords::Hash);
                        std::optional<std::string> index = ModBranches::firstVal(tpl, IniKeywords::MatchFirstIndex);
                        if (!hash.has_value() || !index.has_value() || !isSourceVb0(StringTools::toLower(*hash))) {
                            continue;
                        }

                        for (std::size_t i = 0; i < source_.slots.size(); ++i) {
                            if (source_.slots[i].indexOffset == *index) {
                                present_[static_cast<int>(i)].push_back(entry.first);
                            }
                        }
                    }

                    if (present_.empty()) {
                        error = "no [TextureOverrideComponent*] section on any vb0 hash the library"
                                " files under " + source_.name + " (" + source_.vb0Hash + " at the version asked)";
                        return false;
                    }

                    // A mod from before WWMI's merged skeleton: no section of it carries a vg_offset,
                    // because a draw could then only address its own component's bones -- so its blend
                    // holds each component's OWN indices. See WWMIFixerConfig::sourceVgMaps.
                    // Its own draw ranges, per source component. Read for EVERY mod, not only a
                    // legacy one: this began as the per-component lift of a legacy blend, and two
                    // later readers want the same ranges for reasons that have nothing to do with the
                    // blend layout -- the remap of a toggled draw through the split, and
                    // TexEditContext::drawRanges, which is how a texture edit knows what a component
                    // covers. Left inside `if (legacy_)` the map was empty on every mod past 256
                    // bones, so the ribbon's colour-grade island found no geometry and the edit
                    // graded nothing while still writing its file and reporting a success.
                    for (const auto& entry : present_) {
                        for (const std::string& section : entry.second) {
                            auto tpl = templates.find(section);
                            if (tpl == templates.end() || tpl->second == nullptr) {
                                continue;
                            }

                            std::size_t partIndex = 0;
                            for (const auto& part : tpl->second->parts()) {
                                ++partIndex;
                                const auto* content = dynamic_cast<const IfTemplate<std::string, std::string>::ContentPart*>(part.get());
                                if (content == nullptr) {
                                    continue;
                                }

                                for (const std::string& draw : content->getVals(IniKeywords::DrawIndexed)) {
                                    // WHICH part, for the back-face twin: all of a component's draws
                                    // in one part means all of them under one condition, which is
                                    // what makes a single block of twins at the end of it right.
                                    drawParts_[entry.first].insert(partIndex);
                                    const std::size_t comma = draw.find(',');
                                    const std::size_t second = draw.find(',', comma + 1);
                                    if (comma == std::string::npos || second == std::string::npos) {
                                        continue;
                                    }

                                    try {
                                        drawRanges_[entry.first].emplace_back(
                                            std::stoll(std::string(StringTools::strip(draw.substr(0, comma)))),
                                            std::stoll(std::string(StringTools::strip(draw.substr(comma + 1, second - comma - 1)))));
                                    } catch (const std::exception&) {
                                        // Draw ranges decide which triangles of a toggled object are
                                        // drawn, so one dropped in silence is a piece of the mesh
                                        // that stops being remapped for no stated reason.
                                        note("could not read the draw range `"
                                             + std::string(StringTools::strip(draw)) + "` in ["
                                             + section + "], so it is not remapped");
                                        continue;
                                    }
                                }
                            }
                        }
                    }

                    legacy_ = true;
                    for (const auto& entry : present_) {
                        for (const std::string& section : entry.second) {
                            auto tpl = templates.find(section);
                            if (tpl != templates.end() && tpl->second != nullptr
                                && ModBranches::firstVal(*tpl->second, VgOffsetKey).has_value()) {
                                legacy_ = false;
                            }
                        }
                    }

                    if (legacy_) {
                        if (config_.sourceVgMaps.empty()) {
                            error = "this mod is from before WWMI's merged skeleton (no `" + VgOffsetKey + "` in any of its sections), "
                                    "so its blend holds each component's OWN bone indices -- and " + source_.name
                                    + "'s config carries no sourceVgMaps to lift them with. Remapping them as merged indices would "
                                      "scramble every bone of the body, so this mod is left alone";
                            return false;
                        }

                        note("this mod is from before WWMI's merged skeleton: its blend is read per component and lifted "
                             "through " + source_.name + "'s vg_map, and the fix supplies the merged skeleton it lacks");
                    }

                    auto constants = templates.find(ConstantsSection);
                    if (constants != templates.end() && constants->second != nullptr) {
                        std::optional<std::string> count = ModBranches::firstVal(*constants->second, MeshVertexCountKey);
                        if (count.has_value()) {
                            try {
                                meshVertexCount_ = std::stoll(*count);
                            } catch (const std::exception&) {
                                // 0 is not a harmless fallback: it turns the zero shape-key stream
                                // OFF (writeZeroStream returns on <= 0), which is the wavy-vertices
                                // fault that stream exists to remove.
                                note("this mod's " + MeshVertexCountKey + " is `" + *count
                                     + "`, which is not a number, so the zero shape-key stream is"
                                     + " not written");
                                meshVertexCount_ = 0;
                            }
                        }
                    }

                    auto index = templates.find(IndexBufferResource);
                    if (index != templates.end() && index->second != nullptr) {
                        std::optional<std::string> file = ModBranches::firstVal(*index->second, IniKeywords::Filename);
                        if (file.has_value()) {
                            indexFile_ = *file;
                        }
                    }

                    // ...and the position and texcoord buffers, off the .ini for the same reason the
                    // index buffer is: a mod-manager-packaged mod names every file by GUID with a
                    // `.assets` extension, so there is no `Position.buf` beside anything. Built as a
                    // sibling name, the path did not exist, the remapped blend was never written, and
                    // the .ini still bound vb4 to it -- which drew NOTHING of the character.
                    //
                    // WITHOUT REGARD TO CASE: one mod spells it ResourceTexCoordBuffer.
                    for (const auto& entry : templates) {
                        if (entry.second == nullptr) {
                            continue;
                        }

                        std::string* into = nullptr;
                        if (StringTools::equalsIgnoreCase(entry.first, PositionBufferResource)) {
                            into = &positionFile_;
                        } else if (StringTools::equalsIgnoreCase(entry.first, TexcoordBufferResource)) {
                            into = &texcoordFile_;
                        } else if (StringTools::equalsIgnoreCase(entry.first, VectorBufferResource)) {
                            into = &vectorFile_;
                        }

                        if (into == nullptr) {
                            continue;
                        }

                        std::optional<std::string> file = ModBranches::firstVal(*entry.second, IniKeywords::Filename);
                        if (file.has_value()) {
                            *into = *file;
                        }
                    }

                    // WHERE the 16-bit ids live, off the .ini rather than a sibling filename: a
                    // mod-manager-packaged mod names every file by GUID with a `.assets` extension,
                    // so the WWMI export's own name is not beside Blend.buf and the search finds
                    // nothing -- and the fallback that then runs is a no-op dressed as a fix.
                    for (const auto& entry : templates) {
                        if (entry.second == nullptr) {
                            continue;
                        }

                        std::string name = StringTools::toLower(entry.first);
                        name.erase(std::remove(name.begin(), name.end(), '_'), name.end());
                        if (name.find("blendremapvertexvg") == std::string::npos) {
                            continue;
                        }

                        std::optional<std::string> file = ModBranches::firstVal(*entry.second, IniKeywords::Filename);
                        if (file.has_value()) {
                            vertexVGFile_ = *file;
                            break;
                        }
                    }

                    // Beside the .ini until the mod says otherwise -- see modFile. A mod whose
                    // buffers sit next to its .ini (Chisa12: GUID names, `.assets` buffers) had a
                    // `Meshes/` built for it holding nothing but the fix's own output.
                    meshFolder_.clear();
                    auto blend = templates.find(BlendBufferResource);
                    if (blend != templates.end() && blend->second != nullptr) {
                        std::optional<std::string> file = ModBranches::firstVal(*blend->second, IniKeywords::Filename);
                        if (file.has_value()) {
                            const std::string forward = FileService::iniPathToRel(*file);

                            // `parentOf` answers "" for a file with no folder part, which is the
                            // mod root -- exactly what meshFolder_ means by empty.
                            meshFolder_ = FileService::parentOf(forward);

                            // Kept whole, because the name of the blend the FIX writes is derived
                            // from it and not from the character -- see blendFixedFile().
                            blendSourceFile_ = *file;

                            // The mod's OWN blend line, derived from its own file rather than the
                            // `stride` it declares: a declaration can disagree with the bytes, and
                            // this is the number WWMI's BlendRemapper is handed.
                            const std::string blendPath =
                                FileService::absPathOfRelPath(forward, ctx_.getIniFile()->getFolder());
                            const std::string positionPath =
                                positionBufPath();
                            blendInfluences_ = wwmiBlendInfluences(blendPath, meshVertexCount_, positionPath);
                            blendStride_ = blendInfluences_ * 2;
                        }
                    }

                    return true;
                }

                // ---- the textures: the run's index, and what this .ini binds of it ----

                // WHICH of the mod's own resources it binds in SEVERAL VARIANTS. A toggle mod writes `if $hair == 0 / this = X / else / this = XA` in its
                // [TextureOverrideTexture*], and a single `<reg> = <resource>` line in the fix can
                // only ever carry one of those -- so the mod's texture toggle stopped working while
                // the toggled `drawindexed` in its own section kept switching the geometry.
                //
                // A section that binds unconditionally with one value is not recorded, and keeps its
                // direct register line: every Sanhua mod is that shape, and this must not move output
                // that has been verified in game.
                void readConditionalBindings() {
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        return;
                    }

                    for (const auto& entry : ini->getIfTemplates()) {
                        if (entry.second == nullptr
                            || !StringTools::startsWith(entry.first, (IniKeywords::TextureOverride + "Texture"))) {
                            continue;
                        }

                        std::vector<std::string> bound;
                        for (const auto& part : entry.second->parts()) {
                            const auto* content =
                                dynamic_cast<const IfTemplate<std::string, std::string>::ContentPart*>(part.get());
                            if (content == nullptr) {
                                continue;
                            }

                            for (const std::string& val : content->getVals(IniKeywords::This)) {
                                bound.push_back(std::string(StringTools::strip(val)));
                            }
                        }

                        // SEVERAL resources, which is what a toggle is. A section binding ONE
                        // resource keeps its direct register line even when it sits under an `if`:
                        // every Sanhua mod wraps its one `this` in `if $object_detected`, which the
                        // remapped section has already set to 1 by the time the list runs, so a copy
                        // of it would be the same binding in more lines -- and moving output that has
                        // been verified in game to say the same thing is not worth it.
                        //
                        // The gap that leaves: one `this` under a REAL condition and no else (`if
                        // $hair == 0 / this = X / endif`) still binds X unconditionally. That is what
                        // the fix did before this, so nothing regresses -- but a mod that renders
                        // wrong on one toggle with a single-variant role is where to look next.
                        if (bound.size() < 2) {
                            continue;
                        }

                        // TWO SECTIONS MAY BIND ONE RESOURCE BEHIND A TOGGLE, AND ONLY ONE OF THEM
                        // MAY BE WHOLE (2026-09-28). SanhuaExorcist4 selects `7 / 7a / 7b` in
                        // `[TextureOverrideTexture7]` and `7 / 7.1 / 7.2` in
                        // `[TextureOverrideTexture7_injured]`, where the author typed names the mod
                        // never declares. Last-writer-wins gave the copy the typo'd one -- one live
                        // branch of three, so on the other two values the TARGET's own diffuse stayed
                        // bound at the mod's UVs. Count the branches that name a section this .ini
                        // has, and keep the section that answers for most of its own variants.
                        const auto& templates = ini->getIfTemplates();
                        const auto resolves = [&templates](const std::vector<std::string>& vals) {
                            std::size_t n = 0;
                            for (const std::string& val : vals) {
                                if (templates.count(val) > 0) {
                                    ++n;
                                }
                            }

                            return n;
                        };

                        const std::size_t score = resolves(bound);
                        for (const std::string& resource : bound) {
                            const std::string key = StringTools::toLower(resource);
                            const auto owner = conditionalOwner_.find(key);
                            if (owner != conditionalOwner_.end()) {
                                const auto held = variantsOf_.find(owner->second);
                                if (held != variantsOf_.end() && resolves(held->second) >= score) {
                                    continue;
                                }
                            }

                            conditionalOwner_[key] = entry.first;
                        }

                        variantsOf_[entry.first] = bound;
                    }
                }

                // WHAT readTextures WORKS OUT, IN THE ORDER IT WORKS IT OUT.
                //
                // A struct rather than members: this is scan state, dead the moment the scan ends,
                // and passing it explicitly is what makes the order the phases run in visible at the
                // call site. They were one 650-line function sharing these by scope, which is how
                // `best` came to mean two different things in it.
                struct TextureScan {
                    const WWMITextureRoles* roles = nullptr;
                    std::string iniFolder;
                    std::string iniPath;

                    // the mod's own file -> the resource section naming it
                    std::unordered_map<std::string, std::string> resourceOfFile;

                    // role -> (file, how the role was decided), every role of every file
                    std::map<std::string, std::vector<std::pair<std::string, std::string>>> byRole;

                    // file -> source component -> the roles that component's own register bindings
                    // offer it for. One file offered two roles of ONE slot is the mask alias.
                    std::map<std::string, std::map<int, std::set<std::string>>> regRolesOfFile;

                    // role -> the source components that can draw it, and the components the plan has
                    std::unordered_map<std::string, std::set<int>> componentsOfRole;
                    std::set<int> sourceComponents;


                    // file -> the roles the mod DECLARES it as, by writing `hash = <h>` over it.
                    // Stronger than a register, which says where a file is used rather than what it
                    // is; see rankCandidate.
                    std::map<std::string, std::set<std::string>> declaredRolesOf;

                    // an ambiguity between two files is reported once, however many components are
                    // offered the role
                    std::set<std::string> saidAmbiguous;
                };

                void readTextures() {
                    TextureScan scan;
                    IniFile* ini = ctx_.getIniFile();
                    scan.iniFolder = ini->getFolder();
                    scan.iniPath = ini->getFile().value_or("");
                    readConditionalBindings();
                    // WHAT THE MOD'S TEXTURES ARE IS THE PARSER'S ANSWER, NOT THIS ONE'S.
                    // A texture is identified by a HASH and a REGISTER -- the hash being the
                    // texture's own (the section IS that texture) or the mesh's (the section is a
                    // draw the classifier placed, and the register says what it binds) -- which is
                    // section classification, so the parser does it and this asks.
                    //
                    // What it replaced walked the folder, climbing up to three parents, and guessed
                    // each file from its name, a hash inside its name, or its pixels. A file the mod
                    // never binds was therefore a candidate: Chisa7 keeps spare colourways beside
                    // the installed one and a spare won `upperDiffuse`, which is what turned a
                    // kimono red. Over ten mods the scan offered 51 such files; none of them is a
                    // candidate now (2026-09-29).
                    if (const auto* facts = dynamic_cast<const WWMIParseFacts*>(this->getParser())) {
                        scan.roles = facts->textureRoles();
                    }

                    if (scan.roles == nullptr) {
                        // Every WuWa character's parse row carries its textures, so this is a
                        // configuration error rather than a mod's doing -- and a fix that cannot
                        // tell what any texture is would bind the game's over all of them.
                        ctx_.log("this mod's textures were not identified: the character's parse row "
                                 "carries no textures, so every role would fall back to a download");
                        giveUp();
                        return;
                    }

                    for (const auto& entry : scan.roles->resourcesOf(scan.iniPath)) {
                        scan.resourceOfFile.emplace(entry.second, entry.first);
                        // ...and the other way, so an edit can reach EVERY variant a toggled role
                        // binds rather than only the one fileOfRole_ resolved to
                        fileOfResource_.emplace(StringTools::toLower(entry.first), entry.second);
                    }

                    // WHERE THE FIX WRITES ITS OWN TEXTURES: the folder MOST of the mod's textures
                    // are already in, and never an absolute one.
                    //
                    // This used to read `scan.resourceOfFile.begin()`, which on an unordered_map is
                    // whichever entry the table hashed first -- so a mod whose textures are not all
                    // in one folder got a folder picked at random, and the pick could move between
                    // runs. That is what made Chisa13's mod.ini come out with four different hashes
                    // over four corpus sweeps while six isolated runs agreed.
                    //
                    // And the folder-scanning index this replaced keyed everything by a
                    // LOWERCASED path and answered with its argument when the key was absent, so an
                    // absolute path came back lowercased -- which `getRelPath` cannot relativise
                    // against a real-cased `scan.iniFolder`. The folder then came out absolute and every
                    // download, edit and created texture was written into the mod's .ini as
                    // `c:/users/.../textures/...`: broken the moment the mod moves, and the fixing
                    // machine's paths in someone else's file. A path that is not relative is
                    // refused, which leaves `Textures` -- what a mod with no textures of its own
                    // already gets (2026-09-29).
                    readTextureFolder(scan);
                    dumpTextureRoles(scan);
                    collectRoleCandidates(scan);
                    narrowRoleCandidates(scan);
                    readComponentTagging(scan);
                    applyTextureRoles(scan);
                }

                void readTextureFolder(TextureScan& scan) {
                    textureFolder_.clear();                 // beside the .ini until the mod says otherwise
                    std::map<std::string, std::size_t> folderCounts;
                    for (const auto& entry : scan.resourceOfFile) {
                        const std::string rel =
                            FileService::iniPathToRel(FileService::getRelPath(entry.first, scan.iniFolder));
                        if (FileService::strToPath(rel).is_absolute()) {
                            continue;                   // the index could not place it: not a folder of this mod
                        }

                        // THE ROOT IS A FOLDER TOO, which is what `parentOf` answers "" for. This
                        // counted a file only when its path had a slash, so "beside the .ini" could
                        // never win however many files were there and a minority folder took it by
                        // default: Chisa12 declares 50 files in its root (37 .dds and 13 .assets)
                        // against 41 in `res/`, which holds only its UI art, and every texture the
                        // fix made went into the UI folder (2026-09-30). The library's own answer had
                        // ...and only a TEXTURE votes on where the textures are. This counted
                        // every resource the .ini declares, a mod's Meshes/*.buf included, and was
                        // only ever right because an already-fixed mod's previous RemapDL / RemapTex
                        // sections were being counted as the mod's own -- many of them, all
                        // textures, outvoting the buffers by accident. The moment those stopped
                        // counting (they name sections the undo is about to delete) five mods wrote
                        // every texture they own into Meshes/ (2026-09-30).
                        if (StringTools::endsWithIgnoreCase(rel, FileExt::Buf)) {
                            continue;
                        }

                        ++folderCounts[FileService::parentOf(rel)];
                    }

                    // most textures wins; a tie goes to the lowest path, so the answer is the mod's
                    // rather than an iteration order's
                    std::size_t best = 0;
                    for (const auto& entry : folderCounts) {
                        if (entry.second > best) {
                            best = entry.second;
                            textureFolder_ = entry.first;
                        }
                    }

                }

                // AGREMAP_WWMI_ROLES=1: one line per file per role, so the identification this does
                // can be diffed against the one the parser is taking over. Off by default; the
                // compiled fixer is silent by request.
                void dumpTextureRoles(const TextureScan& scan) {
                    if (std::getenv("AGREMAP_WWMI_ROLES") != nullptr) {
                        for (const auto& entry : scan.roles->rolesOf()) {
                            for (const WWMITextureRoles::Role& role : entry.second) {
                                std::fprintf(stderr, "WWMIROLE\t%s\t%s\t%s\t%s\n", scan.iniPath.c_str(),
                                             FileService::pathKey(entry.first).c_str(),
                                             role.role.c_str(), role.how.c_str());
                            }
                        }
                    }

                    // ...and what the PARSER decided, by hash + register, for the same mod. Both
                    // under one env var so the two can be diffed exactly rather than approximated.
                    if (std::getenv("AGREMAP_WWMI_ROLES") != nullptr) {
                        if (const auto* facts = dynamic_cast<const WWMIParseFacts*>(this->getParser())) {
                            if (const WWMITextureRoles* roles = facts->textureRoles()) {
                                for (const auto& entry : roles->rolesOf()) {
                                    for (const auto& role : entry.second) {
                                        std::fprintf(stderr, "WWMIROLE2\t%s\t%s\t%s\t%s\n", scan.iniPath.c_str(),
                                                     FileService::pathKey(entry.first).c_str(),
                                                     role.role.c_str(), role.how.c_str());
                                    }
                                }
                            } else {
                                std::fprintf(stderr, "WWMIROLE2\t%s\t(no parser roles)\t\t\n", scan.iniPath.c_str());
                            }
                        } else {
                            std::fprintf(stderr, "WWMIROLE2\t%s\t(parser is not WWMIParseFacts)\t\t\n",
                                         scan.iniPath.c_str());
                        }
                    }

                }

                // Every candidate file for every role: the parser's answer, plus the roles the
                // mod's own sections name by the register they bind at.
                void collectRoleCandidates(TextureScan& scan) {
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        return;
                    }

                    // role -> (file, how the role was decided), every role of every file
                    for (const auto& entry : scan.roles->rolesOf()) {
                        for (const WWMITextureRoles::Role& role : entry.second) {
                            scan.byRole[role.role].emplace_back(entry.first, role.how);

                            // ...and separately, what the mod DECLARED the file as. The parser
                            // writes this prefix for a role read off the file's own `hash =`.
                            if (StringTools::startsWith(role.how, WWMITextureRoles::DeclaredHash)) {
                                scan.declaredRolesOf[entry.first].insert(role.role);
                            }
                        }
                    }

                    // ...and the roles the mod's OWN sections name by the register they bind at --
                    // see WWMITextureFacts::registerRoles. Added as candidates beside the
                    // others, which is what the ranking below expects.
                    // file -> source component -> the roles that component's own register
                    // bindings offer it for. One file offered two roles of ONE slot is the alias
                    // the mask drop below is about.
                    if (!config_.sourceTextures.registerRoles.empty()) {
                        std::unordered_map<std::string, std::string> fileOfResource;
                        for (const auto& entry : scan.roles->resourcesOf(scan.iniPath)) {
                            fileOfResource.emplace(StringTools::toLower(entry.first), entry.second);
                        }

                        const auto& templates = ini->getIfTemplates();
                        for (const auto& entry : present_) {
                            const auto layout = config_.sourceTextures.registerRoles.find(entry.first);
                            if (layout == config_.sourceTextures.registerRoles.end()) {
                                continue;
                            }

                            for (const std::string& section : entry.second) {
                                const auto tpl = templates.find(section);
                                if (tpl == templates.end() || tpl->second == nullptr) {
                                    continue;
                                }

                                for (const auto& [reg, role] : layout->second) {
                                    std::optional<std::string> bound = ModBranches::firstValLoose(*tpl->second, reg);
                                    if (!bound.has_value()) {
                                        continue;
                                    }

                                    // `ps-t1 = ref ResourceFoo` names the same resource as
                                    // `ps-t1 = ResourceFoo`
                                    const std::string name = IniNamingTools::removeRefPrefix(*bound);

                                    const auto file = fileOfResource.find(StringTools::toLower(name));
                                    if (file == fileOfResource.end()) {
                                        continue;
                                    }

                                    scan.byRole[role].emplace_back(
                                        file->second, "the " + reg + " its own section binds it at");
                                    scan.regRolesOfFile[file->second][entry.first].insert(role);
                                }
                            }
                        }
                    }

                }

                // Three passes that take candidates AWAY: the same file found twice, a mask that is
                // really the slot's normal map, and a file left to the game.
                void narrowRoleCandidates(TextureScan& scan) {
                    // ONE ENTRY PER FILE. A file is routinely found for a role more than one way
                    // -- by its hash AND by the register its own section binds it at -- and two
                    // entries naming one path rank identically, which made the "two textures are
                    // equally good answers" warning below fire with the SAME file on both sides of
                    // it. Keep the first way it was decided, which is the more specific one.
                    for (auto& entry : scan.byRole) {
                        std::vector<std::pair<std::string, std::string>> unique;
                        for (const auto& candidate : entry.second) {
                            const bool seen = std::any_of(unique.begin(), unique.end(),
                                                          [&](const auto& kept) { return kept.first == candidate.first; });
                            if (!seen) {
                                unique.push_back(candidate);
                            }
                        }

                        entry.second = std::move(unique);
                    }

                    // A MASK ROLE SATISFIED BY THE FILE THAT ALSO SERVES THE SLOT'S NORMAL IS
                    // NOT A MASK (2026-09-27). RabbitFX's Lightmap is this fix's material mask, and
                    // a mod may point its Lightmap and its Normalmap at ONE resource -- Chisa13
                    // does, for both hair slots. The mask role then resolves to a real file, the
                    // flat test below never fires, and the target's shader reads SLOPE data as
                    // material codes. Measured: what it bound is statistically indistinguishable
                    // from Chisa's own normal maps (`d8ed7611` R mean 8.6 / G 52.8 / B 41.1 against
                    // `e921181d` 8.6 / 52.8 / 41.0; `d0d2cc80` against `9ccd7ea7` likewise) where a
                    // real mask of hers is R = 255, G = 0, B = 126 flat. Chisa13 was the only mod of
                    // 50 in the corpus binding a hair mask at all -- the other 17 Chisa mods emit
                    // none and are confirmed in game -- so this puts the aliased mod on their path.
                    //
                    // Narrow on purpose. Only a region-marking role can be dropped, and only when
                    // the SAME source component offers that same file for another role as well: a
                    // file legitimately plays every role its HASHES name, across components, and
                    // that is untouched here. The role then takes the ordinary "the mod has no file
                    // for this role" path, exactly as a flat one does.
                    for (auto& entry : scan.byRole) {
                        const bool toGame = config_.flatLeftToGame.count(entry.first) > 0;
                        if (!toGame && config_.flatFallsBackToSource.count(entry.first) == 0) {
                            continue;
                        }

                        std::vector<std::pair<std::string, std::string>> kept;
                        for (const auto& candidate : entry.second) {
                            const auto perComponent = scan.regRolesOfFile.find(candidate.first);
                            bool aliased = false;
                            if (perComponent != scan.regRolesOfFile.end()) {
                                for (const auto& slot : perComponent->second) {
                                    if (slot.second.count(entry.first) > 0 && slot.second.size() > 1) {
                                        aliased = true;
                                        break;
                                    }
                                }
                            }

                            if (!aliased) {
                                kept.push_back(candidate);
                                continue;
                            }

                            ctx_.log(FileService::getRelPath(candidate.first, scan.iniFolder)
                                     + " is bound for another role of its own slot too, so it is not"
                                     + " the mod's " + entry.first + "; "
                                     + (toGame ? "left to the game" : "the source's own is used instead"));
                        }

                        if (kept.empty() && !entry.second.empty() && toGame) {
                            leftToGame_.insert(entry.first);
                        }

                        entry.second = std::move(kept);
                    }

                    // A FLAT candidate for a region-marking role is not a usable file. Dropping it
                    // here rather than special-casing it later is what makes it take the ordinary
                    // "the mod has no file for this role" path -- and which path that is, a download
                    // of the source's own or nothing at all, is the difference between
                    // WWMIFixerConfig::flatFallsBackToSource and WWMIFixerConfig::flatLeftToGame.
                    for (auto& entry : scan.byRole) {
                        const bool toGame = config_.flatLeftToGame.count(entry.first) > 0;
                        if (!toGame && config_.flatFallsBackToSource.count(entry.first) == 0) {
                            continue;
                        }

                        std::vector<std::pair<std::string, std::string>> varying;
                        for (const auto& candidate : entry.second) {
                            // channel 0: the material code, which is what says where the regions are
                            if (!TexThumbprint::channelIsConstant(candidate.first, 0)) {
                                varying.push_back(candidate);
                                continue;
                            }

                            ctx_.log(FileService::getRelPath(candidate.first, scan.iniFolder)
                                     + " is a flat " + entry.first + ", which marks no regions; "
                                     + (toGame ? "left to the game" : "the source's own is used instead"));
                        }

                        if (varying.empty() && !entry.second.empty() && toGame) {
                            // Nothing of the mod's survives for this role AND nothing may stand in
                            // for it, so the fallback download is suppressed below
                            leftToGame_.insert(entry.first);
                        }

                        entry.second = std::move(varying);
                    }

                }

                // Which source component each role BELONGS to, and whether this mod's
                // `Components-<n>` tags are in the SOURCE's numbering at all.
                void readComponentTagging(TextureScan& scan) {
                    // Which source component each role BELONGS to, off the two config tables that
                    // are written per component. Not the same question as which component's slot is
                    // asking for it: Chisa's accessory slot and four of her extra passes bind
                    // `frontHairDiffuse`, which is component 0's.
                    for (const auto& entry : config_.sourceTextures.registerRoles) {
                        for (const auto& reg : entry.second) {
                            scan.componentsOfRole[reg.second].insert(entry.first);
                        }
                    }

                    for (const auto& entry : config_.typeRoles) {
                        for (const auto& type : entry.second) {
                            scan.componentsOfRole[type.second].insert(entry.first);
                        }
                    }

                    // ...and whether this mod's tags are in the SOURCE's numbering at all. A
                    // `Components-<N>` tag is written by the exporter in the numbering of the
                    // character the mod was made for, which for a mod installed on the other half
                    // of a pair is the other character's: Sanhua has seven components and
                    // SanhuaExorcist six, so a Sanhua mod fixed as the Exorcist carries a
                    // `Components-6` her plan has no component for. A tag naming a component the
                    // source does not have says the whole numbering is somebody else's, so the
                    // refusal below is switched off for the file -- rank() reads the same tag and
                    // can only mis-PREFER, where a refusal deletes.
                    for (const auto& entry : config_.plan) {
                        scan.sourceComponents.insert(entry.first);
                    }


                    // How well a file serves ONE source component: first how specifically its
                    // WWMI-Tools `Components-<a>-<b>... t=<hash>.dds` name is tagged for that component,
                    // then whether this .ini already has a resource for it, then its distance.
                    //
                    // A hash override binds ONE file per hash wherever it is drawn, so an exporter that
                    // writes both a per-component texture and a shared one under the same hash leaves the
                    // per-component art unbound in the mod's own .ini: sanhua_qiming ships its bangs' own
                    // mask as `Components-0 t=d153e37f.dds` and the game's shared mask as
                    // `Components-0-1-2-3-4 t=d153e37f.dds`, and binds the second. Sampled at the mod's
                    // own atlas UVs that put wrong-coloured patches over the fringe in game (2026-09-19).
                    // A register binding is per component and can honour the specific one.
                }

                // How good a candidate FILE is for a role on one source component -- lower is
                // better, compared as a tuple.
                auto rankCandidate(const TextureScan& scan, const std::string& role, const std::string& file,
                                   int component) const {
                        // A FILE THE MOD DECLARES AS ANOTHER ROLE IS NOT THIS ONE (2026-09-29).
                        //
                        // `hash = <h>` over a texture is the mod naming the game texture its file
                        // replaces, and the library says what role <h> is. A register binding says
                        // only where the file is USED, and a mod may point two registers at one
                        // resource -- Chisa3 binds its front hair NORMAL at the RabbitFX Lightmap
                        // register, which this character's layout reads as the mask, so the normal
                        // map won the mask role and the target's shader read slope data as material
                        // codes.
                        //
                        // Only a file whose declared roles EXIST and exclude this one is demoted: a
                        // mod may alias one file onto several hashes (Chisa2 puts seven on one
                        // resource), and demoting those is what the filename version of this test
                        // got wrong.
                        int contradicted = 0;
                        const auto declared = scan.declaredRolesOf.find(file);
                        if (declared != scan.declaredRolesOf.end() && !declared->second.empty()
                                && declared->second.count(role) == 0) {
                            contradicted = 1;
                        }

                        std::string rel = FileService::pathKey(FileService::getRelPath(file, scan.iniFolder));
                        std::size_t ups = 0;
                        std::size_t pos = 0;
                        while ((pos = rel.find("../", pos)) != std::string::npos) {
                            ++ups;
                            pos += 3;
                        }

                        // WHAT THE MOD SAYS, NOT WHAT THE FILE IS CALLED (2026-09-29).
                        //
                        // This read `Components-<i>-<j>... t=<hash>.dds`, WWMI-Tools' export name,
                        // and ranked a file by which components that name claimed. A mod author may
                        // call a texture anything -- a mod-manager-packaged mod names every texture
                        // a GUID, and `Component3.dds`, `Upper_D.dds` and `wumao.dds` are all in
                        // this corpus -- so the name was a guess about a fact the mod states
                        // outright elsewhere.
                        //
                        // `regRolesOfFile` is that statement: the author writing `ps-t5 =
                        // ResourceFoo` inside component 1's section IS the author saying what
                        // component 1's normal map is. A file no register names makes no claim and
                        // is not evidence against itself, which is what the untagged case meant.
                        int specificity = 2;                               // nothing binds it: says nothing
                        const auto bound = scan.regRolesOfFile.find(file);
                        if (bound != scan.regRolesOfFile.end() && !bound->second.empty()) {
                            const auto here = bound->second.find(component);
                            if (here != bound->second.end()) {
                                specificity = bound->second.size() == 1 ? 0 : 1;
                            } else {
                                specificity = 3;                           // bound, and NOT by this one
                            }
                        }

                        return std::make_tuple(contradicted, specificity,
                                               scan.resourceOfFile.count(file) > 0 ? 0 : 1, ups,
                                               rel.size(), rel);
                }

                // Which of the mod's files serves one role on one source component, if any.
                //
                // The choice is per (role, source component), not per role -- but an ambiguity
                // BETWEEN TWO FILES is a property of the files, so it is reported once however many
                // components are offered the role (hence TextureScan::saidAmbiguous).
                void assignRole(TextureScan& scan, const std::string& role, int component) {
                        {
                            auto found = scan.byRole.find(role);
                            if (found == scan.byRole.end() || found->second.empty()
                                || resourceOfSlotRole_.count({role, component}) > 0) {
                                return;
                            }

                            std::vector<std::pair<std::string, std::string>> candidates = found->second;

                            // A file the exporter TAGGED for other components is not this ROLE's
                            // texture, whatever hash the mod aliased onto it (2026-09-28). Chisa2
                            // binds one file -- its component-3 kimono atlas
                            // `Components-3 t=4c7e5ddf.dds` -- under SEVEN hashes, a shotgun so the
                            // mod survives a game version bump; one of them, `6616fe2c`, is
                            // genuinely a lowerDiffuse, so once that generation was filed in
                            // HashData the kimono was bound at the lower body's ps-t3 and the whole
                            // garment rendered red in game. The hash row is right; the mod's
                            // aliasing is what is not.
                            //
                            // "A file plays EVERY role its hashes name" is unchanged -- that is how
                            // one atlas serves two components, and such a file's name LISTS both
                            // (`Components-2-4 t=<hash>.dds`). Refused only when the name names
                            // components and the role's own is not among them, so a file with no
                            // tag (`Upper_D.dds`, a GUID, `Component3.dds`) and a role no config
                            // table places are both untouched. Dropping every candidate is a real
                            // answer too: the mod ships nothing for that role, which is what the
                            // fallback download is for.


                            // THE DEMOTION BELONGS AFTER rank()'s FIRST TWO ELEMENTS, NOT ABOVE ALL
                            // OF THEM (2026-09-28). rank() already orders by how specifically a
                            // file's `Components-<list>` name is tagged for this component, and then
                            // by whether this .ini DECLARES a resource for it -- a file nothing names
                            // is a spare the user copies in by hand. Sorting the 2026-09-27
                            // contradiction above both let a spare win on a name:
                            //
                            // Chisa2 ships its active upper atlas as `Textures/Components-3
                            // t=4c7e5ddf.dds` (its `[ResourceBase]`) with three spare colourways
                            // beside it under `2Color Variation/{Black,Red,White}/`. Its own file
                            // takes upperDiffuse from `2970cef1`, one of SEVEN hashes it aliases onto
                            // that resource, and its name carries `4c7e5ddf` -- so it was demoted and
                            // the untouched RED spare, whose name and hash agree, was bound over it.
                            // In game: the kimono all red, which is what was reported.
                            //
                            // Specificity stays FIRST, which is what keeps the 2026-09-19 rule:
                            // sanhua_qiming ships its bangs' own mask as `Components-0
                            // t=d153e37f.dds` and the game's shared one as `Components-0-1-2-3-4
                            // t=d153e37f.dds`, declaring only the shared one, and the per-component
                            // file must still win. Hoisting the declared test above specificity moved
                            // 56 bindings onto different bytes across Sanhua and Chisa.
                            std::sort(candidates.begin(), candidates.end(),
                                      [&](const auto& a, const auto& b) {
                                          return rankCandidate(scan, role, a.first, component)
                                                 < rankCandidate(scan, role, b.first, component);
                                      });
                            const std::string& best = candidates.front().first;
                            if (candidates.size() > 1) {
                                const auto first = rankCandidate(scan, role, best, component);
                                const auto second = rankCandidate(scan, role, candidates[1].first, component);
                                if (std::get<0>(first) == std::get<0>(second) && std::get<1>(first) == std::get<1>(second)
                                    && std::get<2>(first) == std::get<2>(second)
                                    && std::get<3>(first) == std::get<3>(second)
                                    && scan.saidAmbiguous.insert(role + "\n" + best + "\n" + candidates[1].first).second) {
                                    // Two shipped textures equally close on one role: the first is bound
                                    // and only a measurement can say which is right -- say so loudly.
                                    ctx_.log("WARNING: " + FileService::getRelPath(candidates[1].first, scan.iniFolder)
                                             + " also has the role " + role + " (" + candidates[1].second
                                             + "), already taken by " + FileService::getRelPath(best, scan.iniFolder)
                                             + " (" + candidates.front().second + "); the first one is bound");
                                }
                            }

                            // AGREMAP_WWMI_PICK=1: which FILE the fix chose for each (role,
                            // component), which is the thing a change to the ranking moves. The
                            // corpus manifest only shows a change a mod's output happens to depend
                            // on; this shows every choice, including the ones that tie.
                            if (std::getenv("AGREMAP_WWMI_PICK") != nullptr) {
                                std::fprintf(stderr, "WWMIPICK\t%s\t%s\t%d\t%s\n", scan.iniPath.c_str(),
                                             role.c_str(), component,
                                             FileService::pathKey(best).c_str());
                            }

                            fileOfRole_[role] = best;
                            auto own = scan.resourceOfFile.find(best);

                            // ...unless a PREVIOUS run of the fix is what declared it. A fix undoes
                            // first and the undo removes every section a fix named, so binding
                            // `[Resource<Role><Target>RemapRef]` -- which exists precisely to point
                            // at one of the MOD'S files -- writes a reference the same run deletes:
                            // 65 dangling `ps-t` bindings across the two corpus mods that arrive
                            // already fixed (Chisa9, Sanhua3), each naming a section nothing
                            // declares, which in game leaves whatever was last in the register. The
                            // file is still the mod's, so it falls through and gets a fresh
                            // declaration below rather than being dropped (2026-09-30).
                            //
                            // IniNamingTools::looksRemapped is the undo's own test, so the two
                            // cannot disagree about what is about to be removed.
                            if (own != scan.resourceOfFile.end()
                                && !IniNamingTools::looksRemapped(own->second, remapNames_)) {
                                resourceOfSlotRole_[{role, component}] = own->second;
                                return;
                            }

                            // A file no resource of this .ini names gets a resource section of the fix's
                            // own -- named with RemapRef, not RemapFix: the section sits inside the fix's
                            // block and names one of the MOD's files, and an undo deletes what a RemapFix
                            // section names. Two files of one role each get their own.
                            auto declaredName = declaredName_.find(best);
                            if (declaredName == declaredName_.end()) {
                                std::string name = IniKeywords::Resource + TextTools::capitalize(role) + toModName_ + IniKeywords::RemapRef;
                                for (std::size_t n = 2; usedDeclaredNames_.count(name) > 0; ++n) {
                                    name = IniKeywords::Resource + TextTools::capitalize(role) + std::to_string(n) + toModName_ + IniKeywords::RemapRef;
                                }

                                usedDeclaredNames_.insert(name);
                                declaredName = declaredName_.emplace(best, name).first;
                                const std::string rel = FileService::pathToIniStr(
                                    FileService::strToPath(FileService::getRelPath(best, scan.iniFolder)));
                                declared_.emplace_back(best, rel);
                            }

                            resourceOfSlotRole_[{role, component}] = declaredName->second;
                        }
                }

                // Every role offered to every component that could serve it, then the roles the fix
                // invents and the downloads for the rest.
                void applyTextureRoles(TextureScan& scan) {
                    for (const auto& planned : config_.plan) {
                        if (present_.count(planned.first) == 0) {
                            continue;
                        }

                        for (const WWMIFixerConfig::Binding& binding : planned.second.bindings) {
                            assignRole(scan, binding.role, planned.first);
                        }
                    }

                    // ...and the roles the plan does not name, which only a slot's OTHER passes or a
                    // shared mesh bind. Until these were asked, a role like Chisa's accessoryNormal
                    // could only ever come from the fallback DOWNLOAD -- so the fix fetched the
                    // game's normal map and bound it over the one the mod ships, at the mod's UVs.
                    // assign() no-ops on a role already resolved or one no file serves, so offering
                    // every present component is safe.
                    for (const std::string& role : passOnlyRoles()) {
                        for (const auto& entry : present_) {
                            assignRole(scan, role, entry.first);
                        }
                    }

                    for (const WWMIFixerConfig::CreatedTexture& created : config_.createdTextures) {
                        resourceOfRole_[created.role] = fixName(IniKeywords::Resource + created.role);
                    }

                    // A planned role the mod has NO file for: the SOURCE's own game texture, as a
                    // download named RemapDL (the file is the fix's, so an undo may delete it). The
                    // mod's UVs are the source's, and the target's texture -- what the register
                    // samples on the target's draw when nothing binds it -- is wrong by construction.
                    if (!config_.sourceTextures.downloadCharFolder.empty()) {
                        for (const auto& entry : config_.plan) {
                            if (present_.count(entry.first) == 0) {
                                continue;
                            }

                            for (const WWMIFixerConfig::Binding& binding : entry.second.bindings) {
                                if (resourceOfSlotRole_.count({binding.role, entry.first}) > 0
                                    || resourceOfRole_.count(binding.role) > 0) {
                                    continue;
                                }

                                auto fallback = config_.sourceTextures.fallbackTextures.find(binding.role);
                                if (fallback == config_.sourceTextures.fallbackTextures.end()
                                        || leftToGame_.count(binding.role) > 0) {
                                    continue;
                                }

                                registerFallback(binding.role, fallback->second);
                            }
                        }

                        // ...and the roles only a slot's OTHER passes or a shared mesh bind. The
                        // loop above walks the plan, which is each component's MAIN pass, so a role
                        // bound nowhere else never downloaded: Chisa's accessoryNormal is bound only
                        // by slot 5's two side-panel passes, and her ribbon rendered with the GAME's
                        // normal map at the mod's UVs.
                        //
                        // The plan's per-component test is a different question from "does any file
                        // of the mod serve this role", so it is left alone and this asks the shared
                        // one.
                        for (const std::string& role : passOnlyRoles()) {
                            if (resourceOfRole_.count(role) > 0) {
                                continue;
                            }

                            bool owned = false;
                            for (const auto& entry : resourceOfSlotRole_) {
                                if (entry.first.first == role) {
                                    owned = true;
                                    break;
                                }
                            }

                            auto fallback = config_.sourceTextures.fallbackTextures.find(role);
                            if (owned || fallback == config_.sourceTextures.fallbackTextures.end()
                                    || leftToGame_.count(role) > 0) {
                                continue;
                            }

                            registerFallback(role, fallback->second);
                        }
                    }
                }

                // ---- the edits ----

                /**
                 * @brief
                 @rst
                 One path the fix WRITES, relative to the mod's ``.ini`` -- inside 'folder' when the
                 mod keeps that kind of file in one, beside the ``.ini`` when it does not
                 @endrst
                 *
                 * @param folder #meshFolder_ or #textureFolder_, empty for the mod's own root
                 * @param name The file's name
                 *
                 * @return The path, with no leading separator when there is no folder
                 */
                /**
                 * @brief
                 @rst
                 The absolute path of the mod's own ``Position.buf``, however its ``.ini`` names it
                 @endrst
                 *
                 * @throws std::runtime_error If the mod's ``.ini`` declares no such resource, which is
                 *      its own failure and not "the file is missing"
                 *
                 * @return The path
                 */
                std::string positionBufPath() const {
                    if (positionFile_.empty()) {
                        throw std::runtime_error(
                            "cannot fix this mod: its .ini declares no [" + PositionBufferResource
                            + "], so there is no mesh to read a vertex count from");
                    }

                    return FileService::absPathOfRelPath(
                        positionFile_, const_cast<IniFileFixContext&>(ctx_).getIniFile()->getFolder());
                }

                static std::string modFile(const std::string& folder, const std::string& name) {
                    return folder.empty() ? name : folder + "/" + name;
                }

                ModObj targetSlotObj(int slot) const {
                    return ModObj(toModName_, config_.slotPrefix + std::to_string(slot));
                }

                // Does the TARGET's merged skeleton pass what an 8-bit blend index can name, and can
                // one WWMI blend remap hold the row? Sets #targetPast256_ and #blendRemapBones_,
                // which every later phase reads.
                void readBlendWidth(const ModType* source, const std::optional<Version>& from,
                                     const std::optional<Version>& to) {
                    // Does the TARGET's merged skeleton pass what an 8-bit blend index can name?
                    // The row's largest target id answers it, and it is a property of the PAIR --
                    // so the .ini and the buffers, written at different moments, cannot disagree
                    // about whether there is a blend remap. Chisa reaches 418; ChisaParfait, going
                    // the other way, stops at 250 and needs none.
                    if (source != nullptr) {
                        const std::optional<VGRemap> row = source->getVGRemap(toModName_, from, to);
                        if (row.has_value()) {
                            std::set<long long> targets;
                            for (const auto& [srcBone, dstBone] : row->getRemap()) {
                                (void)srcBone;
                                targets.insert(dstBone);
                                if (dstBone > WWMIMaxByteBone) {
                                    targetPast256_ = true;
                                }
                            }

                            blendRemapBones_ = targets.size();

                            // A row naming more distinct targets than one remap holds would need
                            // WWMI's per-component scheme back.
                            //
                            // THIS SAID "refused rather than truncated" AND THEN TRUNCATED (noted
                            // 2026-09-29). Clearing `targetPast256_` sends the fix to an 8-bit lift,
                            // whose ids are one byte -- so every bone past 255 lands on bone 0, the
                            // exact outcome the comment claimed to be avoiding. `writeBlendRemap`
                            // does refuse (it throws), but it is unreachable once this has turned
                            // the path off.
                            //
                            // The line now says what happens rather than the opposite. Which of the
                            // two guards should win -- fall back and lose the far bones, or refuse
                            // the mod outright -- is the maintainer's call, and both are in game.
                            if (targetPast256_ && blendRemapBones_ > WWMIBlendRemapSize) {
                                note("the vertex group row names " + std::to_string(blendRemapBones_)
                                     + " distinct target bones, past the "
                                     + std::to_string(WWMIBlendRemapSize)
                                     + " one blend remap holds; falling back to an 8-bit blend, so"
                                     + " every bone past " + std::to_string(WWMIMaxByteBone)
                                     + " will land on bone 0");
                                targetPast256_ = false;
                            }
                        }
                    }
                }

                void buildEdits() {
                    buildTexcoordCopy();
                    const ModType* source = ctx_.modType();
                    const std::optional<Version> from = fromVersion();
                    const std::optional<Version> to = toVersion();

                    readBlendWidth(source, from, to);

                    assetRemap_ = std::make_unique<RegAssetRemap<>>(
                        std::vector<std::pair<std::string, RegAssetRemap<>::AssetSpec>>{
                            {IniKeywords::Hash, RegAssetRemap<>::AssetSpec(ctx_.modTypeHashes(), IniKeywords::HashNotFound)},
                            {ShapeKeyChecksumKey, RegAssetRemap<>::AssetSpec(source->shapeKeyChecksums.get(), ChecksumNotFound)}},
                        toModName_, ctx_.modTypeName().value_or(""), from, to);
                    assetAdapter_ = std::make_unique<RegPartEdit<>>(assetRemap_.get());

                    // In this order because each reads what the one before it set: the removals
                    // need the blend width, the groups decide which slots are drawn, and everything
                    // after that is per drawn slot.
                    buildRegRemovals();
                    buildGroups();
                    buildSlotValues();
                    buildComponentAdditions();
                    buildSlotRemap();
                    buildPerGroupEdits();
                    buildBlendCollects(source, from, to);
                }

                // Lines a remapped section drops -- see WWMIFixerConfig::removedRegs for why each
                // kind is there. Built once and hung on every remapped slot section.
                void buildRegRemovals() {
                    std::vector<WWMIFixerConfig::RegRemoval> removals = config_.removedRegs;
                    if (targetPast256_) {
                        // The MOD's own merge list, dropped from the remapped sections. It writes
                        // the TARGET's bones -- Chisa's slot 3 starts at merged offset 142 and runs
                        // to 269 -- into the mod's own merged skeleton, which every ChisaParfait mod
                        // declares for 256 bones. The writes past the end are dropped by D3D, but
                        // the ones that land corrupt the SOURCE's skeleton whenever both characters
                        // are on screen at once. The fix's own merge list (CommandListMergeSlot<N>)
                        // does the same work into a buffer sized for the target, and is added to
                        // these sections beside it.
                        //
                        // Matched by prefix, which covers the name both before and after the group
                        // remap renames it. A mod whose merge list is named something else keeps it,
                        // which is wasted work rather than a wrong picture.
                        removals.push_back(WWMIFixerConfig::RegRemoval{IniKeywords::Run, "commandlistmergeskeleton"});

                        // AND the two other routes to that same list (2026-10-01), without which
                        // dropping the `run` above achieves nothing.
                        //
                        // `CheckTextureOverride = vs-cb4` makes 3dmigoto run whatever section
                        // matches the buffer bound there, and on a mod of a character WWMI merges a
                        // skeleton for, that IS the mod's own merge. So it does the work this block
                        // exists to prevent -- and, like the explicit binding below it, consumes the
                        // `boneDataFilter` marker on the way.
                        //
                        // That matters because the fix's own `CommandListMergeSlot<N>` is guarded by
                        // the same marker. With either line present it never runs: measured in
                        // 3dmigoto's log as `if vs-cb4 == 3381.7777` false 2565 times and true ZERO,
                        // its SkeletonMerger and SkeletonRemapper never firing, and the model
                        // invisible in game -- the draws issue, but they skin blend indices in the
                        // TARGET's space against the SOURCE's skeleton, so the mesh collapses.
                        // Removing both puts the guard at true 2630 and runs both shaders 4734 times.
                        //
                        // Nothing is lost by dropping the mod's own binding: `CommandListMergeSlot<N>`
                        // binds the remapped skeleton itself, which is the one sized for the target.
                        // A mod that does not carry these lines -- every Chisa mod, which is why the
                        // forward direction never hit this -- is unaffected.
                        removals.push_back(WWMIFixerConfig::RegRemoval{"CheckTextureOverride", "vs-cb3"});
                        removals.push_back(WWMIFixerConfig::RegRemoval{"CheckTextureOverride", "vs-cb4"});
                        removals.push_back(WWMIFixerConfig::RegRemoval{"vs-cb3", "resourceextramergedskeleton"});
                        removals.push_back(WWMIFixerConfig::RegRemoval{"vs-cb4", "resourcemergedskeleton"});

                        // AND THE THIRD BRANCH (2026-10-02). A real WWMI Tools mod writes
                        // `if vs-cb4 == marker / ... / elif vs-cb3 == marker / vs-cb3 = ref
                        // ResourceMergedSkeleton`, and that last line is named by NEITHER pair
                        // above -- it is cb3 carrying the NON-extra resource.
                        //
                        // `vs-cb3` is the second skeleton: the PREVIOUS frame's pose, which the
                        // shader turns into motion vectors. The survivor consumes its marker, so
                        // the fix's merge list skips cb3 on those draws and they keep the game's --
                        // the current pose from our remapped skeleton and the previous pose from
                        // the TARGET's own. The motion vectors are then wrong and TAA smears a
                        // second body, visible only while the character moves and in no still.
                        // Measured on ChisaParfait1, inside the fix's own merge list:
                        // `if vs-cb3 == 3381.7777` true 8827, FALSE 4418 -- a third of the draws.
                        removals.push_back(WWMIFixerConfig::RegRemoval{"vs-cb3", "resourcemergedskeleton"});
                    }

                    if (!removals.empty()) {
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>> keys;
                        keys.reserve(removals.size());
                        for (const WWMIFixerConfig::RegRemoval& removal : removals) {
                            if (removal.valuePrefix.empty()) {
                                keys.emplace_back(removal.reg, std::nullopt);
                                continue;
                            }

                            const std::string prefix = StringTools::toLower(removal.valuePrefix);
                            keys.emplace_back(removal.reg, RegRemove<>::RemoveKeyCheck(
                                [prefix](long long, const std::string& value) {
                                    // STEP OVER `ref` (2026-10-02). The prefix names the RESOURCE,
                                    // and a binding may reach it either way: an identity mod built
                                    // by wwmiIdentityMod.py writes `vs-cb4 = ResourceMergedSkeleton`
                                    // and WWMI Tools -- which every downloadable mod is built with --
                                    // writes `vs-cb4 = ref ResourceMergedSkeleton`. Comparing the raw
                                    // value matched the first and missed the second.
                                    //
                                    // On a >256-bone target that is fatal and silent: the surviving
                                    // line binds the MOD's merged skeleton and consumes the
                                    // `boneDataFilter` marker, so the fix's own merge list -- guarded
                                    // by that marker -- never runs and the draw skins target-space
                                    // blend indices against the source's skeleton. The model is
                                    // invisible, which is what this function's comment above already
                                    // predicted. It showed on no identity mod, which is every mod the
                                    // earlier rounds were tested on.
                                    std::string text = StringTools::toLower(StringTools::lstrip(value));
                                    const std::string ref = StringTools::toLower(IniKeywords::Ref);
                                    if (StringTools::startsWith(text, ref + " ")) {
                                        text = StringTools::lstrip(text.substr(ref.size()));
                                    }

                                    return StringTools::startsWith(text, prefix);
                                }));
                        }

                        regRemove_ = std::make_unique<RegRemove<>>(std::move(keys));
                        removeAdapter_ = std::make_unique<RegPartEdit<>>(regRemove_.get());
                    }

                }

                // The groups: one remapped section per target draw per file. The first source
                // component claiming a slot stays in the mod's own .ini, the second lands in the
                // first copy, and so on -- in the source components' numeric order, which is the
                // order the remap below creates the target graphs in and so the order collisions
                // are resolved in.
                void buildGroups() {
                    std::map<int, std::size_t> claimants;
                    for (const auto& entry : present_) {
                        auto planned = config_.plan.find(entry.first);
                        if (planned == config_.plan.end()) {
                            dropped_.push_back(entry.first);
                            continue;
                        }

                        const std::size_t group = claimants[planned->second.slot]++;
                        if (groups_.size() <= group) {
                            groups_.resize(group + 1);
                        }

                        groups_[group].push_back(entry.first);
                        drawnSlots_.insert(planned->second.slot);
                    }

                    if (groups_.empty()) {
                        groups_.resize(1);
                    }

                    // A COMPONENT THE PLAN DOES NOT NAME IS THROWN AWAY, AND USED TO SAY NOTHING.
                    // `dropped_` was collected here and read nowhere, so a mod carrying geometry
                    // this character has no slot for lost it with every line of the run reporting
                    // success -- the part is missing in game and the log that would have named it
                    // was a vector nobody printed.
                    if (!dropped_.empty()) {
                        std::string names;
                        for (const int component : dropped_) {
                            names += (names.empty() ? "" : ", ") + config_.slotPrefix
                                     + std::to_string(component);

                            // `sourceLabels` exists for exactly this and had never been read:
                            // `component3` tells a user nothing that `component3 (torso, arms,
                            // ribbons)` does not tell them better.
                            const auto label = config_.sourceLabels.find(component);
                            if (label != config_.sourceLabels.end() && !label->second.empty()) {
                                names += " (" + label->second + ")";
                            }
                        }

                        note("this mod draws " + std::to_string(dropped_.size())
                             + (dropped_.size() == 1 ? " component " : " components ") + names
                             + " that " + source_.name + "'s plan has no target slot for -- "
                             + (dropped_.size() == 1 ? "it is" : "they are") + " not remapped");
                    }


                }

                // Per target slot: its numbers, written over the source's.
                void buildSlotValues() {
                    for (int slot : drawnSlots_) {
                        const Slot& s = target_.slots.at(static_cast<std::size_t>(slot));
                        auto newVals = std::make_unique<RegNewVals<>>(
                            std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                                {IniKeywords::MatchFirstIndex, RegNewVals<>::NewVal(s.indexOffset)},
                                {IniKeywords::MatchIndexCount, RegNewVals<>::NewVal(s.indexCount)},
                                {VgOffsetKey, RegNewVals<>::NewVal(s.vgOffset)},
                                {VgCountKey, RegNewVals<>::NewVal(s.vgCount)}},
                            false);
                        for (const auto& planned : config_.plan) {
                            if (planned.second.slot != slot || present_.count(planned.first) == 0) {
                                continue;
                            }

                            for (const std::string& section : present_.at(planned.first)) {
                            }
                        }

                        auto adapter = std::make_unique<RegPartEdit<>>(newVals.get());
                        newValsOf_[slot] = adapter.get();
                        newVals_.push_back(std::move(newVals));
                        regAdapters_.push_back(std::move(adapter));
                    }

                }

                // Per drawn source component: the zero stream and its texture command list right
                // after the shared-resource override. No after-register on the add: `drawindexed`
                // as one is a MUST fact, and behind a `$draw_x` toggle no draw is certain, which
                // parked the additions inside the first toggle.
                void buildComponentAdditions() {
                    for (const auto& entry : config_.plan) {
                        const int component = entry.first;
                        if (present_.count(component) == 0) {
                            continue;
                        }

                        const WWMIFixerConfig::SourceComponent& planned = entry.second;
                        RegSurroundedAdd<>::Additions additions;
                        if (legacy_ || targetPast256_) {
                            // The fix's own merge list, not the mod's: for a legacy mod because it
                            // has none, and for a target past 256 bones because the mod's own
                            // merged skeleton is declared for 256 and the target's bones run past it.
                            additions.emplace_back(IniKeywords::Run, mergeListName(planned.slot));
                        }

                        if (targetPast256_) {
                            // After the merge list, so the private skeleton exists, and after the
                            // shared override, whose `vb4` this rebinds to the remapped blend.
                            additions.emplace_back(IniKeywords::Run, blendRemapInitList());
                        }

                        if (config_.zeroShapeKeyStream && meshVertexCount_ > 0) {
                            additions.emplace_back(config_.shapeKeyStreamReg, fixName(IniKeywords::Resource + ShapeKeyZero));
                        }

                        if (texcoordResource_.has_value()) {
                            additions.emplace_back(config_.texcoordReg, *texcoordResource_);
                        }

                        std::vector<Kvp> bindings;
                        for (const WWMIFixerConfig::Binding& binding : planned.bindings) {
                            if (binding.role == IniKeywords::Null) {
                                bindings.push_back(nullLine(binding.reg));
                                continue;
                            }

                            const std::string* resource = resourceFor(binding.role, component);
                            if (resource != nullptr) {
                                bindings.push_back(bindLine(binding.role, binding.reg, *resource));
                            }
                        }

                        if (!bindings.empty()) {
                            const std::string cmdList = fixName(IniKeywords::CommandList + source_.name + TextTools::capitalize(config_.slotPrefix)
                                                                + std::to_string(component) + "Textures");
                            textureLists_.push_back(
                                SectionText(z3_, cmdList)
                                    .open(passCondition(config_.slotPasses.at(static_cast<std::size_t>(planned.slot))))
                                    .keys(bindings)
                                    .close()
                                    .str());
                            additions.emplace_back(IniKeywords::Run, cmdList);
                        }

                        // A slot's OTHER passes bind the same art at DIFFERENT registers, so each
                        // gets its own guarded list beside the plan's -- see
                        // WWMIFixerConfig::extraPassRegs.
                        const auto extra = config_.extraPassRegs.find(planned.slot);
                        if (extra != config_.extraPassRegs.end()) {
                            int n = 0;
                            for (const auto& [pass, regs] : extra->second) {
                                std::vector<Kvp> extraBindings;
                                for (const WWMIFixerConfig::Binding& binding : regs) {
                                    // A binding for ONE source, on a slot two sources merge onto --
                                    // see Binding::srcComponent. The default -1 takes every source,
                                    // which is what every config written before 2026-09-28 means.
                                    if (binding.srcComponent >= 0 && binding.srcComponent != component) {
                                        continue;
                                    }

                                    const std::string* resource = resourceFor(binding.role, component);
                                    if (resource != nullptr) {
                                        extraBindings.push_back(bindLine(binding.role, binding.reg, *resource));
                                    }
                                }

                                if (extraBindings.empty()) {
                                    ++n;
                                    continue;
                                }

                                const std::string extraList =
                                    fixName(IniKeywords::CommandList + source_.name + TextTools::capitalize(config_.slotPrefix)
                                            + std::to_string(component) + "TexturesPass" + std::to_string(n));
                                textureLists_.push_back(SectionText(z3_, extraList)
                                                            .open(passCondition(pass))
                                                            .keys(extraBindings)
                                                            .close()
                                                            .str());
                                additions.emplace_back(IniKeywords::Run, extraList);
                                ++n;
                            }
                        }

                        // THE MOD'S OWN `ps-t` LINES, RE-KEYED BY ROLE (2026-10-02). Before the
                        // twin because it reads what the mod wrote and the twin only adds draws.
                        //
                        // `regOfRole` is this component's own plan row, so the registers are the
                        // ones the fix's list for this component already uses -- the carried lines
                        // end up speaking the same layout as the list they sit after, which is the
                        // whole point. `roleOf` prefers a role this component serves and falls back
                        // to any, because one file may serve several components.
                        {
                            std::map<std::string, std::string> regOfRole;
                            for (const WWMIFixerConfig::Binding& binding : planned.bindings) {
                                if (binding.role != IniKeywords::Null) {
                                    regOfRole.emplace(binding.role, binding.reg);
                                }
                            }

                            std::map<std::string, std::string> roleOfResource;
                            for (const auto& [key, resource] : resourceOfSlotRole_) {
                                const std::string name = StringTools::toLower(resource);
                                if (key.second == component || roleOfResource.count(name) == 0) {
                                    roleOfResource[name] = key.first;
                                }
                            }

                            auto roleOf = [roleOfResource](const std::string& resource)
                                    -> std::optional<std::string> {
                                const auto found = roleOfResource.find(resource);
                                if (found == roleOfResource.end()) {
                                    return std::nullopt;
                                }

                                return found->second;
                            };

                            // ...and the same row by KIND, for a carried binding whose role belongs
                            // to another component. `ResourceTexture4_3` is a `lowerDiffuse` and
                            // this row names `panelDiffuse`; both are the config's OWN names, so
                            // their shared tail is a convention this repo controls rather than a
                            // guess about the mod. Only where the tail is unambiguous in the row --
                            // `irisDiffuse` and `eyeDiffuse` share one, at different registers.
                            std::map<std::string, std::string> regOfKind;
                            std::set<std::string> ambiguous;
                            for (const auto& [role, reg] : regOfRole) {
                                const std::string kind = roleKind(role);
                                const auto seen = regOfKind.find(kind);
                                if (seen != regOfKind.end() && seen->second != reg) {
                                    ambiguous.insert(kind);
                                } else {
                                    regOfKind.emplace(kind, reg);
                                }
                            }

                            auto regOf = [regOfRole, regOfKind, ambiguous](const std::string& role)
                                    -> std::optional<std::string> {
                                const auto found = regOfRole.find(role);
                                if (found != regOfRole.end()) {
                                    return found->second;
                                }

                                const std::string kind = roleKind(role);
                                const auto byKind = regOfKind.find(kind);
                                if (kind.empty() || ambiguous.count(kind) > 0 || byKind == regOfKind.end()) {
                                    return std::nullopt;
                                }

                                return byKind->second;
                            };

                            // Same rule as bindLine's `toEdited`: swap in the fix's edited copy, but
                            // only for the role that edit was registered for, since one file may
                            // serve several
                            auto editedOf = [this](const std::string& resource, const std::string& role) {
                                const std::string bound = StringTools::toLower(resource);
                                const auto swap = editedResourceOf_.find(bound);
                                const auto owns = editedRoleOf_.find(bound);
                                if (swap != editedResourceOf_.end() && owns != editedRoleOf_.end()
                                        && owns->second == role) {
                                    return swap->second;
                                }

                                return resource;
                            };

                            auto carried = std::make_unique<CarriedTexRegs>(
                                config_.texRegPrefix, roleOf, regOf, editedOf);
                            auto adapter = std::make_unique<RegPartEdit<>>(carried.get());
                            editsOf_[component].push_back(adapter.get());
                            carriedEdits_.push_back(std::move(carried));
                            regAdapters_.push_back(std::move(adapter));
                        }

                        // The mirrored twin, AFTER the component's own draw -- an empty
                        // predicate on `drawindexed` accepts any value, and `latest = false` puts
                        // the block at the earliest position that follows one. Its own add, because
                        // everything in `additions` goes BEFORE the draw.
                        if (mirroredSet().count(component) > 0) {
                            auto twin = std::make_unique<MirrorTwin>(
                                fixName(IniKeywords::Resource + "MirrorIndex" + std::to_string(component)),
                                fixName(IniKeywords::Resource + "MirrorVector"),
                                IndexBufferResource, VectorBufferResource, config_.vectorReg,
                                mirrorSpan(component).first);
                            auto adapter = std::make_unique<RegPartEdit<>>(twin.get());
                            editsOf_[component].push_back(adapter.get());
                            twinEdits_.push_back(std::move(twin));
                            regAdapters_.push_back(std::move(adapter));
                        }

                        if (!additions.empty()) {
                            // The remap has already renamed the called list by the time this runs,
                            // so the anchor is matched under either name.
                            const std::string sharedList = config_.sharedResourcesList;
                            const std::string renamedList = fixName(sharedList);
                            auto add = std::make_unique<RegSurroundedAdd<>>(
                                std::move(additions),
                                RegSurroundedAdd<>::RegMap{{IniKeywords::Run, [sharedList, renamedList](const std::string& value) {
                                    return value == sharedList || value == renamedList;
                                }}},
                                RegSurroundedAdd<>::RegMap{}, false);
                            auto adapter = std::make_unique<GraphPartEdit<>>(add.get());
                            editsOf_[component].push_back(adapter.get());
                            surroundedAdds_.push_back(std::move(add));
                            graphAdapters_.push_back(std::move(adapter));
                        }
                    }

                }

                // The hash-only objects: the hidden ones (the shape keys) commented out of the
                // original and copied nowhere, the rest (the bone-data override) into every group
                void buildSlotRemap() {
                    // Members rather than locals: buildPerGroupEdits below reads both, and the two
                    // phases were one function when they shared them by scope.
                    hashOnlyObjs_.clear();
                    if (auto* parser = dynamic_cast<GIMIParser<>*>(this->getParser())) {
                        for (const ModObj& obj : parser->modObjs()) {
                            if (!StringTools::startsWith(obj.second, config_.slotPrefix)) {
                                hashOnlyObjs_.push_back(obj);
                            }
                        }
                    }

                    hiddenObjs_ = std::unordered_set<std::string>(config_.hiddenObjs.begin(), config_.hiddenObjs.end());
                    const std::vector<ModObj>& hashOnlyObjs = hashOnlyObjs_;
                    const std::unordered_set<std::string>& hidden = hiddenObjs_;

                    // The remap: every source slot section onto its target slot's object, named
                    // after the TARGET so a target never collides with a source still to be moved;
                    // two claimants of one slot collide with each other, which is what makes the
                    // further groups (GraphGroupRemap's own rule). Every section keeps its own name
                    // plus the fix suffix, as the prototype's output has it.
                    const GraphGroupRemap<>::RenameFunc rename = [this](const std::string& name) { return fixName(name); };
                    GraphGroupRemap<>::RemapList remap;
                    for (const auto& entry : present_) {
                        std::vector<GraphGroupRemap<>::RemapTarget> targets;
                        auto planned = config_.plan.find(entry.first);
                        if (planned != config_.plan.end()) {
                            const ModObj target = targetSlotObj(planned->second.slot);
                            targets.emplace_back(GraphId(0, target.first, target.second), rename);
                        }

                        const ModObj src = slotObj(entry.first);
                        remap.emplace_back(GraphId(0, src.first, src.second), std::move(targets));
                    }

                    for (const ModObj& obj : hashOnlyObjs) {
                        std::vector<GraphGroupRemap<>::RemapTarget> targets;
                        if (hidden.count(obj.second) > 0) {
                            this->hiddenModObjs.insert(obj);
                        } else {
                            for (std::size_t g = 0; g < groups_.size(); ++g) {
                                targets.emplace_back(GraphId(0, obj.first, obj.second), rename);
                            }
                        }

                        remap.emplace_back(GraphId(0, obj.first, obj.second), std::move(targets));
                    }

                    slotRemap_ = std::make_unique<GraphGroupRemap<>>(std::move(remap));

                }

                // The edits, per group, keyed by the target objects the remap created.
                void buildPerGroupEdits() {
                    const std::vector<ModObj>& hashOnlyObjs = hashOnlyObjs_;
                    const std::unordered_set<std::string>& hidden = hiddenObjs_;
                    std::vector<ObjGroupEdit::IniEdits> perGroup(groups_.size());
                    for (std::size_t g = 0; g < groups_.size(); ++g) {
                        for (int component : groups_[g]) {
                            const int slot = config_.plan.at(component).slot;
                            const ModObj obj = targetSlotObj(slot);
                            std::vector<PartEdit*> edits = editsOf_[component];
                            if (removeAdapter_ != nullptr) {
                                edits.push_back(removeAdapter_.get());
                            }

                            edits.push_back(newValsOf_.at(slot));
                            edits.push_back(assetAdapter_.get());
                            perGroup[g].edits[obj] = std::move(edits);
                            perGroup[g].trackKeys[obj] = false;
                        }

                        for (const ModObj& obj : hashOnlyObjs) {
                            if (hidden.count(obj.second) > 0) {
                                continue;
                            }

                            perGroup[g].edits[obj] = {assetAdapter_.get()};
                            perGroup[g].trackKeys[obj] = false;
                            if (g == 0) {
                                }
                        }
                    }

                    mainEdits_ = std::make_unique<ObjGroupEdit>(std::move(perGroup), false);

                }

                // The blend: the register bound in the shared override, collected out of every
                // drawn slot section -- one collect PER GROUP, so every copy declares the
                // RemapBlend resource its own sections bind, as a GIMI merge's copies do (a
                // collect is addressed by GraphId, whose iniIndex is the group).
                void buildBlendCollects(const ModType* source, const std::optional<Version>& from,
                                         const std::optional<Version>& to) {
                    for (std::size_t g = 0; g < groups_.size(); ++g) {
                        std::function<bool(RemapBlendResource&)> lift;
                        if (targetPast256_) {
                            // The target cannot be named by an 8-bit blend index, so the fix writes
                            // WWMI's blend remap. Takes precedence over both lifts below: each of
                            // them writes 8-bit ids only, which for this target is the silent
                            // truncation this exists to stop (81004 weighted slots of one real mod,
                            // not one of them landing on the bone the row asks for).
                            const std::string vgRel = FileService::iniPathToRel(vertexVGFile_.value_or(""));
                            const std::string vgPath = vgRel.empty()
                                ? std::string()
                                : FileService::absPathOfRelPath(vgRel, ctx_.getIniFile()->getFolder());
                            const std::string posPath =
                                positionBufPath();
                            const BlendRemapOut out{
                                FileService::absPathOfRelPath(blendRemapFile("VertexVG"), ctx_.getIniFile()->getFolder()),
                                FileService::absPathOfRelPath(blendRemapFile("Forward"), ctx_.getIniFile()->getFolder()),
                                FileService::absPathOfRelPath(blendRemapFile("Reverse"), ctx_.getIniFile()->getFolder())};
                            lift = [vgPath, posPath, out](RemapBlendResource& resource) {
                                return writeBlendRemap(resource, vgPath, posPath, out);
                            };
                        } else if (vertexVGFile_.has_value()) {
                            const std::string vgRel = FileService::iniPathToRel(*vertexVGFile_);
                            const std::string vgPath =
                                FileService::absPathOfRelPath(vgRel, ctx_.getIniFile()->getFolder());
                            const std::string posPath =
                                positionBufPath();
                            lift = [vgPath, posPath](RemapBlendResource& resource) {
                                return remapFromVertexVG(resource, vgPath, posPath);
                            };
                        } else if (legacy_) {
                            const std::string indexPath = FileService::absPathOfRelPath(indexFile_, ctx_.getIniFile()->getFolder());
                            const std::string positionPath =
                            positionBufPath();
                            const std::map<int, std::vector<std::pair<long long, long long>>> ranges = drawRanges_;
                            const std::map<int, std::vector<int>> maps = config_.sourceVgMaps;
                            lift = [indexPath, positionPath, ranges, maps](RemapBlendResource& resource) {
                                return liftLegacyBlend(resource, indexPath, positionPath, ranges, maps);
                            };
                        }

                        auto replace = std::make_unique<WWMIBlendReplace>(GraphId(g, "", "blend"), makeResEditConfig(), source, from, to,
                                                                          std::move(lift), config_.anchorChains,
                                                                          meshVertexCount_,
                                                                          positionBufPath());
                        auto collect = std::make_unique<Collector>();
                        for (int component : groups_[g]) {
                            const ModObj obj = targetSlotObj(config_.plan.at(component).slot);
                            collect->srcRegs[GraphId(g, obj.first, obj.second)] = config_.blendReg;

                            // A MOD OF A CHARACTER PAST 256 BONES BINDS THE BLEND REGISTER TWICE:
                            // `vb4 = ResourceBlendBuffer` when no blend remap is active, and
                            // `vb4 = ref ResourceBlendBufferOverride` when one is. Only the first
                            // names a FILE -- the second is a buffer WWMI's BlendRemapper fills at
                            // load -- so take that one and leave the other. Without this the run
                            // dies looking for a section called `Resourceref Resource...` and the
                            // whole mod is skipped.
                            //
                            // A mod that binds it once, plainly, is unaffected: the predicate passes.
                            collect->resPredicates[GraphId(g, obj.first, obj.second)] =
                                [](const std::string&, const std::string& value, const Collector::IterData&) {
                                    return !IniNamingTools::hasRefPrefix(value);
                                };
                        }

                        collect->resEdits = {{BlendResGroupMember, replace.get()}};
                        blendReplaces_.push_back(std::move(replace));
                        blendCollects_.push_back(std::move(collect));
                    }
                }

                // ---- what the WRITTEN text must say ----


                // The resource a component binds for a role: its own choice (readTextures), else what
                // every component shares -- a created texture, or a download of the source's own
                // A shared mesh has no component, so take whichever present component owns the art
                // Every role bound only by a slot's other passes or by a shared mesh -- the ones
                // config_.plan does not name
                std::vector<std::string> passOnlyRoles() const {
                    std::vector<std::string> roles;
                    for (const auto& [slot, byPass] : config_.extraPassRegs) {
                        bool drawn = false;
                        for (const auto& planned : config_.plan) {
                            if (planned.second.slot == slot && present_.count(planned.first) > 0) {
                                drawn = true;
                                break;
                            }
                        }

                        if (!drawn) {
                            continue;
                        }

                        for (const auto& [pass, binds] : byPass) {
                            (void)pass;
                            for (const WWMIFixerConfig::Binding& binding : binds) {
                                roles.push_back(binding.role);
                            }
                        }
                    }

                    for (const auto& [mesh, byPass] : config_.sharedMeshes) {
                        (void)mesh;
                        for (const auto& [pass, binds] : byPass) {
                            (void)pass;
                            for (const WWMIFixerConfig::Binding& binding : binds) {
                                roles.push_back(binding.role);
                            }
                        }
                    }

                    std::vector<std::string> out;
                    for (const std::string& role : roles) {
                        ListTools::pushDistinct(out, role);
                    }

                    return out;
                }

                // The line binding one role at one register inside a fix-written texture list.
                //
                // `<reg> = <resource>` normally. For a role the mod binds behind its own toggle, a
                // `run =` into a copy of the mod's [TextureOverrideTexture*] instead, with 3dmigoto's
                // matching keys dropped and `this` renamed to the register -- so the toggle comes
                // across untouched rather than collapsing to its first variant.
                /**
                 * @brief `<reg> = null` -- bind NOTHING, rather than letting the game's own
                 *        texture serve the register (see WWMIFixerConfig::Binding::role)
                 */
                using Kvp = std::pair<std::string, std::string>;

                Kvp nullLine(const std::string& reg) {
                    return {reg, std::string(IniKeywords::Null)};
                }

                Kvp bindLine(const std::string& role, const std::string& reg, const std::string& resource) {
                    const Kvp direct{reg, resource};

                    // An edit's output is not one of the mod's resources, so ask under the resource
                    // the edit READ
                    const auto edited = sourceOfEdited_.find(StringTools::toLower(resource));
                    const std::string base = edited == sourceOfEdited_.end() ? resource : edited->second;
                    const auto owner = conditionalOwner_.find(StringTools::toLower(base));
                    if (owner == conditionalOwner_.end()) {
                        return direct;
                    }

                    const auto cached = roleLists_.find({role, reg});
                    if (cached != roleLists_.end()) {
                        return {std::string(IniKeywords::Run), cached->second};
                    }

                    IniFile* ini = ctx_.getIniFile();
                    const auto& templates = ini->getIfTemplates();
                    const auto tpl = templates.find(owner->second);
                    if (tpl == templates.end() || tpl->second == nullptr) {
                        return direct;
                    }

                    const std::string name =
                        fixName(IniKeywords::CommandList + source_.name + TextTools::capitalize(role) + IniNamingTools::getRegTag(reg));

                    // A COPY OF THE MOD'S OWN SECTION, EDITED THROUGH THE SAME regEdits AS THE REST
                    // OF THE FIX. This used to render the section to text and rewrite the lines,
                    // which is the section model written out and read straight back in.
                    std::unique_ptr<IfTemplate<std::string, std::string>> list = tpl->second->deepcopy();
                    list->name = name;
                    list->prefix = "";                      // the mod's own comment is about the mod's section

                    // 1. A BRANCH NAMING A RESOURCE THE MOD NEVER DECLARES IS THE AUTHOR'S TYPO, AND
                    //    COPYING IT CAN ONLY MAKE THINGS WORSE (2026-09-28). SanhuaExorcist4's
                    //    `_injured` override says `ResourceTexture7.1` / `.2` where it declares
                    //    `ResourceTexture7a` / `7b`; this list runs AFTER the mod's own component
                    //    section, whose own $yifu toggle binds all three correctly, so a dead branch
                    //    replaces a good binding with nothing. Dropped, the mod's own stands for
                    //    those values and the fix binds the one branch that resolves.
                    //    ...and a resource the FIX declares is not dead either. A value may already
                    //    have been swapped for an edited resource of ours, which is in none of the
                    //    MOD's maps -- so an earlier form of this test dropped every branch of a
                    //    toggled role that carries an edit and the whole list collapsed to the
                    //    single direct binding of the resolved variant. Chisa7 toggles its hair
                    //    normal between `Components-1 t=d8ed7611.dds` and `... A.dds`; on a clean fix
                    //    the second repack was written, declared and bound by NOTHING, so at
                    //    `$Char != 0` the hair took variant 0's normal map. `sourceOfEdited_` is
                    //    keyed by exactly those names.
                    const RegRemove<>::RemoveKeyCheck isDead =
                        [this, &templates](long long, const std::string& val) {
                            const std::string bound = StringTools::toLower(val);
                            return editedResourceOf_.count(bound) == 0 && sourceOfEdited_.count(bound) == 0
                                   && fileOfResource_.count(bound) == 0 && templates.count(val) == 0;
                        };

                    // 2. `this = <a resource>` becomes `this = <our edited copy of it>` -- but ONLY
                    //    for the role the edit was registered for. One file can serve several roles
                    //    -- Chisa13 points RabbitFX's Lightmap AND Normalmap at one resource, and
                    //    this fix reads that Lightmap as the material MASK -- so a swap keyed on the
                    //    RESOURCE alone put the repacked normal map on the mask register too:
                    //    material code 0 over the whole head and A = 255 where the target's mask
                    //    carries 0 (2026-09-27).
                    const RegNewVals<>::OldValProducer toEdited =
                        [this, &role](const std::string& oldValue, const ModType*) {
                            const std::string bound = StringTools::toLower(oldValue);
                            const auto swap = editedResourceOf_.find(bound);
                            const auto owns = editedRoleOf_.find(bound);
                            if (swap != editedResourceOf_.end()
                                    && owns != editedRoleOf_.end() && owns->second == role) {
                                return swap->second;
                            }

                            return oldValue;
                        };

                    std::size_t bindings = 0;
                    for (const std::unique_ptr<IfTemplatePart>& part : list->parts()) {
                        auto* content = dynamic_cast<IfContentPart<std::string, std::string>*>(part.get());
                        if (content == nullptr) {
                            continue;                       // an `if` / `endif`, which the renderer keeps
                        }

                        // The rules are matched against the part's OWN keys rather than against the
                        // literal spellings: `THIS = ResourceX` is legal 3dmigoto and a regEdit
                        // matches a key exactly, so the exact key is what it is handed.
                        std::vector<std::string> binds;
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>> drops;
                        std::unordered_set<std::string> seen;
                        for (const auto& item : content->items()) {
                            if (!seen.insert(item.key).second) {
                                continue;
                            }

                            const std::string key = StringTools::toLower(item.key);
                            if (IniKeywords::MatchKeys.count(key) > 0) {
                                drops.emplace_back(item.key, std::nullopt);   // meaningless in a list
                            } else if (key == IniKeywords::This) {
                                drops.emplace_back(item.key, isDead);
                                binds.push_back(item.key);
                            }
                        }

                        RegRemove<>(std::move(drops)).edit(*content, name);

                        for (const std::string& key : binds) {
                            RegNewVals<>({{key, RegNewVals<>::NewVal(toEdited)}}).edit(*content, name);

                            // 3. ...and the key itself is the register the target's draw reads.
                            bindings += content->count(key);
                            RegRemap<>({{key, RegRemap<>::KeyRemapValue(
                                RemapList<std::string, std::string>{reg})}})
                                .edit(*content, name);
                        }
                    }

                    if (bindings == 0) {
                        return direct;
                    }

                    list->rebuild();
                    roleLists_.emplace(std::pair<std::string, std::string>{role, reg}, name);
                    // Their own vector, emitted at the END of buildAppended: textureLists_ is
                    // written out before the shared-mesh loop runs, so a list created there would be
                    // dropped. Section order in an .ini does not matter.
                    // `renderIfTemplate` returns no trailing newline; the consumer adds the blank line
                    roleListTexts_.push_back(renderIfTemplate(*list) + "\n");
                    return {std::string(IniKeywords::Run), name};
                }

                const std::string* sharedResourceFor(const std::string& role) const {
                    for (const auto& entry : present_) {
                        const std::string* resource = resourceFor(role, entry.first);
                        if (resource != nullptr) {
                            return resource;
                        }
                    }

                    return nullptr;
                }

                const std::string* resourceFor(const std::string& role, int component) const {
                    auto slotRole = resourceOfSlotRole_.find({role, component});
                    if (slotRole != resourceOfSlotRole_.end()) {
                        return &slotRole->second;
                    }

                    auto shared = resourceOfRole_.find(role);
                    return shared == resourceOfRole_.end() ? nullptr : &shared->second;
                }

                // The components a WWMI-Tools export name is tagged for: `Components-0-2 t=<hash>.dds`
                // is {0, 2}. Empty for a file named anything else

                // The shaders TAGGED for a pass: its own pixel shader by default, or the vertex
                // shaders it is drawn with -- see WWMIFixerConfig::passVertexShaders.
                std::vector<std::string> taggedFor(const std::string& pass) const {
                    if (config_.passVertexShaders.empty()) {
                        return {pass};
                    }

                    const auto at = config_.passVertexShaders.find(pass);
                    if (at == config_.passVertexShaders.end() || at->second.empty()) {
                        // Not a fallback to tagging the pixel shader: that would be silent, and it
                        // is the bug the map exists to avoid. A pass reaches here only when a
                        // compiled config is written wrong, so it is loud and immediate -- the same
                        // check the prototype makes with an assert.
                        throw std::runtime_error(
                            "WWMIFixer: pass " + pass + " has no vertex shader in passVertexShaders. "
                            "Read the pair off a frame dump's draw table; tagging its pixel shader "
                            "instead would switch RabbitFX off for every mod using that shader.");
                    }

                    return at->second;
                }

                // The `.ini` condition true on a draw of 'pass'
                // The condition for a set of passes: the OR over the shaders they are gated by,
                // each named ONCE. Two passes of one slot can share a vertex shader -- Chisa's bangs
                // and hair are both gated by d83a54772fc666f9 -- and joining their conditions wrote
                // `vs == X || vs == X`. Harmless, but it is noise in every diff and it would hide the
                // real version of the same shape: two DIFFERENT shaders resolving to one index.
                std::string passCondition(const std::vector<std::string>& passes) {
                    passFilter("");
                    const std::string reg = config_.passVertexShaders.empty() ? "ps" : "vs";
                    std::vector<std::string> named;
                    std::string out;
                    for (const std::string& pass : passes) {
                        for (const std::string& tagged : taggedFor(pass)) {
                            if (std::find(named.begin(), named.end(), tagged) != named.end()) {
                                continue;
                            }

                            named.push_back(tagged);
                            out += (out.empty() ? "" : " || ") + reg + " == " + passFilters_[tagged];
                        }
                    }

                    return out;
                }

                std::string passCondition(const std::string& pass) {
                    return passCondition(std::vector<std::string>{pass});
                }

                std::string passFilter(const std::string& pass) {
                    // One filter_index per distinct shader, in order of first appearance over the
                    // slots -- except for a shader config.filterIndices names, which takes the value
                    // it is given there. 3dmigoto keys a [ShaderOverride] by its shader hash across
                    // EVERY loaded .ini, so two pairs that tag the same shader (both directions of one
                    // pair always do: the hair, face and eye shaders are the same on a skin and its
                    // character) must agree on its value, or whichever file loads last wins and the
                    // other mod's `if ps == ...` never matches -- its textures silently unbound.
                    if (passFilters_.empty()) {
                        std::size_t i = 0;
                        // every pass either table names -- an extraPassRegs pass with no
                        // filter_index has nothing to match on, so its command list never fires and
                        // that slot draws with the game's textures on that pass
                        std::vector<std::vector<std::string>> allPasses = config_.slotPasses;
                        for (const auto& [mesh, byPass] : config_.sharedMeshes) {
                            (void)mesh;
                            allPasses.push_back(passNamesOf(byPass));
                        }

                        for (const auto& [slot, byPass] : config_.extraPassRegs) {
                            (void)slot;
                            allPasses.push_back(passNamesOf(byPass));
                        }

                        for (const auto& passes : allPasses) {
                            for (const std::string& p : passes) {

                                for (const std::string& tagged : taggedFor(p)) {
                                    if (passFilters_.count(tagged) != 0) {
                                        continue;
                                    }

                                    auto given = config_.filterIndices.find(tagged);
                                    if (given != config_.filterIndices.end()) {
                                        passFilters_[tagged] = given->second;
                                    } else {
                                        passFilters_[tagged] = NumTools::formatDouble(config_.filterBase + config_.filterStep * static_cast<double>(i));
                                        ++i;
                                    }

                                    passOrder_.push_back(tagged);
                                }
                            }
                        }
                    }

                    return passFilters_[pass];
                }

                // The merge list the fix supplies per target slot for a legacy mod
                std::string mergeListName(int slot) const {
                    return fixName("CommandListMergeSlot" + std::to_string(slot));
                }

                // What a mod from before WWMI's merged skeleton has none of: the two skeleton buffers and
                // their read-only copies, the marker on the game's bone-data constant buffer, and one
                // merge list per target slot -- each gated on the marker, so a pass with something else in
                // that slot cannot merge junk into the skeleton.
                std::string legacySkeletonSections() {
                    const std::string merged = fixName(IniKeywords::Resource + std::string("MergedSkeleton"));
                    const std::string mergedRW = fixName(IniKeywords::Resource + std::string("MergedSkeletonRW"));
                    const std::string extra = fixName(IniKeywords::Resource + std::string("ExtraMergedSkeleton"));
                    const std::string extraRW = fixName(IniKeywords::Resource + std::string("ExtraMergedSkeletonRW"));
                    std::string out = SectionText(z3_, merged).str() + SectionText(z3_, extra).str();
                    for (const std::string& name : {mergedRW, extraRW}) {
                        out += SectionText(z3_, name)
                                   .keys({{IniKeywords::Type, "RWBuffer"},
                                          {IniKeywords::Format, "R32G32B32A32_FLOAT"},
                                          {IniKeywords::Array, std::to_string(config_.mergedSkeletonSlots)}})
                                   .str();
                    }

                    out += SectionText(z3_, fixName("TextureOverrideMarkBoneDataCB"))
                               .keys({{IniKeywords::Hash, target_.cb4Hash},
                                      {IniKeywords::MatchPriority, "0"},
                                      {IniKeywords::FilterIndex, config_.boneDataFilter}})
                               .str();

                    // The private skeleton a blend remap needs, gathered right after the merge:
                    // SkeletonRemapper writes remapped[i] = merged[forward[i]], so a vertex whose
                    // blend names LOCAL i reaches the bone forward[i] -- which is how an 8-bit index
                    // addresses a 420-slot skeleton at all. Bound in place of the merged one.
                    const std::string remappedRW = fixName(IniKeywords::Resource + std::string("RemappedSkeletonRW"));
                    const std::string remapped = fixName(IniKeywords::Resource + std::string("RemappedSkeleton"));
                    const std::string extraRemappedRW = fixName(IniKeywords::Resource + std::string("ExtraRemappedSkeletonRW"));
                    const std::string extraRemapped = fixName(IniKeywords::Resource + std::string("ExtraRemappedSkeleton"));
                    if (targetPast256_) {
                        for (const std::string& name : {remapped, extraRemapped, remappedRW, extraRemappedRW}) {
                            out += SectionText(z3_, name).str();
                        }
                    }

                    // The remap, run ONCE PER FRAME from the COMPLETE merged skeleton rather than per draw
                    // from a partly filled one (2026-10-02).
                    //
                    // WWMI's own design: each component's draw merges its window into a persistent RW
                    // buffer, and `[Present]` snapshots the result and remaps it, so the NEXT frame's draws
                    // all bind one complete skeleton. Remapping inside each draw instead reads a buffer
                    // holding only the windows merged so far this frame -- which differs per slot and per
                    // frame, so the render comes out UNSTABLE rather than wrong: five reloads of one mod,
                    // same files and same build, gave four different pictures.
                    //
                    // Anchored on the frame's first remapped draw rather than on `[Present]`, which is
                    // equivalent -- nothing but these merge lists writes the RW buffer, so between
                    // `[Present]` and the first draw it still holds exactly the previous frame's complete
                    // merge. It also keeps the hook inside the fix's OWN sections: the mod's `[Present]` is
                    // the host's, and no .ini in the corpus carries two of them, so appending one would bet
                    // on 3dmigoto merging duplicate sections within a file, which nothing here establishes.
                    //
                    // `$state_id` is the host's frame counter, flipped once a frame at `[Present]`.
                    if (targetPast256_) {
                        // NO FRAME LATCH (2026-10-02). It was latched on `$state_id`, which the identity mod
                        // maintains and a real WWMI Tools mod of the current generation does NOT -- it declares
                        // `global $state_id = 0`, never assigns it, and keeps its own `$merge_status_id`. The
                        // latch then read 0 != 0 forever: `if $remapped_state != $state_id: false` 118841 times
                        // and true ZERO in 3dmigoto's log, so the remap never ran, the remapped skeleton was
                        // never created, and binding that null UNBOUND vs-cb4 -- an invisible model.
                        //
                        // Unconditional costs a few dispatches a frame and needs no frame counter, because the
                        // property that matters never came from the latch: the merged RW buffer is PERSISTENT
                        // and accumulates, so at any draw it holds every window from this frame or the last, and
                        // a remap taken at any point reads a COMPLETE skeleton. That is what moving it out of
                        // the per-draw path was for, and it survives a mod that keeps no frame counter.
                        SectionText remap(z3_, fixName("CommandListRemapMergedSkeleton"));
                        for (const auto& cb : {std::make_tuple(mergedRW, merged, remappedRW, remapped),
                                               std::make_tuple(extraRW, extra, extraRemappedRW, extraRemapped)}) {
                            remap.keys({{std::get<1>(cb), "copy " + std::get<0>(cb)},
                                            {"cs-t37", fixName(IniKeywords::Resource + std::string("BlendRemapForwardBuffer"))},
                                            {"$\\WWMIv1\\blend_remap_id", "0"},
                                            {VgCountKey, std::to_string(blendRemapBones_)},
                                            {"cs-t38", std::get<1>(cb)},
                                            // Seeded from the merged RW, which is declared with a size. An empty
                                            // resource bound to `cs-u5` has no backing buffer: the dispatch writes
                                            // nowhere, the `copy` below then finds no source, and binding that null
                                            // UNBINDS the slot -- invisible rather than wrong.
                                            {std::get<2>(cb), "copy " + std::get<0>(cb)},
                                            {"cs-u5", std::get<2>(cb)},
                                            {IniKeywords::Run, "CustomShader\\WWMIv1\\SkeletonRemapper"},
                                            {std::get<3>(cb), "copy " + std::get<2>(cb)}});
                        }

                        out += remap.str();
                    }

                    // The merge a HIDDEN slot runs. A target slot nothing is remapped onto still
                    // contributes bones -- Chisa's [391, 419) carries bone 409, which 1185 of the identity
                    // mod's weighted influence slots name -- and its `...RemapHide` section used to run the
                    // host's copied `CommandListMergeSkeleton`, which writes the MOD's merged skeleton,
                    // sized for the source. The bone was then never written into the buffer the remap
                    // reads, so every vertex on it skinned against a zero matrix and collapsed to the
                    // origin -- and a triangle with one corner there is a plane across the scene.
                    //
                    // Merge only, no bind: the caller sets vg_offset / vg_count and the draw is skipped.
                    if (targetPast256_) {
                        SectionText window(z3_, fixName("CommandListMergeWindow"));
                        window.key("$\\WWMIv1\\custom_mesh_scale", "1.00");
                        for (const auto& cb : {std::make_pair(std::string("vs-cb4"), mergedRW),
                                               std::make_pair(std::string("vs-cb3"), extraRW)}) {
                            window.open(cb.first + " == " + config_.boneDataFilter)
                                  .keys({{"cs-cb8", IniKeywords::Ref + " " + cb.first},
                                         {"cs-u6", cb.second},
                                         {IniKeywords::Run, "CustomShader\\WWMIv1\\SkeletonMerger"}})
                                  .close();
                        }

                        out += window.str();
                    }

                    for (std::size_t slot = 0; slot < target_.slots.size(); ++slot) {
                        const Slot& s = target_.slots[slot];
                        SectionText mergeList(z3_, mergeListName(static_cast<int>(slot)));
                        if (targetPast256_) {
                            // Before the merge, so the frame's first remapped draw remaps from the previous
                            // frame's COMPLETE merge rather than from this frame's first window.
                            mergeList.key(IniKeywords::Run, fixName("CommandListRemapMergedSkeleton"));
                        }

                        for (const auto& cb : {std::make_tuple(std::string("vs-cb4"), mergedRW, merged, remappedRW, remapped),
                                               std::make_tuple(std::string("vs-cb3"), extraRW, extra, extraRemappedRW, extraRemapped)}) {
                            mergeList.open(std::get<0>(cb) + " == " + config_.boneDataFilter)
                                     .keys({{VgOffsetKey, s.vgOffset},
                                            {VgCountKey, s.vgCount},
                                            {"$\\WWMIv1\\custom_mesh_scale", "1.00"},
                                            {"cs-cb8", IniKeywords::Ref + " " + std::get<0>(cb)},
                                            {"cs-u6", std::get<1>(cb)},
                                            {IniKeywords::Run, "CustomShader\\WWMIv1\\SkeletonMerger"}});

                            if (targetPast256_) {
                                // Bind what the frame's remap produced. The snapshot and the SkeletonRemapper
                                // dispatch that used to sit here moved into CommandListRemapMergedSkeleton,
                                // which runs once a frame from the complete merge.
                                mergeList.key(std::get<0>(cb), std::get<4>(cb));
                            } else {
                                // The snapshot the two branches used to share, kept here so a legacy mod writes
                                // exactly the lines it wrote before.
                                mergeList.keys({{std::get<2>(cb), "copy " + std::get<1>(cb)},
                                                {std::get<0>(cb), std::get<2>(cb)}});
                            }

                            mergeList.close();
                        }

                        out += mergeList.str();
                    }

                    if (targetPast256_) {
                        out += blendRemapSections();
                    }

                    return out;
                }

                // The blend remap's resources, and the one-time run of WWMI's BlendRemapper that
                // rewrites the remapped blend's ids into the remap's LOCAL space. Guarded by a
                // `local`, so it costs one dispatch on the first drawn frame and nothing after.
                //
                // The strided view is the fix's own declaration of the same file rather than the
                // collect's resource: `copy_desc` needs a buffer with the blend's stride, and
                // declaring it here keeps this text independent of what the collect happened to
                // name its resource.
                std::string blendRemapSections() {
                    const std::string vertexVG = fixName(IniKeywords::Resource + std::string("BlendRemapVertexVGBuffer"));
                    const std::string forward = fixName(IniKeywords::Resource + std::string("BlendRemapForwardBuffer"));
                    const std::string reverse = fixName(IniKeywords::Resource + std::string("BlendRemapReverseBuffer"));
                    const std::string noStride = fixName(IniKeywords::Resource + std::string("BlendNoStride"));
                    const std::string strided = fixName(IniKeywords::Resource + std::string("BlendStrided"));
                    const std::string blendRW = fixName(IniKeywords::Resource + std::string("RemappedBlendBufferRW"));
                    const std::string blendOut = fixName(IniKeywords::Resource + std::string("RemappedBlendBuffer"));
                    const std::string blendPath = blendFixedFile();

                    std::string out;
                    // no stride on the three the compute shader reads: a Buffer declared with one
                    // cannot be addressed by a compute shader (wwmiIdentityMod.py's own note)
                    for (const auto& entry : {std::make_pair(vertexVG, blendRemapFile("VertexVG")),
                                              std::make_pair(forward, blendRemapFile("Forward")),
                                              std::make_pair(reverse, blendRemapFile("Reverse")),
                                              std::make_pair(noStride, blendPath)}) {
                        out += SectionText(z3_, entry.first)
                                   .keys({{IniKeywords::Type, "Buffer"},
                                          {IniKeywords::Format, entry.first == noStride ? "DXGI_FORMAT_R8_UINT"
                                                                             : "DXGI_FORMAT_R16_UINT"},
                                          {IniKeywords::Filename, entry.second}})
                                   .str();
                    }

                    out += SectionText(z3_, strided)
                               .keys({{IniKeywords::Type, "Buffer"},
                                      {IniKeywords::Format, "DXGI_FORMAT_R8_UINT"},
                                      {IniKeywords::Stride, std::to_string(blendStride_)},
                                      {IniKeywords::Filename, blendPath}})
                               .str();
                    out += SectionText(z3_, blendRW).str();
                    out += SectionText(z3_, blendOut).str();

                    out += SectionText(z3_, blendRemapInitList())
                               .key("local $blendRemapReady")
                               .open("!$blendRemapReady")
                               .keys({{"$\\WWMIv1\\custom_vertex_count", "$mesh_vertex_count"},
                                      {"$\\WWMIv1\\weights_per_vertex_count", std::to_string(blendInfluences_)},
                                      {"$\\WWMIv1\\blend_remap_id", "0"},
                                      {"cs-t34", IniKeywords::Ref + " " + reverse},
                                      {"cs-t35", IniKeywords::Ref + " " + vertexVG},
                                      {blendRW, "copy " + noStride},
                                      {"cs-u4", IniKeywords::Ref + " " + blendRW},
                                      {IniKeywords::Run, "CustomShader\\WWMIv1\\BlendRemapper"},
                                      {blendOut, "copy " + blendRW},
                                      {blendOut, "copy_desc " + strided},
                                      {"$blendRemapReady", "1"}})
                               .close()
                               .key(config_.blendReg, blendOut)
                               .str();
                    return out;
                }

                // ---- the fix's own sections ----

                void buildAppended() {
                    std::string out;
                    for (const auto& entry : declared_) {
                        out += SectionText(z3_, declaredName_[entry.first])
                                   .key(IniKeywords::Filename, entry.second)
                                   .str();
                    }

                    // its OWN name -- see Fallback::resource. resourceOfRole_ may by now be the
                    // resource of a texture EDIT of this role, and writing the download's section
                    // under that name declares it twice, raw file and edited file.
                    for (const auto& entry : fallbacks_) {
                        out += SectionText(z3_, entry.second.resource)
                                   .key(IniKeywords::Filename, entry.second.relPath)
                                   .str();
                    }

                    if (config_.zeroShapeKeyStream && meshVertexCount_ > 0) {
                        out += SectionText(z3_, fixName(IniKeywords::Resource + ShapeKeyZero))
                                   .keys({{IniKeywords::Type, "Buffer"},
                                          {IniKeywords::Format, "DXGI_FORMAT_R32G32B32_FLOAT"},
                                          {IniKeywords::Stride, std::to_string(config_.shapeKeyStride)},
                                          {IniKeywords::Filename, zeroStreamFile()}})
                                   .str();
                    }

                    const std::set<int> mirrored = mirroredSet();
                    if (!mirrored.empty()) {
                        out += SectionText(z3_, fixName(IniKeywords::Resource + "MirrorVector"))
                                   .keys({{IniKeywords::Type, "Buffer"},
                                          {IniKeywords::Format, "DXGI_FORMAT_R8G8B8A8_SNORM"},
                                          {IniKeywords::Stride, "8"},
                                          {IniKeywords::Filename, mirrorVectorFile()}})
                                   .str();
                    }

                    for (int component : mirrored) {
                        out += SectionText(z3_, fixName(IniKeywords::Resource + "MirrorIndex"
                                                        + std::to_string(component)))
                                   .keys({{IniKeywords::Type, "Buffer"},
                                          {IniKeywords::Format, "DXGI_FORMAT_R32_UINT"},
                                          {IniKeywords::Stride, "12"},
                                          {IniKeywords::Filename, mirrorIndexFile(component)}})
                                   .str();
                    }

                    for (const std::string& list : textureLists_) {
                        out += list + "\n";
                    }

                    for (std::size_t slot = 0; slot < target_.slots.size(); ++slot) {
                        if (drawnSlots_.count(static_cast<int>(slot)) > 0) {
                            continue;
                        }

                        const Slot& s = target_.slots[slot];
                        auto label = config_.targetLabels.find(static_cast<int>(slot));
                        const std::string labelText = label == config_.targetLabels.end() ? std::to_string(slot) : label->second;
                        const std::string state = "$state_id_" + std::to_string(slot);
                        SectionText hide(z3_, IniKeywords::TextureOverride + toModName_
                                                 + TextTools::capitalize(config_.slotPrefix)
                                                 + std::to_string(slot) + IniKeywords::Remap + "Hide");
                        hide.prefix("; nothing of the mod is drawn through " + toModName_ + "'s " + labelText
                                    + " slot: the skin's own geometry is skipped and its bones still merged")
                            .keys({{IniKeywords::Hash, target_.vb0Hash},
                                   {IniKeywords::MatchFirstIndex, s.indexOffset},
                                   {IniKeywords::MatchIndexCount, s.indexCount},
                                   {"$object_detected", "1"}})
                            .open("$mod_enabled");

                        if (legacy_) {
                            hide.keys({{IniKeywords::Run, mergeListName(slot)}, {IniKeywords::Handling, "skip"}});
                        } else {
                            if (targetPast256_) {
                                // Unconditional, for the reason above: these mods keep no `$state_id`, so the
                                // guard never fires and a hidden slot's window never reaches the fix's skeleton.
                                // The merge is idempotent, so running it per draw is waste rather than a fault.
                                hide.keys({{VgOffsetKey, s.vgOffset},
                                           {VgCountKey, s.vgCount},
                                           {IniKeywords::Run, fixName("CommandListMergeWindow")}});
                            } else {
                                hide.key("local " + state)
                                    .open(state + " != $state_id")
                                    .keys({{state, "$state_id"},
                                           {VgOffsetKey, s.vgOffset},
                                           {VgCountKey, s.vgCount},
                                           {IniKeywords::Run, fixName("CommandListMergeSkeleton")}})
                                    .close();
                            }

                            hide.open("ResourceMergedSkeleton !== null")
                                .key("handling", "skip")
                                .close();
                        }

                        out += hide.close().str();
                    }

                    if (legacy_ || targetPast256_) {
                        out += legacySkeletonSections();
                    }

                    passFilter("");
                    std::size_t i = 0;
                    for (const std::string& pass : passOrder_) {
                        out += SectionText(z3_, fixName("ShaderOverridePass" + std::to_string(i)))
                                   .keys({{IniKeywords::Hash, pass}, {"filter_index", passFilters_[pass]}})
                                   .str();
                        ++i;
                    }

                    // Other meshes the character draws -- see WWMIFixerConfig::sharedMeshes
                    std::size_t meshNum = 0;
                    for (const auto& [meshHash, byPass] : config_.sharedMeshes) {
                        SectionText mesh(z3_, fixName(IniKeywords::TextureOverride + source_.name
                                                      + "SharedMesh" + std::to_string(meshNum)));
                        mesh.key(IniKeywords::Hash, meshHash);

                        bool drew = false;
                        for (const auto& [pass, bindings] : byPass) {
                            std::vector<Kvp> lines;
                            for (const WWMIFixerConfig::Binding& binding : bindings) {
                                const std::string* resource = sharedResourceFor(binding.role);
                                if (resource != nullptr) {
                                    lines.push_back(bindLine(binding.role, binding.reg, *resource));
                                }
                            }

                            if (!lines.empty()) {
                                mesh.open(passCondition(pass)).keys(lines).close();
                                drew = true;
                            }
                        }

                        if (drew) {
                            out += mesh.str();
                        }

                        ++meshNum;
                    }

                    if (texcoordSection_.has_value()) {
                        out += *texcoordSection_;
                    }

                    for (const auto& edited : editedResources_) {
                        out += SectionText(z3_, edited.first)
                                   .key(IniKeywords::Filename, edited.second)
                                   .str();
                    }

                    for (const WWMIFixerConfig::CreatedTexture& created : config_.createdTextures) {
                        out += SectionText(z3_, resourceOfRole_[created.role])
                                   .key(IniKeywords::Filename, createdTextureFile(created))
                                   .str();
                    }

                    for (const std::string& list : roleListTexts_) {
                        out += list + "\n";
                    }

                    this->appendedSections = std::string(StringTools::rstrip(out));
                }

                // The three blend remap buffers, named so the undo takes them: a file the fix
                // wrote carries the target's name and `Remap`, and the resource section that names
                // it lives inside the fix's block.
                std::string blendRemapInitList() const {
                    return fixName("CommandListBlendRemap");
                }

                // The remapped blend as the collect writes it, which is what the BlendRemapper reads
                // ASK FOR THE NAME, DO NOT ASSUME IT. The blend the fix writes goes through the
                // resource machinery, which names it from the MOD's own file
                // (IniNamingTools::getFixedBlendFile: the file's stem with the element keyword
                // replaced, or appended when the stem does not contain it). For an ordinary mod
                // whose blend is `Meshes/Blend.buf` that lands on `Meshes\<Mod>RemapBlend.buf`,
                // which is what this used to hardcode -- and for a MOD-MANAGER-PACKAGED mod, whose
                // buffers are GUIDs under an `.assets` extension, it lands on
                // `Meshes\<guid><Mod>RemapBlend.buf` and the hardcoded name names nothing.
                //
                // The 2026-09-28 audit built exactly that mod (chisaParfaitSynth.py's SynthPackaged)
                // and found 2 of its 88 references dangling -- both of them the blend, which is the
                // one buffer every draw needs. In game that is the 2026-09-25 symptom: a packaged
                // mod rendering nothing but its weapon. No real ChisaParfait mod is packaged, so
                // nothing in hand could have shown it.
                std::string blendFixedFile() const {
                    return IniNamingTools::getFixedBlendFile(blendSourceFile_, toModName_);
                }

                std::string blendRemapFile(const std::string& which) const {
                    return modFile(meshFolder_,
                                   toModName_ + IniKeywords::Remap + "BlendRemap" + which + ".buf");
                }

                std::string zeroStreamFile() const {
                    return modFile(meshFolder_, toModName_ + IniKeywords::Remap + ShapeKeyZero + ".buf");
                }

                std::string mirrorIndexFile(int component) const {
                    return modFile(meshFolder_, toModName_ + IniKeywords::Remap + "MirrorIndex"
                                                    + std::to_string(component) + ".buf");
                }

                std::string mirrorVectorFile() const {
                    return modFile(meshFolder_, toModName_ + IniKeywords::Remap + "MirrorVector.buf");
                }

                /**
                 * @brief
                 @rst
                 The components a mirrored twin is actually written for -- :cpp:member:`
                 WWMIFixerConfig::mirroredComponents` minus the ones this mod cannot carry one for
                 @endrst
                 *
                 * Computed rather than stored: it is read while the edits are built, while the
                 * files are written and while the ``.ini`` is rendered, and those run in that order
                 * but through different entry points -- a member set in one of them is a member
                 * read before it was filled in another.
                 *
                 * @return The components
                 */
                std::set<int> mirroredSet() const {
                    std::set<int> out;
                    if (config_.mirroredComponents.empty() || indexFile_.empty() || vectorFile_.empty()) {
                        return out;
                    }

                    for (int component : config_.mirroredComponents) {
                        if (present_.count(component) == 0) {
                            continue;
                        }

                        const auto ranges = drawRanges_.find(component);
                        if (ranges == drawRanges_.end() || ranges->second.empty()) {
                            continue;
                        }

                        out.insert(component);
                    }

                    return out;
                }

                std::string createdTextureFile(const WWMIFixerConfig::CreatedTexture& created) const {
                    return modFile(textureFolder_,
                                   created.role + toModName_ + IniKeywords::RemapTex + FileExt::DDS);
                }

                // ---- at fix time ----

                void writeZeroStream() {
                    if (!config_.zeroShapeKeyStream || meshVertexCount_ <= 0) {
                        return;
                    }

                    const std::string path = FileService::absPathOfRelPath(zeroStreamFile(), ctx_.getIniFile()->getFolder());
                    const auto size = static_cast<std::uintmax_t>(meshVertexCount_)
                                      * static_cast<std::uintmax_t>(config_.shapeKeyStride);
                    if (FileService::fileSize(path) == size) {
                        return;                                       // already there, at the right size
                    }

                    // THROUGH BufFile, AND THE RESULT IS READ (2026-09-30). This wrote the bytes with
                    // its own ofstream and discarded the result -- the defect the guides record for
                    // `TextureFile::save`, where a run reported editing 18 textures having written
                    // none. The stride now comes from the element declaration rather than from a byte
                    // count, and `BufFile::fix` throws a named error when it cannot write.
                    FileService::makeFolderFor(path);
                    try {
                        BufFile zeros{ByteVec(static_cast<std::size_t>(size), 0),
                                      zeroStreamElements(config_.shapeKeyStride)};
                        if (!zeros.isValid()) {
                            throw std::runtime_error("its zero shape-key stream could not be built");
                        }

                        zeros.fix(path);
                    } catch (const std::exception& exception) {
                        throw std::runtime_error("cannot write " + zeroStreamFile() + ": " + exception.what());
                    }
                }

                // The mirrored twin's two buffers -- see WWMIFixerConfig::mirroredComponents. The
                // index one per component, holding that component's window wound the other way; the
                // vector one once, the mod's own normals negated. Both are byte transforms of files
                // the mod already has, so they go out as bytes rather than through BufFile's element
                // declarations -- and the write is READ BACK, which is the lesson the zero stream
                // above carries: a discarded write result is how a run reports editing files it
                // never wrote.
                void writeMirrorBuffers() {
                    const std::set<int> mirrored = mirroredSet();
                    if (mirrored.empty()) {
                        if (!config_.mirroredComponents.empty()) {
                            note("no component could carry a mirrored twin -- a mod needs one draw"
                                 " range per mirrored component and an [" + IndexBufferResource
                                 + "] and [" + VectorBufferResource + "] of its own");
                        }

                        return;
                    }

                    const std::string folder = ctx_.getIniFile()->getFolder();
                    writeMirrorVector(folder);
                    for (int component : mirrored) {
                        writeMirrorIndex(folder, component);
                    }
                }

                static ByteVec readWhole(const std::string& path, const std::string& what) {
                    std::ifstream in(FileService::strToPath(path), std::ios::binary);
                    if (!in) {
                        throw std::runtime_error("cannot read " + what + " at " + path);
                    }

                    return ByteVec((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
                }

                static void writeWhole(const std::string& path, const ByteVec& bytes, const std::string& what) {
                    FileService::makeFolderFor(path);
                    {
                        std::ofstream out(FileService::strToPath(path), std::ios::binary);
                        if (!out) {
                            throw std::runtime_error("cannot write " + what + " to " + path);
                        }

                        out.write(reinterpret_cast<const char*>(bytes.data()),
                                  static_cast<std::streamsize>(bytes.size()));
                    }

                    if (FileService::fileSize(path) != static_cast<std::uintmax_t>(bytes.size())) {
                        throw std::runtime_error("wrote " + what + " to " + path + " and it is not there");
                    }
                }

                // R8G8B8A8_SNORM, 8 bytes a vertex: the normal in 0..2 and the tangent in 4..7. -128
                // has no positive counterpart in SNORM, so it clamps rather than wrapping to itself.
                void writeMirrorVector(const std::string& folder) {
                    const std::string path = FileService::absPathOfRelPath(mirrorVectorFile(), folder);
                    ByteVec bytes = readWhole(FileService::absPathOfRelPath(vectorFile_, folder),
                                              "this mod's vector buffer");
                    for (std::size_t i = 0; i + 3 < bytes.size(); i += 8) {
                        for (std::size_t c = 0; c < 3; ++c) {
                            int v = static_cast<int>(bytes[i + c]);
                            v = (v > 127) ? v - 256 : v;
                            v = std::max(-127, std::min(127, -v));
                            bytes[i + c] = static_cast<std::uint8_t>((v < 0) ? v + 256 : v);
                        }
                    }

                    writeWhole(path, bytes, "the mirrored vector buffer");
                }

                // The span a component's draws cover, as (first index, index count). Several
                // ranges are one object split into pieces, so the twin buffer is their whole span
                // and each twin draw indexes into it at `its first - the span's first`.
                std::pair<long long, long long> mirrorSpan(int component) const {
                    const auto& ranges = drawRanges_.at(component);
                    long long low = ranges.front().second;
                    long long high = ranges.front().second + ranges.front().first;
                    for (const auto& range : ranges) {
                        low = std::min(low, range.second);
                        high = std::max(high, range.second + range.first);
                    }

                    return {low, high - low};
                }

                void writeMirrorIndex(const std::string& folder, int component) {
                    const auto span = mirrorSpan(component);
                    const long long first = span.first;
                    const long long count = span.second;

                    const ByteVec source = readWhole(FileService::absPathOfRelPath(indexFile_, folder),
                                                     "this mod's index buffer");
                    const std::size_t end = static_cast<std::size_t>(first + count) * 4;
                    if (end > source.size()) {
                        throw std::runtime_error("component " + std::to_string(component)
                                                 + " draws past the end of this mod's index buffer");
                    }

                    // Swapping two corners of each triangle reverses its winding, which is the whole
                    // of the twin: the SAME vertices, presented to the rasterizer the other way.
                    ByteVec out(static_cast<std::size_t>(count) * 4);
                    for (long long t = 0; t + 2 < count; t += 3) {
                        const std::size_t from = static_cast<std::size_t>(first + t) * 4;
                        const std::size_t to = static_cast<std::size_t>(t) * 4;
                        std::copy(source.begin() + from, source.begin() + from + 4, out.begin() + to);
                        std::copy(source.begin() + from + 8, source.begin() + from + 12, out.begin() + to + 4);
                        std::copy(source.begin() + from + 4, source.begin() + from + 8, out.begin() + to + 8);
                    }

                    writeWhole(FileService::absPathOfRelPath(mirrorIndexFile(component), folder), out,
                               "the mirrored index buffer");
                }

                // The edits the fix makes to a role's texture before binding it -- see
                // WWMIFixerConfig::texEdits. Queued as resources rather than run here, so a role
                // whose file comes from the DOWNLOAD is edited after the download lands: editing a
                // file that is not there yet writes nothing and says nothing.
                // WHAT each texture edit will be called, and which of the mod's files it reads --
                // decided here rather than in addTexEdits(), which runs at fix time, AFTER
                // buildEdits() has already turned every role into a register addition. Setting
                // resourceOfRole_ there updated a map nothing read again, so every edit was written
                // to disk and bound by no section, with the summary reporting it as a success.
                //
                // Same split as the fallback downloads: the name is planned at read time and the
                // resource registered at fix time, so a parse alone still writes nothing.
                void planTexEdits() {
                    if (config_.texEdits.empty() || gaveUp_) {
                        return;
                    }

                    const std::string folder = ctx_.getIniFile()->getFolder();
                    for (const WWMIFixerConfig::TexEdit& edit : config_.texEdits) {
                        if (!edit.makeFilter || sharedResourceFor(edit.role) == nullptr) {
                            continue;
                        }

                        // the mod's own file, or -- for a role it has none for -- the one the
                        // fallback download lands
                        std::string source;
                        const auto srcAt = fileOfRole_.find(edit.role);
                        if (srcAt != fileOfRole_.end()) {
                            source = srcAt->second;
                        } else {
                            const auto back = fallbacks_.find(edit.role);
                            if (back == fallbacks_.end()) {
                                continue;
                            }

                            source = FileService::absPathOfRelPath(back->second.relPath, folder);
                        }

                        const std::string fixedRel =
                            modFile(textureFolder_,
                                    config_.sourceTextures.downloadPrefix + TextTools::capitalize(edit.role)
                                    + edit.name + IniKeywords::RemapTex + FileExt::DDS);
                        plannedEdits_.push_back(PlannedEdit{&edit, source, fixedRel});

                        // every binding of the role follows the edited file
                        const std::string resource = fixName(IniKeywords::Resource + TextTools::capitalize(edit.role) + edit.name
                                                             + IniKeywords::RemapTex);

                        // Which of the mod's resources this replaces, so a copied toggle chain can
                        // swap it in.
                        const std::string* was = sharedResourceFor(edit.role);
                        if (was != nullptr) {
                            editedResourceOf_[StringTools::toLower(*was)] = resource;
                            editedRoleOf_[StringTools::toLower(*was)] = edit.role;
                            sourceOfEdited_[StringTools::toLower(resource)] = *was;

                            // EVERY variant of the role, not just the one fileOfRole_ resolved to.
                            // An edit reads one file, so a mod that binds its mask two ways -- Chisa12
                            // does, on $sockscolor -- had the second variant carried through
                            // unrepacked and shaded as the wrong material. Harmless while nothing
                            // could reach that variant; a live defect once the toggle chains could.
                            //
                            // The resolved variant keeps the name it already had, so no shipped
                            // output moves; the others are indexed after it.
                            const auto owner = conditionalOwner_.find(StringTools::toLower(*was));
                            if (owner != conditionalOwner_.end()) {
                                std::size_t n = 1;
                                for (const std::string& variant : variantsOf_[owner->second]) {
                                    if (StringTools::equalsIgnoreCase(variant, *was)) {
                                        continue;
                                    }

                                    const auto file = fileOfResource_.find(StringTools::toLower(variant));
                                    if (file == fileOfResource_.end()) {
                                        continue;
                                    }

                                    ++n;
                                    const std::string suffix = std::to_string(n);
                                    const std::string variantRel =
                                        modFile(textureFolder_,
                                                config_.sourceTextures.downloadPrefix
                                                + TextTools::capitalize(edit.role) + edit.name + suffix
                                                + IniKeywords::RemapTex + FileExt::DDS);
                                    const std::string variantResource =
                                        fixName(IniKeywords::Resource + TextTools::capitalize(edit.role) + edit.name + suffix
                                                + IniKeywords::RemapTex);
                                    plannedEdits_.push_back(PlannedEdit{&edit, file->second, variantRel});
                                    editedResources_.emplace_back(variantResource, variantRel);
                                    editedResourceOf_[StringTools::toLower(variant)] = variantResource;
                                    editedRoleOf_[StringTools::toLower(variant)] = edit.role;
                                    sourceOfEdited_[StringTools::toLower(variantResource)] = variant;
                                }

                                if (n > 1) {
                                    ctx_.log(edit.role + ": " + std::to_string(n) + " variants bound by "
                                             + owner->second + ", each given its own " + edit.name + " edit");
                                }
                            }
                        }
                        editedResources_.emplace_back(resource, fixedRel);
                        resourceOfRole_[edit.role] = resource;
                        for (auto& entry : resourceOfSlotRole_) {
                            if (entry.first.first == edit.role) {
                                entry.second = resource;
                            }
                        }
                    }
                }

                void addTexEdits() {
                    if (plannedEdits_.empty()) {
                        return;
                    }

                    IniFile* ini = ctx_.getIniFile();
                    const std::string folder = ini->getFolder();

                    WWMIFixerConfig::TexEditContext context;
                    context.iniFolder = folder;
                    context.indexFile = FileService::absPathOfRelPath(indexFile_, folder);
                    context.positionFile = positionBufPath();
                    context.texcoordFile = FileService::absPathOfRelPath(texcoordFile_, folder);
                    std::error_code err;

                    context.drawRanges = drawRanges_;
                    for (const auto& entry : fileOfRole_) {
                        context.fileOfRole[entry.first] = entry.second;
                    }

                    for (const auto& entry : fallbacks_) {
                        context.fileOfRole.emplace(
                            entry.first, FileService::absPathOfRelPath(entry.second.relPath, folder));
                    }

                    for (const PlannedEdit& planned : plannedEdits_) {
                        TexEditor::Filter filter = planned.edit->makeFilter(context);
                        if (!filter) {
                            continue;
                        }

                        const std::string fixedPath = FileService::absPathOfRelPath(planned.fixedRel, folder);
                        FileService::makeFolderFor(fixedPath);
                        ini->getResources().push_back(std::make_unique<RemapTexEditResource>(
                            folder, planned.source, fixedPath,
                            TexEditor({std::move(filter)}, planned.edit->compress)));
                    }
                }

                // Where the mod's texcoord buffer is and how wide one vertex of it is.
                struct TexcoordBuffer {
                    const IfTemplate<std::string, std::string>* resource = nullptr;
                    std::string rel;                    // as the .ini spells it
                    std::string path;                   // ...and on disk
                    std::size_t stride = 0;
                };

                // The mod's texcoord buffer, or nothing -- and when it is nothing, the log says
                // which of the four ways it was nothing.
                std::optional<TexcoordBuffer> findTexcoordBuffer() {
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        return std::nullopt;
                    }

                    // WITHOUT REGARD TO CASE: this mod spells it ResourceTexCoordBuffer, and asking
                    // for ResourceTexcoordBuffer finds nothing and says nothing.
                    TexcoordBuffer found;
                    for (const auto& entry : ini->getIfTemplates()) {
                        if (entry.second != nullptr
                            && StringTools::equalsIgnoreCase(entry.first, TexcoordBufferResource)) {
                            found.resource = entry.second.get();
                            break;
                        }
                    }

                    if (found.resource == nullptr) {
                        note("this mod declares no " + TexcoordBufferResource
                             + ", so its UVs are left alone");
                        return std::nullopt;
                    }

                    const std::optional<std::string> name =
                        ModBranches::firstVal(*found.resource, IniKeywords::Filename);
                    if (!name.has_value()) {
                        note(TexcoordBufferResource + " names no file, so this mod's UVs are left alone");
                        return std::nullopt;
                    }

                    found.rel = FileService::iniPathToRel(*name);
                    found.path = FileService::absPathOfRelPath(found.rel, ini->getFolder());

                    // A DECLARED STRIDE THAT DOES NOT PARSE IS NOT A REASON TO GUESS ONE. This
                    // fell back to 16, which mis-reads every vertex of a buffer that is not 16 -- the
                    // mod said a number and the fix ignored it.
                    const std::optional<std::string> strideVal =
                        ModBranches::firstVal(*found.resource, IniKeywords::Stride);
                    found.stride = DefaultTexcoordStride;
                    if (strideVal.has_value()) {
                        try {
                            found.stride = static_cast<std::size_t>(std::stoul(StringTools::strip(*strideVal).data()));
                        } catch (const std::exception&) {
                            note(TexcoordBufferResource + " declares a stride of `"
                                 + std::string(StringTools::strip(*strideVal))
                                 + "`, which is not a number, so this mod's UVs are left alone");
                            return std::nullopt;
                        }
                    }

                    if (found.stride < 4 || found.stride % 2 != 0) {
                        note(TexcoordBufferResource + " declares a stride of "
                             + std::to_string(found.stride)
                             + ", which cannot hold a UV pair of halves, so this mod's UVs are left alone");
                        return std::nullopt;
                    }

                    return found;
                }

                // One element of `stride / 2` halves, rounded the way a folded UV needs. That mode
                // is the whole reason this can be a BufFile at all -- see the note where this file's
                // own half codec used to be.
                static std::vector<std::unique_ptr<BufElementType>> texcoordElements(std::size_t stride) {
                    std::vector<std::unique_ptr<BufElementType>> elements;
                    elements.push_back(BufElementType::repeated(
                        TexcoordElement, stride / 2,
                        [] { return std::make_unique<BufFloat16>(false, BufFloat16::Rounding::NearestEven); }));

                    return elements;
                }

                // Every vertex's U, with a NaN read as the 0 the clean pass will make it -- the
                // straddle test has to see the same values the write does.
                static std::vector<float> readTexcoordU(BufFile& texcoord) {
                    std::vector<float> u;
                    for (const double value : bufColumn(texcoord, TexcoordElement, 0)) {
                        u.push_back(std::isnan(value) ? 0.0f : static_cast<float>(value));
                    }

                    return u;
                }

                // The vertices of any triangle whose corners sit in DIFFERENT U tiles, which the
                // fold must leave alone.
                //
                // Folding one corner of a straddling triangle would widen its U span from a few
                // hundredths to nearly 1 and interpolate it backwards across the atlas, so such a
                // vertex is left alone -- which also protects deliberate TILING.
                //
                // Still read at 4 bytes per index, which is what the hand-rolled loop assumed;
                // `IbFile::bytesPerIndexOf(<the declared format>)` is how a mod declaring
                // `DXGI_FORMAT_R16_UINT` gets answered, and that is a BEHAVIOUR change rather than a
                // refactor. A failure here leaves every entry false, which folds more rather than
                // less -- the same answer the unopenable-file branch gave before.
                std::vector<bool> straddlingVertices(const std::vector<float>& u) const {
                    std::vector<bool> keep(u.size(), false);
                    if (indexFile_.empty()) {
                        return keep;
                    }

                    IniFile* ini = const_cast<IniFileFixContext&>(ctx_).getIniFile();
                    if (ini == nullptr) {
                        return keep;
                    }

                    const std::size_t vertices = u.size();
                    const std::string indexPath = FileService::absPathOfRelPath(
                        FileService::iniPathToRel(indexFile_), ini->getFolder());

                    try {
                        IbFile indices{indexPath};
                        const BufFile::Filter straddles =
                            [&keep, &u, vertices](const BufLineData& line, long long, double, long long) {
                                const auto at = line.find(IbFile::TriangleBufElementKey);
                                if (at == line.end() || at->second.size() < IbFile::VerticesPerTriangle) {
                                    return line;
                                }

                                std::size_t tri[3] = {0, 0, 0};
                                for (std::size_t i = 0; i < IbFile::VerticesPerTriangle; ++i) {
                                    tri[i] = static_cast<std::size_t>(bufValueAsInt(at->second[i]));
                                    if (tri[i] >= vertices) {
                                        return line;
                                    }
                                }

                                const float a = std::floor(u[tri[0]]);
                                const float b = std::floor(u[tri[1]]);
                                const float c = std::floor(u[tri[2]]);
                                if (a != b || b != c) {
                                    keep[tri[0]] = true;
                                    keep[tri[1]] = true;
                                    keep[tri[2]] = true;
                                }

                                return line;
                            };

                        indices.fix(std::nullopt, {straddles});
                    } catch (const std::exception&) {
                        // no index buffer to read: every vertex stays foldable, as before
                    }

                    return keep;
                }

                // How many NaNs a clean pass cleared and how many UVs it folded.
                struct TexcoordCleanTally {
                    std::size_t cleared = 0;
                    std::size_t folded = 0;
                };

                // A NaN in any half set to 0, and the fold in U. A NaN is `std::isnan` on the
                // decoded value rather than an exponent and mantissa test on the raw bits.
                //
                // Returned rather than applied so the caller can run it TWICE -- once to count and
                // once to write -- which is how the counts it reports describe the file it wrote.
                static BufFile::Filter texcoordCleaner(TexcoordCleanTally& tally,
                                                        const std::vector<bool>& keep) {
                    const std::size_t vertices = keep.size();
                    return [&tally, &keep, vertices](const BufLineData& line, long long,
                                                      double index, long long) {
                        BufLineData out = line;
                        const auto at = out.find(TexcoordElement);
                        if (at == out.end() || at->second.empty()) {
                            return out;
                        }

                        for (BufValue& value : at->second) {
                            if (std::holds_alternative<double>(value)
                                    && std::isnan(std::get<double>(value))) {
                                value = 0.0;
                                ++tally.cleared;
                            }
                        }

                        if (!std::holds_alternative<double>(at->second.front())) {
                            return out;
                        }

                        const auto vertex = static_cast<std::size_t>(index);
                        const auto first = static_cast<float>(std::get<double>(at->second.front()));
                        if (first >= 1.0f && vertex < vertices && !keep[vertex]) {
                            at->second.front() = static_cast<double>(std::fmod(first, 1.0f));
                            ++tally.folded;
                        }

                        return out;
                    };
                }

                // A remap-only copy of the mod's texcoord buffer -- see
                // WWMIFixerConfig::cleanTexcoords for what is wrong with the original and why
                // neither fault is the mod's bug.
                void buildTexcoordCopy() {
                    if (!config_.cleanTexcoords || texcoordResource_.has_value()) {
                        return;
                    }

                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        return;
                    }

                    const std::optional<TexcoordBuffer> source = findTexcoordBuffer();
                    if (!source.has_value()) {
                        return;                         // findTexcoordBuffer said which way
                    }

                    const IfTemplate<std::string, std::string>* resource = source->resource;
                    const std::size_t stride = source->stride;

                    BufFile texcoord{source->path, texcoordElements(stride)};
                    if (!texcoord.isValid()) {
                        return;                         // not a whole number of vertices: leave it alone
                    }

                    std::vector<float> u;
                    try {
                        u = readTexcoordU(texcoord);
                    } catch (const std::exception& exception) {
                        note(std::string("its Texcoord.buf could not be read: ") + exception.what()
                             + " -- so this mod's UVs are left alone");
                        return;
                    }

                    const std::size_t vertices = u.size();
                    if (vertices == 0) {
                        return;
                    }

                    // U at or above 1, except on a triangle whose vertices straddle a tile.
                    //
                    // NOT U BELOW 0 (2026-09-27). The fold exists for a mod that UVs half a part
                    // into the [1, 2) TILE and relies on the sampler wrapping -- a large, coherent,
                    // deliberate region (47% of one Chisa mod's component 3). A U just BELOW zero is
                    // the opposite thing: the edge bleed an authoring tool leaves around a UV
                    // island, a fringe a few hundredths wide that a clamping sampler is meant to
                    // extend. Folding it sends those texels to the FAR SIDE of the atlas --
                    // `-0.054` became `0.945` -- so the fringe of every island sampled unrelated
                    // art. On Chisa13's black hair dye that drew a regular checkerboard of blonde
                    // blocks through the lock: 905 such vertices on the bangs and 2 elsewhere in
                    // the mesh, which is why one part of one mod showed it.
                    //
                    // The fold is wrap-equivalent per VERTEX, which is what was measured when it
                    // went in, and that holds only while the pass WRAPS. Neither character's own
                    // model leaves [0, 1), so the game never exercises its own address mode there
                    // and the two passes are free to differ -- and this one demonstrably does not
                    // wrap: the mod's own UVs and a fold restricted to `>= 1` both render the dye
                    // as one solid sweep, and the full fold does not. Leaving the fringe alone is
                    // also the pre-fold behaviour for it, so it cannot regress a mod that was
                    // right before the fold existed.
                    //
                    // Folding one corner of a straddling triangle would widen its U span from a few
                    // hundredths to nearly 1 and interpolate it backwards across the atlas, so such
                    // a vertex is left alone -- which also protects deliberate TILING.
                    //
                    // Still read at 4 bytes per index, which is what the hand-rolled loop assumed;
                    // `IbFile::bytesPerIndexOf(<the declared format>)` is how a mod declaring
                    // `DXGI_FORMAT_R16_UINT` gets answered, and that is a BEHAVIOUR change rather
                    // than a refactor. A failure here leaves `keep` false, which folds more rather
                    // than less -- the same answer the unopenable-file branch gave before.
                    const std::vector<bool> keep = straddlingVertices(u);

                    TexcoordCleanTally tally;
                    const BufFile::Filter clean = texcoordCleaner(tally, keep);

                    // Counted before anything is written, because the copy exists only when it
                    // differs -- a mod with clean texcoords keeps its own buffer and its own binding.
                    try {
                        texcoord.fix(std::nullopt, {clean});
                    } catch (const std::exception& exception) {
                        note(std::string("its Texcoord.buf could not be read: ") + exception.what()
                             + " -- so this mod's UVs are left alone");
                        return;
                    }

                    const std::size_t clearedCount = tally.cleared;
                    const std::size_t foldedCount = tally.folded;
                    if (clearedCount == 0 && foldedCount == 0) {
                        return;
                    }

                    const std::string fixedRel =
                        modFile(meshFolder_, toModName_ + IniKeywords::Remap + "Texcoord.buf");
                    const std::string fixedPath = FileService::absPathOfRelPath(fixedRel, ini->getFolder());
                    FileService::makeFolderFor(fixedPath);

                    // The same filter again, writing this time -- so the counters run up a second
                    // time and the snapshot above is what the log reports.
                    //
                    // `fix(path, ...)` RETURNS THE PATH ON SUCCESS: the string alternative of
                    // FixResult means "written to this file", not "failed", and a failure throws.
                    // Reading it as an error is what made an earlier version of this write the
                    // buffer correctly and then never bind it.
                    try {
                        texcoord.fix(fixedPath, {clean});
                    } catch (const std::exception& exception) {
                        note(std::string("its cleaned Texcoord.buf could not be measured: ") + exception.what()
                             + " -- so this mod's UVs are left alone");
                        return;
                    }

                    const std::optional<std::string> format = ModBranches::firstVal(*resource, IniKeywords::Format);
                    texcoordResource_ = fixName(IniKeywords::Resource + "TexcoordNoNaN");
                    texcoordSection_ = SectionText(z3_, *texcoordResource_)
                                           .keys({{IniKeywords::Type, "Buffer"},
                                                  {IniKeywords::Format, std::string(format.has_value()
                                                                             ? StringTools::strip(*format)
                                                                             : std::string_view("DXGI_FORMAT_R16G16_FLOAT"))},
                                                  {IniKeywords::Stride, std::to_string(stride)},
                                                  {IniKeywords::Filename, fixedRel}})
                                           .str();
                    ctx_.log("texcoords: " + std::to_string(clearedCount) + " NaN halves set to 0 and "
                             + std::to_string(foldedCount) + " U values folded into [0, 1) in a remap-only copy");
                }

                /**
                 * @brief
                 @rst
                 Registers the download of the SOURCE's own game texture for one role, and points
                 every component with no file of its own at it
                 @endrst
                 *
                 * @param role The role with no file of the mod behind it
                 * @param hash The source's texture hash of that role
                 */
                void registerFallback(const std::string& role, const std::string& hash) {
                    const WWMITextureFacts& source = config_.sourceTextures;
                    const std::string kind = TextTools::capitalize(role);
                    const std::string fileName = DownloadTools::fixedFileName(source.downloadPrefix, kind, FileExt::DDS);
                    const std::string resource = IniKeywords::Resource + source.downloadPrefix + kind + IniKeywords::RemapDL;
                    fallbacks_[role] = Fallback{
                        DownloadTools::downloadFolder() + "/"
                            + DownloadTools::urlPath(source.downloadGameFolder, source.downloadCharFolder,
                                                     source.downloadVersionFolder, source.downloadPrefix,
                                                     "Texture" + hash, FileExt::DDS),
                        fileName, modFile(textureFolder_, fileName), resource};
                    resourceOfRole_[role] = resource;
                }

                void addCreatedTextures() {
                    IniFile* ini = ctx_.getIniFile();
                    for (const WWMIFixerConfig::CreatedTexture& created : config_.createdTextures) {
                        const std::string path = FileService::absPathOfRelPath(createdTextureFile(created), ini->getFolder());
                        if (listsSrcPath(ini->getResources(), path)) {
                            continue;
                        }

                        FileService::makeFolderFor(path);
                        ini->getResources().push_back(std::make_unique<RemapTexAddResource>(
                            ini->getFolder(), path, TexCreator(created.size, created.size, created.colour, false, false)));
                    }
                }

                // The source's game texture bound for a planned role the mod has no file for:
                // the download's url, the name it is saved under, and its path relative to the .ini
                struct Fallback {
                    std::string url;
                    std::string fileName;
                    std::string relPath;
                    /**
                     * @brief
                     @rst
                     The section name this download DECLARES itself under.

                     Not ``resourceOfRole_[role]``: a texture edit of the same role overwrites that
                     with its OWN resource, on purpose, so every BINDING follows the edited file.
                     Reading it back here wrote the download's section under the edit's name, so the
                     name was declared twice -- once naming the raw download and once the edited
                     file -- and the raw one won. 21 such pairs across one corpus (2026-09-27).
                     @endrst
                     */
                    std::string resource;
                };

                void addFallbackDownloads() {
                    IniFile* ini = ctx_.getIniFile();
                    for (const auto& entry : fallbacks_) {
                        const std::string path = FileService::absPathOfRelPath(entry.second.relPath, ini->getFolder());
                        if (listsSrcPath(ini->getFileDownloads(), path)) {
                            continue;
                        }

                        FileService::makeFolderFor(path);
                        ini->getFileDownloads().push_back(std::make_unique<RemapIniDownload>(
                            ini->getFolder(), entry.second.relPath,
                            std::make_unique<FileDownload>(entry.second.url, entry.second.fileName)));
                    }
                }

                IniFileFixContext ctx_;
                std::string toModName_;
                WWMIFixerConfig config_;
                bool gaveUp_ = false;
                GraphGroupRemove<> removeEveryGroup_;

                std::optional<ModType> targetType_;
                Character source_;
                Character target_;
                std::map<int, std::vector<std::string>> present_;     // source component -> its sections
                std::vector<int> dropped_;
                std::vector<std::vector<int>> groups_;
                std::vector<ModObj> hashOnlyObjs_;                    // ...and the mod's objects that are not slots
                std::unordered_set<std::string> hiddenObjs_;          // of those, the ones commented out rather than copied
                std::set<int> drawnSlots_;
                long long meshVertexCount_ = 0;
                std::string meshFolder_;
                std::string textureFolder_;
                std::map<std::string, std::string> resourceOfRole_;   // role -> resource, for what every component shares (a created texture, a download)
                std::map<std::pair<std::string, int>, std::string> resourceOfSlotRole_;   // (role, source component) -> the resource that component binds
                std::vector<std::string> remapNames_;   // the mod names a fix of this .ini could have named its sections after
                std::vector<std::pair<std::string, std::string>> declared_;   // (file, path relative to the .ini) for a file no resource of the .ini names
                std::map<std::string, std::string> declaredName_;      // that file -> the resource section the fix declares for it
                std::set<std::string> usedDeclaredNames_;
                std::map<std::string, Fallback> fallbacks_;           // role -> the source's game texture, for a planned role the mod has no file for
                std::set<std::string> leftToGame_;                    // roles whose only file was flat and whose config says not to stand anything in
                std::vector<std::string> textureLists_;
                std::unordered_map<std::string, std::string> passFilters_;



                bool legacy_ = false;                                 // a mod from before WWMI's merged skeleton
                // EMPTY UNTIL THE MOD'S OWN `.ini` SAYS OTHERWISE (2026-09-30). Each of these used to
                // start at `Meshes/<Name>.buf`, so a mod declaring no such section was read at a path
                // the fix invented -- the same assumption about a mod's folder structure that
                // `DefaultTextureFolder` / `DefaultMeshFolder` made on the write side. A mod-manager
                // -packaged mod names every file by GUID with a `.assets` extension and has no
                // `Meshes/` at all.
                std::string indexFile_;                               // the mod's own [ResourceIndexBuffer]
                std::string positionFile_;                            // ...and [ResourcePositionBuffer]
                std::string blendSourceFile_;                         // ...and [ResourceBlendBuffer]; blendFixedFile() derives the fix's name FROM it
                std::string texcoordFile_;                            // ...and [ResourceTexcoordBuffer]
                std::string vectorFile_;                              // ...and [ResourceVectorBuffer]; the normals the mirrored twin negates
                std::map<int, std::vector<std::pair<long long, long long>>> drawRanges_;   // source component -> its (index count, first index) draws
                std::map<int, std::set<std::size_t>> drawParts_;      // ...and which PARTS those draws sit in -- see mirroredSet()

                // Whether the TARGET's merged skeleton passes what an 8-bit blend index can name.
                // Read off the vertex group row's largest target id, which is a property of the
                // PAIR -- so the .ini and the buffers, written at different times, cannot disagree
                // about whether there is a blend remap.
                bool targetPast256_ = false;

                // How many bones the one remap holds -- the DISTINCT targets the vertex group row
                // names, so the .ini (written first) and the buffers agree without either having to
                // read the other. 182 for ChisaParfait -> Chisa.
                std::size_t blendRemapBones_ = 0;

                // The mod's own blend line: N ids + N weights, and its byte stride
                std::size_t blendInfluences_ = 4;
                std::size_t blendStride_ = 8;
                std::vector<std::string> passOrder_;

                std::unique_ptr<GraphGroupRemap<>> slotRemap_;
                std::unique_ptr<RegAssetRemap<>> assetRemap_;
                std::unique_ptr<RegPartEdit<>> assetAdapter_;
                std::unique_ptr<RegRemove<>> regRemove_;
                std::unique_ptr<RegPartEdit<>> removeAdapter_;
                std::map<int, PartEdit*> newValsOf_;
                std::vector<std::unique_ptr<RegSurroundedAdd<>>> surroundedAdds_;
                std::vector<std::unique_ptr<GraphPartEdit<>>> graphAdapters_;
                std::vector<std::unique_ptr<RegNewVals<>>> newVals_;
                std::vector<std::unique_ptr<RegPartEdit<>>> regAdapters_;
                std::vector<std::unique_ptr<MirrorTwin>> twinEdits_;   // owned beside the adapters that wrap them
                std::vector<std::unique_ptr<CarriedTexRegs>> carriedEdits_;   // ...and the same for the carried-register edits
                std::map<int, std::vector<PartEdit*>> editsOf_;
                std::unique_ptr<ObjGroupEdit> mainEdits_;
                std::vector<std::unique_ptr<WWMIBlendReplace>> blendReplaces_;
                std::optional<std::string> vertexVGFile_;                            // WWMI's 16-bit ids, as the .ini names them
                std::optional<std::string> texcoordResource_;                        // the cleaned texcoord copy's resource
                std::optional<std::string> texcoordSection_;                         // ...and its section text
                std::unordered_map<std::string, std::string> fileOfRole_;            // role -> the file it resolved to
                // One texture edit, named and sourced at read time and registered at fix time
                struct PlannedEdit {
                    const WWMIFixerConfig::TexEdit* edit = nullptr;   // owned by config_, so stable
                    std::string source;                               // absolute, the mod's file or the fallback's
                    std::string fixedRel;                             // relative to the .ini
                };

                std::unordered_map<std::string, std::string> conditionalOwner_;      // the mod's resource -> the section that binds it behind a condition
                std::unordered_map<std::string, std::vector<std::string>> variantsOf_;  // that section -> every resource it binds, in order
                std::unordered_map<std::string, std::string> fileOfResource_;         // the mod's resource -> the file it names
                std::unordered_map<std::string, std::string> editedResourceOf_;       // the mod's resource -> the edited copy of it
                std::unordered_map<std::string, std::string> editedRoleOf_;           // ...and the ROLE that edit was registered for, since one file may serve several
                std::unordered_map<std::string, std::string> sourceOfEdited_;         // ...and back
                std::map<std::pair<std::string, std::string>, std::string> roleLists_;   // (role, register) -> the copied list's name
                std::vector<std::string> roleListTexts_;                              // ...and their section text
                std::vector<PlannedEdit> plannedEdits_;
                std::vector<std::pair<std::string, std::string>> editedResources_;   // (resource, path relative to the .ini)
                std::vector<std::unique_ptr<Collector>> blendCollects_;
        };
    }


    IniFixBuilder::Factory makeWWMIFixer(WWMIFixerConfig config) {
        return [config](BaseIniParser<>* parser, const std::string& toModName, std::optional<int> modTypeId) {
            return std::make_shared<WWMIFixerImpl>(parser, toModName, modTypeId, config);
        };
    }
}
