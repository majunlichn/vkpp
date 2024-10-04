#pragma once

#include <vkpp/Rendering/Scene.h>
#include <vkpp/Rendering/SceneNode.h>
#include <vkpp/Rendering/Mesh.h>
#include <vkpp/Rendering/Material.h>
#include <vkpp/Rendering/Light.h>
#include <vkpp/Rendering/Camera.h>

#include <assimp/scene.h>

#include <rad/IO/FileSystem.h>

namespace vkpp
{

class SceneImporter : public rad::RefCounted<SceneImporter>
{
public:
    SceneImporter(rad::Ref<Scene> scene);
    ~SceneImporter();

    bool Import(std::string_view fileName);
    rad::Ref<Mesh> ImportMesh(const aiMesh* meshData);
    rad::Ref<Material> ImportMaterial(const aiMaterial* materialData);
    rad::Ref<TextureInfo> GetTextureInfo(
        const aiMaterial* materialData, aiTextureType type, int index = 0);
    rad::Ref<Light> ImportLight(const aiLight* lightData);
    rad::Ref<Camera> ImportCamera(const aiCamera* cameraData);
    void ImportNodes(SceneNode* node, const aiNode* nodeData);

    rad::Ref<Scene> m_scene;
    rad::FilePath m_filePath;
    const aiScene* m_sceneData = nullptr;
    rad::Ref<SceneNode> m_rootNode;
    std::vector<rad::Ref<Mesh>> m_meshes;
    std::vector<rad::Ref<Material>> m_materials;
    std::vector<rad::Ref<Light>> m_lights;
    std::vector<rad::Ref<Camera>> m_cameras;

}; // class SceneImporter

} // namespace vkpp
