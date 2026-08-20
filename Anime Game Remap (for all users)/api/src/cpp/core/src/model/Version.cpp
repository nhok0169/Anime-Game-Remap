#include "AGRemapCore/model/Version.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <stdexcept>


namespace AGRemapCore {

    namespace {

        // ---- low-level scanning helpers, operating on an already-trimmed std::string ----

        bool isAsciiDigit(char c) {
            return c >= '0' && c <= '9';
        }

        bool isAsciiAlnum(char c) {
            return isAsciiDigit(c) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        }

        char toAsciiLower(char c) {
            return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
        }

        std::string toAsciiLower(const std::string& s) {
            std::string result = s;
            for (char& c : result) {
                c = toAsciiLower(c);
            }
            return result;
        }

        // Case-insensitive literal match of 'lit' at 's[pos]'; advances 'pos' and returns true on success.
        bool matchLiteralCI(const std::string& s, std::size_t& pos, const char* lit) {
            std::size_t n = 0;
            while (lit[n] != '\0') {
                ++n;
            }
            if (pos + n > s.size()) {
                return false;
            }
            for (std::size_t i = 0; i < n; ++i) {
                if (toAsciiLower(s[pos + i]) != lit[i]) {
                    return false;
                }
            }
            pos += n;
            return true;
        }

        // Consumes a single optional '-', '_', or '.' at 'pos'. Always "succeeds" (it's optional).
        void consumeOptionalSeparator(const std::string& s, std::size_t& pos) {
            if (pos < s.size() && (s[pos] == '-' || s[pos] == '_' || s[pos] == '.')) {
                ++pos;
            }
        }

        // Consumes one-or-more ASCII digits at 'pos'. Returns nullopt (leaving 'pos' unchanged) if
        // there are none.
        std::optional<std::string> consumeDigits(const std::string& s, std::size_t& pos) {
            std::size_t start = pos;
            while (pos < s.size() && isAsciiDigit(s[pos])) {
                ++pos;
            }
            if (pos == start) {
                return std::nullopt;
            }
            return s.substr(start, pos - start);
        }

        std::optional<std::uint64_t> parseUint64(const std::string& digits) {
            try {
                std::size_t consumed = 0;
                unsigned long long value = std::stoull(digits, &consumed);
                if (consumed != digits.size()) {
                    return std::nullopt;
                }
                if (value > std::numeric_limits<std::uint64_t>::max()) {
                    return std::nullopt;
                }
                return static_cast<std::uint64_t>(value);
            } catch (const std::exception&) {
                return std::nullopt;
            }
        }

        // Tries each word in 'words' (in order -- matching PEP 440's regex alternation, which is
        // first-match-wins, not longest-match) as a case-insensitive literal at 'pos'. Returns the
        // matched (lowercase) word and advances 'pos' on success.
        std::optional<std::string> matchFirstOf(const std::string& s, std::size_t& pos, const std::vector<const char*>& words) {
            for (const char* word : words) {
                std::size_t tryPos = pos;
                if (matchLiteralCI(s, tryPos, word)) {
                    pos = tryPos;
                    return std::string(word);
                }
            }
            return std::nullopt;
        }

        // Parses an optional "[-_.]? <letter> [-_.]? <digits>?" group (shared shape used by the
        // pre-release, word-form post-release, and dev-release groups). Returns the matched
        // (already-lowercased) letter and the parsed number (0 if no digits followed), or nullopt
        // if none of 'words' matched at the current position -- in which case 'pos' is left
        // untouched (fully rewound, including any speculatively-consumed separator).
        std::optional<std::pair<std::string, std::uint64_t>> parseLetterGroup(const std::string& s, std::size_t& pos, const std::vector<const char*>& words, bool* ok) {
            *ok = true;
            std::size_t save = pos;

            consumeOptionalSeparator(s, pos);
            std::optional<std::string> letter = matchFirstOf(s, pos, words);
            if (!letter.has_value()) {
                pos = save;
                return std::nullopt;
            }

            consumeOptionalSeparator(s, pos);
            std::uint64_t number = 0;
            std::optional<std::string> digits = consumeDigits(s, pos);
            if (digits.has_value()) {
                std::optional<std::uint64_t> value = parseUint64(*digits);
                if (!value.has_value()) {
                    *ok = false;
                    return std::nullopt;
                }
                number = *value;
            }

            return std::make_pair(*letter, number);
        }

        std::string normalizePreLetter(const std::string& letter) {
            if (letter == "alpha") return "a";
            if (letter == "beta") return "b";
            if (letter == "c" || letter == "pre" || letter == "preview") return "rc";
            return letter;  // already "a", "b", or "rc"
        }
    }

    // ---- Version::PrePostDevBound ----

    int Version::PrePostDevBound::compare(const PrePostDevBound& other) const {
        if (kind != other.kind) {
            return static_cast<int>(kind) < static_cast<int>(other.kind) ? -1 : 1;
        }
        if (kind != Kind::Value) {
            return 0;
        }
        if (letter != other.letter) {
            return letter < other.letter ? -1 : 1;
        }
        if (number != other.number) {
            return number < other.number ? -1 : 1;
        }
        return 0;
    }

    bool Version::PrePostDevBound::operator==(const PrePostDevBound& other) const {
        return compare(other) == 0;
    }

    // ---- Version::LocalSegment ----

    int Version::LocalSegment::compare(const LocalSegment& other) const {
        if (isNumeric != other.isNumeric) {
            // Alphanumeric segments always sort before numeric ones (PEP 440).
            return isNumeric ? 1 : -1;
        }
        if (isNumeric) {
            if (number != other.number) {
                return number < other.number ? -1 : 1;
            }
            return 0;
        }
        if (text != other.text) {
            return text < other.text ? -1 : 1;
        }
        return 0;
    }

    bool Version::LocalSegment::operator==(const LocalSegment& other) const {
        return compare(other) == 0;
    }

    std::string Version::LocalSegment::toString() const {
        return isNumeric ? std::to_string(number) : text;
    }

    // ---- Version ----

    Version::Version(std::vector<std::uint64_t> release): release_(std::move(release)) {
        strippedRelease_ = stripTrailingZeros(release_);
        preKey_.kind = PrePostDevBound::Kind::PosInf;   // no pre, no post, no dev -> pre sorts as +inf
        postKey_.kind = PrePostDevBound::Kind::NegInf;  // no post -> sorts as -inf
        devKey_.kind = PrePostDevBound::Kind::PosInf;   // no dev -> sorts as +inf
    }

    std::vector<std::uint64_t> Version::stripTrailingZeros(const std::vector<std::uint64_t>& release) {
        std::size_t end = release.size();
        while (end > 0 && release[end - 1] == 0) {
            --end;
        }
        return std::vector<std::uint64_t>(release.begin(), release.begin() + end);
    }

    int Version::compareUintVectors(const std::vector<std::uint64_t>& a, const std::vector<std::uint64_t>& b) {
        std::size_t n = std::min(a.size(), b.size());
        for (std::size_t i = 0; i < n; ++i) {
            if (a[i] != b[i]) {
                return a[i] < b[i] ? -1 : 1;
            }
        }
        if (a.size() != b.size()) {
            return a.size() < b.size() ? -1 : 1;
        }
        return 0;
    }

    std::optional<Version> Version::parse(const std::string& raw) {
        // Trim surrounding whitespace (the real regex is anchored with \s* on both ends).
        std::size_t begin = 0;
        std::size_t end = raw.size();
        while (begin < end && std::isspace(static_cast<unsigned char>(raw[begin]))) {
            ++begin;
        }
        while (end > begin && std::isspace(static_cast<unsigned char>(raw[end - 1]))) {
            --end;
        }
        std::string s = raw.substr(begin, end - begin);

        std::size_t pos = 0;

        // Optional leading 'v'.
        if (pos < s.size() && (s[pos] == 'v' || s[pos] == 'V')) {
            ++pos;
        }

        // Optional epoch: DIGITS '!'.
        std::uint64_t epoch = 0;
        {
            std::size_t save = pos;
            std::optional<std::string> digits = consumeDigits(s, pos);
            if (digits.has_value() && pos < s.size() && s[pos] == '!') {
                std::optional<std::uint64_t> value = parseUint64(*digits);
                if (!value.has_value()) {
                    return std::nullopt;
                }
                epoch = *value;
                ++pos;  // consume '!'
            } else {
                pos = save;  // not an epoch -- these digits belong to the release segment
            }
        }

        // Release: DIGITS ('.' DIGITS)*, at least one group required.
        std::vector<std::uint64_t> release;
        {
            std::optional<std::string> digits = consumeDigits(s, pos);
            if (!digits.has_value()) {
                return std::nullopt;
            }
            std::optional<std::uint64_t> value = parseUint64(*digits);
            if (!value.has_value()) {
                return std::nullopt;
            }
            release.push_back(*value);

            while (pos < s.size() && s[pos] == '.') {
                std::size_t save = pos;
                ++pos;
                std::optional<std::string> nextDigits = consumeDigits(s, pos);
                if (!nextDigits.has_value()) {
                    pos = save;
                    break;
                }
                std::optional<std::uint64_t> nextValue = parseUint64(*nextDigits);
                if (!nextValue.has_value()) {
                    return std::nullopt;
                }
                release.push_back(*nextValue);
            }
        }

        // Pre-release: [-_.]? (alpha|a|beta|b|preview|pre|c|rc) [-_.]? DIGITS? -- alternatives
        // tried in this exact order, matching PEP 440's first-match-wins regex alternation.
        std::optional<std::pair<std::string, std::uint64_t>> pre;
        {
            static const std::vector<const char*> preWords = {"alpha", "a", "beta", "b", "preview", "pre", "c", "rc"};
            bool ok = true;
            std::optional<std::pair<std::string, std::uint64_t>> match = parseLetterGroup(s, pos, preWords, &ok);
            if (!ok) {
                return std::nullopt;
            }
            if (match.has_value()) {
                pre = std::make_pair(normalizePreLetter(match->first), match->second);
            }
        }

        // Post-release: either "-" DIGITS (implicit form, tried first), or
        // [-_.]? (post|rev|r) [-_.]? DIGITS?.
        std::optional<std::uint64_t> post;
        {
            std::size_t save = pos;
            if (pos < s.size() && s[pos] == '-') {
                std::size_t tryPos = pos + 1;
                std::optional<std::string> digits = consumeDigits(s, tryPos);
                if (digits.has_value()) {
                    std::optional<std::uint64_t> value = parseUint64(*digits);
                    if (!value.has_value()) {
                        return std::nullopt;
                    }
                    post = value;
                    pos = tryPos;
                }
            }

            if (!post.has_value()) {
                pos = save;
                static const std::vector<const char*> postWords = {"post", "rev", "r"};
                bool ok = true;
                std::optional<std::pair<std::string, std::uint64_t>> match = parseLetterGroup(s, pos, postWords, &ok);
                if (!ok) {
                    return std::nullopt;
                }
                if (match.has_value()) {
                    post = match->second;
                }
            }
        }

        // Dev-release: [-_.]? dev [-_.]? DIGITS?.
        std::optional<std::uint64_t> dev;
        {
            static const std::vector<const char*> devWords = {"dev"};
            bool ok = true;
            std::optional<std::pair<std::string, std::uint64_t>> match = parseLetterGroup(s, pos, devWords, &ok);
            if (!ok) {
                return std::nullopt;
            }
            if (match.has_value()) {
                dev = match->second;
            }
        }

        // Local version: "+" [a-zA-Z0-9]+ ([-_.][a-zA-Z0-9]+)*.
        bool hasLocal = false;
        std::vector<Version::LocalSegment> localSegments;
        if (pos < s.size() && s[pos] == '+') {
            ++pos;

            auto consumeSegmentText = [&](std::size_t& p) -> std::optional<std::string> {
                std::size_t start = p;
                while (p < s.size() && isAsciiAlnum(s[p])) {
                    ++p;
                }
                if (p == start) {
                    return std::nullopt;
                }
                return s.substr(start, p - start);
            };

            auto makeSegment = [](const std::string& segText) -> Version::LocalSegment {
                Version::LocalSegment segment;
                bool numeric = !segText.empty() && std::all_of(segText.begin(), segText.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
                if (numeric) {
                    segment.isNumeric = true;
                    segment.number = parseUint64(segText).value_or(0);
                } else {
                    segment.isNumeric = false;
                    segment.text = toAsciiLower(segText);
                }
                return segment;
            };

            std::optional<std::string> firstSeg = consumeSegmentText(pos);
            if (!firstSeg.has_value()) {
                return std::nullopt;  // '+' with no local content at all is invalid
            }

            bool numericOverflow = !firstSeg->empty() && std::all_of(firstSeg->begin(), firstSeg->end(), [](unsigned char c) { return std::isdigit(c) != 0; }) && !parseUint64(*firstSeg).has_value();
            if (numericOverflow) {
                return std::nullopt;
            }

            hasLocal = true;
            localSegments.push_back(makeSegment(*firstSeg));

            while (pos < s.size() && (s[pos] == '-' || s[pos] == '_' || s[pos] == '.')) {
                std::size_t save = pos;
                ++pos;
                std::optional<std::string> seg = consumeSegmentText(pos);
                if (!seg.has_value()) {
                    pos = save;
                    break;
                }
                bool overflow = !seg->empty() && std::all_of(seg->begin(), seg->end(), [](unsigned char c) { return std::isdigit(c) != 0; }) && !parseUint64(*seg).has_value();
                if (overflow) {
                    return std::nullopt;
                }
                localSegments.push_back(makeSegment(*seg));
            }
        }

        // The whole trimmed string must be consumed -- anything left over is invalid.
        if (pos != s.size()) {
            return std::nullopt;
        }

        Version result;
        result.epoch_ = epoch;
        result.release_ = std::move(release);
        result.pre_ = pre;
        result.post_ = post;
        result.dev_ = dev;
        result.hasLocal_ = hasLocal;
        result.localSegments_ = std::move(localSegments);

        result.strippedRelease_ = stripTrailingZeros(result.release_);

        // Mirrors packaging.version._cmpkey exactly:
        if (!result.pre_.has_value() && !result.post_.has_value() && result.dev_.has_value()) {
            result.preKey_.kind = PrePostDevBound::Kind::NegInf;
        } else if (!result.pre_.has_value()) {
            result.preKey_.kind = PrePostDevBound::Kind::PosInf;
        } else {
            result.preKey_ = PrePostDevBound{PrePostDevBound::Kind::Value, result.pre_->first, result.pre_->second};
        }

        if (!result.post_.has_value()) {
            result.postKey_.kind = PrePostDevBound::Kind::NegInf;
        } else {
            result.postKey_ = PrePostDevBound{PrePostDevBound::Kind::Value, "post", *result.post_};
        }

        if (!result.dev_.has_value()) {
            result.devKey_.kind = PrePostDevBound::Kind::PosInf;
        } else {
            result.devKey_ = PrePostDevBound{PrePostDevBound::Kind::Value, "dev", *result.dev_};
        }

        return result;
    }

    int Version::compareLocal(const Version& other) const {
        if (hasLocal_ != other.hasLocal_) {
            // No local segment sorts before having one.
            return hasLocal_ ? 1 : -1;
        }
        if (!hasLocal_) {
            return 0;
        }

        std::size_t n = std::min(localSegments_.size(), other.localSegments_.size());
        for (std::size_t i = 0; i < n; ++i) {
            int c = localSegments_[i].compare(other.localSegments_[i]);
            if (c != 0) {
                return c;
            }
        }
        if (localSegments_.size() != other.localSegments_.size()) {
            return localSegments_.size() < other.localSegments_.size() ? -1 : 1;
        }
        return 0;
    }

    int Version::compare(const Version& other) const {
        if (epoch_ != other.epoch_) {
            return epoch_ < other.epoch_ ? -1 : 1;
        }

        int c = compareUintVectors(strippedRelease_, other.strippedRelease_);
        if (c != 0) {
            return c;
        }

        c = preKey_.compare(other.preKey_);
        if (c != 0) {
            return c;
        }

        c = postKey_.compare(other.postKey_);
        if (c != 0) {
            return c;
        }

        c = devKey_.compare(other.devKey_);
        if (c != 0) {
            return c;
        }

        return compareLocal(other);
    }

    bool Version::operator<(const Version& other) const { return compare(other) < 0; }
    bool Version::operator<=(const Version& other) const { return compare(other) <= 0; }
    bool Version::operator>(const Version& other) const { return compare(other) > 0; }
    bool Version::operator>=(const Version& other) const { return compare(other) >= 0; }
    bool Version::operator==(const Version& other) const { return compare(other) == 0; }
    bool Version::operator!=(const Version& other) const { return compare(other) != 0; }

    std::string Version::toString() const {
        std::string result;

        if (epoch_ != 0) {
            result += std::to_string(epoch_);
            result += '!';
        }

        for (std::size_t i = 0; i < release_.size(); ++i) {
            if (i > 0) {
                result += '.';
            }
            result += std::to_string(release_[i]);
        }

        if (pre_.has_value()) {
            result += pre_->first;
            result += std::to_string(pre_->second);
        }

        if (post_.has_value()) {
            result += ".post";
            result += std::to_string(*post_);
        }

        if (dev_.has_value()) {
            result += ".dev";
            result += std::to_string(*dev_);
        }

        if (hasLocal_) {
            result += '+';
            for (std::size_t i = 0; i < localSegments_.size(); ++i) {
                if (i > 0) {
                    result += '.';
                }
                result += localSegments_[i].toString();
            }
        }

        return result;
    }

    std::uint64_t Version::getEpoch() const {
        return epoch_;
    }

    const std::vector<std::uint64_t>& Version::getRelease() const {
        return release_;
    }

    const std::optional<std::pair<std::string, std::uint64_t>>& Version::getPre() const {
        return pre_;
    }

    std::optional<std::uint64_t> Version::getPost() const {
        return post_;
    }

    std::optional<std::uint64_t> Version::getDev() const {
        return dev_;
    }

    std::optional<std::string> Version::getLocal() const {
        if (!hasLocal_) {
            return std::nullopt;
        }

        std::string result;
        for (std::size_t i = 0; i < localSegments_.size(); ++i) {
            if (i > 0) {
                result += '.';
            }
            result += localSegments_[i].toString();
        }
        return result;
    }

    bool Version::isPrerelease() const {
        return pre_.has_value() || dev_.has_value();
    }

    bool Version::isPostrelease() const {
        return post_.has_value();
    }

    bool Version::isDevrelease() const {
        return dev_.has_value();
    }

    std::uint64_t Version::getMajor() const {
        return release_.size() >= 1 ? release_[0] : 0;
    }

    std::uint64_t Version::getMinor() const {
        return release_.size() >= 2 ? release_[1] : 0;
    }

    std::uint64_t Version::getMicro() const {
        return release_.size() >= 3 ? release_[2] : 0;
    }

    std::string Version::getPublic() const {
        std::string full = toString();
        std::size_t plusPos = full.find('+');
        return plusPos == std::string::npos ? full : full.substr(0, plusPos);
    }

    std::string Version::getBaseVersion() const {
        std::string result;

        if (epoch_ != 0) {
            result += std::to_string(epoch_);
            result += '!';
        }

        for (std::size_t i = 0; i < release_.size(); ++i) {
            if (i > 0) {
                result += '.';
            }
            result += std::to_string(release_[i]);
        }

        return result;
    }

    std::size_t Version::hashValue() const {
        std::size_t seed = 0;
        auto combine = [&seed](std::size_t h) {
            seed ^= h + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        };

        combine(std::hash<std::uint64_t>{}(epoch_));
        for (std::uint64_t part : strippedRelease_) {
            combine(std::hash<std::uint64_t>{}(part));
        }

        auto combineBound = [&](const PrePostDevBound& bound) {
            combine(std::hash<int>{}(static_cast<int>(bound.kind)));
            if (bound.kind == PrePostDevBound::Kind::Value) {
                combine(std::hash<std::string>{}(bound.letter));
                combine(std::hash<std::uint64_t>{}(bound.number));
            }
        };
        combineBound(preKey_);
        combineBound(postKey_);
        combineBound(devKey_);

        combine(std::hash<bool>{}(hasLocal_));
        if (hasLocal_) {
            for (const LocalSegment& segment : localSegments_) {
                combine(std::hash<bool>{}(segment.isNumeric));
                if (segment.isNumeric) {
                    combine(std::hash<std::uint64_t>{}(segment.number));
                } else {
                    combine(std::hash<std::string>{}(segment.text));
                }
            }
        }

        return seed;
    }
}


namespace std {
    std::size_t hash<AGRemapCore::Version>::operator()(const AGRemapCore::Version& version) const noexcept {
        return version.hashValue();
    }
}
