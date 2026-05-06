// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t06 03_nvi_pattern.cpp
// Demonstrates: Non-Virtual Interface pattern — base controls invariants, derived customizes behavior

#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <string>

// Abstract base: public non-virtual API, private virtual customization point
class Logger {
public:
    // Non-virtual public interface: enforces invariants before/after every log call
    void log(const std::string& msg) {
        if (msg.empty()) throw std::invalid_argument("empty message");
        std::string formatted = std::string("[") + __TIME__ + "] " + msg;
        do_log(formatted);  // delegate to derived
        call_count_++;
    }

    size_t call_count() const { return call_count_; }
    virtual ~Logger() = default;

private:
    // Pure virtual customization point — derived classes override this only
    virtual void do_log(const std::string& formatted) = 0;
    size_t call_count_ = 0;
};

// Derived 1: logs to stdout
class ConsoleLogger : public Logger {
private:
    void do_log(const std::string& formatted) override {
        printf("ConsoleLogger: %s\n", formatted.c_str());
    }
};

// Derived 2: logs to an in-memory string (simulated file)
class MemoryLogger : public Logger {
    std::ostringstream buf_;
private:
    void do_log(const std::string& formatted) override {
        buf_ << formatted << "\n";
    }
public:
    std::string contents() const { return buf_.str(); }
};

int main() {
    printf("--- NVI pattern demo ---\n\n");

    ConsoleLogger console;
    console.log("system started");
    console.log("request received");
    printf("ConsoleLogger call count: %zu\n\n", console.call_count());

    MemoryLogger mem;
    mem.log("system started");
    mem.log("request received");
    printf("MemoryLogger call count: %zu\n", mem.call_count());
    printf("MemoryLogger contents:\n%s", mem.contents().c_str());

    printf("--- Empty message blocked by base class ---\n");
    try {
        console.log("");
    } catch (const std::invalid_argument& e) {
        printf("Caught (as expected): %s\n", e.what());
    }

    // do_log is private — cannot be called from outside the class
    // console.do_log("hack");  // would not compile: 'virtual void Logger::do_log' is private

    return 0;
}
