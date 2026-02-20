#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <glad/gl.h>
#include "stb_image.h"

#include <string>
#include <vector>

class Texture {
public:
    unsigned int ID;
    std::string type;
    std::string path;
    
    Texture() : ID(0) {}
    
    ~Texture() {
        if (ID) {
            glDeleteTextures(1, &ID);
        }
    }
};

class TextureLoader {
public:
    static unsigned int loadTexture(const std::string& path, bool generateMipmaps = true) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        
        int width, height, nrComponents;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
        
        if (data) {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;
            else
                format = GL_RGB;
            
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            
            if (generateMipmaps) {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            
            // Set texture wrapping/filtering options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            
            stbi_image_free(data);
            
            std::cout << "Loaded texture: " << path << " (" << width << "x" << height << ")" << std::endl;
        } else {
            std::cerr << "Failed to load texture: " << path << std::endl;
            // Create a fallback texture
            createFallbackTexture(textureID);
            stbi_image_free(data);
        }
        
        return textureID;
    }
    
    // Generate a procedural normal map
    static unsigned int generateNormalMap(int width, int height, float roughness = 0.5f) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        
        std::vector<unsigned char> data(width * height * 4);
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Generate procedural normal map with some noise
                float nx = ((float)rand() / RAND_MAX - 0.5f) * roughness;
                float ny = ((float)rand() / RAND_MAX - 0.5f) * roughness;
                float nz = 1.0f;
                
                // Normalize
                float len = sqrt(nx*nx + ny*ny + nz*nz);
                nx /= len;
                ny /= len;
                nz /= len;
                
                // Convert to 0-255 range
                int idx = (y * width + x) * 4;
                data[idx + 0] = (unsigned char)((nx * 0.5f + 0.5f) * 255);
                data[idx + 1] = (unsigned char)((ny * 0.5f + 0.5f) * 255);
                data[idx + 2] = (unsigned char)((nz * 0.5f + 0.5f) * 255);
                data[idx + 3] = 255;
            }
        }
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        return textureID;
    }
    
    // Generate a procedural diffuse texture with a pattern
    static unsigned int generateProceduralTexture(int width, int height, float r, float g, float b) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        
        std::vector<unsigned char> data(width * height * 4);
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                float u = (float)x / width;
                float v = (float)y / height;
                
                // Create a gradient with a pattern
                float pattern = sin(u * 20.0f) * sin(v * 20.0f) * 0.1f + 0.9f;
                
                int idx = (y * width + x) * 4;
                data[idx + 0] = (unsigned char)(r * 255 * pattern);
                data[idx + 1] = (unsigned char)(g * 255 * pattern);
                data[idx + 2] = (unsigned char)(b * 255 * pattern);
                data[idx + 3] = 255;
            }
        }
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        return textureID;
    }
    
private:
    static void createFallbackTexture(unsigned int textureID) {
        // Create a simple gray texture as fallback
        unsigned char data[] = {128, 128, 128, 255};
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
};

#endif // TEXTURE_LOADER_H
