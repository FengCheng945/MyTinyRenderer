#ifndef __RASTERIZER_H__
#define __RASTERIZER_H__
#include "shader.h"
//光栅化类，用于管理光栅化方法
class Rasterizer
{
public:

	Rasterizer() = default;
	~Rasterizer() = default;

	void line(Vector2i t0, Vector2i t1, TGAImage& image, TGAColor color);
	void LineTriangle(Vector2i t0, Vector2i t1, Vector2i t2, TGAImage& image, TGAColor color);
	void flat_triangle(Vector3f* vertex, Vector2f* tex, TGAImage& image, Model* model, float& intensity);
	void triangle(Vex& vex, TGAImage& image, Model* model, Shader& shader);
};


#endif //__RASTERIZER_H__