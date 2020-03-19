#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include <vector>
#include <string>

#include "memory.hpp"

struct ResourceInfo
{
	std::string identifier;
};

template< typename T >
struct Resource
{
	shared_ptr< T > resource;
	ResourceInfo resourceInfo;
};

template< typename T >
using ResourceList = std::vector< shared_ptr< Resource< T > > >;

#endif // RESOURCE_HPP