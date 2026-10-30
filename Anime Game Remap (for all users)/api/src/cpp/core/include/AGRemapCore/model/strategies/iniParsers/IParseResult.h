#ifndef AGRemapCore_IParseResult_H
#define AGRemapCore_IParseResult_H

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

    class IniParseData;

    /**
     * @brief
     @rst
     An edit with a result a parser keeps: after running an edit of :cpp:member:`GIMIParser::parseSteps` or
     :cpp:member:`GIMIParser::deferredParseSteps`, the parser hands its :cpp:class:`IniParseData` to the edit's
     #fileInto when the edit implements this :raw-html:`<br />` :raw-html:`<br />`

     :cpp:class:`GraphCollect` files its rows this way
     @endrst
     */
    class IParseResult {
        public:
            virtual ~IParseResult() = default;

            /**
             * @brief Files what the last run found into 'data'
             *
             * @param data The parser's data
             */
            virtual void fileInto(IniParseData& data) const = 0;
    };
}

#endif
