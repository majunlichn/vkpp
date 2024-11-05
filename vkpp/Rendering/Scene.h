#pragma once

#include <vkpp/Core/Context.h>
#include <vkpp/Core/Math.h>

#include <map>

namespace vkpp
{

class SceneNode;
class Mesh;
class Material;
class Light;
class Camera;

class Scene : public rad::RefCounted<Scene>
{
public:
    Scene(rad::Ref<Context> context);
    ~Scene();

    bool Upload();

    rad::Ref<Context> m_context;
    rad::Ref<SceneNode> m_root;

    std::vector<rad::Ref<Mesh>> m_meshes;
    std::vector<rad::Ref<Material>> m_materials;
    std::vector<rad::Ref<Light>> m_lights;
    std::vector<rad::Ref<Camera>> m_cameras;
    rad::Ref<Camera> m_camera;

    // AbsolutePath=>Index
    std::map<std::string, size_t, rad::StringLess> m_imagePathIndexMap;
    std::vector<rad::Ref<Image>> m_images;
    std::vector<rad::Ref<ImageView>> m_imageViews;

    Image* GetImage(std::string_view path)
    {
        auto iter = m_imagePathIndexMap.find(path);
        if (iter != m_imagePathIndexMap.end())
        {
            return m_images[iter->second].get();
        }
        return nullptr;
    }

    AABB GetBoundingBox() const;

    // glTF uses a right-handed coordinate system.
    // glTF defines +Y as up, +Z as forward, and -X as right; the front of a glTF asset faces +Z.
    void SetCameraFrontView();

}; // class Scene

} // namespace vkpp
