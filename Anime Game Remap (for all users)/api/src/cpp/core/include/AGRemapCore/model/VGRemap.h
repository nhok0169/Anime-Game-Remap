#ifndef AGRemapCore_VGRemap_H
#define AGRemapCore_VGRemap_H

#include <optional>
#include <unordered_map>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Class for handling the vertex group remaps for mods
     @endrst
     */
    class VGRemap {
        public:

            /**
             * @brief Constructs a new vertex group remap
             *
             * @param remap The vertex group remap from one type of mod to another
             */
            explicit VGRemap(std::unordered_map<long long, long long> remap = {});

            /**
             * @brief The vertex group remap
             */
            const std::unordered_map<long long, long long>& getRemap() const;

            /**
             * @brief Sets the vertex group remap (recomputes #getMaxIndex)
             *
             * @param remap The new remap
             */
            void setRemap(std::unordered_map<long long, long long> remap);

            /**
             * @brief The maximum index in the vertex group remap, or ``std::nullopt`` if #getRemap is empty
             */
            std::optional<long long> getMaxIndex() const;

        private:
            std::unordered_map<long long, long long> remap_;
            std::optional<long long> maxIndex_;
    };
}

#endif
