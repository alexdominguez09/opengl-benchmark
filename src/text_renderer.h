#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <string>
#include <map>
#include <vector>

// Simple bitmap font character
struct Character {
    unsigned int textureID;
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned int advance;
};

class TextRenderer {
private:
    std::map<char, Character> characters;
    unsigned int VAO, VBO;
    
public:
    TextRenderer() : VAO(0), VBO(0) {}
    
    ~TextRenderer() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
    }
    
    // Generate a simple built-in font (for demo purposes)
    void generateDefaultFont() {
        // Create a simple 8x8 monospace font texture
        const int charWidth = 8;
        const int charHeight = 8;
        const int numChars = 128;
        
        unsigned char fontData[charHeight][charWidth * numChars];
        
        // Initialize with spaces (0)
        for (int y = 0; y < charHeight; y++) {
            for (int x = 0; x < charWidth * numChars; x++) {
                fontData[y][x] = 0;
            }
        }
        
        // Simple font patterns (very basic - just enough to show FPS)
        // Numbers 0-9
        const unsigned char digitPatterns[10][8] = {
            {0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00}, // 0
            {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00}, // 1
            {0x3C, 0x66, 0x06, 0x1C, 0x30, 0x66, 0x7E, 0x00}, // 2
            {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00}, // 3
            {0x0E, 0x1E, 0x36, 0x66, 0x7F, 0x06, 0x06, 0x00}, // 4
            {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00}, // 5
            {0x1C, 0x30, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00}, // 6
            {0x7E, 0x66, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x00}, // 7
            {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00}, // 8
            {0x3C, 0x66, 0x66, 0x3E, 0x06, 0x0C, 0x38, 0x00}  // 9
        };
        
        // Fill in digits
        for (int d = 0; d < 10; d++) {
            int baseX = (('0' + d) - 32) * charWidth;
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    if (digitPatterns[d][y] & (1 << (7 - x))) {
                        fontData[y][baseX + x] = 255;
                    }
                }
            }
        }
        
        // Letter F
        int fx = ('F' - 32) * charWidth;
        const unsigned char fPattern[8] = {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00};
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 5; x++) {
                if (fPattern[y] & (1 << (4 - x))) {
                    fontData[y][fx + x] = 255;
                }
            }
        }
        
        // Letter P
        int px = ('P' - 32) * charWidth;
        const unsigned char pPattern[8] = {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00};
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 6; x++) {
                if (pPattern[y] & (1 << (5 - x))) {
                    fontData[y][px + x] = 255;
                }
            }
        }
        
        // Letter S
        int sx = ('S' - 32) * charWidth;
        const unsigned char sPattern[8] = {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00};
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 6; x++) {
                if (sPattern[y] & (1 << (5 - x))) {
                    fontData[y][sx + x] = 255;
                }
            }
        }
        
        // Period
        int dotx = ('.' - 32) * charWidth;
        fontData[6][dotx + 2] = 255;
        fontData[7][dotx + 2] = 255;
        
        // Colon
        int colx = (':' - 32) * charWidth;
        fontData[2][colx + 2] = 255;
        fontData[5][colx + 2] = 255;
        
        // Space and other basic chars
        for (int c = 32; c < 128; c++) {
            int baseX = (c - 32) * charWidth;
            Character ch;
            ch.textureID = 0;
            ch.size = glm::ivec2(charWidth, charHeight);
            ch.bearing = glm::ivec2(0, 0);
            ch.advance = charWidth;
            
            // Create texture for this character
            glGenTextures(1, &ch.textureID);
            glBindTexture(GL_TEXTURE_2D, ch.textureID);
            
            unsigned char charData[charHeight][charWidth];
            for (int y = 0; y < charHeight; y++) {
                for (int x = 0; x < charWidth; x++) {
                    charData[y][x] = fontData[y][baseX + x];
                }
            }
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, charWidth, charHeight, 0, 
                        GL_RED, GL_UNSIGNED_BYTE, charData);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            
            characters[c] = ch;
        }
        
        // Setup VAO/VBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    
    void renderText(const std::string& text, float x, float y, float scale, float r, float g, float b) {
        // Use legacy OpenGL for simple text rendering
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(glm::value_ptr(glm::ortho(0.0f, 800.0f, 0.0f, 600.0f)));
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        glEnable(GL_TEXTURE_2D);
        
        for (const char& c : text) {
            auto it = characters.find(c);
            if (it == characters.end()) continue;
            
            Character& ch = it->second;
            
            float xpos = x + ch.bearing.x * scale;
            float ypos = y - (ch.size.y - ch.bearing.y) * scale;
            float w = ch.size.x * scale;
            float h = ch.size.y * scale;
            
            glBindTexture(GL_TEXTURE_2D, ch.textureID);
            
            glColor4f(r, g, b, 1.0f);
            
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(xpos, ypos);
            glTexCoord2f(1.0f, 0.0f); glVertex2f(xpos + w, ypos);
            glTexCoord2f(1.0f, 1.0f); glVertex2f(xpos + w, ypos + h);
            glTexCoord2f(0.0f, 1.0f); glVertex2f(xpos, ypos + h);
            glEnd();
            
            x += (ch.advance >> 6) * scale;
        }
        
        glDisable(GL_BLEND);
    }
};

#endif // TEXT_RENDERER_H
