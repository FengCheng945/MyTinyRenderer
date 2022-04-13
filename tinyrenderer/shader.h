#ifndef __SHADER_H__
#define __SHADER_H__
#include "rasterizer.h"

extern Model* model;
extern Vector3f eye_pos;
class Shader
{
public:
	Shader() = default;
	virtual ~Shader() = default;
	virtual Vector4f vertex(int i, int j, Vex& vex) = 0;
};

class GouraudShader :public Shader
{
public:
	GouraudShader() = default;
	virtual ~GouraudShader() = default;
	virtual Vector4f vertex(int i, int j, Vex& vex);
	Matrix4f get_view_matrix(Vector3f eye_pos);
	Matrix4f get_model_matrix(char n, float rotation_angle);
	Matrix4f get_random_model_matrix(Vector3f n, float rotation_angle);
	Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar);
	Vector3f get_viewport(Vector4f& v, const int& width, const int& height);
};


#endif //__SHADER_H__