#include <coroutine>
#include <iostream>
#include <chrono>
#include <thread>

struct Sleeper {
    constexpr bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> h) const {
        auto t = std::jthread([h,l = length] {
            std::this_thread::sleep_for(l);
            h.resume();
        });
    }
    constexpr void await_resume() const noexcept {}
    const std::chrono::duration<int, std::milli> length;
};

struct Task {
    // The coroutine level type
    struct promise_type {
        Task get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() { }
        void unhandled_exception() { }
    };
};


// Coroutine 需要返回一个包含 promise_type 的对象
Task myCoroutine() {
    using namespace std::chrono_literals;
    auto before = std::chrono::steady_clock::now();
    std::cout << "Going to sleep on thread " <<
              std::this_thread::get_id() << "\n";
    // 如果注释下面的语句，线程就不会发生切换
    // co_await 之后的对象必须是一个 awaitable 对象
    co_await Sleeper{200ms};
    auto after = std::chrono::steady_clock::now();
    std::cout << "Slept for " << (after-before) / 1ms << " ms\n";
    std::cout << "Now on thread " << std::this_thread::get_id() << "\n";
}

// 所以也就是说，C++ 协程的两大概念就是 promise_type 和 awaitable 对象
// 协程和callback 并没有太多区别， 都是在阻塞的时候，让出计算，等待事件发生，然后事件就绪，callback 就会被调用
// 对于协程来说，callback 其实就是叫醒runtime,或者说叫醒executor, 让它继续执行协程，也就是协程的resume
int main() {
    std::cout << "Main thread" << std::this_thread::get_id() << "\n";
    myCoroutine();
    std::cout << "Main thread" << std::this_thread::get_id() << "\n";
}