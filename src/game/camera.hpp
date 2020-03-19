#ifndef CAMERA_HPP
#define CAMERA_HPP

struct Camera
{
	glm::vec3 forward;
	glm::vec3 back;
	glm::vec3 up;
	glm::vec3 down;
	glm::vec3 right;
	glm::vec3 left;
};

#endif // CAMERA_HPP