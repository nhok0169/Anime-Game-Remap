#ifndef AGRemapCore_ParseNode_H
#define AGRemapCore_ParseNode_H

#include <optional>
#include <utility>

#include "AGRemapCore/tools/nodes/Node.h"
#include "AGRemapCore/tools/parsing/Token.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`Node`

     A node within a parse tree, created from a parser that interprets some `CFG`_
     @endrst
     *
     * @tparam Id The type for the id of the node, and for the id of a chosen production (#prodId)
     */
    template <typename Id>
    class ParseNode : public Node<Id> {
        public:

            /**
             * @brief Constructs a new parse node
             *
             * @param id The id for the node
             * @param prodId The id for the chosen production from the `CFG`_, if this node was
             *      created from reducing a production rather than shifting a token
             * @param token The token that this node references, if this node was created from
             *      shifting a token rather than reducing a production
             */
            explicit ParseNode(Id id, std::optional<Id> prodId = std::nullopt, std::optional<Token> token = std::nullopt);

            /**
             * @brief The id for the chosen production from the `CFG`_
             */
            std::optional<Id> prodId;

            /**
             * @brief The token that this node references
             */
            std::optional<Token> token;
    };
}

#include "ParseNode.tpp"

#endif
