#include "shader.h"

constexpr double MY_PI = 3.1415926;

Matrix4f GouraudShader::get_view_matrix(Vector3f eye_pos)
{
	//camera transformations: M_view = R_view * T_view
	Vector3f up(0, 1, 0);
	Matrix4f view; view.identity();
	Vector3f z = eye_pos; z.normalize();
	Vector3f x = (up.cross(z)).normalize();
	Vector3f y = (z.cross(x)).normalize();
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

Matrix4f GouraudShader::get_model_matrix(char n, float rotation_angle)
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

Matrix4f GouraudShader::get_random_model_matrix(Vector3f n, float rotation_angle)
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

	Matrix3f R = I * std::cos(angle) + v_dot_vT(n) * (1 - std::cos(angle)) + N * std::sin(angle);
	model << std::vector<float>{
		R(0, 0), R(0, 1), R(0, 2), 0,
			R(1, 0), R(1, 1), R(1, 2), 0,
			R(2, 0), R(2, 1), R(2, 2), 0,
			0, 0, 0, 1
	};
	return model;
}

Matrix4f GouraudShader::get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar)
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

Vector3f GouraudShader::get_viewport(Vector4f& v, const int& width, const int& height)
{
	return Vector3f((v.x + 1.) * width * 0.5, (v.y + 1.) * height * 0.5, v.z);
}

Vector4f GouraudShader::vertex(int i, int j, Vex& vex)
{
	vex.world_coords[j] = model->vert(i, j);
	vex.texture_coords[j] = model->uv(i, j);
	vex.normal_coords[j] = model->normal(i,j);


	Matrix4f M_model = get_model_matrix('Z', 45);
	Matrix4f M_view = get_view_matrix(eye_pos);
	Matrix4f M_pro = get_projection_matrix(45, 1, 0.1, 50);
	Vector4f v(vex.world_coords[j].x, vex.world_coords[j].y, vex.world_coords[j].z, 1.f);
	v = M_pro * M_view * M_model * v;
	v = v / v.w;
	return v;
}