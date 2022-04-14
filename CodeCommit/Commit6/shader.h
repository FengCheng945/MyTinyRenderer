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
private:
	Matrix4f M_model;
	Matrix4f M_view;
	Matrix4f M_projection;
public:
	GouraudShader();
	virtual ~GouraudShader() = default;
	virtual Vector4f vertex(int i, int j, Vex& vex);
	void set_view_matrix(Vector3f eye_pos);
	void set_model_matrix(char n, float rotation_angle);
	void set_random_model_matrix(Vector3f n, float rotation_angle);
	void set_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar);
	Matrix4f get_view_matrix();
	Matrix4f get_model_matrix();
	Matrix4f get_projection_matrix();
	Vector3f get_viewport(Vector4f& v, const int& width, const int& height);
};
Matrix3f v_dot_vT(const Vector3f& n);

#endif //__SHADER_H__