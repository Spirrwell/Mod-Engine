#include "modelsystem.hpp"
#include "engine.hpp"
#include "vfile.hpp"
#include "log.hpp"
#include "resourcepool.hpp"
#include "nlohmann/json.hpp"

#include "assimp/IOStream.hpp"
#include "assimp/IOSystem.hpp"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

class assimpIOStream : public Assimp::IOStream
{
public:
	assimpIOStream( const std::filesystem::path &relpath, const std::string &pathid, FileSystem *fileSystem )
	{
		file.open( relpath, pathid, fileSystem );
	}

	size_t Read( void *buffer, size_t size, size_t count ) override
	{
		return file.read( ( char* )buffer, size, count );
	}

	size_t Write( const void *buffer, size_t size, size_t count ) override
	{
		Log::PrintWarn( "[assimpIOStream::Write] FileSystem is read-only" );
		return 0;
	}

	aiReturn Seek( size_t offset, aiOrigin origin ) override
	{
		if ( !file.seek( static_cast< long int >( offset ), origin ) )
			return aiReturn_FAILURE;

		return aiReturn_SUCCESS;
	}

	// Why the hell does this have to be const
	size_t Tell() const override
	{
		return static_cast< size_t >( file.tell() );
	}

	// :/
	size_t FileSize() const override
	{
		return file.file_size();
	}

	void Flush() override
	{
		Log::PrintlnWarn( "[assimpIOStream::Flush] Does not flush" );
	}

private:

	mutable VFile file; // Screw your const Tell() function Assimp
};

class assimpIOSystem : public Assimp::IOSystem
{
public:
	assimpIOSystem( FileSystem *fileSystem, const std::string &pathid ) :
		fileSystem( fileSystem ),
		pathid( pathid )
	{
	}

	bool Exists( const char *pszFile ) const override
	{
		return fileSystem->Exists( pszFile, pathid );
	}

	char getOsSeparator() const override
	{
		return '/';
	}

	Assimp::IOStream *Open( const char *pszFile, const char *pszMode /*= "rb"*/ ) override
	{
		return new assimpIOStream( pszFile, pathid, fileSystem );
	}

	void Close( Assimp::IOStream *file ) override
	{
		delete file;
	}

private:
	FileSystem *fileSystem = nullptr;
	const std::string pathid;
};

void ModelSystem::configure( Engine *engine )
{
	EngineSystem::configure( engine );

	fileSystem = engine->GetFileSystem();
	vulkanSystem = engine->GetVulkanSystem();
	materialSystem = engine->GetMaterialSystem();
}

void ModelSystem::unconfigure( Engine *engine )
{
	fileSystem = nullptr;
	vulkanSystem = nullptr;
	materialSystem = nullptr;

	EngineSystem::unconfigure( engine );
}

IModel *ModelSystem::LoadModel( const std::filesystem::path &relpath, const std::string &pathid, IResourcePool *resourcePoolPtr )
{
	ResourcePool *resourcePool = ResourcePool::ToResourcePool( resourcePoolPtr );
	
	if ( !resourcePool )
	{
		Log::PrintlnWarn( "[ModelSystem]Resource pool is NULL" );
		return nullptr;
	}

	if ( IModel *model = FindModel_Internal( relpath, resourcePoolPtr ); model )
		return model;

	Assimp::Importer Importer;
	Importer.SetIOHandler( new assimpIOSystem( fileSystem, pathid ) );

	const aiScene *pScene = Importer.ReadFile( relpath.string(), aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_GenNormals | aiProcess_CalcTangentSpace );
	if ( !pScene )
		engine->Error( fmt::format( "Importer.ReadFile failed: {}", Importer.GetErrorString() ) );

	Log::Println( "Loading model file: {}", relpath.string() );

	std::unordered_map< std::string, std::filesystem::path > materialMap;
	const std::filesystem::path materialDefinitionsPath = fmt::format( "{}/{}.json", relpath.parent_path().generic_string(), relpath.stem().string() );
	std::vector< char > materialDefinitions;

	if ( !fileSystem->ReadToBuffer( materialDefinitionsPath, pathid, materialDefinitions ) )
		Log::PrintlnWarn( "Failed to load material definitions file {}", materialDefinitionsPath.generic_string() );
	else
	{
		Log::PrintlnRainbow( "Loading {}", materialDefinitionsPath.string() );
		using json = nlohmann::json;
		json j = json::parse( materialDefinitions );

		for ( auto kv : j.items() )
		{
			materialMap[ kv.key() ] = ( std::string )kv.value();
		}
	}

	auto resource = ResourcePool::createResource< Model >( ResourceInfo { relpath.generic_string() } );
	Model *model = resource->resource.get();
	model->meshes.resize( pScene->mNumMeshes );

	for ( unsigned int meshidx = 0; meshidx < pScene->mNumMeshes; ++meshidx )
	{
		Material *material = nullptr;

		model->meshes[ meshidx ] = make_unique< Mesh >( vulkanSystem );
		const aiMesh *pAIMesh = pScene->mMeshes[ meshidx ];

		if ( pAIMesh->mMaterialIndex >= 0 )
		{
			aiMaterial *pAIMaterial = pScene->mMaterials[ pAIMesh->mMaterialIndex ];

			if ( pAIMaterial != nullptr )
			{
				aiString matName;
				pAIMaterial->Get( AI_MATKEY_NAME, matName );

				//std::filesystem::path matPath = relpath.parent_path() / matName.C_Str() += ".json";

				if ( auto it = materialMap.find( matName.C_Str() ); it != materialMap.end() )
				{
					const std::filesystem::path matPath = it->second;
					material = Material::ToMaterial( materialSystem->LoadMaterial( matPath, pathid, resourcePoolPtr ) );
				}
				else
				{
					Log::PrintlnWarn( "Material definition for {} material not found", matName.C_Str() );
					material = Material::ToMaterial( materialSystem->GetErrorMaterial() );
				}
			}
		}

		if ( !material )
			return nullptr;

		const VertexLayout vertexLayout = material->GetShader()->GetVertexLayout();

		shared_ptr< VertexArray > vertices = std::make_shared< VertexArray >( vertexLayout );
		vertices->Resize( pAIMesh->mNumVertices );

		shared_ptr< std::vector< uint32_t > > indices = std::make_shared< std::vector< uint32_t > >( pAIMesh->mNumFaces * 3 ); // * 3 because we're triangulating

		for ( unsigned int v = 0; v < pAIMesh->mNumVertices; ++v )
		{
			constexpr glm::vec4 defaultColor = { 1.0f, 1.0f, 1.0f, 1.0f };

			if ( pAIMesh->HasPositions() ) {
				vertices->SetPosition( v, { pAIMesh->mVertices[ v ].x, -pAIMesh->mVertices[ v ].y, pAIMesh->mVertices[ v ].z }, 0 );
			}

			if ( pAIMesh->HasNormals() ) {
				vertices->SetNormal( v, { pAIMesh->mNormals[ v ].x, pAIMesh->mNormals[ v ].y, pAIMesh->mNormals[ v ].z }, 0 );
			}

			if ( pAIMesh->HasVertexColors( 0 ) ) {
				vertices->SetColor( v, { pAIMesh->mColors[ 0 ][ v ].r, pAIMesh->mColors[ 0 ][ v ].g, pAIMesh->mColors[ 0 ][ v ].b, pAIMesh->mColors[ 0 ][ v ].a  }, 0 );
			}
			else {
				vertices->SetColor( v, defaultColor, 0 );
			}
			
			if ( pAIMesh->HasTextureCoords( 0 ) ) {
				vertices->SetUV( v, { pAIMesh->mTextureCoords[ 0 ][ v ].x, -pAIMesh->mTextureCoords[ 0 ][ v ].y }, 0 );
			}

			if ( pAIMesh->HasTangentsAndBitangents() ) {
				vertices->SetTangent( v, { pAIMesh->mTangents[ v ].x, -pAIMesh->mTangents[ v ].y, pAIMesh->mTangents[ v ].z }, 0 );
				vertices->SetBiTangent( v, { pAIMesh->mBitangents[ v ].x, -pAIMesh->mBitangents[ v ].y, pAIMesh->mBitangents[ v ].z }, 0 );
			}
		}

		for ( unsigned int face = 0; face < pAIMesh->mNumFaces; ++face )
		{
			for ( unsigned int idx = 0; idx < pAIMesh->mFaces[ face ].mNumIndices; ++idx )
				indices->at( ( face * 3 ) + idx ) = pAIMesh->mFaces[ face ].mIndices[ idx ];
		}

		model->meshes[ meshidx ]->Init( vulkanSystem, vertices, indices, material );
	}

	modelsMutex.lock();
	resourcePool->models.push_back( resource );
	modelsMutex.unlock();

	return model;
}

IModel *ModelSystem::FindModel( const std::filesystem::path &relpath, IResourcePool *resourcePoolPtr ) const
{
	if ( ResourcePool *resourcePool = ResourcePool::ToResourcePool( resourcePoolPtr ); resourcePool )
		return resourcePool->findResource< Model >( relpath.generic_string() );
	
	return nullptr;
}

IModel *ModelSystem::FindModel_Internal( const std::filesystem::path &relpath, IResourcePool *resourcePoolPtr ) const
{
	// We only need to lock guard our own lookups
	std::lock_guard< std::mutex > lock( modelsMutex );
	return FindModel( relpath, resourcePoolPtr );
}
