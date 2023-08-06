#include "Camera.h"

#define ITEM(M, I, J) M[J][I]

MORISA_NAMESPACE_BEGIN

Camera::Camera(uint32_t width, uint32_t height, float fov, float near, float far) :
	_width(width)
	, _height(height)
	, _aspect((float)width / (float)height)
	, _fov(fov)
	, _near(near)
	, _far(far)
	, _speed(2.0f)
	, _rotateSpeed(0.01f)
	, _position(0.0f, 0.0f, 2.0f)
	, _forward(0.0f, 0.0f, -1.0f)
	, _wordUp(0.0f, 1.0f, 0.0f)
	, _view(0.0f)
	, _projection(0.0f)
{
	UpdateViewRH();
	UpdateProjectionRH_ZO();
}

void Camera::ResetExtent(uint32_t width, uint32_t height)
{
	_width = width;
	_height = height;
	_aspect = ((float)width / (float)height);

	UpdateViewRH();
	UpdateProjectionRH_ZO();
}

void Camera::Update()
{
	UpdateViewRH();
}

void Camera::Move(CameraDirection direction, float delta)
{
	switch (direction)
	{
		case morisa::kCameraDirectionForward:
		{
			_position += _forward * _speed * delta;
			break;
		}
		case morisa::kCameraDirectionBackward:
		{
			_position -= _forward * _speed * delta;
			break;
		}
		case morisa::kCameraDirectionLeft:
		{
			_position -= _right * _speed * delta;
			break;
		}
		case morisa::kCameraDirectionRight:
		{
			_position += _right * _speed * delta;
			break;
		}
		case morisa::kCameraDirectionUp:
		{
			_position += _wordUp * _speed * delta;
			break;
		}
		case morisa::kCameraDirectionDown:
		{
			_position -= _wordUp * _speed * delta;
			break;
		}
		default:
			break;
	}
}

void Camera::MoveTo(glm::vec3& position)
{
	_position = position;
}

void Camera::Rotate(float deltaX, float deltaY)
{
	if (abs(deltaX) > abs(deltaY))
	{
		const float delta = deltaX * _rotateSpeed;
		const float sinX = sin(glm::radians(delta));
		const float cosX = cos(glm::radians(delta));
		const float x = _forward.x;
		const float z = _forward.z;
		_forward.x = x * cosX - z * sinX;
		_forward.z = z * cosX + x * sinX;
	}
	else
	{
		const float r0 = sqrtf(_forward.x * _forward.x + _forward.z * _forward.z);
		const float delta = deltaY * _rotateSpeed;
		const float sinY = sin(glm::radians(delta));
		const float cosY = cos(glm::radians(delta));
		const float y = cosY * _forward.y + sinY * r0;
		const float ratio = sqrtf(1.0f - y * y) / r0;
		if (r0 < 0.7f && ratio < 1.0f)
		{
			return;
		}
		_forward.x *= ratio;
		_forward.y = y;
		_forward.z *= ratio;
	}
}

//https://github.com/morisa66/GraphicsMath/blob/main/ModelTransform.pdf
void Camera::UpdateViewLH()
{
	_right = glm::normalize(glm::cross(_wordUp, _forward));
	_up = glm::normalize(glm::cross(_forward, _right));
	ITEM(_view, 0, 0) = _right.x;
	ITEM(_view, 0, 1) = _right.y;
	ITEM(_view, 0, 2) = _right.z;
	ITEM(_view, 0, 3) = -dot(_position, _right);

	ITEM(_view, 1, 0) = _up.x;
	ITEM(_view, 1, 1) = _up.y;
	ITEM(_view, 1, 2) = _up.z;
	ITEM(_view, 1, 3) = -dot(_position, _up);

	ITEM(_view, 2, 0) = _forward.x;
	ITEM(_view, 2, 1) = _forward.y;
	ITEM(_view, 2, 2) = _forward.z;
	ITEM(_view, 2, 3) = -dot(_position, _forward);

	ITEM(_view, 3, 0) = 0;
	ITEM(_view, 3, 1) = 0;
	ITEM(_view, 3, 2) = 0;
	ITEM(_view, 3, 3) = 1;
}

void Camera::UpdateViewRH()
{
	_right = glm::normalize(glm::cross(_forward, _wordUp));
	_up = glm::normalize(glm::cross(_right, _forward));
	ITEM(_view, 0, 0) = _right.x;
	ITEM(_view, 0, 1) = _right.y;
	ITEM(_view, 0, 2) = _right.z;
	ITEM(_view, 0, 3) = -dot(_position, _right);

	ITEM(_view, 1, 0) = _up.x;
	ITEM(_view, 1, 1) = _up.y;
	ITEM(_view, 1, 2) = _up.z;
	ITEM(_view, 1, 3) = -dot(_position, _up);

	ITEM(_view, 2, 0) = -_forward.x;
	ITEM(_view, 2, 1) = -_forward.y;
	ITEM(_view, 2, 2) = -_forward.z;
	ITEM(_view, 2, 3) = dot(_position, _forward);

	ITEM(_view, 3, 0) = 0;
	ITEM(_view, 3, 1) = 0;
	ITEM(_view, 3, 2) = 0;
	ITEM(_view, 3, 3) = 1;
}

void Camera::UpdateProjectionLH_NO()
{
	const float inverseHalfTanFlov = 1.0f / glm::tan(glm::radians(_fov * 0.5f));
	const float subFarNear = _far - _near;
	ITEM(_projection, 0, 0) = inverseHalfTanFlov / _aspect;
	ITEM(_projection, 1, 1) = inverseHalfTanFlov;
	ITEM(_projection, 2, 2) = (_far + _near) / subFarNear;
	ITEM(_projection, 2, 3) = -(2.0f * _far * _near) / subFarNear;
	ITEM(_projection, 3, 2) = 1.0f;
}

void Camera::UpdateProjectionLH_ZO()
{
	const float inverseHalfTanFlov = 1.0f / glm::tan(glm::radians(_fov * 0.5f));
	const float subFarNear = _far - _near;
	ITEM(_projection, 0, 0) = inverseHalfTanFlov / _aspect;
	ITEM(_projection, 1, 1) = inverseHalfTanFlov;
	ITEM(_projection, 2, 2) = _far / subFarNear;
	ITEM(_projection, 2, 3) = -(_far * _near) / subFarNear;
	ITEM(_projection, 3, 2) = 1.0f;
}

void Camera::UpdateProjectionRH_NO()
{
	UpdateProjectionLH_NO();
	ProjectionLHToRH();
}

void Camera::UpdateProjectionRH_ZO()
{
	UpdateProjectionLH_ZO();
	ProjectionLHToRH();
}

void Camera::ProjectionLHToRH()
{
	static glm::mat4 tansition(1.0f);
	ITEM(tansition, 2, 2) = -1.0f;
	_projection *= tansition;
}
MORISA_NAMESPACE_END

