#include "rasterizer.h"
constexpr float INF = 1e-4;

void Rasterizer::line(Vector2i t0, Vector2i t1, TGAImage& image, TGAColor color)
{
	bool steep = false;
	if (std::abs(t0.x - t1.x) < std::abs(t0.y - t1.y))//表示当斜率>1时，就对调xy转换为<1
	{
		std::swap(t0.x, t0.y);
		std::swap(t1.x, t1.y);
		steep = true;
	}
	if (t0.x > t1.x) //始终按照由近到远的渲染顺序
	{
		std::swap(t0.x, t1.x);
		std::swap(t0.y, t1.y);
	}
	int dx = t1.x - t0.x;
	int dy = t1.y - t0.y;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = t0.y;
	for (int x = t0.x; x <= t1.x; x++)
	{
		if (steep)
		{
			image.set(y, x, color);
		}
		else
		{
			image.set(x, y, color);
		}
		error2 += derror2;
		if (error2 > dx)
		{
			y += (t1.y > t0.y ? 1 : -1);
			error2 -= dx * 2;
		}
	}
}
void Rasterizer::LineTriangle(Vector2i t0, Vector2i t1, Vector2i t2, TGAImage& image, TGAColor color)
{
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);
	line(t0, t1, image, color);
	line(t1, t2, image, color);
	line(t2, t0, image, color);
}

static bool insideTriangle_byCross(float x, float y, const Vector3f* _v)
{
	Vector3f v[3];
	for (int i = 0; i < 3; i++)
		v[i] = { _v[i].x,_v[i].y, 1.0 };
	Vector3f f0, f1, f2;
	f0 = v[1].cross(v[0]);
	f1 = v[2].cross(v[1]);
	f2 = v[0].cross(v[2]);
	Vector3f p(x, y, 1.);
	if (((p * f0) * (f0 * v[2]) > 0) && ((p * f1) * (f1 * v[0]) > 0) && ((p * f2) * (f2 * v[1]) > 0))
		return true;
	return false;
}

static bool insideTriangle(float x, float y, const Vector3f* v)
{

	float c2 = (x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * y + v[2].x * v[0].y - v[0].x * v[2].y) / (v[1].x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * v[1].y + v[2].x * v[0].y - v[0].x * v[2].y);
	float c3 = (x * (v[0].y - v[1].y) + (v[1].x - v[0].x) * y + v[0].x * v[1].y - v[1].x * v[0].y) / (v[2].x * (v[0].y - v[1].y) + (v[1].x - v[0].x) * v[2].y + v[0].x * v[1].y - v[1].x * v[0].y);
	float c1 = 1.f - c2 - c3;

	if (c1 - 1.f < INF && c1 > -INF && c2 - 1.f < INF && c2 > -INF && c3 - 1.f < INF && c3 > -INF) return true; //INF防止浮点数陷阱
	else return false;

}

static Vector3f computeBarycentric2D(float x, float y, const Vector3f* v)
{

	float c2 = (x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * y + v[2].x * v[0].y - v[0].x * v[2].y) / (v[1].x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * v[1].y + v[2].x * v[0].y - v[0].x * v[2].y);
	float c3 = (x * (v[0].y - v[1].y) + (v[1].x - v[0].x) * y + v[0].x * v[1].y - v[1].x * v[0].y) / (v[2].x * (v[0].y - v[1].y) + (v[1].x - v[0].x) * v[2].y + v[0].x * v[1].y - v[1].x * v[0].y);
	float c1 = 1.f - c2 - c3;

	return Vector3f{ c1,c2,c3 };
}

static Vector3f interpolate(float alpha, float beta, float gamma, const Vector3f& vert1, const Vector3f& vert2, const Vector3f& vert3, float weight)
{
	return (alpha * vert1 + beta * vert2 + gamma * vert3) / weight;
}

static Vector2f interpolate(float alpha, float beta, float gamma, const Vector2f& vert1, const Vector2f& vert2, const Vector2f& vert3, float weight)
{
	auto u = (alpha * vert1[0] + beta * vert2[0] + gamma * vert3[0]);
	auto v = (alpha * vert1[1] + beta * vert2[1] + gamma * vert3[1]);

	u /= weight;
	v /= weight;

	return Vector2f(u, v);
}

static float interpolate(float alpha, float beta, float gamma, const float& vert1, const float& vert2, const float& vert3, float weight)
{
	return (alpha * vert1 + beta * vert2 + gamma * vert3) / weight;
}

void Rasterizer::flat_triangle(Vector3f* vertex, Vector2f* tex, TGAImage& image, Model* model, float& intensity)
{
	int width = image.get_width();
	float* zbuffer = image.get_zbuffer();
	Vector2f bboxleft(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vector2f bboxright(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vector2f clamp(image.get_width() - 1, image.get_height() - 1);
	for (int i = 0; i < 3; i++) //选取左下角与右上角的点构建包围盒
	{
		bboxleft.x = std::max(0.f, std::min(bboxleft.x, vertex[i].x));
		bboxleft.y = std::max(0.f, std::min(bboxleft.y, vertex[i].y));

		bboxright.x = std::min(clamp.x, std::max(bboxright.x, vertex[i].x));
		bboxright.y = std::min(clamp.y, std::max(bboxright.y, vertex[i].y));
	}
	Vector3i P;
	
	for (P.x = static_cast<int>(bboxleft.x); P.x <= static_cast<int>(bboxright.x); P.x++)
	{
		for (P.y = static_cast<int>(bboxleft.y); P.y <= static_cast<int>(bboxright.y); P.y++)
		{
			if (insideTriangle_byCross(static_cast<float>(P.x) + 0.5, static_cast<float>(P.y) + 0.5, vertex))
			{
				
				Vector3f Barycentric = computeBarycentric2D(static_cast<float>(P.x) + 0.5, static_cast<float>(P.y) + 0.5, vertex);
				float z = interpolate(Barycentric.x, Barycentric.y, Barycentric.z, vertex[0], vertex[1], vertex[2], 1).z;//用三个顶点的z轴坐标插值出P点z值
				Vector2f tex_coords = interpolate(Barycentric.x, Barycentric.y, Barycentric.z, tex[0], tex[1], tex[2], 1);
				int idx = P.x + P.y * width;
				if (zbuffer[idx] > z)
				{
					TGAColor tex_color = model->diffuse(tex_coords);
					zbuffer[idx] = z;
					image.set(P.x, P.y, tex_color * pow(intensity,2.2));
				}
			}
		}
	}
}

void Rasterizer::triangle(Vex& vex, TGAImage& image, Model* model)
{
	Vector3f* vertex = vex.screen_coords;
	Vector2f* tex = vex.texture_coords;
	
	float* zbuffer = image.get_zbuffer();
	Vector2f bboxleft(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vector2f bboxright(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vector2f clamp(image.get_width() - 1, image.get_height() - 1);
	for (int i = 0; i < 3; i++) //选取左下角与右上角的点构建包围盒
	{
		bboxleft.x = std::max(0.f, std::min(bboxleft.x, vertex[i].x));
		bboxleft.y = std::max(0.f, std::min(bboxleft.y, vertex[i].y));

		bboxright.x = std::min(clamp.x, std::max(bboxright.x, vertex[i].x));
		bboxright.y = std::min(clamp.y, std::max(bboxright.y, vertex[i].y));
	}
	Vector3i P;

	for (P.x = static_cast<int>(bboxleft.x); P.x <= static_cast<int>(bboxright.x); P.x++)
	{
		for (P.y = static_cast<int>(bboxleft.y); P.y <= static_cast<int>(bboxright.y); P.y++)
		{
			if (insideTriangle_byCross(static_cast<float>(P.x) + 0.5, static_cast<float>(P.y) + 0.5, vertex))
			{
				Vector3f Barycentric = computeBarycentric2D(static_cast<float>(P.x) + 0.5, static_cast<float>(P.y) + 0.5, vertex);
				float z = interpolate(Barycentric.x, Barycentric.y, Barycentric.z, vertex[0], vertex[1], vertex[2], 1).z;//用三个顶点的z轴坐标插值出P点z值
				Vector2f tex_coords = interpolate(Barycentric.x, Barycentric.y, Barycentric.z, tex[0], tex[1], tex[2], 1);

				float intensity = interpolate(Barycentric.x, Barycentric.y, Barycentric.z, vex.intensity[0], vex.intensity[1], vex.intensity[2], 1); //gouraudshading 特别处：用顶点的法向量计算顶点光照再插值给片段

				int idx = P.x + P.y * image.get_width();
				if (zbuffer[idx] > z)
				{
					TGAColor tex_color = model->diffuse(tex_coords);
					zbuffer[idx] = z;
					image.set(P.x, P.y, tex_color * pow(intensity, 1/2.2));
				}
			}
		}
	}
}