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

#include "AGRemapCore/tools/files/FileService.h"
#include "AGRemapCore/model/IniNamingTools.h"

#include <filesystem>
#include <optional>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/constants/FilePrefixes.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/TextTools.h"
#include "AGRemapCore/tools/grapheme/GraphemeRange.h"


namespace AGRemapCore {

    namespace {
        namespace fs = std::filesystem;

        // Case-insensitive search for the LAST occurrence of 'needle' in 'haystack' -- mirrors what
        // the pure-Python original's getObjRemapFixName achieves indirectly (reverse both strings,
        // then re.split(..., maxsplit = 1) from the front is equivalent to finding the last match
        // from the end of the un-reversed string; verified empirically against the Python original
        // before simplifying it down to this direct form).
        //
        // Compared grapheme by grapheme, each lowered through StringTools, so a non-ASCII mod
        // object name (or one with combining marks / emojis) matches as one would expect. Returns
        // the byte offset and byte length of the match within the *original* 'haystack', since
        // lowering can change a character's byte length and the caller splices the original.
        std::optional<std::pair<size_t, size_t>> rfindCaseInsensitive(std::string_view haystack, std::string_view needle) {
            if (needle.empty()) {
                return std::nullopt;
            }

            std::vector<std::string> needleGraphemes;
            for (std::string_view grapheme : GraphemeRange(needle)) {
                needleGraphemes.push_back(StringTools::toLower(grapheme));
            }

            std::vector<std::string> haystackGraphemes;
            std::vector<size_t> graphemeStarts;
            size_t pos = 0;
            for (std::string_view grapheme : GraphemeRange(haystack)) {
                haystackGraphemes.push_back(StringTools::toLower(grapheme));
                graphemeStarts.push_back(pos);
                pos += grapheme.size();
            }
            // One past the last grapheme, so a match ending on it has an end offset too.
            graphemeStarts.push_back(pos);

            if (needleGraphemes.size() > haystackGraphemes.size()) {
                return std::nullopt;
            }

            for (size_t i = haystackGraphemes.size() - needleGraphemes.size() + 1; i-- > 0; ) {
                bool match = true;
                for (size_t j = 0; j < needleGraphemes.size(); ++j) {
                    if (haystackGraphemes[i + j] != needleGraphemes[j]) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    size_t start = graphemeStarts[i];
                    size_t end = graphemeStarts[i + needleGraphemes.size()];
                    return std::make_pair(start, end - start);
                }
            }

            return std::nullopt;
        }

        // pathlib.Path(file).parent always returns Path(".") (never an empty path) for a bare
        // filename with no directory component -- unlike std::filesystem::path::parent_path(),
        // which returns an empty path for the same input. getFixedFile/getFixedElementFile below
        // rely on this "." fallback (the pure-Python original's own getFixedElementFile even checks
        // "folder == pathlib.Path('.')" explicitly). getFixedTexFile, built on
        // os.path.dirname/os.path.join instead, deliberately does NOT get this treatment --
        // os.path.dirname returns "" for a bare filename, and os.path.join("", x) == x with no
        // "./" prefix at all -- see its own comment.
        fs::path pathlibStyleParent(const fs::path& path) {
            fs::path parent = path.parent_path();
            return parent.empty() ? fs::path(".") : parent;
        }
    }

    std::size_t IniNamingTools::variantSuffixStart(const std::string& name) {
        // A trailing '.' followed by digits and nothing else -- the shape a merged mod gives its
        // per-branch resources. Anything else (a '.dds' on the end, a bare '.', a '.' with
        // letters after it) is part of the name proper and is left where it is.
        const std::size_t dot = name.rfind('.');
        if (dot == std::string::npos || dot + 1 >= name.size()) {
            return std::string::npos;
        }

        for (std::size_t i = dot + 1; i < name.size(); ++i) {
            if (name[i] < '0' || name[i] > '9') {
                return std::string::npos;
            }
        }

        return dot;
    }

    bool IniNamingTools::isDisabled(const std::string& name) {
        return StringTools::startsWith(StringTools::toLower(name), FilePrefixes::DisabledPrefix);
    }


    std::string IniNamingTools::getRegTag(const std::string& reg) {
        std::string out;
        for (char c : reg) {
            if (c != '-') {
                out += c;
            }
        }

        return TextTools::capitalize(out);
    }


    std::string IniNamingTools::getResourceName(const std::string& name) {
        if (!name.starts_with(IniKeywords::Resource)) {
            return IniKeywords::Resource + name;
        }
        return name;
    }

    std::string IniNamingTools::removeResourceName(const std::string& name) {
        if (name.starts_with(IniKeywords::Resource)) {
            return name.substr(IniKeywords::Resource.size());
        }
        return name;
    }

    std::string IniNamingTools::removeRefPrefix(const std::string& value) {
        const std::string_view stripped = StringTools::strip(value);

        // The prefix is a WORD, so `reference` must not match it -- the space is part of the test
        // rather than part of the constant, and `ref` alone (a binding with no name after it) has
        // nothing to strip down to.
        const std::string prefix = IniKeywords::Ref + " ";
        if (!StringTools::startsWith(StringTools::toLower(std::string(stripped)), prefix)) {
            return std::string(stripped);
        }

        return std::string(StringTools::strip(stripped.substr(prefix.size())));
    }

    bool IniNamingTools::hasRefPrefix(const std::string& value) {
        return StringTools::startsWith(
            StringTools::toLower(std::string(StringTools::lstrip(value))), IniKeywords::Ref + " ");
    }

    std::string IniNamingTools::getRemapElementName(const std::string& name, const std::string& elementName, const std::string& modName) {
        std::string remapName = modName + IniKeywords::Remap + elementName;

        if (elementName.empty()) {
            return name + remapName;
        }

        // Replaces the LAST occurrence of 'elementName' with 'remapName' -- equivalent to the
        // Python original's "name.rsplit(elementName, 1)" (split on the last occurrence) followed
        // by "remapName.join(nameParts)".
        size_t pos = name.rfind(elementName);
        if (pos == std::string::npos) {
            return name + remapName;
        }

        return name.substr(0, pos) + remapName + name.substr(pos + elementName.size());
    }

    std::string IniNamingTools::getRemapBlendName(const std::string& name, const std::string& modName) {
        return getRemapElementName(name, IniKeywords::Blend, modName);
    }

    std::string IniNamingTools::getRemapPositionName(const std::string& name, const std::string& modName) {
        return getRemapElementName(name, IniKeywords::Position, modName);
    }

    std::string IniNamingTools::getRemapTexcoordName(const std::string& name, const std::string& modName) {
        return getRemapElementName(name, IniKeywords::Texcoord, modName);
    }

    std::string IniNamingTools::getRemapIbName(const std::string& name, const std::string& modName) {
        // Literal "IB", matching the Python original exactly -- NOT IniKeywords::Ib (which would be
        // lowercase "ib"; that member isn't ported here since nothing else in this class uses it).
        return getRemapElementName(name, "IB", modName);
    }

    std::string IniNamingTools::getModSuffixedName(const std::string& name, const std::string& suffix, const std::string& modName) {
        std::string remapName = modName + suffix;

        if (name.ends_with(remapName)) {
            return name;
        }

        if (name.ends_with(suffix)) {
            // The pure-Python original has a confirmed bug here: it returns
            // "name[:len(suffix)] + remapName" (the first len(suffix) characters of 'name') instead
            // of "name[:-len(suffix)] + remapName" (name with the trailing 'suffix' stripped off) --
            // e.g. getRemapFixName("EiIsDoneWithRemapFix", "Raiden") returns
            // "EiIsDoneRaidenRemapFix" in Python, contradicting that very method's own docstring
            // example of "EiIsDoneWithRaidenRemapFix". Confirmed by running the Python original
            // directly. Per the maintainer (asked explicitly), this C++ port implements the
            // documented/intended behavior below, not the buggy one.
            return name.substr(0, name.size() - suffix.size()) + remapName;
        }

        return name + remapName;
    }

    std::string IniNamingTools::getRemapFixName(const std::string& name, const std::string& modName) {
        return getModSuffixedName(name, IniKeywords::RemapFix, modName);
    }

    std::string IniNamingTools::getRemapTexName(const std::string& name, const std::string& modName) {
        // THE VARIANT SUFFIX GOES LAST, not in the middle.
        //
        // A merged mod numbers its resources ResourceXHeadDiffuse.0, .1, .2 -- one per branch of
        // its $swapvar -- and a plain append buries that number in the middle of the fixed name:
        //
        //   ResourceHuTaoFaceHeadDiffuse.1CherryHuTaoTransparentFaceDiffuseRemapTex
        //
        // which is not wrong, only unreadable. The blend and position names never had the problem
        // because getRemapElementName splices at the element word and leaves whatever follows it
        // alone (ResourceKeqingKeqingOpulentRemapBlend.0), so this brings the texture names into
        // line with them:
        //
        //   ResourceHuTaoFaceHeadDiffuseCherryHuTaoTransparentFaceDiffuseRemapTex.1
        //
        // The suffix still has to be THERE -- it is the only thing telling one branch's edited
        // texture from another's.
        const std::size_t suffixAt = variantSuffixStart(name);
        if (suffixAt == std::string::npos) {
            return getModSuffixedName(name, IniKeywords::RemapTex, modName);
        }

        return getModSuffixedName(name.substr(0, suffixAt), IniKeywords::RemapTex, modName)
                + name.substr(suffixAt);
    }

    std::string IniNamingTools::getRemapDLName(const std::string& name, const std::string& modName) {
        return getModSuffixedName(name, IniKeywords::RemapDL, modName);
    }

    std::string IniNamingTools::getRemapFixResourceName(const std::string& name, const std::string& modName) {
        return getResourceName(getRemapFixName(name, modName));
    }

    std::string IniNamingTools::getRemapTexResourceName(const std::string& name, const std::string& modName) {
        return getResourceName(getRemapTexName(name, modName));
    }

    std::string IniNamingTools::getRemapDLResourceName(const std::string& name, const std::string& modName) {
        return getResourceName(getRemapDLName(name, modName));
    }

    std::string IniNamingTools::getRemapBlendResourceName(const std::string& name, const std::string& modName) {
        return getResourceName(getRemapBlendName(name, modName));
    }

    std::string IniNamingTools::getRemapPositionResourceName(const std::string& name, const std::string& modName) {
        return getResourceName(getRemapPositionName(name, modName));
    }

    // FileService::strToPath on the way in AND on the way out of every one of these, never
    // `fs::path(str)` or `folder / str`: on Windows those read a narrow string as the ACTIVE CODE
    // PAGE, so a UTF-8 name goes in and a DIFFERENT name comes out -- see strToPath's own danger
    // note, and the ten sites the 2026-09-11 sweep found. These three were not among them.
    //
    // What it cost: a mod whose .ini has a non-Latin FILENAME -- `mod-自动生成.ini`, on a real
    // ChisaParfait mod -- had every generated file named from the mangled round trip, so the fix
    // wrote `mod-è‡ªåŠ¨ç”ŸæˆRemapFix1.ini` beside it. The undo then could not match that back to the
    // .ini it belongs to and left it on disk, still carrying remapped sections, so the mod went on
    // drawing on the target with no fix installed. A non-Latin FOLDER name was already covered and
    // works; the filename was the untested half.
    std::string IniNamingTools::getFixedFile(const std::string& file, const std::string& modName, std::optional<std::string> fileExt) {
        fs::path path = FileService::strToPath(file);
        fs::path folder = pathlibStyleParent(path);
        std::string baseName = FileService::pathToStr(path.stem());
        std::string ext = fileExt.has_value() ? *fileExt : FileService::pathToStr(path.extension());

        std::string newName = getRemapFixName(baseName, modName) + ext;
        return FileService::pathToIniStr((folder / FileService::strToPath(newName)));
    }

    // strToPath on the way in and out -- see getFixedFile's note.
    std::string IniNamingTools::getFixedElementFile(const std::string& file, const std::string& elementName, const std::string& modName, std::optional<std::string> fileExt) {
        fs::path path = FileService::strToPath(file);
        fs::path folder = pathlibStyleParent(path);
        std::string baseName = FileService::pathToStr(path.stem());
        std::string ext = fileExt.has_value() ? *fileExt : FileService::pathToStr(path.extension());

        std::string newName = getRemapElementName(baseName, elementName, modName) + ext;
        if (folder == fs::path(".")) {
            return newName;
        }

        return FileService::pathToIniStr((folder / FileService::strToPath(newName)));
    }

    std::string IniNamingTools::getFixedBlendFile(const std::string& blendFile, const std::string& modName) {
        return getFixedElementFile(blendFile, IniKeywords::Blend, modName, FileExt::Buf);
    }

    std::string IniNamingTools::getFixedPositionFile(const std::string& positionFile, const std::string& modName) {
        return getFixedElementFile(positionFile, IniKeywords::Position, modName, FileExt::Buf);
    }

    // strToPath on the way in and out -- see getFixedFile's note.
    std::string IniNamingTools::getFixedTexFile(const std::string& texFile, const std::string& modName) {
        fs::path path = FileService::strToPath(texFile);
        fs::path folder = path.parent_path();  // no "." fallback here -- see pathlibStyleParent's comment
        std::string baseName = FileService::pathToStr(path.filename());

        // Matches Python's "basename.rsplit('.', 1)[0]" -- strip only the LAST "." extension,
        // keeping everything before it (including any earlier dots). NOT the same as
        // std::filesystem::path::stem(), which parses extensions with filesystem-style rules (eg. a
        // leading-dot-only "dotfile" like ".gitignore" has no extension to std::filesystem, but
        // rsplit('.', 1) still splits it into "" + "gitignore"). Kept as a manual rsplit to match
        // the Python original exactly rather than std::filesystem::path::stem()'s edge-case rules.
        size_t dotPos = baseName.rfind('.');
        if (dotPos != std::string::npos) {
            baseName = baseName.substr(0, dotPos);
        }

        std::string newName = getRemapTexName(baseName, modName) + FileExt::DDS;
        return FileService::pathToStr((folder / FileService::strToPath(newName)));
    }

    std::string IniNamingTools::getTextureOverrideRemapFix(const std::string& component, const std::string& obj, const std::string& modName) {
        return getRemapFixName(IniKeywords::TextureOverride + TextTools::capitalize(modName) + TextTools::capitalize(component) + TextTools::capitalize(obj));
    }

    std::string IniNamingTools::getObjRemapFixName(const std::string& name, const std::string& modName,
                                                    const std::pair<std::string, std::string>& objName,
                                                    const std::pair<std::string, std::string>& newObjName) {
        std::string objNameStr = TextTools::capitalize(objName.first) + TextTools::capitalize(objName.second);
        std::string newObjNameStr = TextTools::capitalize(newObjName.first) + TextTools::capitalize(newObjName.second);
        std::string capModName = TextTools::capitalize(modName);

        std::optional<std::pair<size_t, size_t>> match = rfindCaseInsensitive(name, objNameStr);
        if (!match.has_value()) {
            return getRemapFixName(name, capModName + newObjNameStr);
        }

        auto [matchStart, matchLen] = *match;
        std::string newName = name.substr(0, matchStart) + newObjNameStr + name.substr(matchStart + matchLen);
        return getRemapFixName(newName, capModName);
    }

    bool IniNamingTools::looksRemapped(const std::string& sectionName, const std::vector<std::string>& modNames,
                                       const std::string& remapKeyword) {
        if (remapKeyword.empty() || sectionName.find(remapKeyword) == std::string::npos) {
            return false;
        }

        // With no names to go on -- a hand-built caller, or a context that does not know its mod
        // types -- the old rule: the keyword anywhere.
        if (modNames.empty()) {
            return true;
        }

        // Otherwise the name has to be shaped the way this software names a fix: '<modName>Remap'
        // (getRemapName). The keyword ALONE is not enough, and the difference is not academic:
        // WWMI's own blend remap declares ResourceBlendRemapVertexVGBuffer / ...ForwardBuffer /
        // ...ReverseBuffer, which the old rule made candidates -- one target reaching them took the
        // whole blend-remap web with it and deleted three .buf files of a mod that had never been
        // fixed, every fix undoing first (Chisa, 2026-09-20).
        for (const std::string& modName : modNames) {
            if (!modName.empty() && sectionName.find(modName + remapKeyword) != std::string::npos) {
                return true;
            }
        }

        // ...or named for ANOTHER mod, by the kind this software appends after the keyword
        // (<mod>RemapBlend / Position / Texcoord / IB, <name><mod>RemapFix / Tex / DL / Ref, and the
        // IBRemapHide a component template writes). The mod-name test alone stopped the last remover
        // sweeping a leftover of a different mod type -- `[TextureOverrideFooRemapBlend]`, which
        // IniFile.removeFix has always removed (test_iniFileRemoveFix_ignoresModType) -- while
        // WWMI's own names stay out: the keyword there is followed by VertexVG / Forward / Reverse /
        // MergedSkeleton / "ped" / "s", or ends the name, and none of those is a kind.
        static const std::vector<std::string> Kinds = {IniKeywords::Texcoord, IniKeywords::Position, IniKeywords::Blend, "IB",
                                                       "Fix", "Tex", "DL", "Ref", "Hide"};
        for (std::size_t at = sectionName.find(remapKeyword); at != std::string::npos; at = sectionName.find(remapKeyword, at + 1)) {
            std::size_t kindAt = at + remapKeyword.size();
            for (const std::string& kind : Kinds) {
                if (sectionName.compare(kindAt, kind.size(), kind) != 0) {
                    continue;
                }
                // the kind must END there: "RemapTex" is a kind, "RemapTexture" is not
                std::size_t after = kindAt + kind.size();
                if (after >= sectionName.size() || sectionName[after] < 'a' || sectionName[after] > 'z') {
                    return true;
                }
            }
        }

        return false;
    }
}
