#ifndef SHADER_STATICMESH_HPP
#define SHADER_STATICMESH_HPP

#include "shader.hpp"

class Shader_StaticMesh : public Shader
{
public:
	using Shader::Shader;

	inline VertexLayout GetVertexLayout() const override {
		return VertexLayout (
			{
				Vertex::Component::Position,
				Vertex::Component::Color,
				Vertex::Component::UV
			}
		);
	}

	VkDescriptorPool CreateDescriptorPool() const override;
	void InitMaterial( Material &material ) override;
	void InitMesh( Mesh *mesh ) override;

	void Update( const uint32_t imageIndex, const MVP &mvp, Mesh *mesh ) override;

	void CreateDescriptorSetLayout() override;
	void CreateGraphicsPipelineLayout() override;
	void CreateGraphicsPipeline() override;

private:
	enum class Uniforms : size_t
	{
		MVP,
		LightState,
		Count
	};
};

#endif // SHADER_STATICMESH_HPP