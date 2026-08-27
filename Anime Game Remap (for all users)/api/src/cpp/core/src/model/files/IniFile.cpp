#include "AGRemapCore/model/files/IniFile.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <variant>

#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/constants/GlobalIniClassifiers.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/strategies/iniClassifiers/IniClassifyStats.h"
#include "AGRemapCore/model/strategies/iniFixers/BaseIniFixer.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"
#include "AGRemapCore/tools/parsing/ParseContext.h"


namespace AGRemapCore {
    IniFile::IniFile(std::optional<std::string> file, std::string txt, std::optional<int> gameTypeId,
                      std::optional<std::unordered_set<int>> filteredModTypeIds,
                      std::optional<std::unordered_set<int>> forcedModTypeIds,
                      std::optional<std::unordered_map<int, ModType>> overrideModTypes,
                      BaseIniClassifier* iniClassifier):
        file_(std::move(file)),
        gameTypeId_(gameTypeId),
        filteredModTypeIds_(std::move(filteredModTypeIds)),
        forcedModTypeIds_(std::move(forcedModTypeIds)),
        overrideModTypes_(overrideModTypes.has_value() ? std::move(*overrideModTypes) : std::unordered_map<int, ModType>()),
        iniClassifier_(iniClassifier != nullptr ? iniClassifier : &GlobalIniClassifiers::classifier()) {
        // Matches the pure-Python original's own "_setupFileLines" -- a file-backed IniFile isn't
        // read from disk until something actually asks for it (see #readFileLines); a file-less one
        // has no other source of data, so the constructor's 'txt' is used immediately.
        if (!file_.has_value()) {
            setFileTxt(std::move(txt));
        }
    }

    const std::optional<std::string>& IniFile::getFile() const {
        return file_;
    }

    const std::string& IniFile::getFileTxt() const {
        return fileTxt_;
    }

    const std::vector<std::string>& IniFile::getFileLines() const {
        return fileLines_;
    }

    bool IniFile::fileLinesRead() const {
        return fileLinesRead_;
    }

    bool IniFile::isClassified() const {
        return isClassified_;
    }

    const std::unordered_map<int, ModType>& IniFile::getModTypes() const {
        return modTypes;
    }

    void IniFile::setFixer(BaseIniFixer* fixer) {
        fixer_ = fixer;
    }

    BaseIniFixer* IniFile::getFixer() const {
        return fixer_;
    }

    std::unordered_map<int, std::vector<IniGraphGroup>> IniFile::parse(bool flushIfTemplates) {
        if (!isClassified_) {
            classify();
        }

        // The equivalent of the pure-Python original's "if (self.availableType is None): return"
        // -- nothing recognized this .ini file as belonging to any mod, so there's nothing to parse.
        if (modTypes.empty()) {
            return {};
        }

        getIfTemplates(flushIfTemplates);

        std::unordered_map<int, std::vector<IniGraphGroup>> result;

        // The pure-Python original has exactly one 'availableType' and so parses exactly once; a
        // C++ IniFile can be classified as several ModTypes at once, so each one's own parser runs
        // and its results are keyed by that mod type's id.
        for (auto& entry : modTypes) {
            int modTypeId = entry.first;
            BaseIniParser* parser = entry.second.iniParser.get();

            // The equivalent of the original's "_getParser" returning None -- that mod type simply
            // contributes no entry, rather than aborting the whole parse.
            if (parser == nullptr) {
                continue;
            }

            // A ModType's parser is shared and starts unbound (a ModType describes a kind of mod,
            // not one specific file), so it has to be pointed at this file before use -- see
            // ModType::iniParser for the concurrency caveat this implies.
            parser->setIniFile(this);
            parser->clear();
            result.emplace(modTypeId, parser->parse());
        }

        return result;
    }

    std::unordered_map<std::string, std::string> IniFile::fix(bool keepBackup, bool fixOnly, bool hideOrig) {
        if (fixer_ == nullptr) {
            return {};
        }

        return fixer_->fix(keepBackup, fixOnly, hideOrig);
    }

    void IniFile::setFileTxt(std::string txt) {
        fileTxt_ = std::move(txt);
        fileLines_.clear();

        // Split fileTxt_ into lines the same way Python's str.splitlines(keepends = True) does --
        // walk the text once, cutting a new line every time a '\n' is found and keeping it attached
        // to the line it ends. A trailing '\n' terminates the last line rather than starting a new,
        // empty one (eg. "abc\n" -> ["abc\n"], not ["abc\n", ""]) -- the loop below naturally does
        // this since it only ever starts a new line when there's still text left to put in it.
        size_t lineStart = 0;
        while (lineStart < fileTxt_.size()) {
            size_t newlinePos = fileTxt_.find('\n', lineStart);
            if (newlinePos == std::string::npos) {
                fileLines_.emplace_back(fileTxt_.substr(lineStart));
                break;
            }

            fileLines_.emplace_back(fileTxt_.substr(lineStart, newlinePos - lineStart + 1));
            lineStart = newlinePos + 1;
        }

        fileLinesRead_ = true;
    }

    void IniFile::readFromDisk(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Unable to open file: " + path);
        }

        std::ostringstream buf;
        buf << file.rdbuf();
        std::string txt = buf.str();

        // Normalize "\r\n"/lone "\r" line endings down to "\n", matching the universal-newline
        // translation Python's text-mode "open(path, 'r')" performs on read -- done manually (rather
        // than relying on the ifstream/OS text-mode behavior above) so this is portable and behaves
        // the same on every platform, not just Windows.
        std::string normalized;
        normalized.reserve(txt.size());
        for (size_t i = 0; i < txt.size(); ++i) {
            char c = txt[i];
            if (c == '\r') {
                normalized.push_back('\n');
                if (i + 1 < txt.size() && txt[i + 1] == '\n') {
                    ++i;
                }
            } else {
                normalized.push_back(c);
            }
        }

        setFileTxt(std::move(normalized));
    }

    const std::vector<std::string>& IniFile::readFileLines() {
        if (file_.has_value()) {
            readFromDisk(*file_);
        }
        return fileLines_;
    }

    std::optional<ModType> IniFile::getModType(int modTypeId) const {
        auto overrideIt = overrideModTypes_.find(modTypeId);
        if (overrideIt != overrideModTypes_.end()) {
            return overrideIt->second;
        }

        // ModTypeIdTools::getModType now takes the raw int directly (no more ModTypeId-enum-only
        // restriction), so a genuinely custom 'modTypeId' (one ModTypeIdTools::getEnum wouldn't
        // recognize) can be resolved via the global registry too now, not just overrideModTypes_
        // above -- as long as it was actually registered there via registerModType.
        return ModTypeIdTools::getModType(modTypeId);
    }

    void IniFile::classify() {
        if (!fileLinesRead_) {
            readFileLines();
        }

        // gameTypeId_ is kept as a plain int (see the constructor's own doc comment on why) -- the
        // classifier's API only understands a real GameTypeId, so an unrecognized custom id simply
        // can't be expressed there and falls back to "unfiltered" (std::nullopt) for classification
        // purposes.
        std::optional<GameTypeId> gameTypeIdEnum = gameTypeId_.has_value() ? GameTypeIdTools::getEnum(*gameTypeId_) : std::nullopt;

        modTypes.clear();

        if (forcedModTypeIds_.has_value()) {
            bool isFixedResult = false;
            bool isModResult = false;
            iniClassifier_->checkIsFixedMod(fileLines_, &isFixedResult, &isModResult, gameTypeIdEnum);
            isFixed = isFixedResult;
            isMod = isModResult;

            for (int modTypeId : *forcedModTypeIds_) {
                std::optional<ModType> modType = getModType(modTypeId);
                if (modType.has_value()) {
                    modTypes.emplace(modTypeId, *modType);
                }
            }

            // Set at each exit rather than up front, so a classifier that throws part-way leaves
            // this false -- an aborted classify hasn't classified anything.
            isClassified_ = true;
            return;
        }

        IniClassifyStats stats = iniClassifier_->classify(fileLines_, gameTypeIdEnum);
        isMod = stats.isMod;
        isFixed = stats.isFixed;

        for (const auto& entry : stats.modType) {
            int modTypeId = entry.first;
            if (filteredModTypeIds_.has_value() && !filteredModTypeIds_->contains(modTypeId)) {
                continue;
            }

            std::optional<ModType> modType = getModType(modTypeId);
            if (modType.has_value()) {
                modTypes.emplace(modTypeId, *modType);
            }
        }

        isClassified_ = true;
    }

    namespace {
        // Trims ASCII whitespace (space/tab/CR/LF) from both ends of 'str' -- not a member, just a
        // small local helper; matches the ' '/'\t' trimming ConfigParser itself performs on keys and
        // values.
        std::string trim(const std::string& str) {
            size_t start = 0;
            while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
                ++start;
            }

            if (start == str.size()) {
                return "";
            }

            size_t end = str.size();
            while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
                --end;
            }

            return str.substr(start, end - start);
        }

        // NOTE: no key-lowercasing helper here -- IniFile.py sets
        // "self._parser.optionxform=str" to disable ConfigParser's default lowercasing, so keys
        // keep their original case (see parseSectionKVPs below).

        // The parsed KVPs of one content chunk, keyed by KVP key -- every value a duplicated key
        // takes is kept, in encounter order, as (orderInd, value) pairs.
        using SectionKVPs = tsl::ordered_map<std::string, std::vector<std::pair<long long, std::string>>>;

        // One entry in the argument IfTemplate::build expects: either a raw conditional line
        // ("if ...", "else", "endif") or a parsed block of KVPs, tagged with its 1-based line number.
        using RawPart = std::pair<int, std::variant<std::string, SectionKVPs>>;

        /*
         * Parses one section's raw KVP text into the shape IfTemplate::build expects for a content
         * part -- a purpose-built single-pass parser, not a port of Python's ConfigParser.
         *
         * Preserved: blank-line and full-line-comment (';'/'#' prefix) skipping; splitting each line
         * on the first '=' or ':'; trimming whitespace around both key and value; original key CASE
         * (the pure-Python original disables ConfigParser's default key-lowercasing 'optionxform'
         * via "self._parser.optionxform=str", so keys are NOT lowercased here either -- confirmed
         * against the real Python original, not just assumed from ConfigParser's documented
         * default); preserving EVERY value for a duplicate key, in encounter order, as
         * (orderInd, value) pairs sharing one section-wide incrementing counter (the direct C++
         * equivalent of what the pure-Python original's KeepAllDict + "{index}_{value}"-string-
         * encoding trick works around -- this builds the same (index, value) shape natively, with no
         * encode/decode round trip).
         *
         * Not preserved (by explicit user direction -- "you don't have to follow exactly what
         * ConfigParser does... I value speed"): '%'-style interpolation (values are taken literally,
         * so a bare '%' never raises); multi-line continuation lines (moot in practice -- the
         * pure-Python original already strips all leading whitespace from every line before parsing
         * a section, which incidentally neuters ConfigParser's own indentation-based continuation
         * feature anyway); ConfigParser's stricter parse-error behavior (a malformed line with no
         * '='/':' is silently skipped here, rather than aborting the whole section).
         */
        SectionKVPs parseSectionKVPs(const std::string& srcTxt) {
            SectionKVPs result;
            long long orderInd = 0;

            size_t lineStart = 0;
            size_t srcLen = srcTxt.size();

            while (lineStart <= srcLen) {
                size_t newlinePos = srcTxt.find('\n', lineStart);
                std::string rawLine = (newlinePos == std::string::npos) ? srcTxt.substr(lineStart) : srcTxt.substr(lineStart, newlinePos - lineStart);

                std::string line = trim(rawLine);
                if (!line.empty() && line[0] != ';' && line[0] != '#') {
                    size_t delimPos = line.find_first_of("=:");
                    if (delimPos != std::string::npos) {
                        // NOTE: real ConfigParser normally lowercases option (key) names via its
                        // 'optionxform' hook, but IniFile.py explicitly overrides that to the identity
                        // function ("self._parser.optionxform=str") -- so keys keep their original
                        // case here too. Confirmed against the real Python original: 'Filename' stays
                        // 'Filename', it is NOT lowered to 'filename'.
                        std::string key = trim(line.substr(0, delimPos));
                        std::string value = trim(line.substr(delimPos + 1));

                        result[key].emplace_back(orderInd, std::move(value));
                        ++orderInd;
                    }
                    // A line with no '=' or ':' delimiter (and not blank/a comment) has no direct
                    // ConfigParser-continuation-line equivalent here (see this function's own doc
                    // comment on why continuation lines are moot in practice) -- silently skipped
                    // rather than aborting the whole section's parse, unlike real ConfigParser.
                }

                if (newlinePos == std::string::npos) {
                    break;
                }
                lineStart = newlinePos + 1;
            }

            return result;
        }

        // The in-progress state for the one section readIfTemplates is currently walking. Grouped
        // into a struct purely so finalizeSection below can take it as a single parameter instead of
        // the 6 separate by-reference arguments it would otherwise need.
        struct SectionAccum {
            // The section's name, from its own "[SectionName]" header line.
            std::string name;

            // The index into fileLines_ of that header line itself.
            size_t startInd = 0;

            // The parts completed so far -- conditional lines, and KVP blocks already flushed.
            std::vector<RawPart> rawParts;

            // Content lines seen since the last flush, not yet parsed into a RawPart.
            std::string pendingKvpText;

            // The 1-based line number 'pendingKvpText' starts at.
            size_t pendingStartLineNo = 0;

            // Whether 'pendingKvpText' has had anything appended since the last flush. Distinct from
            // "pendingKvpText is non-empty" -- a run of blank lines appends only "\n"s, which is
            // still a pending chunk that a following conditional line must flush.
            bool hasPending = false;
        };

        /*
         * Finishes building the IfTemplate for the section 'accum' has been accumulating, and stores
         * it into 'sectionIfTemplates' -- but only if this is that section name's FIRST occurrence
         * (matching readIfTemplates' own 'handleDuplicateFunc = lambda duplicates: duplicates[0]').
         * Any still-pending KVP text is flushed first, and -- unlike the mid-scan flush inside
         * readIfTemplates' loop -- only appended if it actually parses to something non-empty
         * (matching the pure-Python original's own "if (currentPart):" check). Resets 'accum's
         * part state afterwards so the next section starts clean.
         *
         * Called once per section boundary as those boundaries are discovered during readIfTemplates'
         * single pass. This replaces what used to be a wholly separate second pass (the old
         * IniFile::buildIfTemplate, which re-walked fileLines_[startInd, endInd) for every section
         * after readIfTemplates had already walked the whole file once just to find those boundaries).
         */
        void finalizeSection(SectionAccum& accum,
                             std::unordered_map<std::string, std::unique_ptr<IfTemplate<std::string, std::string>>>& sectionIfTemplates,
                             const std::optional<std::string>& file,
                             Z3Context& z3Ctx) {
            if (!accum.pendingKvpText.empty()) {
                SectionKVPs parsed = parseSectionKVPs(accum.pendingKvpText);
                if (!parsed.empty()) {
                    accum.rawParts.emplace_back(static_cast<int>(accum.pendingStartLineNo), std::move(parsed));
                }
            }

            if (sectionIfTemplates.find(accum.name) == sectionIfTemplates.end()) {
                ParseContext ctx("", file, accum.startInd + 1);
                IfTemplateRunConfig<std::string, std::string> runConfig{
                    IniKeywords::Run,
                    [](const std::string& v) { return v; },
                    [](const std::string& s) { return s; }
                };
                sectionIfTemplates.emplace(accum.name,
                    IfTemplate<std::string, std::string>::build(accum.rawParts, runConfig, accum.name, &ctx, &z3Ctx));
            }

            accum.rawParts.clear();
            accum.pendingKvpText.clear();
            accum.hasPending = false;
        }
    }

    bool IniFile::isSectionHeaderLine(const std::string& line) {
        size_t i = 0;
        while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i]))) {
            ++i;
        }

        if (i >= line.size() || line[i] != '[') {
            return false;
        }

        return line.find(']', i) != std::string::npos;
    }

    std::string IniFile::getSectionNameFromLine(const std::string& line) {
        size_t leftPos = line.find('[');
        size_t rightPos = line.rfind(']');
        std::string name;

        if (leftPos != std::string::npos && rightPos != std::string::npos && rightPos > leftPos) {
            name = line.substr(leftPos + 1, rightPos - leftPos - 1);
        } else if (rightPos != std::string::npos) {
            name = line.substr(0, rightPos);
        } else if (leftPos != std::string::npos) {
            name = line.substr(leftPos + 1);
        } else {
            name = line;
        }

        return trim(name);
    }

    bool IniFile::isConditionalLine(const std::string& line) {
        size_t i = 0;
        while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i]))) {
            ++i;
        }

        // Checked in the same order as the pure-Python original's own compiled pattern
        // ("endif|else|if|elif") -- order doesn't actually matter for correctness here (none of
        // these 4 keywords is a prefix of another), kept only for easy comparison against that
        // source pattern.
        static const std::string keywords[] = {"endif", "else", "if", "elif"};
        for (const std::string& keyword : keywords) {
            if (line.compare(i, keyword.size(), keyword) == 0) {
                return true;
            }
        }

        return false;
    }

    void IniFile::stripHideOriginalComment(std::string& line) {
        const std::string& marker = IniKeywords::HideOriginalComment;
        size_t pos;
        while ((pos = line.find(marker)) != std::string::npos) {
            line.erase(pos, marker.size());
        }
    }

    const std::unordered_map<std::string, std::unique_ptr<IfTemplate<std::string, std::string>>>& IniFile::readIfTemplates() {
        if (!fileLinesRead_) {
            readFileLines();
        }

        sectionIfTemplates_.clear();

        bool inSection = false;
        SectionAccum accum;

        size_t fileLinesLen = fileLines_.size();

        for (size_t i = 0; i < fileLinesLen; ++i) {
            std::string line = fileLines_[i];
            stripHideOriginalComment(line);

            bool isHeader = isSectionHeaderLine(line);

            if (isHeader) {
                if (inSection) {
                    finalizeSection(accum, sectionIfTemplates_, file_, z3Ctx_);
                }

                accum.name = getSectionNameFromLine(line);
                accum.startInd = i;
                inSection = true;
                continue;
            }

            if (!inSection) {
                continue;
            }

            bool isConditional = isConditionalLine(line);

            // Mirrors the pure-Python original's own '_processIfTemplate': flushing here is
            // unconditional (even an empty parse result gets appended) -- only the tail flush inside
            // finalizeSection is gated on a non-empty result.
            if (isConditional && accum.hasPending) {
                accum.rawParts.emplace_back(static_cast<int>(accum.pendingStartLineNo), parseSectionKVPs(accum.pendingKvpText));
                accum.pendingKvpText.clear();
                accum.hasPending = false;
            }

            if (isConditional) {
                accum.rawParts.emplace_back(static_cast<int>(i + 1), line);
                continue;
            }

            accum.pendingKvpText += line;
            if (!accum.hasPending) {
                accum.pendingStartLineNo = i + 1;
            }
            accum.hasPending = true;
        }

        if (inSection) {
            finalizeSection(accum, sectionIfTemplates_, file_, z3Ctx_);
        }

        ifTemplatesRead_ = true;
        return sectionIfTemplates_;
    }

    const std::unordered_map<std::string, std::unique_ptr<IfTemplate<std::string, std::string>>>& IniFile::getIfTemplates(bool flush) {
        if (!ifTemplatesRead_ || flush) {
            readIfTemplates();
        }
        return sectionIfTemplates_;
    }
}
