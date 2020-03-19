#ifndef WIRESHADER_HPP
#define WIRESHADER_HPP

#include "shader.hpp"

class WireShader : public Shader
{
public:
	using Shader::Shader;

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
		Count
	};
};

#endif // WIRESHADER_HPP