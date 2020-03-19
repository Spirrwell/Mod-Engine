#include "materialsystem.hpp"
#include "engine.hpp"
#include "shadersystem.hpp"
#include "texturesystem.hpp"
#include "resourcepool.hpp"
#include "log.hpp"
#include "nlohmann/json.hpp"

#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"

#include <array>

void MaterialSystem::configure( Engine *engine )
{
	EngineSystem::configure( engine );

	fileSystem = engine->GetFileSystem();
	vulkanSystem = engine->GetVulkanSystem();
	textureSystem = engine->GetTextureSystem();
	shaderSystem = engine->GetShaderSystem();
}

void MaterialSystem::unconfigure( Engine *engine )
{
	fileSystem = nullptr;
	vulkanSystem = nullptr;
	shaderSystem = nullptr;

	EngineSystem::unconfigure( engine );
}

void MaterialSystem::LoadDefaultMaterials()
{
	errorMaterial = Material::ToMaterial( LoadMaterial( "materials/error.json", "GAME", engine->GetGlobalResourcePool() ) );

	if ( !errorMaterial )
		engine->Error( "[MaterialSystem]Failed to load error material!" );
}

IMaterial *MaterialSystem::LoadMaterial( const std::filesystem::path &relpath, const std::string &pathid, IResourcePool *resourcePoolPtr )
{
	using std::array;
	using std::string;
	using nlohmann::json;

	ResourcePool *resourcePool = ResourcePool::ToResourcePool( resourcePoolPtr );
	if ( !resourcePool )
	{
		Log::PrintlnWarn( "[MaterialSystem]Resource pool is NULL" );
		return errorMaterial;
	}

	std::vector< char > materialBuffer;
	if ( !fileSystem->ReadToBuffer( relpath, pathid, materialBuffer ) )
	{
		Log::PrintlnWarn( "Failed to read material {}", relpath.generic_string() );
		return errorMaterial;
	}

	MaterialBindings bindings = {};

	json j = json::parse( materialBuffer );

	string shaderName = j[ "shader" ].get< string >();

	for ( auto kv : j[ "texture" ].items() )
	{
		const string identifier = kv.key();
		const string path = kv.value();
		
		bindings.SetTexture( identifier, path );

		// TODO: Remove
		Log::Println( "{}: {}", identifier, path );
	}

	for ( auto kv : j[ "vec2i" ].items() )
	{
		const string identifier = kv.key();
		const array< glm::i32, 2 > value = kv.value();

		const glm::ivec2 vec2i( glm::make_vec2( value.data() ) );
		bindings.SetVec2i( identifier, vec2i );

		// TODO: Remove
		Log::Println( "{}: ({}, {})", identifier, vec2i.x, vec2i.y );
	}

	for ( auto kv : j[ "vec3i" ].items() )
	{
		const string identifier = kv.key();
		const array< glm::i32, 3 > value = kv.value();

		const glm::ivec3 vec3i( glm::make_vec3( value.data() ) );
		bindings.SetVec3i( identifier, vec3i );

		// TODO: Remove
		Log::Println( "{}: ({}, {}, {})", identifier, vec3i.x, vec3i.y, vec3i.z );
	}

	for ( auto kv : j[ "vec4i" ].items() )
	{
		const string identifier = kv.key();
		const array< glm::i32, 4 > value = kv.value();

		const glm::ivec4 vec4i( glm::make_vec4( value.data() ) );
		bindings.SetVec4i( identifier, vec4i );

		// TODO: Remove
		Log::Println( "{}: ({}, {}, {}, {})", identifier, vec4i.x, vec4i.y, vec4i.z, vec4i.w );
	}

	for ( auto kv : j[ "vec2f" ].items() )
	{
		const string identifier = kv.key();
		const array< glm::f32, 2 > value = kv.value();

		const glm::f32vec2 vec2f( glm::make_vec2( value.data() ) );
		bindings.SetVec2f( identifier, vec2f );

		// TODO: Remove
		Log::Println( "{}: ({}, {})", identifier, vec2f.x, vec2f.y );
	}

	for ( auto kv : j[ "vec3f" ].items() )
	{
		const string identifier = kv.key();
		const array< glm::f32, 3 > value = kv.value();

		const glm::f32vec3 vec3f( glm::make_vec3( value.data() ) );
		bindings.SetVec3f( identifier, vec3f );

		// TODO: Remove
		Log::Println( "{}: ({}, {}, {})", identifier, vec3f.x, vec3f.y, vec3f.z );
	}

	for ( auto kv : j[ "vec4f" ].items() )
	{
		const string identifier = kv.key();
		const array< glm::f32, 4 > value = kv.value();

		const glm::f32vec4 vec4f( glm::make_vec4( value.data() ) );
		bindings.SetVec4f( identifier, vec4f );

		// TODO: Remove
		Log::Println( "{}: ({}, {}, {}, {})", identifier, vec4f.x, vec4f.y, vec4f.z, vec4f.w );
	}

	for ( auto kv : j[ "mat2f" ].items() )
	{
		const string identifier = kv.key();
		const array< glm::f32, 2*2 > value = kv.value();

		const glm::mat2 mat2f( glm::make_mat2( value.data() ) );
		bindings.SetMat2f( identifier, mat2f );

		// TODO: Remove
		Log::Println( "{}:\n[{}, {}]\n[{}, {}]", identifier, mat2f[0][0], mat2f[0][1], mat2f[1][0], mat2f[1][1] );
	}

	for ( auto kv : j[ "mat3f" ].items() )
	{
		const string identifier = kv.key();
		const array< glm::f32, 3*3 > value = kv.value();

		// glm::mat3 is 16 byte aligned, but our array is 4 byte aligned
		// so we can't use glm::make_mat3 which does a normal memcpy
		auto my_make_mat3 = []( const std::array< glm::f32, 3*3 > &val )
		{
			glm::mat3 Result;

			Result[ 0 ].x = val[ 0 ];
			Result[ 0 ].y = val[ 1 ];
			Result[ 0 ].z = val[ 2 ];

			Result[ 1 ].x = val[ 3 ];
			Result[ 1 ].y = val[ 4 ];
			Result[ 1 ].z = val[ 5 ];

			Result[ 2 ].x = val[ 6 ];
			Result[ 2 ].y = val[ 7 ];
			Result[ 2 ].z = val[ 8 ];
			
			return Result;
		};

		const glm::mat3 mat3f( my_make_mat3( value ) );

		bindings.SetMat3f( identifier, mat3f );

		// TODO: Remove
		Log::Println( "{}:\n"
			"[{}, {}, {}]\n"
			"[{}, {}, {}]\n"
			"[{}, {}, {}]", identifier,
			mat3f[0][0], mat3f[0][1], mat3f[0][2],
			mat3f[1][0], mat3f[1][1], mat3f[1][2],
			mat3f[2][0], mat3f[2][1], mat3f[2][2] );
	}

	for ( auto kv : j[ "mat4f" ].items() )
	{
		const string identifier = kv.key();
		const array< glm::f32, 4*4 > value = kv.value();

		const glm::mat4 mat4f( glm::make_mat4( value.data() ) );
		bindings.SetMat4f( identifier, mat4f );

		// TODO: Remove
		Log::Println( "{}:\n"
			"[{}, {}, {}, {}]\n"
			"[{}, {}, {}, {}]\n"
			"[{}, {}, {}, {}]\n"
			"[{}, {}, {}, {}]", identifier,
			mat4f[0][0], mat4f[0][1], mat4f[0][2], mat4f[0][3],
			mat4f[1][0], mat4f[1][1], mat4f[1][2], mat4f[1][3],
			mat4f[2][0], mat4f[2][1], mat4f[2][2], mat4f[2][3],
			mat4f[3][0], mat4f[3][1], mat4f[3][2], mat4f[3][3] );
	}

	for ( auto kv : j[ "i32" ].items() )
	{
		const string identifier = kv.key();
		const glm::i32 i32 = kv.value();

		bindings.SetInt( identifier, i32 );

		Log::Println( "{}: {}", identifier, i32 );
	}

	for ( auto kv : j[ "f32" ].items() )
	{
		const string identifier = kv.key();
		const glm::f32 f32 = kv.value();

		bindings.SetFloat( identifier, f32 );

		Log::Println( "{}: {}", identifier, f32 );
	}

	Shader *shader = Shader::ToShader( shaderSystem->FindShader( shaderName ) );

	if ( !shader ) {
		Log::PrintlnWarn( "Failed to find shader {} for material!", shaderName );
		return errorMaterial;
	}

	auto resource = ResourcePool::createResource< Material >( ResourceInfo{ relpath.generic_string() }, shader, bindings );
	Material *material = resource->resource.get();

	for ( const auto &kv : bindings.textures )
	{
		Texture *texture = Texture::ToTexture( textureSystem->LoadTexture( kv.second, pathid, resourcePoolPtr ) );
		material->textures[ kv.first ] = texture;
	}

	materialsMutex.lock();
	resourcePool->materials.push_back( resource );
	materialsMutex.unlock();

	return material;
}

IMaterial *MaterialSystem::FindMaterial( const std::filesystem::path &relpath, IResourcePool *resourcePoolPtr ) const
{
	if ( ResourcePool *resourcePool = ResourcePool::ToResourcePool( resourcePoolPtr ); resourcePool )
		return resourcePool->findResource< Material >( relpath.generic_string() );
	
	return nullptr;
}

IMaterial *MaterialSystem::FindMaterial_Internal( const std::filesystem::path &relpath, IResourcePool *resourcePoolPtr )
{
	// We only need to lock guard our own lookups
	std::lock_guard< std::mutex > lock( materialsMutex );
	return FindMaterial( relpath, resourcePoolPtr );
}