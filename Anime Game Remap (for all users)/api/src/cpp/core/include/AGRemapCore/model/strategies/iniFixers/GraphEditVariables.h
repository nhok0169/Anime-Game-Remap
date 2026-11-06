#ifndef AGRemapCore_GraphEditVariables_H
#define AGRemapCore_GraphEditVariables_H
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
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/iftemplate/IfTemplate.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     Named variables that edits save data into, and read or write back out later -- the data a list of edits
     passes from one edit to another. A variable holds a value of any type (:cpp:class:`GraphEditVariables::Value`)
     :raw-html:`<br />` :raw-html:`<br />`

     A fixer keeps the `sections`_ it writes in one (:cpp:class:`GraphEditVariables::Sections`): an edit copies
     sections of a graph into it (:cpp:class:`GraphVarTake`), code adds sections it builds itself
     (:cpp:func:`Sections::add`), and an edit writes it out as a graph (:cpp:member:`GraphCreate::sectionsOf`). A
     parser's data keeps its variables too (:cpp:func:`IniParseData::variables`): each :cpp:class:`GraphCollect` files
     its rows into one, and a :cpp:class:`GraphVarApply` makes a variable from others (:cpp:func:`apply`)

     .. warning::
        A variable is a snapshot. An edit that changes a graph afterwards does not change what a variable copied from
        it holds, and an edit that changes the graph a variable was written to does not change the variable
     @endrst
     */
    class GraphEditVariables {
        public:
            /**
             * @brief A `section`_ held in a variable
             */
            using Section = IfTemplate<std::string, std::string>;

            /**
             * @brief
             @rst
             A variable holding the `sections`_ of one graph, in order :raw-html:`<br />` :raw-html:`<br />`

             Each :cpp:func:`add` is one place in the order. :cpp:func:`reserve` holds a place open so sections known
             only later (a :cpp:class:`GraphVarTake` that runs at fix time) land where the reservation was made
             rather than after everything added since
             @endrst
             */
            class Sections {
                public:
                    /**
                     * @brief A place :cpp:func:`reserve` held open: its index, and how many times the variable was
                     * cleared when it was held
                     */
                    using Place = std::pair<std::size_t, std::size_t>;

                    Sections() = default;
                    Sections(const Sections&) = delete;
                    Sections& operator=(const Sections&) = delete;

                    /**
                     * @brief Adds a section at the end
                     *
                     * @param section The section
                     */
                    void add(std::unique_ptr<Section> section);

                    /**
                     * @brief Holds a place open at the end, for sections added later with :cpp:func:`addAt`
                     *
                     * @return The place
                     */
                    Place reserve();

                    /**
                     * @brief
                     @rst
                     Adds a section at a place :cpp:func:`reserve` held, after the sections already added there --
                     or at the end, when the variable was cleared since the place was held
                     @endrst
                     *
                     * @param section The section
                     * @param place The place
                     */
                    void addAt(std::unique_ptr<Section> section, const Place& place);

                    /**
                     * @brief The sections, in order
                     */
                    std::vector<Section*> list() const;

                    /**
                     * @brief Whether the variable holds no section
                     */
                    bool empty() const;

                    /**
                     * @brief Drops every section; a place held before is held no more
                     */
                    void clear();

                private:
                    std::vector<std::vector<std::unique_ptr<Section>>> places_;
                    std::size_t cleared_ = 0;
            };

            /**
             * @brief
             @rst
             The value of a variable: an object of any type, or nothing :raw-html:`<br />` :raw-html:`<br />`

             A copy shares the object it holds rather than copying it, so a type that cannot be copied
             (:cpp:class:`GraphEditVariables::Sections`) can be held too
             @endrst
             */
            class Value {
                public:
                    /**
                     * @brief An empty value
                     */
                    Value() = default;

                    /**
                     * @brief A value holding 'object'
                     *
                     * @tparam T The object's type
                     * @param object The object
                     *
                     * @return The value
                     */
                    template <typename T>
                    static Value of(T object) {
                        Value value;
                        value.object_ = std::make_shared<T>(std::move(object));
                        value.type_ = std::type_index(typeid(T));
                        return value;
                    }

                    /**
                     * @brief A value holding a new object of type 'T', made from 'args'
                     *
                     * @tparam T The object's type
                     * @param args What the object is made from
                     *
                     * @return The value
                     */
                    template <typename T, typename... Args>
                    static Value make(Args&&... args) {
                        Value value;
                        value.object_ = std::make_shared<T>(std::forward<Args>(args)...);
                        value.type_ = std::type_index(typeid(T));
                        return value;
                    }

                    /**
                     * @brief The object, or ``nullptr`` when the value is empty or holds another type
                     *
                     * @tparam T The object's type
                     */
                    template <typename T>
                    T* as() const {
                        return type_ == std::type_index(typeid(T)) ? static_cast<T*>(object_.get()) : nullptr;
                    }

                    /**
                     * @brief Whether the value holds nothing
                     */
                    bool empty() const;

                    /**
                     * @brief The type of the object held -- ``void`` when the value is empty
                     */
                    std::type_index type() const;

                private:
                    std::shared_ptr<void> object_;
                    std::type_index type_ = std::type_index(typeid(void));
            };

            /**
             * @brief
             @rst
             A function :cpp:func:`apply` makes a variable's new value with: given the input variables' values, in
             order (``nullptr`` for a variable that does not exist), it returns the new value -- or nothing, to leave
             the variable as it is
             @endrst
             */
            using Apply = std::function<std::optional<Value>(const std::vector<const Value*>& inputs)>;

            GraphEditVariables() = default;
            GraphEditVariables(const GraphEditVariables&) = delete;
            GraphEditVariables& operator=(const GraphEditVariables&) = delete;
            GraphEditVariables(GraphEditVariables&&) = default;
            GraphEditVariables& operator=(GraphEditVariables&&) = default;

            /**
             * @brief Sets the variable named 'name' to 'value', replacing what it held
             *
             * @param name The variable's name
             * @param value The value
             */
            void set(const std::string& name, Value value);

            /**
             * @brief Sets the variable named 'name' to a value holding 'object', replacing what it held
             *
             * @tparam T The object's type
             * @param name The variable's name
             * @param object The object
             */
            template <typename T>
            void setObject(const std::string& name, T object) {
                set(name, Value::of<T>(std::move(object)));
            }

            /**
             * @brief The value of the variable named 'name', or ``nullptr`` if there is none
             *
             * @param name The variable's name
             *
             * @return The value
             */
            const Value* value(const std::string& name) const;

            /**
             * @brief The object the variable named 'name' holds, or ``nullptr`` if there is none or it holds another type
             *
             * @tparam T The object's type
             * @param name The variable's name
             *
             * @return The object
             */
            template <typename T>
            T* find(const std::string& name) const {
                const Value* found = value(name);
                return found == nullptr ? nullptr : found->template as<T>();
            }

            /**
             * @brief
             @rst
             The object the variable named 'name' holds :raw-html:`<br />` :raw-html:`<br />`

             Raises ``std::out_of_range`` naming the variable when there is none, or it holds another type
             @endrst
             *
             * @tparam T The object's type
             * @param name The variable's name
             *
             * @return The object
             */
            template <typename T>
            T& get(const std::string& name) const {
                T* found = find<T>(name);
                if (found == nullptr) {
                    throw std::out_of_range(has(name) ? "the variable '" + name + "' holds another type"
                                                      : "there is no variable '" + name + "'");
                }
                return *found;
            }

            /**
             * @brief Whether there is a variable named 'name'
             *
             * @param name The variable's name
             */
            bool has(const std::string& name) const;

            /**
             * @brief Removes the variable named 'name', if there is one
             *
             * @param name The variable's name
             */
            void erase(const std::string& name);

            /**
             * @brief The names of the variables, in the order they were made
             */
            std::vector<std::string> names() const;

            /**
             * @brief
             @rst
             Makes the variable named 'into' from the variables named 'inputs': 'fn' is given their values, in order
             (``nullptr`` for one that does not exist), and what it returns replaces the value of 'into'. When it
             returns nothing, 'into' is left as it is
             @endrst
             *
             * @param inputs The names of the variables read
             * @param fn The function
             * @param into The name of the variable written
             */
            void apply(const std::vector<std::string>& inputs, const Apply& fn, const std::string& into);

            /**
             * @brief
             @rst
             The sections variable named 'name', made empty if there is none :raw-html:`<br />` :raw-html:`<br />`

             Raises ``std::invalid_argument`` when the variable holds a value of another type
             @endrst
             *
             * @param name The variable's name
             *
             * @return The variable
             */
            Sections& sections(const std::string& name);

            /**
             * @brief The sections variable named 'name', or ``nullptr`` if there is none
             *
             * @param name The variable's name
             *
             * @return The variable
             */
            const Sections* findSections(const std::string& name) const;

            /**
             * @brief The names of the sections variables, in the order they were made
             */
            std::vector<std::string> sectionsNames() const;

            /**
             * @brief
             @rst
             Drops every section of every sections variable -- a place held before is held no more -- and every
             other variable. A sections variable stays, empty: an edit may hold a place in it
             @endrst
             */
            void clear();

        private:
            // the variables in the order they were made; a value shares its object, so an object keeps its address
            tsl::ordered_map<std::string, Value> values_;
    };
}

#endif
