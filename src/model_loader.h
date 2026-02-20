#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>

struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoords;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    unsigned int VAO, VBO, EBO;
    
    Mesh() : VAO(0), VBO(0), EBO(0) {}
    
    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        
        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        
        // TexCoords
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
        
        // Normal
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        
        // Tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
        
        // Bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
        
        glBindVertexArray(0);
    }
    
    void draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
    void cleanup() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
    }
};

class Model {
public:
    std::vector<Mesh> meshes;
    std::string directory;
    
    Model() {}
    
    // Note: Don't cleanup in destructor - OpenGL context may already be destroyed
    // Call cleanup() manually before exiting if needed
    ~Model() {
        // meshes will be cleaned up automatically
        // OpenGL objects will be leaked but that's fine since context is destroyed
    }
    
    void cleanup() {
        for (auto& mesh : meshes) {
            mesh.cleanup();
        }
    }
    
    bool loadOBJ(const std::string& path) {
        std::ifstream file(path);
        if (!file) {
            std::cerr << "Failed to open OBJ file: " << path << std::endl;
            return false;
        }
        
        // Determine directory
        size_t lastSlash = path.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            directory = path.substr(0, lastSlash + 1);
        } else {
            directory = "";
        }
        
        std::vector<glm::vec3> tempPositions;
        std::vector<glm::vec2> tempTexCoords;
        std::vector<glm::vec3> tempNormals;
        
        std::vector<unsigned int> vertexIndices, texCoordIndices, normalIndices;
        
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;
            
            if (prefix == "v") {
                glm::vec3 position;
                iss >> position.x >> position.y >> position.z;
                tempPositions.push_back(position);
            } else if (prefix == "vt") {
                glm::vec2 texCoord;
                iss >> texCoord.x >> texCoord.y;
                tempTexCoords.push_back(texCoord);
            } else if (prefix == "vn") {
                glm::vec3 normal;
                iss >> normal.x >> normal.y >> normal.z;
                tempNormals.push_back(normal);
            } else if (prefix == "f") {
                std::string v1, v2, v3;
                iss >> v1 >> v2 >> v3;
                
                processFace(v1, vertexIndices, texCoordIndices, normalIndices);
                processFace(v2, vertexIndices, texCoordIndices, normalIndices);
                processFace(v3, vertexIndices, texCoordIndices, normalIndices);
            }
        }
        
        // Build mesh from data
        Mesh mesh;
        for (unsigned int i = 0; i < vertexIndices.size(); i++) {
            Vertex vertex;
            
            unsigned int posIdx = vertexIndices[i];
            vertex.position = tempPositions[posIdx];
            
            if (!tempTexCoords.empty() && texCoordIndices[i] < tempTexCoords.size()) {
                vertex.texCoords = tempTexCoords[texCoordIndices[i]];
            } else {
                vertex.texCoords = glm::vec2(0.0f);
            }
            
            if (!tempNormals.empty() && normalIndices[i] < tempNormals.size()) {
                vertex.normal = tempNormals[normalIndices[i]];
            } else {
                vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            
            mesh.vertices.push_back(vertex);
        }
        
        // Generate indices
        for (unsigned int i = 0; i < mesh.vertices.size(); i++) {
            mesh.indices.push_back(i);
        }
        
        // Compute tangent space
        computeTangentSpace(mesh.vertices, mesh.indices);
        
        mesh.setupMesh();
        meshes.push_back(mesh);
        
        std::cout << "Loaded model: " << mesh.vertices.size() << " vertices, " 
                  << mesh.indices.size() << " indices" << std::endl;
        
        return true;
    }
    
    // Generate a high-poly cube for testing
    void generateHighPolyCube(int subdivisions = 50) {
        Mesh mesh;
        
        float size = 1.0f;
        
        // Generate vertices for each face with subdivisions
        for (int face = 0; face < 6; face++) {
            for (int y = 0; y < subdivisions; y++) {
                for (int x = 0; x < subdivisions; x++) {
                    float u1 = (float)x / subdivisions;
                    float v1 = (float)y / subdivisions;
                    float u2 = (float)(x + 1) / subdivisions;
                    float v2 = (float)(y + 1) / subdivisions;
                    
                    std::vector<glm::vec3> positions;
                    std::vector<glm::vec2> uvs;
                    std::vector<glm::vec3> normal;
                    
                    getFaceGeometry(face, size, u1, v1, u2, v2, positions, uvs, normal);
                    
                    // Add quad as two triangles
                    for (int i = 0; i < 6; i++) {
                        Vertex vertex;
                        int idx = (i < 3) ? i : i - 2;
                        
                        vertex.position = positions[idx];
                        vertex.texCoords = uvs[idx];
                        vertex.normal = normal[0];
                        vertex.tangent = glm::vec3(0);
                        vertex.bitangent = glm::vec3(0);
                        
                        mesh.vertices.push_back(vertex);
                        mesh.indices.push_back(mesh.vertices.size() - 1);
                    }
                }
            }
        }
        
        computeTangentSpace(mesh.vertices, mesh.indices);
        mesh.setupMesh();
        meshes.push_back(mesh);
        
        std::cout << "Generated high-poly cube: " << mesh.vertices.size() << " vertices, "
                  << mesh.indices.size() << " indices" << std::endl;
    }
    
    // Generate a sphere for testing
    void generateSphere(int latitudeBands = 64, int longitudeBands = 64, float radius = 1.0f) {
        Mesh mesh;
        
        for (int lat = 0; lat <= latitudeBands; lat++) {
            float theta = lat * glm::pi<float>() / latitudeBands;
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);
            
            for (int lon = 0; lon <= longitudeBands; lon++) {
                float phi = lon * 2 * glm::pi<float>() / longitudeBands;
                float sinPhi = sin(phi);
                float cosPhi = cos(phi);
                
                float x = cosPhi * sinTheta;
                float y = cosTheta;
                float z = sinPhi * sinTheta;
                
                float u = 1.0f - (float)lon / longitudeBands;
                float v = 1.0f - (float)lat / latitudeBands;
                
                Vertex vertex;
                vertex.position = glm::vec3(radius * x, radius * y, radius * z);
                vertex.normal = glm::vec3(x, y, z);
                vertex.texCoords = glm::vec2(u, v);
                vertex.tangent = glm::vec3(0);
                vertex.bitangent = glm::vec3(0);
                
                mesh.vertices.push_back(vertex);
            }
        }
        
        // Generate indices
        for (int lat = 0; lat < latitudeBands; lat++) {
            for (int lon = 0; lon < longitudeBands; lon++) {
                unsigned int first = (lat * (longitudeBands + 1)) + lon;
                unsigned int second = first + longitudeBands + 1;
                
                mesh.indices.push_back(first);
                mesh.indices.push_back(second);
                mesh.indices.push_back(first + 1);
                
                mesh.indices.push_back(second);
                mesh.indices.push_back(second + 1);
                mesh.indices.push_back(first + 1);
            }
        }
        
        computeTangentSpace(mesh.vertices, mesh.indices);
        mesh.setupMesh();
        meshes.push_back(mesh);
        
        std::cout << "Generated sphere: " << mesh.vertices.size() << " vertices, "
                  << mesh.indices.size() << " indices" << std::endl;
    }
    
    void draw() {
        for (auto& mesh : meshes) {
            mesh.draw();
        }
    }
    
private:
    void processFace(const std::string& vertexStr, 
                     std::vector<unsigned int>& posIndices,
                     std::vector<unsigned int>& texIndices,
                     std::vector<unsigned int>& normIndices) {
        std::vector<std::string> parts;
        std::string part;
        std::istringstream iss(vertexStr);
        
        while (std::getline(iss, part, '/')) {
            parts.push_back(part);
        }
        
        // Position index (required)
        if (parts.size() >= 1 && !parts[0].empty()) {
            posIndices.push_back(std::stoi(parts[0]) - 1);
        }
        
        // Texture coordinate index
        if (parts.size() >= 2 && !parts[1].empty()) {
            texIndices.push_back(std::stoi(parts[1]) - 1);
        } else {
            texIndices.push_back(0);
        }
        
        // Normal index
        if (parts.size() >= 3 && !parts[2].empty()) {
            normIndices.push_back(std::stoi(parts[2]) - 1);
        } else {
            normIndices.push_back(0);
        }
    }
    
    void getFaceGeometry(int face, float size, float u1, float v1, float u2, float v2,
                        std::vector<glm::vec3>& positions, std::vector<glm::vec2>& uvs,
                        std::vector<glm::vec3>& normal) {
        glm::vec3 p1, p2, p3, p4;
        glm::vec3 n;
        
        float hs = size / 2.0f;
        
        switch(face) {
            case 0: // Front
                p1 = {-hs, -hs, hs}; p2 = {hs, -hs, hs};
                p3 = {hs, hs, hs}; p4 = {-hs, hs, hs};
                n = {0, 0, 1};
                break;
            case 1: // Back
                p1 = {hs, -hs, -hs}; p2 = {-hs, -hs, -hs};
                p3 = {-hs, hs, -hs}; p4 = {hs, hs, -hs};
                n = {0, 0, -1};
                break;
            case 2: // Top
                p1 = {-hs, hs, hs}; p2 = {hs, hs, hs};
                p3 = {hs, hs, -hs}; p4 = {-hs, hs, -hs};
                n = {0, 1, 0};
                break;
            case 3: // Bottom
                p1 = {-hs, -hs, -hs}; p2 = {hs, -hs, -hs};
                p3 = {hs, -hs, hs}; p4 = {-hs, -hs, hs};
                n = {0, -1, 0};
                break;
            case 4: // Right
                p1 = {hs, -hs, hs}; p2 = {hs, -hs, -hs};
                p3 = {hs, hs, -hs}; p4 = {hs, hs, hs};
                n = {1, 0, 0};
                break;
            case 5: // Left
                p1 = {-hs, -hs, -hs}; p2 = {-hs, -hs, hs};
                p3 = {-hs, hs, hs}; p4 = {-hs, hs, -hs};
                n = {-1, 0, 0};
                break;
        }
        
        // Bilinear interpolation
        positions.push_back(p1 + (p2 - p1) * u1 + (p4 - p1) * v1);
        positions.push_back(p1 + (p2 - p1) * u2 + (p4 - p1) * v1);
        positions.push_back(p1 + (p2 - p1) * u2 + (p4 - p1) * v2);
        positions.push_back(p1 + (p2 - p1) * u1 + (p4 - p1) * v2);
        
        uvs.push_back({u1, v1});
        uvs.push_back({u2, v1});
        uvs.push_back({u2, v2});
        uvs.push_back({u1, v2});
        
        normal.push_back(n);
    }
    
    void computeTangentSpace(std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
        for (size_t i = 0; i < indices.size(); i += 3) {
            Vertex& v0 = vertices[indices[i]];
            Vertex& v1 = vertices[indices[i + 1]];
            Vertex& v2 = vertices[indices[i + 2]];
            
            glm::vec3 edge1 = v1.position - v0.position;
            glm::vec3 edge2 = v2.position - v0.position;
            
            glm::vec2 deltaUV1 = v1.texCoords - v0.texCoords;
            glm::vec2 deltaUV2 = v2.texCoords - v0.texCoords;
            
            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
            
            glm::vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
            tangent = glm::normalize(tangent);
            
            glm::vec3 bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);
            bitangent = glm::normalize(bitangent);
            
            v0.tangent += tangent;
            v1.tangent += tangent;
            v2.tangent += tangent;
            
            v0.bitangent += bitangent;
            v1.bitangent += bitangent;
            v2.bitangent += bitangent;
        }
        
        // Normalize and orthogonalize
        for (auto& vertex : vertices) {
            vertex.tangent = glm::normalize(vertex.tangent);
            vertex.bitangent = glm::normalize(vertex.bitangent);
            
            // Gram-Schmidt orthogonalize
            vertex.tangent = glm::normalize(vertex.tangent - glm::dot(vertex.tangent, vertex.normal) * vertex.normal);
            vertex.bitangent = glm::normalize(glm::cross(vertex.normal, vertex.tangent));
        }
    }
};

#endif // MODEL_LOADER_H
