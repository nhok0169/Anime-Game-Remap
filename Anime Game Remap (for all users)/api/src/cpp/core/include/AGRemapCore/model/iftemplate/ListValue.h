#ifndef AGRemapCore_ListValue_H
#define AGRemapCore_ListValue_H

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

#include <cstddef>
#include <optional>
#include <string>
#include <vector>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The value of a `KVP`_ read as a list of fields: ``a``, ``a, b`` or ``a, b, c, d, e, f, g, h`` -- whatever
     length the value has :raw-html:`<br />` :raw-html:`<br />`

     A value with no separator is a list of one field, so every `KVP`_ is one: ``vb0 = ResourceX`` and
     ``drawindexed = 12, 0, 0`` are read the same way. An empty value has no fields, and a trailing separator
     gives an empty last field (``a, b,`` is three fields). Each field is held without its surrounding
     whitespace :raw-html:`<br />` :raw-html:`<br />`

     A field is addressed by its position, counting from ``0``; a negative position counts from the end, so
     ``-1`` is the last field whatever the length. Reading a field past either end gives ``std::nullopt``
     rather than an error :raw-html:`<br />` :raw-html:`<br />`

     #str writes the value back: exactly as it was given while no field has been set, and otherwise its
     fields joined with the separator and a space

     .. code-block:: ini

        drawindexed = 12, 30, 0     ; size 3, number(1) = 30
        drawindexed = auto          ; size 1, number(0) = nullopt
        match_first_index = 0       ; size 1, number(0) = 0
     @endrst
     */
    class ListValue {
        public:
            /**
             * @brief Reads a value as a list
             *
             * @param value The value of the `KVP`_
             * @param separator The character the fields are separated by. **Default**: ``,``
             */
            explicit ListValue(std::string value = "", char separator = ',');

            /**
             * @brief How many fields the value has -- ``0`` for an empty value
             */
            std::size_t size() const;

            /**
             * @brief Every field, in order, without its surrounding whitespace
             */
            const std::vector<std::string>& fields() const;

            /**
             * @brief
             @rst
             The field at 'pos', counting from the end when 'pos' is negative, or ``std::nullopt`` past either end
             @endrst
             *
             * @param pos The field's position
             */
            std::optional<std::string> at(long long pos) const;

            /**
             * @brief
             @rst
             The field at 'pos' as a whole number -- an optional ``-`` followed by digits only -- or ``std::nullopt``
             for any other field (``auto``, ``$var``, ``1.5``, an empty field) or a position past either end
             @endrst
             *
             * @param pos The field's position
             */
            std::optional<long long> number(long long pos) const;

            /**
             * @brief
             @rst
             Every field as a whole number (see #number), or ``std::nullopt`` when the value is empty or any one of its
             fields is not one
             @endrst
             */
            std::optional<std::vector<long long>> numbers() const;

            /**
             * @brief Sets the field at 'pos', counting from the end when 'pos' is negative
             *
             * @param pos The field's position
             * @param field The new field, held without its surrounding whitespace
             *
             * @return Whether 'pos' named a field: a position past either end changes nothing
             */
            bool set(long long pos, const std::string& field);

            /**
             * @brief Sets the field at 'pos' to a whole number -- see #set
             *
             * @param pos The field's position
             * @param number The new field
             *
             * @return Whether 'pos' named a field
             */
            bool setNumber(long long pos, long long number);

            /**
             * @brief Adds a field at the end
             *
             * @param field The new field, held without its surrounding whitespace
             */
            void append(const std::string& field);

            /**
             * @brief Whether any field has been set or added since the value was read
             */
            bool changed() const;

            /**
             * @brief
             @rst
             The value: as it was given when nothing has changed, otherwise every field joined with the separator and a
             space (``a, b, c``)
             @endrst
             */
            std::string str() const;

        private:
            std::optional<std::size_t> indexOf(long long pos) const;

            std::string original_;
            char separator_;
            std::vector<std::string> fields_;
            bool changed_ = false;
    };
}

#endif
