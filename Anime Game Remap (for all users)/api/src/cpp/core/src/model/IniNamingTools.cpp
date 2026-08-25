#include "AGRemapCore/model/IniNamingTools.h"

#include <cctype>
#include <filesystem>
#include <optional>

#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/tools/TextTools.h"


namespace AGRemapCore {

    namespace {
        namespace fs = std::filesystem;

        bool asciiIEquals(char a, char b) {
            return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
        }

        // Case-insensitive (ASCII) search for the LAST occurrence of 'needle' in 'haystack' --
        // mirrors what the pure-Python original's getObjRemapFixName achieves indirectly (reverse
        // both strings, then re.split(..., maxsplit = 1) from the front is equivalent to finding
        // the last match from the end of the un-reversed string; verified empirically against the
        // Python original before simplifying it down to this direct form). ASCII-only, matching
        // this codebase's existing precedent for fixed-shape identifier matching
        // (IfPredPartTypeTools's toLowerAscii) rather than pulling in full Unicode case-folding for
        // what are, in practice, always ASCII component/mod names.
        std::optional<size_t> rfindCaseInsensitiveAscii(std::string_view haystack, std::string_view needle) {
            if (needle.empty() || needle.size() > haystack.size()) {
                return std::nullopt;
            }

            for (size_t i = haystack.size() - needle.size() + 1; i-- > 0; ) {
                bool match = true;
                for (size_t j = 0; j < needle.size(); ++j) {
                    if (!asciiIEquals(haystack[i + j], needle[j])) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    return i;
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
        return getModSuffixedName(name, IniKeywords::RemapTex, modName);
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

    std::string IniNamingTools::getFixedFile(const std::string& file, const std::string& modName, std::optional<std::string> fileExt) {
        fs::path path(file);
        fs::path folder = pathlibStyleParent(path);
        std::string baseName = path.stem().string();
        std::string ext = fileExt.has_value() ? *fileExt : path.extension().string();

        std::string newName = getRemapFixName(baseName, modName) + ext;
        return (folder / newName).string();
    }

    std::string IniNamingTools::getFixedElementFile(const std::string& file, const std::string& elementName, const std::string& modName, std::optional<std::string> fileExt) {
        fs::path path(file);
        fs::path folder = pathlibStyleParent(path);
        std::string baseName = path.stem().string();
        std::string ext = fileExt.has_value() ? *fileExt : path.extension().string();

        std::string newName = getRemapElementName(baseName, elementName, modName) + ext;
        if (folder == fs::path(".")) {
            return newName;
        }

        return (folder / newName).string();
    }

    std::string IniNamingTools::getFixedBlendFile(const std::string& blendFile, const std::string& modName) {
        return getFixedElementFile(blendFile, IniKeywords::Blend, modName, FileExt::Buf);
    }

    std::string IniNamingTools::getFixedPositionFile(const std::string& positionFile, const std::string& modName) {
        return getFixedElementFile(positionFile, IniKeywords::Position, modName, FileExt::Buf);
    }

    std::string IniNamingTools::getFixedTexFile(const std::string& texFile, const std::string& modName) {
        fs::path path(texFile);
        fs::path folder = path.parent_path();  // no "." fallback here -- see pathlibStyleParent's comment
        std::string baseName = path.filename().string();

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
        return (folder / newName).string();
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

        std::optional<size_t> pos = rfindCaseInsensitiveAscii(name, objNameStr);
        if (!pos.has_value()) {
            return getRemapFixName(name, capModName + newObjNameStr);
        }

        std::string newName = name.substr(0, *pos) + newObjNameStr + name.substr(*pos + objNameStr.size());
        return getRemapFixName(newName, capModName);
    }
}
