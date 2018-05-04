#include "stdafx.h"
#include "MeshReader.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

BOOL MeshReader::ObjModel::readObj(const std::string &path)
{
    clear();
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::ios_base::failure("Can't open file");
    }
    m_path = path;
    std::map<std::string, ObjGroup>::iterator it_group = findAndAddGroup("default");
    UINT materialIndex = 0;

    std::string token;
    std::string line;
    std::stringstream lineBuffer;
    while (std::getline(file, line))
    {
        lineBuffer.str("");
        lineBuffer.clear();
        lineBuffer.sync();
        while (line.length() > 0 && line[line.length() - 1] == '\\')
        {
            line.pop_back();
            lineBuffer << line;
            std::getline(file, line);
        }
        lineBuffer << line;

        /* Each line */
        if (lineBuffer >> token)
        {
            /* Comment */
            if (token[0] == '#')
            {
                /* Ignore */
            }
            /* Material lib */
            else if (token == "mtllib")
            {
                lineBuffer >> token;
                readMtl(path.substr(0, path.find_last_of('/')) + "/" + token);
            }
            /* Group */
            else if (token == "g")
            {
                lineBuffer >> token;
                it_group = findAndAddGroup(token);
            }
            /* Use material */
            else if (token == "usemtl")
            {
                lineBuffer >> token;
                materialIndex = findMaterial(token);
            }
            /* Face */
            else if (token == "f")
            {
                UINT idx = 0;
                std::stringstream tokenBuf;
                UINT vIdx, tIdx, nIdx;
                m_triangles.emplace_back();
                m_triangles.back().materialIndex = materialIndex;
                it_group->second.triangleIndices.push_back(static_cast<UINT>(m_triangles.size() - 1));

                while (lineBuffer >> token)
                {
                    tokenBuf.str("");
                    tokenBuf.clear();
                    tokenBuf.sync();
                    tokenBuf << token;

                    if (!parseFaceVertex(tokenBuf, vIdx, tIdx, nIdx))
                    {
                        throw "Invalid OBJ file!";
                    }

                    if (idx < 3)
                    {
                        m_triangles.back().vertexIndex[idx] = vIdx;
                        m_triangles.back().textureIndex[idx] = tIdx;
                        m_triangles.back().normalIndex[idx] = nIdx;
                    }
                    else
                    {
                        m_triangles.emplace_back();
                        m_triangles.back().materialIndex = materialIndex;
                        it_group->second.triangleIndices.push_back(static_cast<UINT>(m_triangles.size() - 1));
                        m_triangles.back().vertexIndex[0] = m_triangles[m_triangles.size() - 2].vertexIndex[0];
                        m_triangles.back().vertexIndex[1] = m_triangles[m_triangles.size() - 2].vertexIndex[2];
                        m_triangles.back().vertexIndex[2] = vIdx;
                        m_triangles.back().textureIndex[0] = m_triangles[m_triangles.size() - 2].textureIndex[0];
                        m_triangles.back().textureIndex[1] = m_triangles[m_triangles.size() - 2].textureIndex[2];
                        m_triangles.back().textureIndex[2] = tIdx;
                        m_triangles.back().normalIndex[0] = m_triangles[m_triangles.size() - 2].normalIndex[0];
                        m_triangles.back().normalIndex[1] = m_triangles[m_triangles.size() - 2].normalIndex[2];
                        m_triangles.back().normalIndex[2] = nIdx;
                    }
                    ++idx;
                }
            }
            /* Vertex */
            else if (token == "v")
            {
                FLOAT x, y, z;
                lineBuffer >> x >> y >> z;
                m_vertices.emplace_back(x, y, z);
            }
            /* Texture */
            else if (token == "vt")
            {
                FLOAT x, y;
                lineBuffer >> x >> y;
                m_textures.emplace_back(x, y);
            }
            /* Normal */
            else if (token == "vn")
            {
                FLOAT x, y, z;
                lineBuffer >> x >> y >> z;
                m_normals.emplace_back(x, y, z);
            }
        }
    }
    file.close();

    /* normalize */
    //PW::Math::Vector3d meanV;
    //PWdouble minX = m_vertices[1].getX();
    //PWdouble minY = m_vertices[1].getY();
    //PWdouble minZ = m_vertices[1].getZ();
    //PWdouble maxX = m_vertices[1].getX();
    //PWdouble maxY = m_vertices[1].getY();
    //PWdouble maxZ = m_vertices[1].getZ();
    //meanV.setX(0);
    //meanV.setY(0);
    //meanV.setZ(0);
    //for (int i = 1; i < m_vertices.size(); ++i)
    //{
    //    minX = std::min(minX, m_vertices[i].getX());
    //    minY = std::min(minY, m_vertices[i].getY());
    //    minZ = std::min(minZ, m_vertices[i].getZ());
    //    maxX = std::max(maxX, m_vertices[i].getX());
    //    maxY = std::max(maxY, m_vertices[i].getY());
    //    maxZ = std::max(maxZ, m_vertices[i].getZ());
    //    meanV += m_vertices[i];
    //}
    //meanV /= m_vertices.size() - 1;
    //PWdouble width = std::max(maxX - minX, std::max(maxY - minY, maxZ - minZ));
    //for (int i = 1; i < m_vertices.size(); ++i)
    //{
    //    m_vertices[i] = (m_vertices[i] - meanV) / width * 25;
    //}
    return true;
}

BOOL MeshReader::ObjModel::readMtl(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::ios_base::failure("Can't open file");
    }
    UINT mtlIdx = 0;
    std::string token;
    std::string line;
    std::stringstream lineBuffer;
    while (std::getline(file, line))
    {
        lineBuffer.str("");
        lineBuffer.clear();
        lineBuffer.sync();
        while (line.length() > 0 && line[line.length() - 1] == '\\')
        {
            line.pop_back();
            lineBuffer << line;
            std::getline(file, line);
        }
        lineBuffer << line;

        /* Each line */
        if (lineBuffer >> token)
        {
            /* Comment */
            if (token[0] == '#')
            {
                /* Ignore */
            }
            /* New material */
            else if (token == "newmtl")
            {
                lineBuffer >> token;
                mtlIdx = findMaterial(token);
                if (mtlIdx == 0)
                {
                    m_materials.emplace_back(token);
                    mtlIdx = static_cast<UINT>(m_materials.size() - 1);
                }
            }
            /* Ambient */
            else if (token == "Ka")
            {
                FLOAT x, y, z;
                lineBuffer >> x >> y >> z;
                m_materials[mtlIdx].Ka.x = x;
                m_materials[mtlIdx].Ka.y = y;
                m_materials[mtlIdx].Ka.z = z;
            }
            /* Diffuse */
            else if (token == "Kd")
            {
                FLOAT x, y, z;
                lineBuffer >> x >> y >> z;
                m_materials[mtlIdx].Kd.x = x;
                m_materials[mtlIdx].Kd.y = y;
                m_materials[mtlIdx].Kd.z = z;
            }
            /* Specular */
            else if (token == "Ks")
            {
                FLOAT x, y, z;
                lineBuffer >> x >> y >> z;
                m_materials[mtlIdx].Ks.x = x;
                m_materials[mtlIdx].Ks.y = y;
                m_materials[mtlIdx].Ks.z = z;
                m_materials[mtlIdx].Ns = 2;
            }
            /* Specular Exponent */
            else if (token == "Ns")
            {
                FLOAT x;
                lineBuffer >> x;
                m_materials[mtlIdx].Ns = x;
            }
            /* Transparency filter */
            else if (token == "Tr")
            {
                FLOAT x;
                lineBuffer >> x;
                m_materials[mtlIdx].Tr = x;
            }
            /* Optical density */
            else if (token == "Ni")
            {
                FLOAT x;
                lineBuffer >> x;
                m_materials[mtlIdx].Ni = x;
            }
        }
    }
    file.close();
    return true;
}
