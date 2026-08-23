#include "AGRemapCore/tools/idGenerator/IncIdGenerator.h"


namespace AGRemapCore {

    namespace {
        // One counter per <K, V, KeyHash, KeyEqual> instantiation -- doesn't need to be shared
        // with IfTemplatePart's own counter (different id namespace entirely: node ids are only
        // ever compared as keys within one node's own 'children' map, never against a part's id).
        template <typename K, typename V, typename KeyHash, typename KeyEqual>
        IncIdGenerator<size_t>& ifTemplateNodeIdGenerator() {
            static IncIdGenerator<size_t> generator(0);
            return generator;
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    size_t IfTemplateNode<K, V, KeyHash, KeyEqual>::generateId() {
        size_t result;
        ifTemplateNodeIdGenerator<K, V, KeyHash, KeyEqual>().getId(result);
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    IfTemplateNode<K, V, KeyHash, KeyEqual>::IfTemplateNode(const std::optional<size_t>& id, IfPredPart* ifPredPart):
        Node<size_t>(id.has_value() ? *id : generateId()), ifPredPart(ifPredPart) {

    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfTemplateNode<K, V, KeyHash, KeyEqual>::addChild(IfTemplateNode<K, V, KeyHash, KeyEqual>* node) {
        parts_.emplace_back(node);
        children_[node->id()] = node;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfTemplateNode<K, V, KeyHash, KeyEqual>::addIfContentPart(ContentPart* part) {
        parts_.emplace_back(part);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const std::unordered_map<size_t, IfTemplateNode<K, V, KeyHash, KeyEqual>*>& IfTemplateNode<K, V, KeyHash, KeyEqual>::children() const {
        return children_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const std::vector<typename IfTemplateNode<K, V, KeyHash, KeyEqual>::PartsElement>& IfTemplateNode<K, V, KeyHash, KeyEqual>::parts() const {
        return parts_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<typename IfTemplateNode<K, V, KeyHash, KeyEqual>::PartsElement>& IfTemplateNode<K, V, KeyHash, KeyEqual>::parts() {
        return parts_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool IfTemplateNode<K, V, KeyHash, KeyEqual>::hasKey(const K& key) const {
        for (const PartsElement& part : parts_) {
            const ContentPart* const* contentPart = std::get_if<ContentPart*>(&part);
            if (contentPart == nullptr) {
                continue;
            }

            if ((*contentPart)->contains(key)) {
                return true;
            }
        }

        return false;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename IfTemplateNode<K, V, KeyHash, KeyEqual>::ContentPart* IfTemplateNode<K, V, KeyHash, KeyEqual>::getKeyPart(const K& key) const {
        for (auto it = parts_.rbegin(); it != parts_.rend(); ++it) {
            ContentPart* const* contentPart = std::get_if<ContentPart*>(&(*it));
            if (contentPart == nullptr) {
                continue;
            }

            if ((*contentPart)->contains(key)) {
                return *contentPart;
            }
        }

        return nullptr;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<V> IfTemplateNode<K, V, KeyHash, KeyEqual>::getKeyVal(const K& key) const {
        ContentPart* part = getKeyPart(key);
        if (part == nullptr) {
            return std::nullopt;
        }

        // Intentional divergence from the pure-Python original's own 'part[key][-1][1]': that
        // expression relied on the deprecated pure-Python IfContentPart's __getitem__ returning a
        // list of (index, value) pairs. The current, live IfContentPart's __getitem__/getVals
        // return plain values (see PyIfContentPart.cpp), so the literal expression would actually
        // be broken today (indexing into a value, not a pair) -- confirmed this method is dead
        // code with no real call site left anywhere in the codebase (grepped), so there's no
        // observed behavior to preserve byte-for-byte here. This implements the clearly-intended
        // semantics instead: the latest value for 'key', in true positional order.
        std::vector<V> vals = part->getVals(key, true);
        if (vals.empty()) {
            return std::nullopt;
        }

        return vals.back();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<std::vector<std::pair<long long, V>>> IfTemplateNode<K, V, KeyHash, KeyEqual>::getKeyValues(const K& key) const {
        std::vector<std::vector<std::pair<long long, V>>> result;

        for (const PartsElement& part : parts_) {
            ContentPart* const* contentPart = std::get_if<ContentPart*>(&part);
            if (contentPart == nullptr || !(*contentPart)->contains(key)) {
                continue;
            }

            result.push_back((*contentPart)->getValsWithInds(key));
        }

        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::pair<typename IfTemplateNode<K, V, KeyHash, KeyEqual>::ContentPart*, bool> IfTemplateNode<K, V, KeyHash, KeyEqual>::getKeyMissingPart(const K& key) const {
        ContentPart* result = nullptr;
        bool hasContentPart = false;

        for (const PartsElement& part : parts_) {
            ContentPart* const* contentPart = std::get_if<ContentPart*>(&part);
            if (contentPart == nullptr) {
                continue;
            }

            if (!hasContentPart) {
                hasContentPart = true;
            }

            if ((*contentPart)->contains(key)) {
                result = nullptr;
                break;
            }

            if (result == nullptr) {
                result = *contentPart;
            }
        }

        return std::make_pair(result, hasContentPart);
    }

}
