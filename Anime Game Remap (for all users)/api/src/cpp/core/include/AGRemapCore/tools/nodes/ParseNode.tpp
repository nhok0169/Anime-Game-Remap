namespace AGRemapCore {

    template <typename Id>
    ParseNode<Id>::ParseNode(Id id, std::optional<Id> prodId, std::optional<Token> token):
        Node<Id>(std::move(id)), prodId(std::move(prodId)), token(std::move(token))
    {

    }
}
