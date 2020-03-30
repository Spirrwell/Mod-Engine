#include "material.hpp"

#include <fstream>
#include <array>

#include "engine.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
#include "stb_image.h"
#include "nlohmann/json.hpp"
#include "rendersystem.hpp"
#include "shadersystem.hpp"
#include "texturesystem.hpp"

Material::Material( Shader *shader, const MaterialBindings &bindings ) :
	shader( shader ),
	bindings( bindings )
{
}

Material::~Material()
{
}