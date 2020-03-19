#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "engine/imaterial.hpp"
#include "vulkansystem.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "memory.hpp"
#include "glm/glm.hpp"
#include "glm/matrix.hpp"
#include "glm/mat4x4.hpp"

#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <string>
#include <vk_mem_alloc.h>

class ShaderSystem;
class TextureSystem;

struct MaterialBindings
{
	friend class Material;
	friend class MaterialSystem;

	void SetTexture( const std::string &identifier, const std::filesystem::path &path ) { textures[ identifier ] = path; }
	void SetVec2i( const std::string &identifier, const glm::ivec2 &vec2i ) { vec2is[ identifier ] = vec2i; }
	void SetVec3i( const std::string &identifier, const glm::ivec3 &vec3i ) { vec3is[ identifier ] = vec3i; }
	void SetVec4i( const std::string &identifier, const glm::ivec4 &vec4i ) { vec4is[ identifier ] = vec4i; }
	void SetVec2f( const std::string &identifier, const glm::f32vec2 &vec2f ) { vec2fs[ identifier ] = vec2f; }
	void SetVec3f( const std::string &identifier, const glm::f32vec3 &vec3f ) { vec3fs[ identifier ] = vec3f; }
	void SetVec4f( const std::string &identifier, const glm::f32vec4 &vec4f ) { vec4fs[ identifier ] = vec4f; }
	void SetMat2f( const std::string &identifier, const glm::mat2 &mat2f ) { mat2fs[ identifier ] = mat2f; }
	void SetMat3f( const std::string &identifier, const glm::mat3 &mat3f ) { mat3fs[ identifier ] = mat3f; }
	void SetMat4f( const std::string &identifier, const glm::mat4 &mat4f ) { mat4fs[ identifier ] = mat4f; }
	void SetInt( const std::string &identifier, const glm::i32 &i32 ) { i32s[ identifier ] = i32; }
	void SetFloat( const std::string &identifier, const glm::f32 &f32 ) { f32s[ identifier ] = f32; }

	std::filesystem::path GetTexture( const std::string &identifier ) { return textures[ identifier ]; }
	glm::ivec2 GetVec2i( const std::string &identifier ) { return vec2is[ identifier ]; }
	glm::ivec3 GetVec3i( const std::string &identifier ) { return vec3is[ identifier ]; }
	glm::ivec4 GetVec4i( const std::string &identifier ) { return vec4is[ identifier ]; }
	glm::f32vec2 GetVec2f( const std::string &identifier ) { return vec2fs[ identifier ]; }
	glm::f32vec3 GetVec3f( const std::string &identifier ) { return vec3fs[ identifier ]; }
	glm::f32vec4 GetVec4f( const std::string &identifier ) { return vec4fs[ identifier ]; }
	glm::mat2 GetMat2f( const std::string &identifier ) { return mat2fs[ identifier ]; }
	glm::mat3 GetMat3f( const std::string &identifier ) { return mat3fs[ identifier ]; }
	glm::mat4 GetMat4f( const std::string &identifier ) { return mat4fs[ identifier ]; }
	glm::i32 GetInt( const std::string &identifier ) { return i32s[ identifier ]; }
	glm::f32 GetFloat( const std::string &identifier ) { return f32s[ identifier ]; }

protected:
	std::unordered_map< std::string, std::filesystem::path > textures;
	std::unordered_map< std::string, glm::ivec2 > vec2is;
	std::unordered_map< std::string, glm::ivec3 > vec3is;
	std::unordered_map< std::string, glm::ivec4 > vec4is;
	std::unordered_map< std::string, glm::f32vec2 > vec2fs;
	std::unordered_map< std::string, glm::f32vec3 > vec3fs;
	std::unordered_map< std::string, glm::f32vec4 > vec4fs;
	std::unordered_map< std::string, glm::mat2 > mat2fs;
	std::unordered_map< std::string, glm::mat3 > mat3fs;
	std::unordered_map< std::string, glm::mat4 > mat4fs;
	std::unordered_map< std::string, glm::i32 > i32s;
	std::unordered_map< std::string, glm::f32 > f32s;
};

class Material : public IMaterial
{
	friend class Renderer;

public:

	Material( Shader *shader, const MaterialBindings &bindings );
	~Material();

	static inline Material *ToMaterial( IMaterial *material ) { return static_cast< Material* >( material ); }

	IShader *GetIShader() const override { return shader; }

	Shader *GetShader() const { return shader; }

public:

	//void SetTexture( const std::string &identifier, const std::filesystem::path &path ) { bindings.SetTexture( identifier, path ); }
	void SetVec2i( const std::string &identifier, const glm::ivec2 &vec2i ) { bindings.SetVec2i( identifier, vec2i ); }
	void SetVec3i( const std::string &identifier, const glm::ivec3 &vec3i ) { bindings.SetVec3i( identifier, vec3i ); }
	void SetVec4i( const std::string &identifier, const glm::ivec4 &vec4i ) { bindings.SetVec4i( identifier, vec4i ); }
	void SetVec2f( const std::string &identifier, const glm::f32vec2 &vec2f ) { bindings.SetVec2f( identifier, vec2f ); }
	void SetVec3f( const std::string &identifier, const glm::f32vec3 &vec3f ) { bindings.SetVec3f( identifier, vec3f ); }
	void SetVec4f( const std::string &identifier, const glm::f32vec4 &vec4f ) { bindings.SetVec4f( identifier, vec4f ); }
	void SetMat2f( const std::string &identifier, const glm::mat2 &mat2f ) { bindings.SetMat2f( identifier, mat2f ); }
	void SetMat3f( const std::string &identifier, const glm::mat3 &mat3f ) { bindings.SetMat3f( identifier, mat3f ); }
	void SetMat4f( const std::string &identifier, const glm::mat4 &mat4f ) { bindings.SetMat4f( identifier, mat4f ); }
	void SetInt( const std::string &identifier, const glm::i32 &i32 ) { bindings.SetInt( identifier, i32 ); }
	void SetFloat( const std::string &identifier, const glm::f32 &f32 ) { bindings.SetFloat( identifier, f32 ); }

	//std::filesystem::path GetTexture( const std::string &identifier ) { return bindings.GetTexture( identifier ); }
	Texture *GetTexture( const std::string &identifier ) { return textures[ identifier ]; }
	glm::ivec2 GetVec2i( const std::string &identifier ) { return bindings.GetVec2i( identifier ); }
	glm::ivec3 GetVec3i( const std::string &identifier ) { return bindings.GetVec3i( identifier ); }
	glm::ivec4 GetVec4i( const std::string &identifier ) { return bindings.GetVec4i( identifier ); }
	glm::f32vec2 GetVec2f( const std::string &identifier ) { return bindings.GetVec2f( identifier ); }
	glm::f32vec3 GetVec3f( const std::string &identifier ) { return bindings.GetVec3f( identifier ); }
	glm::f32vec4 GetVec4f( const std::string &identifier ) { return bindings.GetVec4f( identifier ); }
	glm::mat2 GetMat2f( const std::string &identifier ) { return bindings.GetMat2f( identifier ); }
	glm::mat3 GetMat3f( const std::string &identifier ) { return bindings.GetMat3f( identifier ); }
	glm::mat4 GetMat4f( const std::string &identifier ) { return bindings.GetMat4f( identifier ); }
	glm::i32 GetInt( const std::string &identifier ) { return bindings.GetInt( identifier ); }
	glm::f32 GetFloat( const std::string &identifier ) { return bindings.GetFloat( identifier ); }

	std::unordered_map< std::string, Texture* > textures;

private:
	MaterialBindings bindings;
	Shader *shader = nullptr;
};

#endif // MATERIAL_HPP