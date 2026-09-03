#ifndef AGRemapCore_Heading_H
#define AGRemapCore_Heading_H

#include <cstddef>
#include <string>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     A heading for pretty printing :raw-html:`<br />` :raw-html:`<br />`

     .. code-block::

        ======= Title: Fix Raiden Boss 2 =======
        ...
        ========================================

     Line 1 is #open, line 3 is #close

     :raw-html:`<br />`

     .. note::
        A direct port of the pure-Python ``Heading`` (``tools/Heading.py``), which is still what
        the `Python`_ side's constants (``IniConsts``, ``ModTypes``) use -- this class has no
        `pybind11`_ binding of its own. It exists because :cpp:class:`RemapIniFixContext` needs
        to build the very same header/footer with no `Python`_ around, and because
        :cpp:class:`BaseLogger` renders its heading stack with it
     @endrst
     */
    class Heading {
        public:

            /**
             * @brief Constructs a new heading
             *
             * @param title The title for the heading. **Default**: ``""``
             * @param sideLen How many characters one side of #open's border has. **Default**: ``0``
             * @param sideChar The character the border is drawn with. **Default**: ``"="``
             */
            explicit Heading(std::string title = "", std::size_t sideLen = 0, std::string sideChar = "=");

            /**
             * @brief The title for the heading
             */
            std::string title;

            /**
             * @brief The number of characters one side of the border of the opening heading has
             */
            std::size_t sideLen;

            /**
             * @brief The character the border of the heading is drawn with
             */
            std::string sideChar;

            /**
             * @brief The opening heading -- line 1 of this class's own example
             */
            std::string open() const;

            /**
             * @brief The closing heading -- line 3 of this class's own example
             */
            std::string close() const;
    };
}

#endif
