#ifndef __RASTERIZER_H__
#define __RASTERIZER_H__

#include "geometry.h"
#include "tgaimage.h"
#include "model.h"
class Rasterizer
{
public:
	
	void line(Vector2i t0, Vector2i t1, TGAImage& image, TGAColor color);
	void LineTriangle(Vector2i t0, Vector2i t1, Vector2i t2, TGAImage& image, TGAColor color);
	void triangle(Vector3f* vertex, TGAImage& image, TGAColor color);
	void triangle(Vector3f* vertex, Vector2i* tex, TGAImage& image, Model* model, float& intensity);
};


#endif //__RASTERIZER_H__