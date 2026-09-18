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

namespace AGRemapCore {

    template <typename Id>
    ParseNode<Id>::ParseNode(Id id, std::optional<Id> prodId, std::optional<Token> token):
        Node<Id>(std::move(id)), prodId(std::move(prodId)), token(std::move(token))
    {

    }
}
