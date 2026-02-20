#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <cmath>

#include "shader.h"
#include "model_loader.h"
#include "texture_loader.h"
#include "fps_counter.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Number of instances to render
const int NUM_INSTANCES = 100;

// Number of lights
const int NUM_LIGHTS = 8;

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Calculate instance transforms for multiple objects - less repetitive movement
std::vector<glm::mat4> calculateInstanceTransforms(int numInstances, float time) {
    std::vector<glm::mat4> transforms(numInstances);
    
    for (int i = 0; i < numInstances; i++) {
        // Different movement patterns for different spheres
        float speed1 = 0.3f + (i % 5) * 0.1f;
        float speed2 = 0.5f + (i % 7) * 0.07f;
        float speed3 = 0.7f + (i % 3) * 0.15f;
        
        // Multiple layered rotations for chaotic movement
        float theta = time * speed1 + i * 1.618f * 0.1f;
        float phi = time * speed2 + i * 0.7f;
        float radius = 2.5f + sin(time * speed3 + i * 0.3f) * 1.5f;
        
        // Spiral pattern with varying heights
        float x = radius * sin(phi) * cos(theta * (1.0f + sin(time * 0.1f + i)));
        float y = (float)i / numInstances * 4.0f - 2.0f + sin(time * speed1 + i * 0.5f) * 1.5f;
        float z = radius * sin(phi) * sin(theta * 0.7f);
        
        // Random rotation axis for each sphere
        glm::vec3 rotAxis(0.5f + sin(i * 0.1f) * 0.5f, 1.0f, 0.3f + cos(i * 0.2f) * 0.5f);
        rotAxis = glm::normalize(rotAxis);
        
        // Varying scale for visual interest
        float scaleBase = 0.25f + sin(i * 0.5f) * 0.1f;
        
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(x, y, z));
        transform = glm::rotate(transform, time * speed2 + i, rotAxis);
        transform = glm::scale(transform, glm::vec3(scaleBase));
        
        transforms[i] = transform;
    }
    
    return transforms;
}

// Get color based on GPU "temperature" (derived from FPS)
// Cold (high FPS) = blue/cyan, Hot (low FPS) = red/orange
glm::vec3 getTemperatureColor(float fps) {
    // FPS range: 0 (hot) to 60+ (cold)
    float t = glm::clamp(fps / 60.0f, 0.0f, 1.0f);
    
    // Hot (red/orange) -> Warm (yellow) -> Cold (cyan/blue)
    glm::vec3 hot(1.0f, 0.2f, 0.1f);      // Red-orange
    glm::vec3 warm(1.0f, 0.7f, 0.1f);    // Yellow
    glm::vec3 cool(0.2f, 0.8f, 1.0f);     // Cyan
    glm::vec3 cold(0.1f, 0.3f, 1.0f);      // Blue
    
    if (t < 0.25f) {
        return glm::mix(hot, warm, t * 4.0f);
    } else if (t < 0.5f) {
        return glm::mix(warm, cool, (t - 0.25f) * 4.0f);
    } else {
        return glm::mix(cool, cold, (t - 0.5f) * 2.0f);
    }
}

// Get GPU temperature description
std::string getTemperatureDescription(float fps) {
    if (fps >= 50.0f) return "COLD";
    if (fps >= 30.0f) return "warm";
    if (fps >= 15.0f) return "HOT";
    return "EXTREME";
}

int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL GPU Benchmark", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create shader
    Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");

    // Create model - HIGH POLY for GPU stress
    Model model;
    int subdivisions = 256;
    std::cout << "Generating high-poly sphere with " << subdivisions << " subdivisions..." << std::endl;
    model.generateSphere(subdivisions, subdivisions, 1.0f);
    
    int vertexCount = model.meshes[0].vertices.size();
    int triangleCount = model.meshes[0].indices.size() / 3;
    int totalTriangles = triangleCount * NUM_INSTANCES;
    
    std::cout << "Model: " << vertexCount << " vertices, " << triangleCount << " triangles per instance" << std::endl;
    std::cout << "Rendering " << NUM_INSTANCES << " instances = " << totalTriangles << " triangles per frame" << std::endl;
    std::cout << "Using " << NUM_LIGHTS << " point lights" << std::endl;

    // Create initial textures
    unsigned int diffuseTexture = 0;
    unsigned int normalTexture = TextureLoader::generateNormalMap(1024, 1024, 0.5f);

    // FPS counter
    FPSCounter fpsCounter;

    // Camera
    glm::vec3 cameraPos(0.0f, 0.0f, 8.0f);
    
    // Create multiple light positions
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;
    for (int i = 0; i < NUM_LIGHTS; i++) {
        float angle = (float)i / NUM_LIGHTS * 6.28318f;
        float radius = 4.0f;
        lightPositions.push_back(glm::vec3(
            cos(angle) * radius,
            sin(angle * 2.0f) * 2.0f,
            sin(angle) * radius
        ));
        
        float r = 0.5f + 0.5f * sin(i * 1.0f);
        float g = 0.5f + 0.5f * sin(i * 1.5f + 1.0f);
        float b = 0.5f + 0.5f * sin(i * 2.0f + 2.0f);
        lightColors.push_back(glm::vec3(r, g, b));
    }

    std::vector<glm::mat4> instanceTransforms(NUM_INSTANCES);

    // Main loop
    double lastTime = glfwGetTime();
    int drawCalls = 0;
    int textureRegenCounter = 0;
    glm::vec3 currentTempColor(0.8f, 0.4f, 0.2f);

    while (!glfwWindowShouldClose(window)) {
        fpsCounter.beginFrame();
        drawCalls = 0;
        
        processInput(window);

        double currentTime = glfwGetTime();
        float time = (float)currentTime;
        
        instanceTransforms = calculateInstanceTransforms(NUM_INSTANCES, time);

        // Get GPU temperature based on FPS
        float currentFPS = fpsCounter.getSmoothedFPS();
        glm::vec3 tempColor = getTemperatureColor(currentFPS);
        std::string tempDesc = getTemperatureDescription(currentFPS);
        
        // Smooth color transition
        currentTempColor = currentTempColor * 0.95f + tempColor * 0.05f;

        // Regenerate texture occasionally to stress GPU more
        textureRegenCounter++;
        if (textureRegenCounter > 60) {
            textureRegenCounter = 0;
            if (diffuseTexture) glDeleteTextures(1, &diffuseTexture);
            diffuseTexture = TextureLoader::generateProceduralTexture(1024, 1024, 
                currentTempColor.r, currentTempColor.g, currentTempColor.b);
        }

        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        shader.setInt("diffuseMap", 0);
        shader.setInt("normalMap", 1);
        shader.setInt("useNormalMap", 1);
        shader.setVec3("viewPos", cameraPos);
        
        for (int i = 0; i < NUM_LIGHTS; i++) {
            std::string posName = "lightPositions[" + std::to_string(i) + "]";
            std::string colName = "lightColors[" + std::to_string(i) + "]";
            shader.setVec3(posName, lightPositions[i]);
            shader.setVec3(colName, lightColors[i]);
        }
        
        shader.setInt("numLights", NUM_LIGHTS);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalTexture);

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, -cameraPos);

        glm::mat4 projection = glm::perspective(glm::radians(60.0f), 
            (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        for (int i = 0; i < NUM_INSTANCES; i++) {
            shader.setMat4("model", instanceTransforms[i]);
            model.draw();
            drawCalls++;
        }

        // Update window title with temperature-based info
        static int frameCounter = 0;
        static double lastTitleUpdate = 0.0;
        
        frameCounter++;
        if (currentTime - lastTitleUpdate >= 0.5) {
            char title[256];
            snprintf(title, sizeof(title), "GPU Benchmark | FPS: %.1f | %s | %d triangles | %d calls",
                    currentFPS, tempDesc.c_str(), totalTriangles, drawCalls);
            glfwSetWindowTitle(window, title);
            lastTitleUpdate = currentTime;
            frameCounter = 0;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
        
        fpsCounter.endFrame();
    }

    // Print benchmark results
    std::cout << "\n=== Benchmark Results ===" << std::endl;
    std::cout << "Total Frames: " << fpsCounter.getFrameCount() << std::endl;
    std::cout << "Average FPS: " << fpsCounter.getSmoothedFPS() << std::endl;
    std::cout << "Min FPS: " << fpsCounter.getMinFPS() << std::endl;
    std::cout << "Max FPS: " << fpsCounter.getMaxFPS() << std::endl;
    std::cout << "Average Frame Time: " << 1000.0f / fpsCounter.getSmoothedFPS() << " ms" << std::endl;
    std::cout << "Triangles per frame: " << totalTriangles << std::endl;
    std::cout << "Draw calls per frame: " << drawCalls << std::endl;
    std::cout << "Point lights: " << NUM_LIGHTS << std::endl;

    // Cleanup - delete textures while OpenGL context is still valid
    if (diffuseTexture) glDeleteTextures(1, &diffuseTexture);
    if (normalTexture) glDeleteTextures(1, &normalTexture);

    glfwTerminate();
    return 0;
}
