#ifndef __SHADER_H__
#define __SHADER_H__
#include "geometry.h"
#include "Vex.h"
#include "model.h"

extern Model* model;
extern Vector3f eye_pos;
class Shader
{
public:
	Shader() = default;
	virtual ~Shader() = default;
	virtual Vector4f vertex(int i, int j, Vex& vex) = 0;
	virtual TGAColor framebuffer(Vector3f& Barycentric, Vex& vex) = 0;

};

class PhongShader :public Shader
{
protected:
	Matrix4f M_model;
	Matrix4f M_view;
	Matrix4f M_projection;
	Matrix4f M_viewport;
	Matrix4f uniform_M;
	Matrix4f uniform_MIT;
	Matrix4f MS_view;
public:
	PhongShader();
	virtual ~PhongShader() = default;
	virtual Vector4f vertex(int i, int j, Vex& vex);
	virtual TGAColor framebuffer(Vector3f& Barycentric, Vex& vex);
	virtual void set_view_matrix(Vector3f eye_pos);
	virtual void set_viewport_matrix(float w, float h);
	virtual void set_model_matrix(char n, float rotation_angle);
	virtual void set_random_model_matrix(Vector3f n, float rotation_angle);
	virtual void set_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar);
	virtual void set_shadowMatrix(Matrix4f MS);
	virtual Matrix4f get_view_matrix();
	virtual Matrix4f get_model_matrix();
	virtual Matrix4f get_projection_matrix();
	virtual Matrix4f get_viewport_matrix();
	virtual Vector3f get_viewport(Vector4f& v, const int& width, const int& height);
	float* shadowbuffer = nullptr;
};

class DepthShader :public PhongShader
{

public:
	virtual ~DepthShader() = default;
	virtual Vector4f vertex(int i, int j, Vex& vex);
	virtual TGAColor framebuffer(Vector3f& Barycentric, Vex& vex);
	Matrix4f get_MS();
};

Matrix3f v_dot_vT(const Vector3f& n);

#endif //__SHADER_H__