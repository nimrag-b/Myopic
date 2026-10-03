// MeshCompiler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <vector>
#include <fstream>
#include <optional>

#include <limits>

#define NGE_VERSION 1
#define NGE_FORMAT_GVERTEX 1

constexpr size_t FLOATS_PER_GVERTEX = (3 + 3 + 2 + 3 + 3);



struct mesh_part {
    uint32_t material_index;
    std::vector<float> verts;
    std::vector<uint32_t> indices;
};

constexpr float max_float = std::numeric_limits<float>::max();

struct float4 {

    float x, y, z, w;

    static constexpr float4 max() {
        return float4{ max_float ,max_float ,max_float ,max_float };
    }

    bool operator ==(float4& other) const {
        return (x == other.x) && (y == other.y) && (z == other.z) && (w == other.w);
    }

};


struct material_part {
    std::string diffuse;
    std::string normal;
    float4 colour = float4::max();
    float specular = std::numeric_limits<float>::max();
};

void print_vec(aiVector3D& vec) {
    std::cout << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
}

void section_header(std::ostream& stream, const char* header, size_t size) {
    stream.write(header, strlen(header) + 1);



    uint32_t sz = size;
    stream.write((char*)&sz, sizeof(uint32_t));
}

void GetMeshPart(const aiMesh* mesh, mesh_part* part) {

    std::cout << mesh->mName.C_Str() << std::endl;
    std::cout << "Material Index = " << mesh->mMaterialIndex << std::endl;
    std::cout << "Vertex Count = " << mesh->mNumVertices << std::endl;
    std::cout << "Face Count = " << mesh->mNumFaces << std::endl;

    part->material_index = mesh->mMaterialIndex;


    for (size_t j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; j++)
    {
        if (mesh->HasTextureCoords(j)) {
            std::cout << "Has Tex Coord [" << j << "]" << std::endl;
        }

    }
    std::vector<float>& verts = part->verts;
    verts.reserve(mesh->mNumVertices);

    for (size_t v = 0; v < mesh->mNumVertices; v++)
    {
        //position
        verts.push_back(mesh->mVertices[v].x);
        verts.push_back(mesh->mVertices[v].y);
        verts.push_back(mesh->mVertices[v].z);
        //std::cout << "Vertex";
        //print_vec(mesh->mVertices[v]);

        //normals
        verts.push_back(mesh->mNormals[v].x);
        verts.push_back(mesh->mNormals[v].y);
        verts.push_back(mesh->mNormals[v].z);
        //std::cout << " Normal";
        //print_vec(mesh->mNormals[v]);

        //tex coord
        verts.push_back(mesh->mTextureCoords[0][v].x);
        verts.push_back(mesh->mTextureCoords[0][v].y);
        //std::cout << " TexCoord";
        //print_vec(mesh->mTextureCoords[0][v]);

        //tangents
        verts.push_back(mesh->mTangents[v].x);
        verts.push_back(mesh->mTangents[v].y);
        verts.push_back(mesh->mTangents[v].z);
        //std::cout << " Tangent";
        //print_vec(mesh->mTangents[v]);

        //bitangents
        //verts.push_back(mesh->mBitangents[v].x);
        //verts.push_back(mesh->mBitangents[v].y);
        //verts.push_back(mesh->mBitangents[v].z);
        //std::cout << " Bitangent";
        //print_vec(mesh->mBitangents[v]);

        //std::cout << std::endl;
    }

    std::vector<uint32_t>& indices = part->indices;
    indices.reserve(mesh->mNumFaces);

    for (size_t f = 0; f < mesh->mNumFaces; f++)
    {

        for (size_t n = 0; n < mesh->mFaces[f].mNumIndices; n++)
        {
            indices.push_back(mesh->mFaces[f].mIndices[n]);
            //std::cout << mesh->mFaces[f].mIndices[n] << ", ";
        }
        //std::cout << std::endl;

    }
    std::cout << "Vertex Count = " << verts.size() / FLOATS_PER_GVERTEX << " (" << verts.size() << ")" << std::endl << "Index Count = " << indices.size() / 3 << " (" << indices.size() << ")" << std::endl;

    std::cout << std::endl;
}



bool GetMaterialPart(const aiMaterial* mat, material_part* part) {

    std::string matName = mat->GetName().C_Str();

    aiString path;
    float fVal;
    aiColor3D col;

    if (mat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), path) == aiReturn_SUCCESS) {

        part->diffuse = path.C_Str();
        //mfs << "Diffuse = " << path.C_Str() << std::endl;
    }
    if (mat->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), path) == aiReturn_SUCCESS) {
        part->normal = path.C_Str();
        //mfs << "Normal = " << path.C_Str() << std::endl;
    }
    if (mat->Get(AI_MATKEY_SHININESS, fVal) == aiReturn_SUCCESS) {
        part->specular = fVal;
        //mfs << "Specular = " << path.C_Str() << std::endl;
    }

    if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, col) == aiReturn_SUCCESS) {
        part->colour = { col.r,col.g,col.b,1 };
        //mfs << "Specular = " << path.C_Str() << std::endl;
    }

    for (size_t i = 0; i < mat->mNumProperties; i++)
    {

        aiMaterialProperty* prop = mat->mProperties[i];
        //mat->Get(prop->mKey,)
        std::cout << prop->mKey.C_Str() << std::endl;
    }

    return true;
}

bool GetPartsForFile(const std::string& inFile, std::vector<mesh_part>& meshes, std::vector<material_part>& materials) {


    // Create an instance of the Importer class
    Assimp::Importer importer;

    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile(inFile,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        (aiProcess_MakeLeftHanded | aiProcess_FlipUVs /* | aiProcess_FlipWindingOrder */ | 0));

    // If the import failed, report it
    if (nullptr == scene) {
        std::cout << (importer.GetErrorString()) << std::endl;
        return false;
    }

    std::cout << "Mesh Count = " << scene->mNumMeshes << std::endl;

    for (size_t i = 0; i < scene->mNumMeshes; i++) {
        std::cout << "Mesh " << i << std::endl;
        mesh_part& part = meshes.emplace_back();
        GetMeshPart(scene->mMeshes[i], &part);
    }

    std::ofstream mfs;
    std::string matName;
    for (size_t i = 0; i < scene->mNumMaterials; i++)
    {
        std::cout << "Material " << i << std::endl;
        material_part& part = materials.emplace_back();
        GetMaterialPart(scene->mMaterials[i], &part);

        //aiMaterial* mat = scene->mMaterials[i];
        //matName = mat->GetName().C_Str();
        //matName += ".mat";
        //mfs.open(matName);

        //mfs << i << std::endl;

        //mfs << mat->GetName().C_Str() << std::endl << std::endl;

        //aiString path;

        //
        //if (mat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), path) == aiReturn_SUCCESS) {
        //    mfs << "Diffuse = " << path.C_Str() << std::endl;       
        //}
        //if (mat->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), path) == aiReturn_SUCCESS) {
        //    mfs << "Normal = " << path.C_Str() << std::endl;
        //}
        //if (mat->Get(AI_MATKEY_SHININESS, path) == aiReturn_SUCCESS) {
        //    mfs << "Specular = " << path.C_Str() << std::endl;
        //}

        
        //mfs << std::endl;
        //for (size_t i = 0; i < mat->mNumProperties; i++)
        //{
        //    aiMaterialProperty* prop = mat->mProperties[i];
        //    mfs << prop->mKey.C_Str() << " : ";
        //    mfs.write(prop->mData, prop->mDataLength);
        //    mfs << std::endl;
        //}

        //mfs.close();
    }

    return true;
}

bool CompileToNGE(const std::string& outFile, std::vector<mesh_part>& meshes, std::vector<material_part>& materials) {

    //write file header
    std::ofstream fs;
    fs.open(outFile, std::ios::binary);
    static char magic[] = { 'N', 'G', 'E', 0 };
    fs.write(magic, sizeof(magic));

    //meta info
    uint32_t ngeInfo[2] = {
        NGE_VERSION,
        NGE_FORMAT_GVERTEX,
    };

    fs.write((char*)ngeInfo, sizeof(ngeInfo));


    //write the mesh count
    uint32_t numMeshes = meshes.size();
    fs.write((char*)&numMeshes, sizeof(uint32_t));

    //write the material count
    uint32_t numMaterials = materials.size();
    fs.write((char*)&numMaterials, sizeof(uint32_t));

    //padding for future expanding of format without breaking compat
    uint32_t padding[4] = { 0 };
    fs.write((char*)padding, sizeof(padding));


    // Iterate over meshes and write each one to file
    for (size_t i = 0; i < meshes.size(); i++)
    {
        auto& it = meshes[i];



        //switch to next mesh
        static char mesh_section[] = ".mesh";

        fs.write(mesh_section, sizeof(mesh_section));
        uint32_t mesh_props[2] = {
            i, //mesh index
            it.material_index //material index
        };

        fs.write((char*)mesh_props, sizeof(mesh_props));

        //write sections
        section_header(fs, ".vertex", it.verts.size() * sizeof(float));
        fs.write((char*)it.verts.data(), it.verts.size() * sizeof(float));

        section_header(fs, ".index", it.indices.size() * sizeof(uint32_t));
        fs.write((char*)it.indices.data(), it.indices.size() * sizeof(uint32_t));
    }

    for (size_t i = 0; i < materials.size(); i++)
    {
        auto& it = materials[i];

        //switch to next material
        static char mat_section[] = ".mat";

        fs.write(mat_section, sizeof(mat_section));

        if (!it.diffuse.empty()) {
            section_header(fs, ".diffuse", it.diffuse.size());
            fs << it.diffuse;
        }
        if (!it.normal.empty()) {
            section_header(fs, ".normal", it.normal.size());
            fs << it.normal;
        }
        if (it.colour != float4::max()) {
            section_header(fs, ".colour", sizeof(float4));
            
            fs.write((char*)&it.colour, sizeof(float4));
        }
        if (it.specular != max_float) {
            section_header(fs, ".spec", sizeof(float));

            fs.write((char*)&it.specular, sizeof(float));
        }
    }

    return true;

}


int main(int argc, char** argv)
{    
    if (argc < 2) {
        std::cout << "usage: MeshCompiler.exe [filename] [flags]" << std::endl;
        return 1;
    }

    std::vector<std::string> input;

    std::string outName;

    for (size_t i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg[0] == '-') {
            if (arg == "-o") {
                //check to make sure we have enough args left
                if (i == (argc - 1)) {

                    std::cout << "ERROR: no output name supplied for -o" << std::endl;
                    return 1;
                }
                i++; //go to next arg
                outName = argv[i];
            }
            else {
                std::cout << "ERROR: unknown arg '" << arg << "'" << std::endl;
                return 1;
            }

            continue;
        }

        input.push_back(arg);
    }

    if (input.size() == 0) {
        std::cout << "ERROR: no input files" << std::endl;
        return 1;
    }

    //output was not explicitly inputted
    if (outName.empty()) {

        if (input.size() > 1) {
            std::cout << "ERROR: must supply output name for multi file compilation" << std::endl;
            return 1;
        }

        //check if has extension and replace with .nge
        auto s = input[0].find_last_of('.');
        if (s != std::string::npos) {
            outName = input[0].substr(0, s) + ".nge";
        }
        else {
            outName = input[0] + ".nge";
        }
    }

    std::vector<mesh_part> meshes;
    std::vector<material_part> materials;
    for (auto& it : input) {
        if (!GetPartsForFile(it, meshes, materials)) {
            return 1;
        }
    }

    CompileToNGE(outName, meshes, materials);
}
