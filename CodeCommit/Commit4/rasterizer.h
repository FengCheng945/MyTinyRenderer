#ifndef __RASTERIZER_H__
#define __RASTERIZER_H__

#include "model.h"
class Rasterizer
{
public:
	void line(Vector2i t0, Vector2i t1, TGAImage& image, TGAColor color);
	void LineTriangle(Vector2i t0, Vector2i t1, Vector2i t2, TGAImage& image, TGAColor color);
	void triangle(Vector3f* vertex, TGAImage& image, TGAColor color);
	void triangle(Vector3f* vertex, Vector2i* tex, TGAImage& image, Model* model, float& intensity);
	Matrix4f get_view_matrix(Vector3f eye_pos);
	Matrix4f get_model_matrix(char n , float rotation_angle);
	Matrix4f get_random_model_matrix(Vector3f n, float rotation_angle);
	Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar);
	Vector3f get_viewport(Vector4f& v, const int& width, const int& height);
};


#endif //__RASTERIZER_H__