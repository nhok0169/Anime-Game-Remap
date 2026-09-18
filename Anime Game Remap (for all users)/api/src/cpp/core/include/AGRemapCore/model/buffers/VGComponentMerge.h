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

#ifndef AGRemapCore_VGComponentMerge_H
#define AGRemapCore_VGComponentMerge_H

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/model/VGRemap.h"
#include "AGRemapCore/model/buffers/BufValue.h"
#include "AGRemapCore/model/buffers/VGComponentSplit.h"

namespace AGRemapCore {
    /**
     * @brief
     @rst
     One SOURCE component of a skin made of several -- YelanTranquil's ``Body``, ``Bang`` or
     ``Eye`` -- and how its own vertex groups reach the target's bones
     @endrst
     */
    struct VGMergeComponentSpec {
        /**
         * @brief The component's name
         */
        std::string name;

        /**
         * @brief
         @rst
         This component's vertex group (source index) to the target's bone. Each component has a
         row of its OWN (``("YelanTranquil", "Bang") -> ("Yelan", "")``), which is the whole reason
         the components cannot be remapped together
         @endrst
         */
        VGRemap remap;
    };

    /**
     * @brief Counts worth reporting about a merge
     */
    struct VGComponentMergeStats {
        /**
         * @brief The merged vertex count -- every component's, summed
         */
        std::size_t vertexCount = 0;

        /**
         * @brief The component names, in merge order
         */
        std::vector<std::string> order;

        /**
         * @brief Per component, its first vertex in the merged buffers
         */
        std::vector<std::size_t> offsets;

        /**
         * @brief Per component, how many vertices it brought
         */
        std::vector<std::size_t> vertices;

        /**
         * @brief The merged ``Texcoord.buf``'s stride -- the widest component's
         */
        std::size_t texcoordStride = 0;

        /**
         * @brief How many components' texcoord lines had to be padded up to #texcoordStride
         */
        std::size_t paddedComponents = 0;

        /**
         * @brief
         @rst
         Every ``(component, vertex group)`` that carries weight and has no entry in that
         component's remap. Such a group becomes the ``-index-1`` sentinel, which is a BONE and not
         nothing: the model kinks there and no other part of the pipeline says a word about it
         @endrst
         */
        std::vector<std::pair<std::string, long long>> unmapped;
    };

    /**
     * @brief
     @rst
     Merges the geometry of a mod built for a skin of SEVERAL components onto a target of one --
     the inverse of :cpp:class:`VGComponentSplit` :raw-html:`<br />` :raw-html:`<br />`

     A single-component target draws through ONE set of buffer hashes, so the components' buffers
     have to become one set: several ``.ini`` files each binding that hash to different bytes does
     not work, because a hash binds once and the other components' index buffers then address the
     winner's vertices (measured in game, 2026-09-13) :raw-html:`<br />` :raw-html:`<br />`

     The components are laid end to end in the order given, so the first keeps its index buffers
     unchanged, and every later one's indices shift by the vertices before it. Each component's
     blend is remapped through its OWN row before the concatenation. Two source objects landing on
     the same target object -- YelanTranquil's ``Bang`` and ``Eye`` both onto Yelan's head -- have
     their index buffers concatenated into one draw, which is what removes the last reason to write
     a second ``.ini`` file :raw-html:`<br />` :raw-html:`<br />`

     Like the split, the buffers cannot be done one at a time -- the index buffers' offsets come
     from the other components' vertex counts -- so this is a grouped resource's job
     (:cpp:class:`VGMergeGroupResource`)
     @endrst
     */
    class VGComponentMerge {
        public:
            using Weights = VGComponentSplit::Weights;
            using Indices = VGComponentSplit::Indices;
            using Triangles = VGComponentSplit::Triangles;

            /**
             * @brief One source component's decoded blend and its raw vertex buffers
             */
            struct Component {
                VGMergeComponentSpec spec;

                /**
                 * @brief Per vertex, its 4 blend weights
                 */
                Weights weights;

                /**
                 * @brief Per vertex, its 4 vertex group indices, in THIS component's numbering
                 */
                Indices indices;

                /**
                 * @brief The whole ``Position.buf``, copied through unchanged
                 */
                ByteVec position;

                /**
                 * @brief The whole ``Texcoord.buf``, padded up to the widest component's stride
                 */
                ByteVec texcoord;
            };

            /**
             * @brief Merges the components, in the order given
             *
             * @param components The source's components; the first takes offset 0
             *
             * @throw std::invalid_argument If there are no components, two share a name, or a
             *                              buffer is not a whole number of lines for its vertices
             */
            explicit VGComponentMerge(std::vector<Component> components);

            /**
             * @brief
             @rst
             One component's bone indices through its remap :raw-html:`<br />` :raw-html:`<br />`

             A slot is remapped only where it carries WEIGHT -- a zero-weight slot keeps its literal
             index, so ``[0, 0, 0, 0]`` with weights ``[1, 0, 0, 0]`` comes out ``[64, 0, 0, 0]``
             and not ``[64, 64, 64, 64]``. That is what the rest of the library does, and the
             difference is invisible except as a byte diff
             @endrst
             *
             * @param indices The component's bone indices, one line of four per vertex
             * @param weights The matching blend weights, used to tell a carried slot from padding
             * @param remap The component's own reverse row, source group -> target group
             * @param unmapped Every weighted group with no entry is appended here, ascending
             */
            static Indices remapIndices(const Indices& indices, const Weights& weights, const VGRemap& remap,
                                        std::vector<long long>& unmapped);

            /**
             * @brief
             @rst
             A fixed-stride buffer widened, every line zero-padded at the END -- which is where a
             missing ``TEXCOORD1`` sits, so a component carrying only one UV set lines up with the
             ones that carry two
             @endrst
             *
             * @throw std::invalid_argument If the source is not a whole number of lines, or is wider than 'to'
             */
            static ByteVec padLines(const ByteVec& src, std::size_t lines, std::size_t to);

            /**
             * @brief The merged ``Blend.buf``
             */
            const ByteVec& blend() const;

            /**
             * @brief The merged ``Position.buf``
             */
            const ByteVec& position() const;

            /**
             * @brief The merged ``Texcoord.buf``
             */
            const ByteVec& texcoord() const;

            /**
             * @brief
             @rst
             One target object's index buffer: each member's triangles offset by its component's
             first vertex, concatenated in the order given
             @endrst
             *
             * @param members The source objects drawn through this target object, as ``(component, triangles)``
             *
             * @throw std::invalid_argument If a member names a component that was not merged
             */
            Triangles mergeIbs(const std::vector<std::pair<std::string, Triangles>>& members) const;

            /**
             * @brief A component's first vertex in the merged buffers
             *
             * @throw std::invalid_argument If no component has that name
             */
            std::size_t offsetOf(const std::string& component) const;

            std::size_t vertexCount() const;
            const VGComponentMergeStats& stats() const;

        private:
            ByteVec blend_;
            ByteVec position_;
            ByteVec texcoord_;
            std::unordered_map<std::string, std::size_t> offsets_;
            VGComponentMergeStats stats_;
    };
}

#endif
