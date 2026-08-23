#include <deque>
#include <string>
#include <utility>

namespace AGRemapCore {

    namespace {
        // Equivalent of the pure-Python original's 're.match(r"^[( |\t)]*", part.src)': the
        // longest prefix of 's' made up only of the characters '(', ' ', '|', '\t', ')'. Since a
        // '*'-quantified regex always matches (possibly zero characters) at position 0, the
        // Python original's own 'if (linePrefix): ... else: linePrefix = ""' branch is actually
        // unreachable (a re.Match object is truthy even for a zero-length match) -- this directly
        // implements the one reachable behavior, a plain longest-matching-prefix scan.
        inline std::string ifTemplateTreeLinePrefix(const std::string& s) {
            static const std::string allowedChars = "( |\t)";
            size_t n = 0;
            while (n < s.size() && allowedChars.find(s[n]) != std::string::npos) {
                ++n;
            }
            return s.substr(0, n);
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename IfTemplateTree<K, V, KeyHash, KeyEqual>::NodeType* IfTemplateTree<K, V, KeyHash, KeyEqual>::root() const {
        return root_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfTemplateTree<K, V, KeyHash, KeyEqual>::clear() {
        root_ = nullptr;
        nodePool_.clear();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename IfTemplateTree<K, V, KeyHash, KeyEqual>::NodeType* IfTemplateTree<K, V, KeyHash, KeyEqual>::makeNode(IfPredPart* ifPredPart) {
        nodePool_.push_back(std::make_unique<NodeType>(std::nullopt, ifPredPart));
        return nodePool_.back().get();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfTemplateTree<K, V, KeyHash, KeyEqual>::setRoot(NodeType* node) {
        root_ = node;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<IfTemplateTree<K, V, KeyHash, KeyEqual>> IfTemplateTree<K, V, KeyHash, KeyEqual>::construct(
            const std::vector<std::unique_ptr<IfTemplatePart>>& parts) {
        using Node = NodeType;
        using ContentPart = typename Node::ContentPart;

        auto result = std::make_unique<IfTemplateTree<K, V, KeyHash, KeyEqual>>();

        Node* node = result->makeNode(nullptr);
        Node* root = node;
        std::deque<Node*> nodeStack;
        size_t partsLen = parts.size();

        for (size_t i = 0; i < partsLen; ++i) {
            IfTemplatePart* partBase = parts[i].get();

            if (auto* contentPart = dynamic_cast<ContentPart*>(partBase)) {
                node->addIfContentPart(contentPart);
                continue;
            }

            // Assumes well-formed input (a part is always either a ContentPart or an IfPredPart)
            // -- the same assumption the pure-Python original makes (no validation/else branch
            // either).
            auto* predPart = dynamic_cast<IfPredPart*>(partBase);
            IfPredPartType predType = predPart->type;

            if (predType == IfPredPartType::If) {
                nodeStack.push_back(node);
                node = result->makeNode(predPart);
                continue;
            }

            bool isChild = !nodeStack.empty();
            if (!isChild) {
                continue;
            }

            Node* parent = nodeStack.back();
            parent->addChild(node);

            if (predType == IfPredPartType::EndIf) {
                node = nodeStack.back();
                nodeStack.pop_back();
            } else if (predType == IfPredPartType::Else || predType == IfPredPartType::Elif) {
                node = result->makeNode(predPart);
            }
        }

        result->setRoot(root);
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<IfTemplateNonEmptyNodeTree<K, V, KeyHash, KeyEqual>> IfTemplateNonEmptyNodeTree<K, V, KeyHash, KeyEqual>::construct(
            std::vector<std::unique_ptr<IfTemplatePart>>& parts) {
        using Node = typename IfTemplateTree<K, V, KeyHash, KeyEqual>::NodeType;
        using ContentPart = typename Node::ContentPart;

        auto result = std::make_unique<IfTemplateNonEmptyNodeTree<K, V, KeyHash, KeyEqual>>();

        Node* node = result->makeNode(nullptr);
        Node* root = node;
        std::deque<Node*> nodeStack;
        size_t partsLen = parts.size();
        int depth = 0;
        size_t i = 0;

        while (i < partsLen) {
            IfTemplatePart* partBase = parts[i].get();

            if (auto* contentPart = dynamic_cast<ContentPart*>(partBase)) {
                node->addIfContentPart(contentPart);
                ++i;
                continue;
            }

            auto* predPart = dynamic_cast<IfPredPart*>(partBase);
            IfPredPartType predType = predPart->type;

            if (predType == IfPredPartType::If) {
                nodeStack.push_back(node);
                node = result->makeNode(predPart);
                ++depth;
                ++i;
                continue;
            }

            bool isChild = !nodeStack.empty();
            if (!isChild) {
                ++i;
                continue;
            }

            Node* parent = nodeStack.back();
            parent->addChild(node);

            if (predType == IfPredPartType::EndIf) {
                if (node->parts().empty()) {
                    // Matches the pure-Python original's own identical check in
                    // IfTemplateNonEmptyNodeTreeOld.construct ('if (not node.parts): ...') --
                    // *that* class, not the plain IfTemplateTreeOld, is what IfTemplateOld actually
                    // used by default for '.tree' (see IfTemplateOld's 'treeCls' default), so this
                    // synthesized empty placeholder is not a new behavior introduced by the port.
                    auto placeholderOwned = std::make_unique<ContentPart>(depth);
                    ContentPart* placeholderPtr = placeholderOwned.get();

                    // Only the just-heap-allocated *pointee* (placeholderPtr) needs to stay valid
                    // after this insert -- a possible reallocation here only moves the vector's
                    // own unique_ptr *slots* around, never the objects they point to. See
                    // IfTemplateNode.h's own top-level note and Architecture.md's
                    // vector-reallocation gotcha for why this distinction matters.
                    parts.insert(parts.begin() + static_cast<std::ptrdiff_t>(i), std::move(placeholderOwned));
                    node->addIfContentPart(placeholderPtr);
                    ++i;
                    ++partsLen;
                }

                node = nodeStack.back();
                nodeStack.pop_back();
                --depth;
            } else if (predType == IfPredPartType::Else || predType == IfPredPartType::Elif) {
                node = result->makeNode(predPart);
            }

            ++i;
        }

        result->setRoot(root);
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<IfTemplateNormTree<K, V, KeyHash, KeyEqual>> IfTemplateNormTree<K, V, KeyHash, KeyEqual>::construct(
            std::vector<std::unique_ptr<IfTemplatePart>>& parts) {
        using Node = typename IfTemplateTree<K, V, KeyHash, KeyEqual>::NodeType;
        using ContentPart = typename Node::ContentPart;

        auto result = std::make_unique<IfTemplateNormTree<K, V, KeyHash, KeyEqual>>();

        Node* node = result->makeNode(nullptr);
        Node* root = node;
        std::deque<Node*> nodeStack;
        size_t partsLen = parts.size();
        bool elseEncountered = false;
        std::deque<bool> elseEncounteredStack;
        int depth = 0;
        size_t i = 0;

        while (i < partsLen) {
            IfTemplatePart* partBase = parts[i].get();

            if (auto* contentPart = dynamic_cast<ContentPart*>(partBase)) {
                node->addIfContentPart(contentPart);
                ++i;
                continue;
            }

            auto* predPart = dynamic_cast<IfPredPart*>(partBase);
            IfPredPartType predType = predPart->type;

            if (predType == IfPredPartType::If) {
                nodeStack.push_back(node);
                node = result->makeNode(predPart);
                elseEncounteredStack.push_back(elseEncountered);
                elseEncountered = false;
                ++depth;
                ++i;
                continue;
            }

            bool isChild = !nodeStack.empty();
            if (!isChild) {
                ++i;
                continue;
            }

            Node* parent = nodeStack.back();
            parent->addChild(node);

            if (predType == IfPredPartType::EndIf) {
                node = nodeStack.back();
                nodeStack.pop_back();
                elseEncountered = elseEncounteredStack.back();
                elseEncounteredStack.pop_back();

                // construct the 'empty else' if an 'else' has not been encountered
                if (!elseEncountered) {
                    std::string linePrefix = ifTemplateTreeLinePrefix(predPart->src);

                    // A throwaway Z3Context is fine here -- this synthetic 'else' part's query is
                    // always just the literal 'true' (see IfPredPart's own Else handling), which
                    // never needs to share variable identity with anything else -- matching the
                    // pure-Python original's own identical reasoning/comment. The underlying Z3
                    // context stays alive via the resulting Z3Predicate's own shared_ptr (see
                    // Architecture.md's Z3Context/Z3Predicate lifetime section), so this local
                    // Z3Context can safely go out of scope once construction finishes.
                    Z3Context tempZ3Ctx;
                    auto emptyElseOwned = std::make_unique<IfPredPart>(linePrefix + "else\n", IfPredPartType::Else, tempZ3Ctx);
                    IfPredPart* emptyElsePtr = emptyElseOwned.get();

                    auto emptyElseContentOwned = std::make_unique<ContentPart>(depth);
                    ContentPart* emptyElseContentPtr = emptyElseContentOwned.get();

                    Node* emptyElseChild = result->makeNode(emptyElsePtr);
                    emptyElseChild->addIfContentPart(emptyElseContentPtr);

                    // Same insertion order as the pure-Python original: insert the content part at
                    // 'i' first, then the predicate part at 'i' again -- net result, in order,
                    // [emptyElse, emptyElseContent] starting at 'i'.
                    auto contentIt = parts.begin() + static_cast<std::ptrdiff_t>(i);
                    parts.insert(contentIt, std::move(emptyElseContentOwned));
                    auto predIt = parts.begin() + static_cast<std::ptrdiff_t>(i);
                    parts.insert(predIt, std::move(emptyElseOwned));

                    node->addChild(emptyElseChild);

                    i += 2;
                    partsLen += 2;
                }

                --depth;
                elseEncountered = false;
            } else if (predType == IfPredPartType::Else || predType == IfPredPartType::Elif) {
                node = result->makeNode(predPart);

                if (predType == IfPredPartType::Else) {
                    elseEncounteredStack.back() = true;
                }
            }

            ++i;
        }

        result->setRoot(root);
        return result;
    }

}
