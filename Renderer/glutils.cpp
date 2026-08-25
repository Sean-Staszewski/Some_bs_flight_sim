#include "glutils.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <glm/glm.hpp>
#include "stb_image.h"

std::string loadShaderSource(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open shader file: " << path << "\n";
        return "";
    }
    std::ostringstream buf;
    buf << file.rdbuf();
    return buf.str();
}



// any polygon, and returns a flat array of
// XYZ floats in separate triangles order
bool loadOBJ(const std::string& path, std::vector<float>& outVertices)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open OBJ file: " << path << "\n";
        return false;
    }

    std::vector<float> positions;
    std::vector<float> texCoords;
    std::vector<std::vector<int>> faces;  // stores [posIdx, texIdx, posIdx, texIdx, ...]
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string token;
        ss >> token;

        if (token == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
        }
        else if (token == "vt") {
            float u, v;
            ss >> u >> v;
            texCoords.push_back(u);
            texCoords.push_back(v);
        }
        else if (token == "f") {
            std::vector<int> faceIndices;
            std::string entry;
            while (ss >> entry) {
                int posIdx = 0;
                int texIdx = 0;
                // parse formats: v, v/vt, v//vn, v/vt/vn
                size_t s1 = entry.find('/');
                if (s1 == std::string::npos) {
                    // just a vertex index
                    posIdx = std::stoi(entry);
                } else {
                    // vertex index present
                    std::string vStr = entry.substr(0, s1);
                    if (!vStr.empty()) posIdx = std::stoi(vStr);

                    size_t s2 = entry.find('/', s1 + 1);
                    if (s2 == std::string::npos) {
                        // v/vt
                        std::string tStr = entry.substr(s1 + 1);
                        if (!tStr.empty()) texIdx = std::stoi(tStr);
                    } else {
                        // v//vn or v/vt/vn
                        std::string tStr = entry.substr(s1 + 1, s2 - s1 - 1);
                        if (!tStr.empty()) texIdx = std::stoi(tStr);
                        // we ignore normal index here (entry.substr(s2+1)) because normals are computed
                    }
                }

                // convert negative indices to positive, OBJ uses 1-based indices
                if (posIdx < 0) posIdx = (int)(positions.size() / 3) + posIdx + 1;
                if (texIdx < 0) texIdx = (int)(texCoords.size() / 2) + texIdx + 1;

                // Ensure we always push a tex index slot (0 if none)
                faceIndices.push_back(posIdx);
                faceIndices.push_back(texIdx);
            }
            if (faceIndices.size() >= 6)
                faces.push_back(faceIndices);
        }
    }

    // Center the mesh at the local origin before packing vertices.
    if (!positions.empty()) {
        glm::vec3 centroid(0.0f);
        int count = 0;
        for (size_t i = 0; i + 2 < positions.size(); i += 3) {
            centroid += glm::vec3(positions[i], positions[i + 1], positions[i + 2]);
            ++count;
        }

        if (count > 0) {
            centroid /= (float)count;
            for (size_t i = 0; i + 2 < positions.size(); i += 3) {
                positions[i]     -= centroid.x;
                positions[i + 1] -= centroid.y;
                positions[i + 2] -= centroid.z;
            }
        }
    }

    int vertexCount = (int)(positions.size() / 3);
    std::vector<glm::vec3> normalSums(vertexCount, glm::vec3(0.0f));

    // Accumulate face normals per position using triangulation fan
    for (const auto& face : faces) {
        int vertsInFace = (int)face.size() / 2; // pairs of (pos, tex)
        if (vertsInFace < 3) continue;
        int i0 = face[0] - 1;
        for (int fi = 1; fi + 1 < vertsInFace; ++fi) {
            int i1 = face[fi * 2] - 1;
            int i2 = face[(fi + 1) * 2] - 1;
            if (i0 < 0 || i1 < 0 || i2 < 0 || i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount)
                continue;

            glm::vec3 p0(positions[i0 * 3 + 0], positions[i0 * 3 + 1], positions[i0 * 3 + 2]);
            glm::vec3 p1(positions[i1 * 3 + 0], positions[i1 * 3 + 1], positions[i1 * 3 + 2]);
            glm::vec3 p2(positions[i2 * 3 + 0], positions[i2 * 3 + 1], positions[i2 * 3 + 2]);

            glm::vec3 faceNormal = glm::cross(p1 - p0, p2 - p0);
            float nLen = glm::length(faceNormal);
            if (nLen <= 1e-6f)
                continue;

            faceNormal /= nLen;
            normalSums[i0] += faceNormal;
            normalSums[i1] += faceNormal;
            normalSums[i2] += faceNormal;
        }
    }

    // Build triangle vertices using same triangulation
    for (const auto& face : faces) {
        int vertsInFace = (int)face.size() / 2;
        if (vertsInFace < 3) continue;
        int basePos = face[0] - 1;
        int baseTex = face[1] - 1;
        for (int fi = 1; fi + 1 < vertsInFace; ++fi) {
            int triPos[3] = { basePos, face[fi * 2] - 1, face[(fi + 1) * 2] - 1 };
            int triTex[3] = { baseTex, face[fi * 2 + 1] - 1, face[(fi + 1) * 2 + 1] - 1 };

            for (int i = 0; i < 3; ++i) {
                int vi = triPos[i];
                int ti = triTex[i];
                if (vi < 0 || vi >= vertexCount)
                    continue;

                glm::vec3 p(positions[vi * 3 + 0], positions[vi * 3 + 1], positions[vi * 3 + 2]);
                glm::vec3 n = normalSums[vi];
                float nLen = glm::length(n);
                if (nLen > 1e-6f) {
                    n /= nLen;
                } else {
                    n = glm::vec3(0.0f, 1.0f, 0.0f);
                }

                float u = 0.0f, v = 0.0f;
                if (ti >= 0 && ti * 2 + 1 < (int)texCoords.size()) {
                    u = texCoords[ti * 2];
                    v = texCoords[ti * 2 + 1];
                }

                outVertices.push_back(p.x);
                outVertices.push_back(p.y);
                outVertices.push_back(p.z);
                outVertices.push_back(n.x);
                outVertices.push_back(n.y);
                outVertices.push_back(n.z);
                outVertices.push_back(u);
                outVertices.push_back(v);
            }
        }
    }

    return true;
}



// Compile + link a shader program
unsigned int buildShaderProgram(const char* vertSrc, const char* fragSrc)
{
    int success;
    char infoLog[512];

    unsigned int vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vertSrc, NULL);
    glCompileShader(vert);
    glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vert, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << "\n";
    }

    unsigned int frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fragSrc, NULL);
    glCompileShader(frag);
    glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(frag, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << "\n";
    }

    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(prog, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << "\n";
    }
    glDeleteShader(vert);
    glDeleteShader(frag);
    return prog;
}

unsigned int loadTexture(const std::string& path)
{
    unsigned int textureID = 0;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << "\n";
        glDeleteTextures(1, &textureID);
        return 0;
    }

    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return textureID;
}


