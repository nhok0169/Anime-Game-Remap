#ifndef AGRemapCore_Generator_H
#define AGRemapCore_Generator_H

#include <coroutine>
#include <exception>
#include <optional>
#include <utility>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     A minimal, single-pass C++20 coroutine generator -- lets a method be written with ``co_yield``
     almost exactly like a Python `generator`_ function, rather than hand-flattened into an explicit
     resumable state machine. Used to port `IniSectionGraph`'s own ``yield``-based iterator methods
     (``__iter__``, ``iterSectsByContentPart``, ``iterByContentPart``, ``iterByQuery``) line-for-line
     against the pure-Python original, instead of a much higher-risk manual state-machine
     translation of their especially intricate explore/clean-state stack logic. :raw-html:`<br />`

     .. note::
        Single-pass and move-only, matching a Python generator's own "exhausted once iterated"
        semantics -- there is no way to restart or fork one. Not thread-safe (never resume the same
        instance from two threads).
     @endrst
     *
     * @tparam T The type of value yielded, by value, at each ``co_yield``
     */
    template <typename T>
    class Generator {
        public:
            /**
             * @brief
             @rst
             The compiler-driven coroutine state machine backing a #Generator -- never touched
             directly by any caller of #Generator itself; ``co_yield``/``co_return`` inside a
             coroutine body compile down to calls against this type.
             @endrst
             */
            struct promise_type {
                /**
                 * @brief The value most recently handed to ``co_yield``, if the coroutine hasn't been resumed since
                 */
                std::optional<T> currentValue;

                /**
                 * @brief The exception the coroutine body most recently threw, if any (see #unhandled_exception)
                 */
                std::exception_ptr currentException;

                /**
                 * @brief Constructs the #Generator this promise is paired with
                 */
                Generator get_return_object();

                /**
                 * @brief Always suspends before the coroutine body runs at all -- #Generator is lazy, not eager
                 */
                std::suspend_always initial_suspend() noexcept;

                /**
                 * @brief Always suspends after the coroutine body finishes, so #Handle::done() can still be observed
                 */
                std::suspend_always final_suspend() noexcept;

                /**
                 * @brief Stores 'value' into #currentValue and suspends -- the body of a ``co_yield``
                 *
                 * @param value The value yielded
                 */
                std::suspend_always yield_value(T value);

                /**
                 * @brief No-op -- a bare ``co_return``/falling off the end of the coroutine body yields nothing further
                 */
                void return_void();

                /**
                 * @brief Captures whatever the coroutine body just threw into #currentException, for #next() to rethrow
                 */
                void unhandled_exception();
            };

            /**
             * @brief The raw coroutine handle type backing this #Generator
             */
            using Handle = std::coroutine_handle<promise_type>;

            /**
             * @brief Constructs a new instance
             *
             * @param handle The coroutine handle to wrap -- ownership is taken by this #Generator
             */
            explicit Generator(Handle handle);

            Generator(const Generator&) = delete;
            Generator& operator=(const Generator&) = delete;

            /**
             * @brief Move-constructs a new instance, taking over 'other's coroutine handle
             *
             * @param other The instance to move from -- left holding no handle afterward
             */
            Generator(Generator&& other) noexcept;

            /**
             * @brief Move-assigns from 'other', destroying this instance's own handle first if it has one
             *
             * @param other The instance to move from -- left holding no handle afterward
             * @return A reference to this instance
             */
            Generator& operator=(Generator&& other) noexcept;

            ~Generator();

            /**
             * @brief
             @rst
             Advances to the next yielded value (or runs the coroutine body to completion, on the
             very first call, up to its first ``co_yield``)
             @endrst
             *
             * @return Whether a new value is available (see #value) -- ``false`` once the
             *      generator is exhausted
             *
             * @throw <whatever the coroutine body itself threw, rethrown here>
             */
            bool next();

            /**
             * @brief The value produced by the most recent successful #next() call
             */
            T& value();

        private:
            Handle handle_;
    };

}

#include "Generator.tpp"

#endif
