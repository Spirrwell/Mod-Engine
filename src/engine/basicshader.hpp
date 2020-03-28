#ifndef BASICSHADER_HPP
#define BASICSHADER_HPP

#include "shader.hpp"

class BasicShader : public Shader
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

	void InitVertexInputBindingDescriptions() override;
	void InitVertexInputAttributeDescriptions() override;

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

#endif // BASICSHADER_HPP