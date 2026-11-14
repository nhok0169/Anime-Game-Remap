#ifndef AGRemapCore_ObjectLifetime_H
#define AGRemapCore_ObjectLifetime_H

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
    /**
     * @brief
     @rst
     Tells a listener when an object that others may point at without owning it is freed
     :raw-html:`<br />` :raw-html:`<br />`

     The `sections`_ of an .ini file (:cpp:class:`IfTemplate`), their parts (:cpp:class:`IfTemplatePart`) and the
     graphs over them (:cpp:class:`IniSectionGraph`) are owned by one object -- an :cpp:class:`IniFile`, a graph that
     copied its `sections`_, a fixer -- and read through plain pointers by many others, as an :cpp:class:`IniFile`
     itself is by the parsers and fixers of a run. Each of these classes calls :cpp:func:`released` from its
     destructor, so whatever holds such a pointer outside the library can learn that it no longer points at
     anything. Nothing is called when no hook is set
     :raw-html:`<br />` :raw-html:`<br />`

     The `Python`_ binding sets the hook: a `Python`_ object that wraps one of these objects without owning it is
     invalidated when the object is freed, so it can neither read freed memory nor be handed back as the wrapper of
     a new object that happens to be allocated at the same address
     @endrst
     */
    class ObjectLifetime {
        public:
            /**
             * @brief A listener told the address of each object freed. It must not throw
             */
            using ReleaseHook = void (*)(const void* object);

            /**
             * @brief Sets the listener, or clears it with ``nullptr``
             *
             * @param hook The listener
             */
            static void setReleaseHook(ReleaseHook hook) noexcept {
                hook_ = hook;
            }

            /**
             * @brief The listener, or ``nullptr`` when none is set
             */
            static ReleaseHook releaseHook() noexcept {
                return hook_;
            }

            /**
             * @brief Tells the listener, if one is set, that the object at 'object' is being freed
             *
             * @param object The address of the object being freed
             */
            static void released(const void* object) noexcept {
                if (hook_ != nullptr) {
                    hook_(object);
                }
            }

        private:
            static inline ReleaseHook hook_ = nullptr;
    };
}

#endif
