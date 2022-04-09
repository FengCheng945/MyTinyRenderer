#include "rasterizer.h"

constexpr float INF = 1e-4;
constexpr double MY_PI = 3.1415926;

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

static Vector2i interpolate(float alpha, float beta, float gamma, const Vector2i& vert1, const Vector2i& vert2, const Vector2i& vert3, float weight)
{
	auto u = (alpha * vert1[0] + beta * vert2[0] + gamma * vert3[0]);
	auto v = (alpha * vert1[1] + beta * vert2[1] + gamma * vert3[1]);

	u /= weight;
	v /= weight;

	return Vector2i(u, v);
}

Matrix4f Rasterizer::get_view_matrix(Vector3f eye_pos)
{
	//camera transformations: M_view = R_view * T_view
	Vector3f up(0, 1, 0);
	Matrix4f view; view.identity();
	Vector3f z = eye_pos; z.normalize();
	Vector3f x = up.cross(z).normalize();
	Vector3f y = z.cross(x).normalize();
	view << std::vector<float>{
			x[0], x[1], x[2], 0,
			y[0], y[1], y[2], 0,
			z[0], z[1], z[2], 0,
			0, 0, 0, 1
	};
	
	Matrix4f translate;
	translate <<
		std::vector<float>{
			1, 0, 0, -eye_pos[0],
			0, 1, 0, -eye_pos[1],
			0, 0, 1, -eye_pos[2],
			0, 0, 0, 1
	};
	view *= translate;
	return view;
}

Matrix4f Rasterizer::get_model_matrix(char n, float rotation_angle)
{
	Matrix4f model; model.identity();
	float angle = rotation_angle * MY_PI / 180.0f;
	if (n == 'X')
	{
		model << std::vector<float>{
				1, 0, 0, 0,
				0, std::cos(angle), -std::sin(angle), 0,
				0, std::sin(angle), std::cos(angle), 0,
				0, 0, 0, 1
		};
	}
	else if (n == 'Y')
	{
		model << std::vector<float>{
				std::cos(angle), 0, std::sin(angle), 0,
				0, 1, 0, 0,
				-std::sin(angle), 0, std::cos(angle), 0,
				0, 0, 0, 1
		};
	}
	else if (n == 'Z')
	{
		model << std::vector<float>{
				std::cos(angle), -std::sin(angle), 0, 0,
				std::sin(angle), std::cos(angle), 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1
		};
	}
	return model;
}

Matrix3f v_dot_vT(const Vector3f& n)
{
	Matrix3f n_T;
	n_T << std::vector<float>{
			n.x, n.y, n.z,
			0, 0, 0,
			0, 0, 0
	};
	Matrix3f tmp;
	tmp.m[0][0] = n.x;
	tmp.m[1][0] = n.y;
	tmp.m[2][0] = n.z;
	tmp *= n_T;
	return tmp;
};

Matrix4f Rasterizer::get_random_model_matrix(Vector3f n, float rotation_angle)
{
	Matrix4f model; model.identity();
	float angle = rotation_angle * MY_PI / 180.0f;
	float nx = n[0], ny = n[1], nz = n[2];
	Matrix3f N;
	N << std::vector<float>{
			0, -nz, ny,
			nz, 0, -nx,
			-ny, nx, 0
	};
	Matrix3f I; I.identity();

	Matrix3f R = I * std::cos(angle) +  v_dot_vT(n) * (1 - std::cos(angle))  + N * std::sin(angle);
	model << std::vector<float>{
			R(0, 0), R(0, 1), R(0, 2), 0,
			R(1, 0), R(1, 1), R(1, 2), 0,
			R(2, 0), R(2, 1), R(2, 2), 0,
			0, 0, 0, 1
	};
	return model;
}

Matrix4f Rasterizer::get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar)
{
	Matrix4f projection; projection.identity();
	Matrix4f M_trans;
	Matrix4f M_persp;
	Matrix4f M_ortho;
	M_persp << std::vector<float>{
		zNear, 0, 0, 0,
			0, zNear, 0, 0,
			0, 0, zNear + zFar, -zFar * zNear,
			0, 0, 1, 0
	};

	float alpha = 0.5 * eye_fov * MY_PI / 180.0f;
	float yTop = -zNear * std::tan(alpha); //
	float yBottom = -yTop;
	float xRight = yTop * aspect_ratio;
	float xLeft = -xRight;

	M_trans << std::vector < float>{
		1, 0, 0, -(xLeft + xRight) / 2,
			0, 1, 0, -(yTop + yBottom) / 2,
			0, 0, 1, -(zNear + zFar) / 2,
			0, 0, 0, 1
	};

	M_ortho << std::vector < float>{
		2 / (xRight - xLeft), 0, 0, 0,
			0, 2 / (yTop - yBottom), 0, 0,
			0, 0, 2 / (zNear - zFar), 0,
			0, 0, 0, 1
	};

	M_ortho = M_ortho * M_trans;
	projection = M_ortho * M_persp * projection;
	return projection;
}

Vector3f Rasterizer::get_viewport(Vector4f& v, const int& width, const int& height)
{
	return Vector3f((v.x + 1.) * width * 0.5, (v.y + 1.) * height * 0.5, v.z);
}

void Rasterizer::triangle(Vector3f* vertex, TGAImage& image, TGAColor color)
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
	for (P.x = bboxleft.x; P.x <= bboxright.x; P.x++)
	{
		for (P.y = bboxleft.y; P.y <= bboxright.y; P.y++)
		{
			if (insideTriangle((float)P.x + 0.5, (float)P.y + 0.5, vertex))
			{
				Vector3f Barycentric = computeBarycentric2D((float)P.x + 0.5, (float)P.y + 0.5, vertex);
				P.z = interpolate(Barycentric.x, Barycentric.y, Barycentric.z, vertex[0], vertex[1], vertex[2], 1).z;//用三个顶点的z轴坐标插值出P点z值
				
				if (zbuffer[(P.x + P.y * width)] < P.z)
				{
					zbuffer[(P.x + P.y * width)] = P.z;
					image.set(P.x, P.y, color);
				}
			}
		}
	}
}

void Rasterizer::triangle(Vector3f* vertex, Vector2i* tex, TGAImage& image, Model* model, float& intensity)
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
	for (P.x = bboxleft.x; P.x <= bboxright.x; P.x++)
	{
		for (P.y = bboxleft.y; P.y <= bboxright.y; P.y++)
		{
			if (insideTriangle_byCross((float)P.x + 0.5, (float)P.y + 0.5, vertex))
			{
				
				Vector3f Barycentric = computeBarycentric2D((float)P.x + 0.5, (float)P.y + 0.5, vertex);
				P.z = interpolate(Barycentric.x, Barycentric.y, Barycentric.z, vertex[0], vertex[1], vertex[2], 1).z;//用三个顶点的z轴坐标插值出P点z值
				Vector2i tex_coords = interpolate(Barycentric.x, Barycentric.y, Barycentric.z, tex[0], tex[1], tex[2], 1);
				if (zbuffer[(P.x + P.y * width)] > P.z)
				{
					TGAColor tex_color = model->diffuse(tex_coords);
					zbuffer[(P.x + P.y * width)] = P.z;
					image.set(P.x, P.y, TGAColor(pow(intensity, 2.2) * tex_color.r, pow(intensity, 2.2) * tex_color.g, pow(intensity, 2.2) * tex_color.b, tex_color.a));
				}
			}
		}
	}
}