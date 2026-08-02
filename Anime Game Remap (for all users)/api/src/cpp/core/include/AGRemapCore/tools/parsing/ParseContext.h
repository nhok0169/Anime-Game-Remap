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
             * @brief Constructs a new parsing context
             * 
             * @param src The source for the parse context
             * 
             * @param file The file path to the source text
             * 
             * @param startLineNo The starting line of the source text
             */
            ParseContext(std::string_view src, std::optional<std::string_view> file = std::nullopt, size_t startLineNo = 1);

            /**
             * @brief Retrieves the line number after the last line
             */
            size_t getEndLineNo();
        
        protected:

            /**
             * @brief The lines of the source text
             */
            std::vector<std::string_view> lines;

            /**
             * @brief The file path to the source text
             */
            std::optional<std::string_view> file;

            /**
             * @brief The starting line of the source text
             */
            size_t startLineNo;
    };
}

#endif