#ifndef FPS_COUNTER_H
#define FPS_COUNTER_H

#include <glad/gl.h>

#include <vector>
#include <chrono>

class FPSCounter {
private:
    std::chrono::high_resolution_clock::time_point lastTime;
    std::chrono::high_resolution_clock::time_point currentTime;
    
    float frameTime;
    float fps;
    float smoothedFPS;
    float minFPS;
    float maxFPS;
    
    std::vector<float> frameTimes;
    size_t frameTimeIndex;
    static const size_t FRAME_TIME_HISTORY = 60;
    
    int frameCount;
    float elapsedTime;
    
    // For GPU timing
    GLuint queryID[2];
    bool querySupported;
    int currentQuery;
    
public:
    FPSCounter() : frameTime(0.0f), fps(0.0f), smoothedFPS(60.0f),
                   minFPS(9999.0f), maxFPS(0.0f), frameTimeIndex(0),
                   frameCount(0), elapsedTime(0.0f), currentQuery(0), querySupported(false) {
        frameTimes.resize(FRAME_TIME_HISTORY, 0.0f);
        lastTime = std::chrono::high_resolution_clock::now();
        
        // Try to initialize timer queries
        if (GLAD_GL_ARB_timer_query) {
            glGenQueries(2, queryID);
            querySupported = true;
        }
    }
    
    ~FPSCounter() {
        // Don't cleanup GL queries here - context may be destroyed
        // The queries will be automatically cleaned up when context is destroyed
    }
    
    void beginFrame() {
        currentTime = std::chrono::high_resolution_clock::now();
        
        if (querySupported) {
            glQueryCounter(queryID[currentQuery], GL_TIMESTAMP);
        }
    }
    
    void endFrame() {
        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> diff = endTime - lastTime;
        frameTime = diff.count();
        
        // Update history
        frameTimes[frameTimeIndex] = frameTime;
        frameTimeIndex = (frameTimeIndex + 1) % FRAME_TIME_HISTORY;
        
        // Calculate FPS
        if (frameTime > 0.0f) {
            fps = 1.0f / frameTime;
            
            // Smooth FPS
            smoothedFPS = 0.95f * smoothedFPS + 0.05f * fps;
            
            // Update min/max
            if (fps < minFPS) minFPS = fps;
            if (fps > maxFPS) maxFPS = fps;
        }
        
        frameCount++;
        elapsedTime += frameTime;
        
        // GPU timing
        if (querySupported) {
            int prevQuery = (currentQuery + 1) % 2;
            GLint available = 0;
            glGetQueryObjectiv(queryID[prevQuery], GL_QUERY_RESULT_AVAILABLE, &available);
            
            if (available) {
                GLuint64 gpuTime;
                glGetQueryObjectui64v(queryID[prevQuery], GL_QUERY_RESULT, &gpuTime);
                // GPU time in nanoseconds - convert to seconds
                float gpuFrameTime = gpuTime / 1000000000.0f;
            }
            
            currentQuery = (currentQuery + 1) % 2;
        }
        
        lastTime = currentTime;
    }
    
    float getFPS() const { return fps; }
    float getSmoothedFPS() const { return smoothedFPS; }
    float getFrameTime() const { return frameTime * 1000.0f; } // ms
    float getMinFPS() const { return minFPS; }
    float getMaxFPS() const { return maxFPS; }
    int getFrameCount() const { return frameCount; }
    float getElapsedTime() const { return elapsedTime; }
    
    void reset() {
        frameCount = 0;
        elapsedTime = 0.0f;
        minFPS = 9999.0f;
        maxFPS = 0.0f;
        smoothedFPS = 60.0f;
        for (auto& ft : frameTimes) ft = 0.0f;
    }
};

#endif // FPS_COUNTER_H
