#ifndef AGRemapCore_ParseContext_H
#define AGRemapCore_ParseContext_H

#include <optional>
#include <string>
#include <vector>


namespace AGRemapCore {

    /**
     * @brief Context for parsing some text
     */
    class ParseContext {
        public:

            /**
             * @brief
             @rst
             Constructs a new parsing context from the source text :raw-html:`<br />` :raw-html:`<br />`

             The source text is split into :cpp:member:`lines` the same way `Python's str.splitlines`_ does
             @endrst
             *
             * @param src The source text to parse
             * @param file The file path to the source text
             * @param startLineNo The starting line of the source text
             */
            explicit ParseContext(std::string src = "", std::optional<std::string> file = std::nullopt, size_t startLineNo = 1);

            /**
             * @brief
             @rst
             Constructs a new parsing context, assuming the lines of the source text are already given
             @endrst
             *
             * @param lines The lines of the source text
             * @param file The file path to the source text
             * @param startLineNo The starting line of the source text
             */
            explicit ParseContext(std::vector<std::string> lines, std::optional<std::string> file = std::nullopt, size_t startLineNo = 1);

            /**
             * @brief Retrieves the line number after the last line
             */
            size_t getEndLineNo() const;

            /**
             * @brief The lines of the source text
             */
            std::vector<std::string> lines;

            /**
             * @brief The file path to the source text
             */
            std::optional<std::string> file;

            /**
             * @brief The starting line of the source text
             */
            size_t startLineNo;
    };
}

#endif
