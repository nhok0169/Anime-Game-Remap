namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool CallGraph<K, V, KeyHash, KeyEqual>::Node::operator==(const Node& other) const {
        return part == other.part && isExit == other.isExit;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    size_t CallGraph<K, V, KeyHash, KeyEqual>::NodeHash::operator()(const Node& node) const {
        size_t h = std::hash<const void*>()(static_cast<const void*>(node.part));
        // A simple, adequate mix -- matches the codebase's other ad-hoc pair-hash-combine helpers
        // (nothing here needs to be cryptographic, just spread the isExit bit out of the pointer's
        // own low, alignment-zeroed bits).
        return h ^ (node.isExit ? static_cast<size_t>(0x9e3779b97f4a7c15ULL) : static_cast<size_t>(0));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    CallGraph<K, V, KeyHash, KeyEqual>::CallGraph(EdgeMap forwardEdges, EdgeMap backwardEdges,
                                                   std::unordered_set<ContentPart*> parts, std::unordered_set<ContentPart*> rootNodes,
                                                   K runKey):
        forwardEdges_(std::move(forwardEdges)), backwardEdges_(std::move(backwardEdges)),
        parts_(std::move(parts)), rootNodes_(std::move(rootNodes)), runKey_(std::move(runKey)) {

    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const typename CallGraph<K, V, KeyHash, KeyEqual>::EdgeMap& CallGraph<K, V, KeyHash, KeyEqual>::forwardEdges() const {
        return forwardEdges_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const typename CallGraph<K, V, KeyHash, KeyEqual>::EdgeMap& CallGraph<K, V, KeyHash, KeyEqual>::backwardEdges() const {
        return backwardEdges_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const std::unordered_set<typename CallGraph<K, V, KeyHash, KeyEqual>::ContentPart*>& CallGraph<K, V, KeyHash, KeyEqual>::parts() const {
        return parts_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const std::unordered_set<typename CallGraph<K, V, KeyHash, KeyEqual>::ContentPart*>& CallGraph<K, V, KeyHash, KeyEqual>::rootNodes() const {
        return rootNodes_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename CallGraph<K, V, KeyHash, KeyEqual>::Node CallGraph<K, V, KeyHash, KeyEqual>::exitNodeOf(ContentPart* part) const {
        bool makesCall = part != nullptr && part->contains(runKey_);
        return Node{part, makesCall};
    }

}
