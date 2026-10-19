#include <Arduino.h>
#include <functional>

class Timer {
    private:
        unsigned long startTime;
        unsigned long durationMS;

    public:
        Timer(unsigned long duration) : startTime(0), durationMS(duration) {}
        void start() {startTime = millis();}
        void onTimerTimeout(std::function<void()> callback) {
            if (millis() - startTime >= durationMS) {
                callback();
            }
        }
        void stop() {startTime = 0;}
};