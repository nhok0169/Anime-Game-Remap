#ifndef AGRemapCore_RemapIniFixContext_TPP
#define AGRemapCore_RemapIniFixContext_TPP

#include <cstddef>
#include <utility>

#include "RemapIniFixContext.h"
#include "AGRemapCore/constants/IniBoilerPlate.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/tools/Heading.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RemapIniFixContext<K, V, KeyHash, KeyEqual>::RemapIniFixContext(std::optional<std::string> header,
                                                                     std::optional<std::string> footer):
        header(std::move(header)), footer(std::move(footer)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<std::string> RemapIniFixContext<K, V, KeyHash, KeyEqual>::modTypeName() const {
        return std::nullopt;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string RemapIniFixContext<K, V, KeyHash, KeyEqual>::modTypeHeadingName() const {
        std::optional<std::string> name = cleanModTypeName(*this);

        // An empty name is a real name here, not a missing one -- only "never classified" falls
        // back, exactly as the pure-Python original's `is None` check does.
        if (!name.has_value()) {
            return IniBoilerPlate::DefaultModTypeHeadingName;
        }

        return *name;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string RemapIniFixContext<K, V, KeyHash, KeyEqual>::headingName() const {
        std::string result = modTypeHeadingName();

        if (!result.empty()) {
            result += " ";
        }

        return result + "Remap";
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string RemapIniFixContext<K, V, KeyHash, KeyEqual>::getFixHeader() const {
        if (header.has_value()) {
            return *header;
        }

        return "; " + Heading(headingName(), IniBoilerPlate::DefaultHeadingSideLen,
                               IniBoilerPlate::DefaultHeadingSideChar).open();
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string RemapIniFixContext<K, V, KeyHash, KeyEqual>::getFixFooter() const {
        if (footer.has_value()) {
            return *footer;
        }

        // The leading blank line is part of the footer in the original too -- addFixBoilerPlate
        // appends this straight onto the fix with nothing in between.
        return "\n\n; " + Heading(headingName(), IniBoilerPlate::DefaultHeadingSideLen,
                                   IniBoilerPlate::DefaultHeadingSideChar).close();
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string RemapIniFixContext<K, V, KeyHash, KeyEqual>::getFixCredit() const {
        std::optional<std::string> name = cleanModTypeName(*this);

        std::string modTypeName = name.has_value() ? *name : IniBoilerPlate::DefaultCreditModTypeName;
        std::string shortModTypeName = name.has_value() ? *name : std::string();

        // Both are only spaced off from the words after them when they are actually there. An
        // .ini file classified as a mod type whose name is the empty string reads as the
        // unclassified one on the short half and as "Mod " on the other -- the original's own
        // behaviour, since only `is None` triggers its fallback.
        if (!modTypeName.empty()) {
            modTypeName += " ";
        }

        if (!shortModTypeName.empty()) {
            shortModTypeName += " ";
        }

        std::string result = replaceAll(IniBoilerPlate::Credit, IniBoilerPlate::ModTypeNameReplaceStr, modTypeName);
        return replaceAll(result, IniBoilerPlate::ShortModTypeNameReplaceStr, shortModTypeName);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void RemapIniFixContext<K, V, KeyHash, KeyEqual>::hideOriginalSections(const std::unordered_set<std::string>& sectionNames) {
        if (sectionNames.empty()) {
            return;
        }

        std::string txt = this->fileTxt();
        std::string result;
        result.reserve(txt.size());

        // Whether the section the walk is currently inside of is one being hidden. It stays on
        // until the *next* header line, which is what makes a hidden section own the blank lines
        // trailing it -- the pure-Python original's ranges do the same.
        bool hiding = false;
        std::size_t lineStart = 0;

        while (lineStart < txt.size()) {
            std::size_t lineEnd = txt.find('\n', lineStart);
            bool lastLine = lineEnd == std::string::npos;
            std::string line = lastLine ? txt.substr(lineStart) : txt.substr(lineStart, lineEnd - lineStart);

            if (IniFile::isSectionHeaderLine(line)) {
                hiding = sectionNames.count(IniFile::getSectionNameFromLine(line)) != 0;
            }

            if (hiding) {
                result += hideOriginalComment;
            }

            result += line;

            if (lastLine) {
                break;
            }

            // The loop condition is '<', not '<=', deliberately: a text ending in a newline has no
            // trailing empty line to comment, and prefixing one would leave a stray marker behind.
            result += '\n';
            lineStart = lineEnd + 1;
        }

        this->setFileTxt(std::move(result));
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string RemapIniFixContext<K, V, KeyHash, KeyEqual>::addFixBoilerPlate(const std::string& fix) const {
        std::string result = getFixHeader();
        result += getFixCredit();
        result += "\n\n" + fix;
        result += getFixFooter();
        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<std::string> RemapIniFixContext<K, V, KeyHash, KeyEqual>::cleanModTypeName(
            const RemapIniFixContext<K, V, KeyHash, KeyEqual>& ctx) {
        std::optional<std::string> name = ctx.modTypeName();

        if (!name.has_value()) {
            return name;
        }

        // The same two characters the pure-Python original strips, and for the same reason: this
        // goes into a ';'-comment, where a newline would end the comment early.
        std::string result;
        result.reserve(name->size());

        for (char c : *name) {
            if (c != '\n' && c != '\t') {
                result += c;
            }
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string RemapIniFixContext<K, V, KeyHash, KeyEqual>::replaceAll(const std::string& txt, const std::string& target,
                                                                        const std::string& replacement) {
        if (target.empty()) {
            return txt;
        }

        std::string result;
        std::size_t searchInd = 0;

        while (true) {
            std::size_t foundInd = txt.find(target, searchInd);

            if (foundInd == std::string::npos) {
                result.append(txt, searchInd, std::string::npos);
                return result;
            }

            result.append(txt, searchInd, foundInd - searchInd);
            result += replacement;
            searchInd = foundInd + target.size();
        }
    }
}

#endif
