namespace AGRemapCore {

    template <typename Id>
    Node<Id>::Node(Id id): id_(std::move(id)) {

    }

    template <typename Id>
    const Id& Node<Id>::id() const {
        return id_;
    }
}
