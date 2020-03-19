#ifndef IMATERIAL_HPP
#define IMATERIAL_HPP

#include "engine/ishader.hpp"

class IMaterial
{
public:
	virtual ~IMaterial() = default;

	virtual IShader *GetIShader() const = 0;
};

#endif // IMATERIAL_HPP