#pragma once
#include "stdafx.h"
#include <exception>
#include <map>
#include <string>
#include <sstream>
#include <vector>

#include <D3DX10math.h>

namespace MeshReader
{
    struct ObjTriangle
    {
        UINT vertexIndex[3];
        UINT textureIndex[3];
        UINT normalIndex[3];
        UINT materialIndex;
    };

    struct ObjMaterial
    {
        ObjMaterial(const std::string &n) :name(n), Ka(0.0, 0.0, 0.0), Kd(0.0, 0.0, 0.0), Ks(0, 0, 0), Ns(1), Tr(0), Ni(1) {};
        std::string name;
        D3DXVECTOR3 Ka;
        D3DXVECTOR3 Kd;
        D3DXVECTOR3 Ks;
        FLOAT Ns;
        FLOAT Tr;
        FLOAT Ni;
    };

    struct ObjGroup
    {
        std::vector<UINT> triangleIndices;
    };

    class ObjModel
    {
    public:
        void clear()
        {
            m_path.clear();
            m_vertices.clear();
            m_vertices.emplace_back(0.0f, 0.0f, 0.0f);
            m_textures.clear();
            m_textures.emplace_back(0.0f, 0.0f);
            m_normals.clear();
            m_normals.emplace_back(0.0f, 0.0f, 0.0f);
            m_triangles.clear();
            m_triangles.emplace_back();
            m_materials.clear();
            m_materials.emplace_back("");
            m_groups.clear();
        }
        BOOL readObj(const std::string &path);

        std::string m_path;
        std::vector<D3DXVECTOR3> m_vertices;
        std::vector<D3DXVECTOR2> m_textures;
        std::vector<D3DXVECTOR3> m_normals;
        std::vector<ObjTriangle> m_triangles;
        std::vector<ObjMaterial> m_materials;
        std::map<std::string, ObjGroup> m_groups;
    private:
        BOOL readMtl(const std::string &path);

        std::map<std::string, ObjGroup>::iterator findAndAddGroup(const std::string &name)
        {
            std::map<std::string, ObjGroup>::iterator it = m_groups.find(name);
            if (it == m_groups.end())
            {
                m_groups[name] = ObjGroup();
                it = m_groups.find(name);
            }
            return it;
        }

        UINT findMaterial(const std::string &name)
        {
            for (size_t i = 1; i < m_materials.size(); i++)
            {
                if (m_materials[i].name == name)
                {
                    return (UINT)i;
                }
            }
            return 0;
        }

        BOOL parseFaceVertex(std::stringstream &buffer, UINT &vertexIdx, UINT &textureIdx, UINT &normalIdx)
        {
            char dummy;
            /* v */
            buffer >> vertexIdx;
            if (buffer.fail())
            {
                return false;
            }
            buffer >> dummy;
            /* no t and no n */
            if (buffer.fail())
            {
                textureIdx = 0;
                normalIdx = 0;
                return true;
            }
            buffer >> textureIdx;
            /* no t */
            if (buffer.fail())
            {
                textureIdx = 0;
                buffer.clear();
                buffer >> dummy;
                buffer >> normalIdx;
                /* no n */
                if (buffer.fail())
                {
                    return false;
                }
                /* with n */
                return true;
            }
            /* with t */
            buffer >> dummy;
            /* no n */
            if (buffer.fail())
            {
                normalIdx = 0;
                return true;
            }
            /* with n */
            buffer >> normalIdx;
            if (buffer.fail())
            {
                return false;
            }
            return true;
        }
    };
}
