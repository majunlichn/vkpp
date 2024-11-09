#pragma once

#include <vkpp/Core/Context.h>

namespace vkpp
{

rad::Ref<Image> CreateImage2DFromFile(
    Context* context, std::string_view fileName, bool genMipmaps);

} // namespace vkpp
