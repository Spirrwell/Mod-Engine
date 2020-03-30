#ifndef MODELSYSTEM_HPP
#define MODELSYSTEM_HPP

#include "engine/imodelsystem.hpp"
#include "enginesystem.hpp"
#include "filesystem.hpp"
#include "materialsystem.hpp"
#include "meshsystem.hpp"
#include "model.hpp"
#include "resource.hpp"

#include <vector>
#include <mutex>

class ModelSystem : public IModelSystem, public EngineSystem
{
public:

	void configure( Engine *engine ) override;
	void unconfigure( Engine *engine ) override;

	// Loads model by relative path, GAME_DIR/models/relpath
	IModel *LoadModel( const std::filesystem::path &relpath, const std::string &pathid, IResourcePool *resourcePoolPtr ) override;
	IModel *FindModel( const std::filesystem::path &relpath, IResourcePool *resourcePoolPtr ) const override;

private:
	IModel *FindModel_Internal( const std::filesystem::path &relpath, IResourcePool *resourcePoolPtr ) const;

	FileSystem *fileSystem = nullptr;
	VulkanSystem *vulkanSystem = nullptr;
	MaterialSystem *materialSystem = nullptr;
	MeshSystem *meshSystem = nullptr;

	mutable std::mutex modelsMutex;
};

#endif // MODELSYSTEM_HPP