namespace AGRemapCore {

    template <typename Id, typename IdHash, typename IdEq>
    ParseTree<Id, IdHash, IdEq>::ParseTree(Nodes nodes, Children children, Id rootId):
        nodes(std::move(nodes)), children(std::move(children)), rootId_(std::move(rootId))
    {
        validateRootId(rootId_);
    }

    template <typename Id, typename IdHash, typename IdEq>
    void ParseTree<Id, IdHash, IdEq>::validateRootId(const Id& newRootId) const {
        if (nodes.find(newRootId) == nodes.end()) {
            throw std::invalid_argument("ParseTree: the new root id does not reference an existing node in the parse tree");
        }
    }

    template <typename Id, typename IdHash, typename IdEq>
    const Id& ParseTree<Id, IdHash, IdEq>::rootId() const {
        return rootId_;
    }

    template <typename Id, typename IdHash, typename IdEq>
    void ParseTree<Id, IdHash, IdEq>::setRootId(Id newRootId) {
        validateRootId(newRootId);
        rootId_ = std::move(newRootId);
    }

    template <typename Id, typename IdHash, typename IdEq>
    const ParseNode<Id>* ParseTree<Id, IdHash, IdEq>::getNode(const Id& nodeId, bool errorOnNotFound) const {
        auto it = nodes.find(nodeId);
        if (it != nodes.end()) {
            return &it->second;
        }

        if (!errorOnNotFound) {
            return nullptr;
        }

        throw std::out_of_range("ParseTree::getNode: no node found for the given id");
    }

    template <typename Id, typename IdHash, typename IdEq>
    bool ParseTree<Id, IdHash, IdEq>::isChild(const Id& nodeId) const {
        if (nodes.find(nodeId) == nodes.end()) {
            return false;
        }

        return children.find(nodeId) == children.end();
    }
}
