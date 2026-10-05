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

#include "AGRemapCore/model/files/TexCache.h"

#include <cstdlib>
#include <utility>

namespace AGRemapCore {

    TexCache::TexCache(std::size_t maxBytes): maxBytes_(maxBytes) {
        // An A/B switch, kept deliberately: the only trustworthy way to measure what this cache
        // is worth is to run it against ITSELF in one binary. Comparing two builds cannot separate
        // the cache from everything else that moved between them, and this machine's run-to-run
        // spread on a large mod (96-158s on the same work) is wider than the effect being measured.
        const char* off = std::getenv("AGREMAP_TEXCACHE");
        if (off != nullptr && off[0] == '0') {
            enabled_ = false;
            maxBytes_ = 0;
        }
    }

    const TexCache::Decoded* TexCache::decoded(const Hash128& source) {
        if (!enabled_) {
            return nullptr;
        }

        auto found = decoded_.find(source);
        if (found == decoded_.end()) {
            ++decodeMisses_;
            return nullptr;
        }

        // Most-recently-used, so the textures a mod keeps coming back to are the ones that survive
        // eviction. splice moves the node rather than reallocating it, which keeps every other
        // entry's stored iterator valid -- erasing and re-pushing would not.
        order_.splice(order_.begin(), order_, found->second.order);
        found->second.order = order_.begin();

        ++decodeHits_;
        return &found->second.value;
    }

    void TexCache::rememberDecoded(const Hash128& source, Decoded value) {
        if (maxBytes_ == 0) {
            return;
        }

        const std::size_t size = value.pixels.size();

        // A single texture bigger than the whole budget is not cached at all, rather than
        // evicting everything else to hold one image that the next put would throw away again.
        if (size > maxBytes_) {
            return;
        }

        auto found = decoded_.find(source);
        if (found != decoded_.end()) {
            bytes_ -= found->second.value.pixels.size();
            order_.splice(order_.begin(), order_, found->second.order);
            found->second.order = order_.begin();
            found->second.value = std::move(value);
            bytes_ += size;
            evict();
            return;
        }

        order_.push_front(source);
        decoded_.emplace(source, Entry{std::move(value), order_.begin()});
        bytes_ += size;
        evict();
    }

    void TexCache::evict() {
        while (bytes_ > maxBytes_ && !order_.empty()) {
            const Hash128 oldest = order_.back();
            auto found = decoded_.find(oldest);
            if (found != decoded_.end()) {
                bytes_ -= found->second.value.pixels.size();
                decoded_.erase(found);
            }
            order_.pop_back();
        }
    }

    std::optional<std::string> TexCache::writtenAs(const Hash128& output) {
        if (!enabled_) {
            return std::nullopt;
        }

        auto found = written_.find(output);
        if (found == written_.end()) {
            return std::nullopt;
        }

        ++writeHits_;
        return found->second;
    }

    void TexCache::rememberWritten(const Hash128& output, std::string path) {
        // Counted here rather than in writtenAs: a miss is a write we are about to actually
        // perform, and writtenAs is also called on paths that never reach a write.
        ++writeMisses_;

        // A destination can be written more than once in a run, with DIFFERENT pixels the second
        // time. The entry that named this path for the old image now describes bytes the file no
        // longer holds, so a later hit on it would copy the wrong texture -- drop it.
        auto owner = pathOwner_.find(path);
        if (owner != pathOwner_.end()) {
            if (!(owner->second == output)) {
                written_.erase(owner->second);
            }
            pathOwner_.erase(owner);
        }

        pathOwner_.emplace(path, output);
        written_.insert_or_assign(output, std::move(path));
    }

    void TexCache::forgetWritten(const Hash128& output) {
        auto found = written_.find(output);
        if (found != written_.end()) {
            pathOwner_.erase(found->second);
            written_.erase(found);
        }
    }

    std::size_t TexCache::getDecodeHits() const {
        return decodeHits_;
    }

    std::size_t TexCache::getDecodeMisses() const {
        return decodeMisses_;
    }

    std::size_t TexCache::getWriteHits() const {
        return writeHits_;
    }

    std::size_t TexCache::getWriteMisses() const {
        return writeMisses_;
    }

    std::size_t TexCache::getBytes() const {
        return bytes_;
    }
}
