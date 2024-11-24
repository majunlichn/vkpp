#include <vkpp/Scene/SceneImporter.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace vkpp
{

SceneImporter::SceneImporter(rad::Ref<Scene> scene) :
    m_scene(std::move(scene))
{
}

SceneImporter::~SceneImporter()
{
}

bool SceneImporter::Import(std::string_view fileName)
{
    Assimp::Importer importer;
    m_sceneData = importer.ReadFile(fileName.data(),
        aiProcessPreset_TargetRealtime_Fast |
        aiProcess_FlipUVs |
        aiProcess_GenBoundingBoxes);

    if (m_sceneData)
    {
        m_filePath = rad::MakeAbsolute((const char8_t*)fileName.data());
        // Import meshes:
        m_meshes.resize(m_sceneData->mNumMeshes);
        for (uint32_t i = 0; i < m_sceneData->mNumMeshes; ++i)
        {
            const aiMesh* meshData = m_sceneData->mMeshes[i];
            m_meshes[i] = ImportMesh(meshData);
        }
        // Import materials:
        m_materials.resize(m_sceneData->mNumMaterials);
        for (uint32_t i = 0; i < m_sceneData->mNumMaterials; ++i)
        {
            const aiMaterial* materialData = m_sceneData->mMaterials[i];
            m_materials[i] = ImportMaterial(materialData);
        }
        // Set material for each mesh:
        for (uint32_t i = 0; i < m_meshes.size(); ++i)
        {
            Mesh* mesh = m_meshes[i].get();
            const aiMesh* meshData = m_sceneData->mMeshes[i];
            mesh->m_material = m_materials[meshData->mMaterialIndex];
        }
        // Import lights:
        m_lights.resize(m_sceneData->mNumLights);
        for (uint32_t i = 0; i < m_lights.size(); ++i)
        {
            m_lights[i] = ImportLight(m_sceneData->mLights[i]);
        }
        // Import cameras:
        m_cameras.resize(m_sceneData->mNumCameras);
        for (uint32_t i = 0; i < m_cameras.size(); ++i)
        {
            m_cameras[i] = ImportCamera(m_sceneData->mCameras[i]);
        }
        // Import nodes:
        m_rootNode = RAD_NEW SceneNode(m_scene.get(), nullptr);
        m_rootNode->m_name = (const char*)m_filePath.filename().u8string().c_str();
        ImportNodes(m_rootNode.get(), m_sceneData->mRootNode);

        if (m_meshes.size() > 0)
        {
            m_scene->m_meshes.reserve(m_scene->m_meshes.size() + m_meshes.size());
            m_scene->m_meshes.insert(m_scene->m_meshes.end(), m_meshes.begin(), m_meshes.end());
        }
        if (m_materials.size() > 0)
        {
            m_scene->m_materials.reserve(m_scene->m_materials.size() + m_materials.size());
            m_scene->m_materials.insert(m_scene->m_materials.end(), m_materials.begin(), m_materials.end());
        }
        if (m_lights.size() > 0)
        {
            m_scene->m_lights.reserve(m_scene->m_lights.size() + m_lights.size());
            m_scene->m_lights.insert(m_scene->m_lights.end(), m_lights.begin(), m_lights.end());
        }
        if (m_cameras.size() > 0)
        {
            m_scene->m_cameras.reserve(m_scene->m_cameras.size() + m_cameras.size());
            m_scene->m_cameras.insert(m_scene->m_cameras.end(), m_cameras.begin(), m_cameras.end());
        }
        m_scene->m_root->m_children.push_back(m_rootNode);
        return true;
    }
    else
    {
        VKPP_LOG(err, "Assimp::Importer::ReadFile(fileName={}): ({})",
            fileName.data(), importer.GetErrorString());
        return false;
    }
}

static_assert(sizeof(glm::vec2) == sizeof(aiVector2D));
static_assert(sizeof(glm::vec3) == sizeof(aiVector3D));
static_assert(sizeof(glm::vec4) == sizeof(aiColor4D));

glm::vec2 ToVec2(const aiVector2D& vec)
{
    return glm::vec2(vec[0], vec[1]);
}

glm::vec3 ToVec3(const aiVector3D& vec)
{
    return glm::vec3(vec[0], vec[1], vec[2]);
}

glm::vec3 ToVec3(const aiColor3D& vec)
{
    return glm::vec3(vec.r, vec.g, vec.b);
}

glm::vec4 ToVec4(const aiColor4D& vec)
{
    return glm::vec4(vec.r, vec.g, vec.b, vec.a);
}

glm::mat4 ToMat4(const aiMatrix4x4& matrix)
{
    return glm::mat4(
        matrix[0][0], matrix[1][0], matrix[2][0], matrix[3][0],
        matrix[0][1], matrix[1][1], matrix[2][1], matrix[3][1],
        matrix[0][2], matrix[1][2], matrix[2][2], matrix[3][2],
        matrix[0][3], matrix[1][3], matrix[2][3], matrix[3][3]
    );
}

rad::Ref<Mesh> SceneImporter::ImportMesh(const aiMesh* meshData)
{
    rad::Ref<Mesh> mesh = RAD_NEW Mesh(m_scene.get());
    mesh->m_name = meshData->mName.C_Str();
    uint32_t numVertPerPrim = 0;
    if (meshData->mPrimitiveTypes == rad::ToUnderlying(aiPrimitiveType_POINT))
    {
        mesh->m_primitiveType = PrimitiveType::Point;
        numVertPerPrim = 1;
    }
    else if (meshData->mPrimitiveTypes == rad::ToUnderlying(aiPrimitiveType_LINE))
    {
        mesh->m_primitiveType = PrimitiveType::Line;
        numVertPerPrim = 2;
    }
    else if (meshData->mPrimitiveTypes == rad::ToUnderlying(aiPrimitiveType_TRIANGLE))
    {
        mesh->m_primitiveType = PrimitiveType::Triangle;
        numVertPerPrim = 3;
    }
    else
    {
        VKPP_LOG(err, "Failed to import aiMesh {}: PrimitiveTypes 0x{:04X}",
            meshData->mName.C_Str(), meshData->mPrimitiveTypes);
        return nullptr;
    }

    if (meshData->HasPositions())
    {
        mesh->m_positions.resize(meshData->mNumVertices);
        memcpy(mesh->m_positions.data(), meshData->mVertices,
            mesh->m_positions.size() * sizeof(glm::vec3));
    }
    if (meshData->HasNormals())
    {
        mesh->m_normals.resize(meshData->mNumVertices);
        memcpy(mesh->m_normals.data(), meshData->mNormals,
            mesh->m_normals.size() * sizeof(glm::vec3));
    }
    if (meshData->HasTangentsAndBitangents())
    {
        mesh->m_tangents.resize(meshData->mNumVertices);
        mesh->m_bitangents.resize(meshData->mNumVertices);
        for (size_t i = 0; i < meshData->mNumVertices; ++i)
        {
            glm::vec3 n = ToVec3(meshData->mNormals[i]);
            glm::vec3 t = ToVec3(meshData->mTangents[i]);
            glm::vec3 b = ToVec3(meshData->mBitangents[i]);
            if (dot(cross(n, t), b) >= 0.0f)
            {
                mesh->m_tangents[i] = glm::vec4(t, +1.0f);
            }
            else
            {
                mesh->m_tangents[i] = glm::vec4(t, -1.0f);
            }
        }
        memcpy(mesh->m_bitangents.data(), meshData->mBitangents,
            mesh->m_bitangents.size() * sizeof(glm::vec3));
    }
    uint32_t colorChannelCount = meshData->GetNumColorChannels();
    if (colorChannelCount > 0)
    {
        mesh->m_colors.resize(colorChannelCount);
        memcpy(mesh->m_colors.data(), meshData->mColors[0],
            mesh->m_colors.size() * sizeof(glm::vec4));
    }
    uint32_t uvChannelCount = meshData->GetNumUVChannels();
    if (uvChannelCount > 0)
    {
        mesh->m_uvChannels.resize(uvChannelCount);
        for (uint32_t channelIndex = 0; channelIndex < uvChannelCount; ++channelIndex)
        {
            uint32_t uvComponentCount = meshData->mNumUVComponents[channelIndex];
            if (uvComponentCount != 2)
            {
                VKPP_LOG(err, "Failed to import aiMesh {}: UV#{} has {} compoments!",
                    meshData->mName.C_Str(), channelIndex, uvComponentCount);
                return nullptr;
            }

            static_assert(sizeof(aiVector3D) == 3 * sizeof(float));
            for (uint32_t i = 0; i < meshData->mNumVertices; ++i)
            {
                aiVector3D uv = meshData->mTextureCoords[channelIndex][i];
                mesh->m_uvChannels[channelIndex].push_back(glm::vec2(uv.x, uv.y));
            }
        }
    }

    if (numVertPerPrim > 0)
    {
        mesh->m_indices.reserve(size_t(meshData->mNumFaces) * size_t(numVertPerPrim));
    }
    for (size_t faceIndex = 0; faceIndex < meshData->mNumFaces; ++faceIndex)
    {
        const aiFace& polygonData = meshData->mFaces[faceIndex];
        assert(polygonData.mNumIndices == numVertPerPrim);
        for (size_t vertIndex = 0; vertIndex < numVertPerPrim; ++vertIndex)
        {
            mesh->m_indices.push_back(polygonData.mIndices[vertIndex]);
        }
    }

    if (meshData->HasBones())
    {
        mesh->m_bones.resize(meshData->mNumBones);
        for (size_t i = 0; i < meshData->mNumBones; ++i)
        {
            Bone& bone = mesh->m_bones[i];
            const aiBone* boneData = meshData->mBones[i];
            bone.name = boneData->mName.C_Str();
            for (size_t weightIndex = 0; weightIndex < boneData->mNumWeights; ++weightIndex)
            {
                VertexWeight weight = {};
                weight.index = boneData->mWeights[i].mVertexId;
                weight.weight = boneData->mWeights[i].mWeight;
                bone.weights.push_back(weight);
            }
            bone.inverseBindMatrix = ToMat4(boneData->mOffsetMatrix);
        }
    }

    mesh->m_aabb.m_min = ToVec3(meshData->mAABB.mMin);
    mesh->m_aabb.m_max = ToVec3(meshData->mAABB.mMax);

    if (mesh->m_primitiveType == PrimitiveType::Triangle)
    {
        if (mesh->m_positions.size() > 0 &&
            mesh->m_normals.size() > 0 &&
            mesh->m_tangents.size() > 0 &&
            mesh->m_uvChannels.size() == 1)
        {
            mesh->m_renderType = RenderType::TriangleListTextured;
        }
    }

    return mesh;
}

rad::Ref<Material> SceneImporter::ImportMaterial(const aiMaterial* materialData)
{
    rad::Ref<Material> material = RAD_NEW Material(m_scene.get());
    material->m_name = materialData->GetName().C_Str();

    aiVector3D baseColor = {};
    if (materialData->Get(AI_MATKEY_BASE_COLOR, baseColor) != aiReturn_SUCCESS)
    {
        materialData->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor);
    }
    material->m_baseColor = glm::vec4(ToVec3(baseColor), 1.0f);

    material->m_baseColorTexture = GetTextureInfo(materialData, AI_MATKEY_BASE_COLOR_TEXTURE);
    if (!material->m_baseColorTexture)
    {
        material->m_baseColorTexture = GetTextureInfo(materialData, aiTextureType_DIFFUSE, 0);
    }

    // Metallic factor. 0.0 = Full Dielectric, 1.0 = Full Metal
    materialData->Get(AI_MATKEY_METALLIC_FACTOR, material->m_metallic);
    // Roughness factor. 0.0 = Perfectly Smooth, 1.0 = Completely Rough
    materialData->Get(AI_MATKEY_ROUGHNESS_FACTOR, material->m_roughness);

    int useMetallicMap = 0;
    materialData->Get(AI_MATKEY_USE_METALLIC_MAP, useMetallicMap);
    int useRoughnessMap = 0;
    materialData->Get(AI_MATKEY_USE_ROUGHNESS_MAP, useRoughnessMap);
    // TODO: combine the two if metallic and roughness textures are separated.
    material->m_metallicRoughnessTexture =
        GetTextureInfo(materialData, AI_MATKEY_METALLIC_TEXTURE);
    rad::Ref<TextureInfo> roughnessTexture =
        GetTextureInfo(materialData, AI_MATKEY_ROUGHNESS_TEXTURE);

    material->m_normalTexture =
        GetTextureInfo(materialData, aiTextureType_NORMAL_CAMERA, 0);
    if (!material->m_normalTexture)
    {
        material->m_normalTexture =
            GetTextureInfo(materialData, aiTextureType_NORMALS, 0);
    }

    int useAmbientMap = 0;
    materialData->Get(AI_MATKEY_USE_AO_MAP, useAmbientMap);
    material->m_ambientTexture =
        GetTextureInfo(materialData, aiTextureType_AMBIENT_OCCLUSION, 0);
    if (!material->m_ambientTexture)
    {
        material->m_ambientTexture =
            GetTextureInfo(materialData, aiTextureType_AMBIENT, 0);
    }

    int useEmissiveMap = 0;
    materialData->Get(AI_MATKEY_USE_EMISSIVE_MAP, useEmissiveMap);
    aiVector3D emissiveIntensity = {};
    materialData->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissiveIntensity);
    material->m_emissiveColor = ToVec3(emissiveIntensity);
    material->m_emissiveTexture =
        GetTextureInfo(materialData, aiTextureType_EMISSION_COLOR, 0);
    if (!material->m_emissiveTexture)
    {
        material->m_emissiveTexture =
            GetTextureInfo(materialData, aiTextureType_EMISSIVE, 0);
    }

    material->m_alphaMode = AlphaMode::Opaque;
    float opacity = 1.0f;
    materialData->Get(AI_MATKEY_OPACITY, opacity);
    if (opacity < 1.0f)
    {
        material->m_alphaMode = AlphaMode::Blend;
    }

    int twoSided = 0;
    materialData->Get(AI_MATKEY_TWOSIDED, twoSided);
    if (twoSided)
    {
        material->m_doubleSided = true;
    }

    return material;
}

VkSamplerAddressMode GetTextureAddressMode(aiTextureMapMode mapMode)
{
    switch (mapMode)
    {
    case aiTextureMapMode_Wrap:     return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case aiTextureMapMode_Clamp:    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case aiTextureMapMode_Decal:    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    case aiTextureMapMode_Mirror:   return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    }
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}

rad::Ref<TextureInfo> SceneImporter::GetTextureInfo(
    const aiMaterial* materialData, aiTextureType type, int index)
{
    if (materialData->GetTextureCount(type) > 0)
    {
        rad::Ref<TextureInfo> texInfo = RAD_NEW TextureInfo();

        aiString path;
        aiTextureMapping texMapping;
        uint32_t texCoordIndex = 0;
        float blend = 0;
        aiTextureOp op = aiTextureOp_Add;
        aiTextureMapMode mapMode[3];

        materialData->GetTexture(type, index,
            &path, &texMapping, &texCoordIndex, &blend, &op, mapMode);

        rad::FilePath texPath = (const char8_t*)path.C_Str();
        texPath = rad::MakeAbsolute(m_filePath.parent_path() / texPath);

        texInfo->fileName = (const char*)texPath.u8string().c_str();
        texInfo->uvIndex = texCoordIndex;
        texInfo->addressModeU = GetTextureAddressMode(mapMode[0]);
        texInfo->addressModeV = GetTextureAddressMode(mapMode[1]);
        return texInfo;
    }
    else
    {
        return nullptr;
    }
}

const char* ToString(aiLightSourceType type)
{
    switch (type)
    {
    case aiLightSource_DIRECTIONAL: return "DIRECTIONAL";
    case aiLightSource_POINT: return "POINT";
    case aiLightSource_SPOT: return "SPOT";
    case aiLightSource_AMBIENT: return "AMBIENT";
    case aiLightSource_AREA: return "AREA";
    }
    return "UNDEFINED";
}

rad::Ref<Light> SceneImporter::ImportLight(const aiLight* lightData)
{
    rad::Ref<Light> light = RAD_NEW Light(m_scene.get());
    light->m_name = lightData->mName.C_Str();
    if (lightData->mType == aiLightSource_DIRECTIONAL)
    {
        light->m_type = Light::Type::Directional;
    }
    else if (lightData->mType == aiLightSource_POINT)
    {
        light->m_type = Light::Type::Point;
    }
    else if (lightData->mType == aiLightSource_SPOT)
    {
        light->m_type = Light::Type::Spot;
    }
    else
    {
        VKPP_LOG(err, "Failed to import aiLight {}: aiLightSource={}!",
            lightData->mName.C_Str(), ToString(lightData->mType));
        return nullptr;
    }
    light->m_position = ToVec3(lightData->mPosition);
    light->m_direction = ToVec3(lightData->mDirection);
    light->m_color = ToVec3(lightData->mColorDiffuse);
    light->m_intensity = 1.0f;
    light->m_innerConeAngle = lightData->mAngleInnerCone;
    light->m_outerConeAngle = lightData->mAngleOuterCone;
    light->m_range = FLT_MAX;
    return light;
}

rad::Ref<Camera> SceneImporter::ImportCamera(const aiCamera* cameraData)
{
    rad::Ref<Camera> camera = RAD_NEW Camera(m_scene.get());
    camera->m_name = cameraData->mName.C_Str();
    if (cameraData->mOrthographicWidth == 0)
    {
        camera->m_type = Camera::Type::Perspective;
    }
    else
    {
        camera->m_type = Camera::Type::Orthographic;
    }
    camera->m_position = ToVec3(cameraData->mPosition);
    camera->m_lookAt = ToVec3(cameraData->mLookAt);
    camera->m_up = ToVec3(cameraData->mUp);
    camera->m_aspectRatio = cameraData->mAspect;
    if (cameraData->mAspect > 0.0f)
    {
        camera->m_yfov = camera->ConvertFovH2V(cameraData->mHorizontalFOV, cameraData->mAspect);
    }
    camera->m_clipNear = cameraData->mClipPlaneNear;
    camera->m_clipFar = cameraData->mClipPlaneFar;
    camera->m_xmag = cameraData->mOrthographicWidth;
    if (cameraData->mAspect > 0.0f)
    {
        camera->m_ymag = cameraData->mOrthographicWidth / cameraData->mAspect;
    }
    return camera;
}

void SceneImporter::ImportNodes(SceneNode* node, const aiNode* nodeData)
{
    node->m_name = nodeData->mName.C_Str();
    node->m_transform = ToMat4(nodeData->mTransformation);
    if (node->m_parent)
    {
        node->m_transformToWorld = node->m_transform * node->m_parent->m_transformToWorld;
    }
    else
    {
        node->m_transformToWorld = node->m_transform;
    }

    for (unsigned int i = 0; i < nodeData->mNumMeshes; i++)
    {
        node->m_meshes.push_back(m_meshes[nodeData->mMeshes[i]]);
    }

    for (unsigned int i = 0; i < nodeData->mNumChildren; ++i)
    {
        rad::Ref<SceneNode> child = new SceneNode(node->m_scene, node);
        ImportNodes(child.get(), nodeData->mChildren[i]);
        node->m_children.push_back(std::move(child));
    }
}

} // namespace vkpp
