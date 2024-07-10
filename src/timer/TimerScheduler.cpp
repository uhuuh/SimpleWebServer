#include "TimerScheduler.hpp"
#include "Logger.hpp"


TimerWheel::TimerWheel() {
    wheel.resize(span_sec * 1000 / every_ms);
    trigger_before_ms = get_slot_timestamp();
}
TimerId TimerWheel::add_timer(Timer timer) {
    auto at_ms =
        get_slot_timestamp(timer.trigger_after_ms + get_now_timestamp());
    if (at_ms <= trigger_before_ms) {
        LOG_DEBUG("timer_old");
        timer.cb();
        return {};
    } else if (get_slot_timestamp(timer.trigger_after_ms) >= span_sec * 1000) {
        LOG_ERROR("timer_over_limit");
        return {};
    }

    auto slot_n = (at_ms - trigger_before_ms) / every_ms;
    auto new_slot_ptr = (slot_ptr + slot_n) % wheel.size();
    wheel[new_slot_ptr].push_back(timer);

    auto it = --wheel[new_slot_ptr].end();
    return {at_ms, it._M_node};
}
void TimerWheel::remove_timer(TimerId id) {
    if (id.trigger_at_ms <= trigger_before_ms)
        return;

    auto slot_n = (id.trigger_at_ms - trigger_before_ms) / every_ms;
    auto new_slot_ptr = (slot_ptr + slot_n) % wheel.size();

    list<Timer>::iterator it;
    it._M_node = reinterpret_cast<decltype(it._M_node)>(id.it);
    wheel[new_slot_ptr].erase(it);
}
uint64_t TimerWheel::trigger_timer(uint64_t now_ms) {
    auto at_ms = get_slot_timestamp(now_ms);
    while (trigger_before_ms < at_ms) {
        trigger_before_ms += every_ms;
        slot_ptr = (slot_ptr + 1) % wheel.size();

        for (auto &ti : wheel[slot_ptr]) {
            ti.cb();
        }
        wheel[slot_ptr].clear();

        if (trigger_before_ms >= at_ms)
            break;
    };

    return every_ms;
}

inline uint64_t TimerWheel::get_slot_timestamp(uint64_t a) {
    if (a == -1)
        a = get_now_timestamp();
    return a - (a % every_ms);
}