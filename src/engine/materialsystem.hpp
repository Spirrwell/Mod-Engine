#ifndef MATERIALSYSTEM_HPP
#define MATERIALSYSTEM_HPP

#include "vulkansystem.hpp"
#include "enginesystem.hpp"
#include "engine/imaterialsystem.hpp"
#include "filesystem.hpp"
#include "material.hpp"

#include <mutex>

class ShaderSystem;
class TextureSystem;

class MaterialSystem : public IMaterialSystem, public EngineSystem
{
public:

	void configure( Engine *engine ) override;
	void unconfigure( Engine *engine ) override;

	void LoadDefaultMaterials();

	IMaterial *GetErrorMaterial() const { return errorMaterial; }

	IMaterial *LoadMaterial( const std::filesystem::path &relpath, const std::string &pathid, IResourcePool *resourcePoolPtr ) override;
	IMaterial *FindMaterial( const std::filesystem::path &relpath, IResourcePool *resourcePoolPtr ) const override;

private:

	IMaterial *FindMaterial_Internal( const std::filesystem::path &relpath, IResourcePool *resourcePoolPtr );

	Material *errorMaterial = nullptr;

	FileSystem *fileSystem = nullptr;
	VulkanSystem *vulkanSystem = nullptr;
	TextureSystem *textureSystem = nullptr;
	ShaderSystem *shaderSystem = nullptr;

	mutable std::mutex materialsMutex;
};

#endif // MATERIALSYSTEM_HPP