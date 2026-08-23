namespace AGRemapCore {

    template <typename T>
    Generator<T> Generator<T>::promise_type::get_return_object() {
        return Generator(std::coroutine_handle<promise_type>::from_promise(*this));
    }

    template <typename T>
    std::suspend_always Generator<T>::promise_type::initial_suspend() noexcept {
        return {};
    }

    template <typename T>
    std::suspend_always Generator<T>::promise_type::final_suspend() noexcept {
        return {};
    }

    template <typename T>
    std::suspend_always Generator<T>::promise_type::yield_value(T value) {
        currentValue = std::move(value);
        return {};
    }

    template <typename T>
    void Generator<T>::promise_type::return_void() {

    }

    template <typename T>
    void Generator<T>::promise_type::unhandled_exception() {
        currentException = std::current_exception();
    }

    template <typename T>
    Generator<T>::Generator(Handle handle): handle_(handle) {

    }

    template <typename T>
    Generator<T>::Generator(Generator&& other) noexcept: handle_(other.handle_) {
        other.handle_ = nullptr;
    }

    template <typename T>
    Generator<T>& Generator<T>::operator=(Generator&& other) noexcept {
        if (this != &other) {
            if (handle_) {
                handle_.destroy();
            }
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }

    template <typename T>
    Generator<T>::~Generator() {
        if (handle_) {
            handle_.destroy();
        }
    }

    template <typename T>
    bool Generator<T>::next() {
        if (!handle_ || handle_.done()) {
            return false;
        }

        handle_.resume();

        if (handle_.promise().currentException) {
            std::exception_ptr exception = handle_.promise().currentException;
            handle_.promise().currentException = nullptr;
            std::rethrow_exception(exception);
        }

        return !handle_.done();
    }

    template <typename T>
    T& Generator<T>::value() {
        return *handle_.promise().currentValue;
    }

}
