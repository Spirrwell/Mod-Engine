#ifndef SHADER_WIREFRAME_HPP
#define SHADER_WIREFRAME_HPP

#include "shader.hpp"

class Shader_Wireframe : public Shader
{
public:
	using Shader::Shader;

	inline VertexLayout GetVertexLayout() const override {
		return VertexLayout (
			{
				Vertex::Component::Position,
				Vertex::Component::Color
			}
		);
	}

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
		Count
	};
};

#endif // SHADER_WIREFRAME_HPP