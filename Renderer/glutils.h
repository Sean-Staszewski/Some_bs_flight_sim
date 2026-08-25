#pragma once
#include <string>
#include <vector>

std::string  loadShaderSource(const std::string& path);
bool         loadOBJ(const std::string& path, std::vector<float>& outVertices);
void         applyPlanarUVXZ(std::vector<float>& packedVertices);
unsigned int buildShaderProgram(const char* vertSrc, const char* fragSrc);
unsigned int loadTexture(const std::string& path);