#pragma once

namespace arm_controller {

struct PidGains { double kp, ki, kd; };

class PidLoop {
public:
    explicit PidLoop(PidGains gains = {1.0, 0.0, 0.0})
        : gains_(gains) {}

    // Returns control output. Call at fixed dt intervals.
    double update(double setpoint, double actual, double dt) {
        if (dt <= 0.0) return 0.0;
        double error  = setpoint - actual;
        integral_    += error * dt;
        double deriv  = (error - prev_error_) / dt;
        prev_error_   = error;
        return gains_.kp * error + gains_.ki * integral_ + gains_.kd * deriv;
    }

    void reset() { integral_ = 0.0; prev_error_ = 0.0; }
    void set_gains(PidGains g) { gains_ = g; reset(); }
    PidGains gains() const { return gains_; }

private:
    PidGains gains_;
    double   integral_{0.0};
    double   prev_error_{0.0};
};

}  // namespace arm_controller
