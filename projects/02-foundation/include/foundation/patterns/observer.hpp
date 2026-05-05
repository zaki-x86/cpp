#pragma once
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace foundation {

using SubscriptionToken = uint64_t;

// Single-threaded event emitter.
// For thread-safe use: guard emit/subscribe/unsubscribe with std::mutex.
template<typename EventT>
class EventEmitter {
    using Handler = std::function<void(const EventT&)>;
    std::unordered_map<SubscriptionToken, Handler> handlers_;
    SubscriptionToken next_{1};

public:
    SubscriptionToken subscribe(Handler h) {
        SubscriptionToken tok = next_++;
        handlers_.emplace(tok, std::move(h));
        return tok;
    }

    void unsubscribe(SubscriptionToken tok) { handlers_.erase(tok); }

    void emit(const EventT& event) const {
        for (auto& [tok, h] : handlers_) h(event);
    }
};

} // namespace foundation
