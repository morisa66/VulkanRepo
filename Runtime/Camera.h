#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "Core/GlobalConfig.h"
#include "Utils/MatrixUtils.h"

MORISA_NAMESPACE_BEGIN

enum CameraDirection
{
	kCameraDirectionForward,
	kCameraDirectionBackward,
	kCameraDirectionLeft,
	kCameraDirectionRight,
	kCameraDirectionUp,
	kCameraDirectionDown,
};

class Camera
{
public:
	Camera(uint32_t width, uint32_t height, float fov, float near, float far);
	void ResetExtent(uint32_t width, uint32_t height);
	const glm::mat4& View() { return _view; }
	const glm::mat4& Projection() { return _projection; }
	const glm::vec3& Position() { return _position; }
	void Update();
	void Move(CameraDirection direction, float delta);
	void MoveTo(glm::vec3& position);
	void Rotate(float deltaX, float deltaY);
private:
	void UpdateViewLH();
	void UpdateViewRH();
	void UpdateProjectionLH_NO();
	void UpdateProjectionLH_ZO();
	void UpdateProjectionRH_NO();
	void UpdateProjectionRH_ZO();
	void ProjectionLHToRH();
private:
	uint32_t _width;
	uint32_t _height;
	float _aspect;
	float _fov;
	float _near;
	float _far;
	float _speed;
	float _rotateSpeed;
	glm::vec3 _wordUp;
	glm::vec3 _position;
	glm::vec3 _forward;
	glm::vec3 _up;
	glm::vec3 _right;

	glm::mat4 _view;
	glm::mat4 _projection;
};

MORISA_NAMESPACE_END


#endif