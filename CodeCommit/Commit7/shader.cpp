#include "shader.h"

constexpr double MY_PI = 3.1415926;

PhongShader::PhongShader()
{
	M_model.identity();
	M_view.identity();
	M_projection.identity();
	M_viewport.identity();
}

void PhongShader::set_view_matrix(Vector3f eye_pos)
{
	//camera transformations: M_view = R_view * T_view
	Vector3f up(0, 1, 0); //任意定义一个向上的向量
	Vector3f z = eye_pos; z.normalize();
	Vector3f x = (up.cross(z)).normalize();
	Vector3f y = (z.cross(x)).normalize();
	M_view << std::vector<float>{
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
	M_view *= translate;
}
void PhongShader::set_viewport_matrix(float w, float h)
{
	M_viewport <<
		std::vector<float>{
		w/2.f, 0, 0, w/2.f,
			0, h/2.f, 0, h/2.f,
			0, 0, 1, 0,
			0, 0, 0, 1
	};
}
void PhongShader::set_model_matrix(char n, float rotation_angle)
{
	float angle = rotation_angle * MY_PI / 180.0f;
	if (n == 'X')
	{
		M_model << std::vector<float>{
			1, 0, 0, 0,
				0, std::cos(angle), -std::sin(angle), 0,
				0, std::sin(angle), std::cos(angle), 0,
				0, 0, 0, 1
		};
	}
	else if (n == 'Y')
	{
		M_model << std::vector<float>{
			std::cos(angle), 0, std::sin(angle), 0,
				0, 1, 0, 0,
				-std::sin(angle), 0, std::cos(angle), 0,
				0, 0, 0, 1
		};
	}
	else if (n == 'Z')
	{
		M_model << std::vector<float>{
			std::cos(angle), -std::sin(angle), 0, 0,
				std::sin(angle), std::cos(angle), 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1
		};
	}
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

void PhongShader::set_random_model_matrix(Vector3f n, float rotation_angle)
{
	float angle = rotation_angle * MY_PI / 180.0f;
	float nx = n[0], ny = n[1], nz = n[2];
	Matrix3f N;
	N << std::vector<float>{
		0, -nz, ny,
			nz, 0, -nx,
			-ny, nx, 0
	};
	Matrix3f I; I.identity();

	Matrix3f R = I * std::cos(angle) + v_dot_vT(n) * (1 - std::cos(angle)) + N * std::sin(angle);
	M_model << std::vector<float>{
		R(0, 0), R(0, 1), R(0, 2), 0,
			R(1, 0), R(1, 1), R(1, 2), 0,
			R(2, 0), R(2, 1), R(2, 2), 0,
			0, 0, 0, 1
	};
}

void PhongShader::set_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar)
{
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
	M_projection = M_ortho * M_persp * M_projection;
}

Vector3f PhongShader::get_viewport(Vector4f& v, const int& width, const int& height)
{
	v = M_viewport * v;
	return Vector3f(v.x/v.w, v.y/v.w, v.z);
	/*return Vector3f((v.x/v.w + 1.) * width * 0.5, (v.y/v.w + 1.) * height * 0.5, v.z);*/
}

Matrix4f PhongShader::get_model_matrix()
{
	return this->M_model;
}

Matrix4f PhongShader::get_view_matrix()
{
	return this->M_view;
}

Matrix4f PhongShader::get_projection_matrix()
{
	return this->M_projection;
}
Matrix4f PhongShader::get_viewport_matrix()
{
	return this->M_viewport;
}
Vector4f PhongShader::vertex(int i, int j, Vex& vex)
{
	vex.world_coords[j] = model->vert(i, j);
	vex.texture_coords[j] = model->uv(i, j);
	vex.normal_coords[j] = model->normal(i,j);

	uniform_M = M_projection * M_view * M_model;
	uniform_MIT = M_model.inverse_transpose();

	Vector4f v(vex.world_coords[j].x, vex.world_coords[j].y, vex.world_coords[j].z, 1.f);
	v = uniform_M * v;
	vex.w[j] = v.w;
	return v;
}

Vector4f DepthShader::vertex(int i, int j, Vex& vex)
{
	vex.world_coords[j] = model->vert(i, j);
	Vector4f v(vex.world_coords[j].x, vex.world_coords[j].y, vex.world_coords[j].z, 1.f);
	uniform_M = M_projection * M_view;
	v = uniform_M * v;
	vex.w[j] = v.w;
	return v;
}

Matrix4f DepthShader::get_MS()
{
	return M_viewport* M_projection * M_view;
}

void PhongShader::set_shadowMatrix(Matrix4f MS)
{
	MS_view = MS;
}
