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
    std::map<std::string, size_t, rad::StringLess> m_imageNameIndexMap;
    // Image at index 0 is a fallback image (black).
    std::vector<rad::Ref<Image>> m_image2Ds;
    std::vector<rad::Ref<ImageView>> m_image2DViews;

    // Only create new image if the fileName is not found, return the index in m_image2Ds.
    uint32_t UploadImage2DFromFile(std::string_view fileName, bool genMipmaps);
    uint32_t GetImage2DIndex(std::string_view path);
    Image* GetImage2D(size_t index) { return m_image2Ds[index].get(); }
    Image* GetImage2D(std::string_view path);

    AABB GetBoundingBox() const;

    // glTF uses a right-handed coordinate system.
    // glTF defines +Y as up, +Z as forward, and -X as right; the front of a glTF asset faces +Z.
    void SetCameraFrontView();

}; // class Scene

} // namespace vkpp
