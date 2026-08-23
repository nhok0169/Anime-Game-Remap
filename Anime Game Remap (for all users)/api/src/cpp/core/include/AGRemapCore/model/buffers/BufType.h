#ifndef AGRemapCore_BufType_H
#define AGRemapCore_BufType_H

#include <string>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The common base for any type used to describe the structure of a ``.buf`` file -- either an
     elementary data type (:cpp:class:`BufDataType`) or the composite element built out of a
     sequence of them (:cpp:class:`BufElementType`) :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The pure-Python original also declares ``decode``/``encode`` on this base class, but
        :cpp:class:`BufDataType`'s decode/encode work on a single :cpp:type:`BufValue` while
        :cpp:class:`BufElementType`'s work on a ``std::vector<BufValue>`` -- two genuinely
        incompatible signatures that Python's duck typing lets slide, but a statically-typed
        virtual interface cannot. This base only formalizes what both branches actually share (a
        ``name``); each branch declares its own decode/encode shape at the point the two diverge
     @endrst
     */
    class BufType {
        public:

            /**
             * @brief Constructs a new type descriptor
             *
             * @param name The name of the type
             */
            explicit BufType(std::string name);

            virtual ~BufType() = default;

            /**
             * @brief The name of the type
             */
            const std::string& getName() const;

            /**
             * @brief Sets the name of the type
             *
             * @param name The new name
             */
            void setName(std::string name);

        private:
            std::string name_;
    };
}

#endif
