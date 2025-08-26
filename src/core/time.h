#pragma once

#include <chrono>
#include <cstdint>

namespace core {

class Time {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::duration<double>;

    Time();

    // フレーム更新
    void update();

    // 時間取得
    double getDeltaTime() const { return deltaTime; }
    double getFixedDeltaTime() const { return fixedDeltaTime; }
    double getTotalTime() const { return totalTime; }
    uint64_t getFrameCount() const { return frameCount; }

    // 固定時間ステップ用
    bool shouldRunFixedStep() const;
    void consumeFixedStep();

private:
    TimePoint startTime;
    TimePoint lastFrameTime;
    TimePoint currentFrameTime;
    TimePoint fixedStepAccumulator;

    double deltaTime = 0.0;
    double fixedDeltaTime = 1.0 / 60.0; // 60Hz
    double totalTime = 0.0;
    uint64_t frameCount = 0;
    double fixedStepRemaining = 0.0;
};

// グローバル時間インスタンス
extern Time g_time;

} // namespace core
