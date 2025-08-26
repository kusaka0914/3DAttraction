#include "time.h"

namespace core {

Time::Time() 
    : startTime(Clock::now())
    , lastFrameTime(startTime)
    , currentFrameTime(startTime)
    , fixedStepAccumulator(startTime) {
}

void Time::update() {
    lastFrameTime = currentFrameTime;
    currentFrameTime = Clock::now();
    
    auto deltaDuration = currentFrameTime - lastFrameTime;
    deltaTime = std::chrono::duration<double>(deltaDuration).count();
    
    auto totalDuration = currentFrameTime - startTime;
    totalTime = std::chrono::duration<double>(totalDuration).count();
    
    frameCount++;
    
    // 固定時間ステップの蓄積
    fixedStepRemaining += deltaTime;
}

bool Time::shouldRunFixedStep() const {
    return fixedStepRemaining >= fixedDeltaTime;
}

void Time::consumeFixedStep() {
    fixedStepRemaining -= fixedDeltaTime;
}

// グローバルインスタンス
Time g_time;

} // namespace core
