# MyTinyRenderer

This software rendering is my personal project following [the wiki](https://github.com/ssloy/tinyrenderer). The main use of the course given obj resources and TGAImage file output framework. I wrote the code for the render pipeline by hand, and I'm constantly adding new features.

## Description
This document is a detailed review of the significant commits to this repository from the very beginning of the project. 
## Commit 5 : Code Refactoring and Gouraud Shading
For this commit I updated the Gouraud Shaing method :<br>
<table>
  <tbody>
    <tr>
      <th align="center">file update</th>
      <th align="center">Description</th>
    </tr>
	<tr>
      <td align="left">
      <ul>
                Vex.h<br>
                shader.h/cpp<br>
	    	</ul>
      </td>
	    <td align="left">
	    	<ul>
	    		<li><b> Update Function:</b>
                <li><b>Vex.h</b>: to store all information about a vertex</li>
                <ul>
                    <li><b>struct Vex:</b>
                    <li>Vector3f screen_coords[3];
                    <li>Vector3f world_coords[3];
                    <li>Vector2f texture_coords[3];
                    <li>Vector3f normal_coords[3];
                    <li>float intensity[3];
                </ul>
	    		<li><b>shader.h/cpp</b>: Build various shader methods
                    <ul>
                    <li><b>class GouraudShader :public Shader{}: </b>The MVP transformation from the previous rasterizer is abstracted into the vertex shader, which is responsible for various operations of vertex shader.
                    <li><b>virtual Vector4f vertex(int i, int j, Vex& vex): Responsible for the major operations of GouraudShader.
                    <li>Matrix4f get_view_matrix(Vector3f eye_pos);
                    <li>Matrix4f get_model_matrix(char n, float rotation_angle);
                    <li>Matrix4f get_random_model_matrix(Vector3f n, float rotation_angle);
                    <li>Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar);
                    <li>Vector3f get_viewport(Vector4f& v, const int& width, const int& height);</b>
                    </ul>
	    	</ul>
	    </td>
	</tr>
  </tbody>
</table>

### Gouraud Shading
The main operation of Gouraud Shading is to calculate intensity of vertexes according to normals of vertexes, and then interpolate to color the inner points. 

    vex.intensity[j] = std::max(0.f, vex.normal_coords[j] * light_dir);
    float intensity = interpolate(Barycentric.x, Barycentric.y, Barycentric.z, vex.intensity[0], vex.intensity[1], vex.intensity[2], 1); 
    
<img width="600" alt="theface" src="https://user-images.githubusercontent.com/74391884/163135581-60b94268-f0f5-406f-9669-44e494c644d1.png"><br>
GouraudShader:

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

## Commit 4* : Corrections of some minor errors
I encountered some errors when testing the model with different camera angles: <br>

<img width="600" alt="theface" src="https://user-images.githubusercontent.com/74391884/162769239-feac4df0-2a99-4b7d-9334-e81ef6d50527.png"><br>

This is a very interesting phenomenon, after discussing with friends and checking one by one. We found that the problem was the use of float type when iterating over fragments in a bounding box. To solve the accuracy problem, I decided to change the loop traversal to an integer type, and to use float type for all design to calculation.<br>

<img width="600" alt="theface" src="https://user-images.githubusercontent.com/74391884/162769214-628a5a68-c102-4c3a-a79b-804253e67807.png"><br>

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
              image.set(P.x, P.y, TGAColor(pow(intensity, 2.2) * tex_color.r, pow(intensity, 2.2) * tex_color.g, pow(intensity, 2.2) * tex_color.b, tex_color.a));
            }
          }
        }
      }

## Commit 4 : Matrix and coordinate transformations
This update adds Matrix templates in geometry.h and implements a series of coordinate transformations in the pipeline.
<table>
  <tbody>
    <tr>
      <th align="center">file update</th>
      <th align="center">Description</th>
    </tr>
	<tr>
      <td align="left">
      <ul>
                geometry.h<br>
                rasterizer.h/cpp<br>
	    	</ul>
      </td>
	    <td align="left">
	    	<ul>
	    		<li><b> Update Function:</b>
                <li><b>rasterizer.h/cpp:</b> added MVP transformation method. </li>
                <ul>
                    <li><b>Matrix4f get_view_matrix(Vector3f eye_pos);</b>
                    <li><b>Matrix4f get_model_matrix(char n , float rotation_angle);</b>
                    <li><b>Matrix4f get_random_model_matrix(Vector3f n, float rotation_angle);</b>
                    <li><b>Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar);</b>
                    <li><b>Vector3f get_viewport(Vector4f& v, const int& width, const int& height);</b>
                </ul>
	    		<li><b>geometry.h:</b> adds Matrix templates.</li>
                    <ul>
                    <b><li>template&lttypename T> struct Vector&ltT, 4>
                    <li>Vector3f Matrix&ltT, NROW, NCOL>::operator*(Vector3f& v) const
                    <li>Matrix();
                    <li>Matrix(T);
                    <li>Matrix(const Matrix&ltT, NROW, NCOL>& );
                    <li>virtual ~Matrix();
                    <li>T& operator()(const size_t i, const size_t j) { assert(i &lt NROW && j < NCOL); return m[i][j]; }
                    <li>const T& operator()(const size_t i, const size_t j) const { assert(i < NROW && j < NCOL); return m[i][j]; }
                    <li>void clear();
                    <li>void identity();
                    <li>void transpose();
                    <li>void show();
                    <li>Matrix&ltT, NROW, NCOL>& operator=(const Matrix&ltT, NROW, NCOL>&);
                    <li>Matrix&ltT, NROW, NCOL>& operator+=(const Matrix&ltT, NROW, NCOL>&);
                    <li>Matrix&ltT, NROW, NCOL>& operator-=(const Matrix&ltT, NROW, NCOL>&);
                    <li>Matrix&ltT, NROW, NCOL>& operator*=(const Matrix&ltT, NROW, NCOL>&);
                    <li>Matrix&ltT, NROW, NCOL>& operator*=(T);
                    <li>Matrix&ltT, NROW, NCOL> operator*(const Matrix&ltT, NROW, NCOL>&) const;
                    <li>Matrix&ltT, NROW, NCOL> operator*(T) const;
                    <li>template &lttypename > friend void operator<<(Matrix&ltT, NROW, NCOL>&, std::vector&ltT>);
                    <li>T ** m = nullptr;</b>
                    </ul>
	    	</ul>
	    </td>
	</tr>
  </tbody>
</table>

### Function Update Description：
This time I overload the operators in a different way (Previously I overloaded vector's operator as global functions. But this time I overloaded matrix as member functions) . The function realized in the template is the basic operation of square matrix, including transpose, Matrix operation, Matrix creation with operator<<. In addition, I manipulate the matrix by a two-dimensional pointer, that opens up space on the heap. 

    template<typename T, size_t NROW, size_t NCOL> struct Matrix 
    {
      Matrix();
      Matrix(T);
      Matrix(const Matrix<T, NROW, NCOL>& );
      virtual ~Matrix();
      T& operator()(const size_t i, const size_t j) { assert(i < NROW && j<NCOL); return m[i][j]; }
      const T& operator()(const size_t i, const size_t j) const { assert(i < NROW && j < NCOL); return m[i][j]; }
      void clear();
      void identity();
      void transpose();
      void show();
      Matrix<T, NROW, NCOL>& operator=(const Matrix<T, NROW, NCOL>&);
      Matrix<T, NROW, NCOL>& operator+=(const Matrix<T, NROW, NCOL>&);
      Matrix<T, NROW, NCOL>& operator-=(const Matrix<T, NROW, NCOL>&);
      Matrix<T, NROW, NCOL>& operator*=(const Matrix<T, NROW, NCOL>&);
      Matrix<T, NROW, NCOL>& operator*=(T);
      Matrix<T, NROW, NCOL> operator*(const Matrix<T, NROW, NCOL>&) const; //*的操作要返回拷贝对象，不是在原对象上修改所以没有&
      Matrix<T, NROW, NCOL> operator*(T) const;
      template <typename > friend void operator<<(Matrix<T, NROW, NCOL>&, std::vector<T>);
      T ** m = nullptr;
    };

The input to matrix is a vector of the corresponding type. I get array and initialize matrix in the form of pass-by-value in the original function:

    template<typename T, size_t NROW, size_t NCOL>
    void operator<<(Matrix<T, NROW, NCOL>& res, std::vector<T> v)
    {
      assert(v.size() == NROW * NCOL);
      for (int i = 0; i < NROW; i++)
      {
        for (int j = 0; j < NCOL; j++)
        {
          res.m[i][j] = v[i * NCOL + j];
        }
      }
    };
    //input:
    int main(int argc, char** argv) {
        Matrix3f m;
        m << std::vector<float>{
            1,2,3,
            1,2,3,
            1,2,3
        };
        Matrix3f n;
        n << std::vector<float>{
            1, 2, 3,
            1, 2, 3,
            1, 2, 3
        };
        Matrix3f mn = m * n;
        mn.show();
        /*6 12 18
          6 12 18
          6 12 18*/
        return 0;
    }

Added a new 4-dimensional vector type and added matrix * 4-dimensional vector to facilitate MVP transformation

    template<typename T, size_t NROW, size_t NCOL>
    Vector4f Matrix<T, NROW, NCOL>::operator*(Vector4f& v) const
    {
      assert(NROW == 4 && NCOL == 4);
      Vector4f res(0, 0, 0, 0);
      for (int i = 0; i < NROW; i++)
      {
        for (int j = 0; j < NCOL; j++)
        {
          res[i] += m[i][j] * v[j];
        }
      }
      return res;
    };

### MVP part
This part we're going to do three transformations of the world vertex coordinates: MVP = M_pro * M_view * M_model * v; (don't forget viewport transformation finally)

#### Model transformation:
In this section I implemented the method of rotation about xyz axis and rotation about any axis: <br>

<img width="400" alt="theface" src="https://user-images.githubusercontent.com/74391884/162553304-c5c10efa-8995-4456-9dfc-aafd6b500531.png"><br>
#### Camera transformation:
This section implements the transformation of View Matrix: <br>

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

<img width="400" alt="theface" src="https://user-images.githubusercontent.com/74391884/162553327-8cc8f0c5-6988-4747-b191-f3ba66edb288.png"><br>
#### Projection transformation:
The last part is projection transformation, including orthogonal transformation and perspective transformation: <br>

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

<img width="400" alt="theface" src="https://user-images.githubusercontent.com/74391884/162553338-2700460a-196c-4b87-83dd-890f643e2add.png"><br>
Added MVP transformation in Main:

      Vector3f vex = model->vert(face[j]);
      Matrix4f M_model = rst.get_model_matrix('Z', 45);
      Matrix4f M_view = rst.get_view_matrix(eye_pos);
      Matrix4f M_pro = rst.get_projection_matrix(45, 1, 0.1, 50);
      Vector4f v(vex.x, vex.y, vex.z, 1.f);
      v =  M_pro * M_view * M_model * v;
      v = v/v.w;
      screen_coords[j] = rst.get_viewport(v, width, height);

#### error record: about memset zbuffer

memset function can only be used for initialization and take care to input the correct nbyte! 

    TGAImage::TGAImage(int w, int h, int bpp) : data(nullptr), zbuffer(nullptr), width(w), height(h), bytespp(bpp) {
      unsigned long nbytes = width*height*bytespp;
      data = new unsigned char[nbytes];
      memset(data, 0, nbytes);
      
      zbuffer = new float[width * height];
      //error: memset(zbuffer, std::numeric_limits<float>::infinity(), sizeof(zbuffer));
      memset(zbuffer, 0, width * height * sizeof(float));
      for (int i = width * height; i--;) zbuffer[i] = std::numeric_limits<float>::infinity();
    }

But! Space discontinuities occur when two dimensional arrays apply for heap memory (memset can only be used for initialization of space continuities)
## Commit 3 : Rasterizer and texture
This update abstracts the previous rendering from tgaimage and adds the texture method.
<table>
  <tbody>
    <tr>
      <th align="center">file update</th>
      <th align="center">Description</th>
    </tr>
	<tr>
      <td align="left">
      <ul>
                model.h/cpp<br>
                rasterizer.h/cpp<br>
	    	</ul>
      </td>
	    <td align="left">
	    	<ul>
	    		<li><b> Update Function:</b>
                <li><b>model.h/cpp</b></li>
                <ul>
                    <li><b>Vector2i uv(int iface, int nvert)</b>: Read UV coordinates
                    <li><b>TGAColor diffuse(Vector2i uv)</b>: Read texture color by texture coordinates
                </ul>
	    		<li><b>rasterizer.h/cpp</b>
                    <ul><li><b>void line(Vec2i t0, Vec2i t1, TGAImage& image, TGAColor color)</b>: render pixels according to linear interpolation method;
                    <li><b>void LineTriangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color)</b>:  to get the contour of the triangle by line()
                    <li><b>void triangle(Vec2i* vertex, TGAImage& image, TGAColor color)</b>: 1) select the lower-left and upper-right points to build the bounding box; 2) Scan the pixels in the bounding box to call bool insideTriangle() and image.set() (framebuffer)
                    <li><b>void triangle(Vector3f* vertex, Vector2i* tex, TGAImage& image, Model* model, float& intensity)</b>: overloading triangle to add texture color interpolation.
                    </ul>
	    	</ul>
	    </td>
	</tr>
  </tbody>
</table>

overloading triangle(Fix accuracy error in Commit 4*):

	void Rasterizer::triangle(Vector3f* vertex, Vector2f* tex, TGAImage& image, Model* model, float& intensity)
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
						image.set(P.x, P.y, TGAColor(pow(intensity, 2.2) * tex_color.r, pow(intensity, 2.2) * tex_color.g, pow(intensity, 2.2) * tex_color.b, tex_color.a));
					}
				}
			}
		}
	}
Added texture colors and merged lighting effects：<br>
<img width="400" alt="theface" src="https://user-images.githubusercontent.com/74391884/162105883-595fa3db-a2b7-41b7-be78-a5ed251a23d8.png"><br>
<br>
## Commit 2 : Code refactoring and zbuffer
This update reconstructs the mathematics of the entire code framework, abandoning the old implementation in the tutorial.<br>
<table>
  <tbody>
    <tr>
      <th align="center">file update</th>
      <th align="center">Description</th>
    </tr>
	<tr>
      <td align="left">
      <ul>
                main.cpp<br>
                geometry.h<br>
                model.h/cpp<br>
                tgaimage.h/cpp<br>
	    	</ul>
      </td>
	    <td align="left">
	    	<ul>
	    		<b> Update Class/Struct:</b>
                <li><b>geometry.h</b></li>
                <ul>
                    <li><b>template &lttypename T, size_t LEN&gt struct Vector</b>: The main template class provides a generic interface to a particular Vector(Vector3f/Vector2f...). Generic function templates and operator overloading are provided for this type.
                    <li><b>template &lttypename T&gt struct Vector&ltT, 2&gt</b>: The specialized template class overload some methods that are specific to two-dimensional vectors.
                    <li><b>template &lttypename T&gt struct Vector&ltT, 3&gt</b>: The specialized template class overload some methods that are specific to three-dimensional vectors.
                </ul>
	    		<li><b>tgaimage.h/cpp</b>
                    <ul><li><b>float* zbuffer</b>: Points to a one-dimensional ZBuffer array (width*height) and allocates memory when the object is created.
                    </ul>
          <b> Update Function:</b>
                <li><b>tgaimage.h/cpp</b></li>
                <ul>
                    <li><b>static Vector3f computeBarycentric2D(float x, float y, const Vector3f* v)</b>: return barycentric coordinate interpolation.
                    <li><b>void TGAImage::triangle(Vector3f* vertex, TGAImage& image, TGAColor color)</b>: Added zbuffer method: Interpolate z values of P by z values of three vertices and render the point with the lower z value. 
                </ul>
                <li><b>geometry.h</b></li>
                <ul>
                <li><b>inline Vector&ltT, LEN&gt operator+(Vector&ltT, LEN&gt res, const Vector&ltT, LEN&gt& v)</b>
                <li><b>inline Vector&ltT, LEN&gt operator-(Vector&ltT, LEN&gt res, const Vector&ltT, LEN&gt& v)</b>
                <li><b>inline Vector&ltT, LEN&gt operator*(Vector&ltT, LEN&gt res, float f)</b>
                <li><b>inline Vector&ltT, LEN&gt operator*(Vector&ltT, LEN&gt& res, const Vector&ltT, LEN&gt& v)</b>
                <li><b>inline Vector&ltT, 2> cwiseProduct(const Vector&ltT, 2>& v)</b>
                <li><b>T& operator[](const size_t i)</b>
                <li><b>const T& operator[](const size_t i) const</b>
                <li><b>T& operator[](const size_t i)</b>
                <li><b>const T& operator[](const size_t i) const</b>
                <li><b>inline Vector&ltT, 3> cwiseProduct(const Vector&ltT, 3>& v) const</b>
                <li><b>inline Vector&ltT, 3> cross(const Vector&ltT, 3>& v) const</b>
                <li><b>float norm() const </b>
                <li><b>Vector&ltT, 3>& normalize(T l = 1)</b>
                </ul>
	    	</ul>
	    </td>
	</tr>
  </tbody>
</table>

#### Function Update Description：

<b>About zbuffer:</b>
Add the initialization of the Zbuffer and space allocation of the Z table to the TGAImage constructor:

    TGAImage::TGAImage(int w, int h, int bpp) : data(nullptr), zbuffer(nullptr), width(w), height(h), bytespp(bpp) {
      unsigned long nbytes = width*height*bytespp;
      data = new unsigned char[nbytes];
      memset(data, 0, nbytes);
      
      zbuffer = new float[width * height];
      memset(zbuffer, 0, width * height * sizeof(float));
      for (int i = width * height; i--;) zbuffer[i] = std::numeric_limits<float>::infinity();
    }

    TGAImage::TGAImage(const TGAImage &img) {
      width = img.width;
      height = img.height;
      bytespp = img.bytespp;
      unsigned long nbytes = width*height*bytespp;
      data = new unsigned char[nbytes];
      memcpy(data, img.data, nbytes);
      zbuffer = new float[width * height];
      memcpy(zbuffer, img.zbuffer, width * height * sizeof(float));
    }

    TGAImage::~TGAImage() {
      if (data) delete [] data;
      if (zbuffer) delete[] zbuffer;
    }

    TGAImage & TGAImage::operator =(const TGAImage &img) {
      if (this != &img) {
        if (data) delete [] data;
        width  = img.width;
        height = img.height;
        bytespp = img.bytespp;
        unsigned long nbytes = width*height*bytespp;
        data = new unsigned char[nbytes];
        memcpy(data, img.data, nbytes);
        zbuffer = new float[width * height];
        memcpy(zbuffer, img.zbuffer, width * height * sizeof(float));
      }
      return *this;
    }

<b>Record a z value for each pixel and render only the pixels with the lowest z value:</b><br>
<img width="400" alt="theface" src="https://user-images.githubusercontent.com/74391884/161718651-2a956e46-0301-4899-bfc3-1f2f0be8ac46.png"><br>
<b>Found that the tutorial images had color differences, so gamma correction was added</b><br>

	image.triangle(vertex, image, TGAColor(pow(intensity,2.2) * 255, pow(intensity, 2.2) * 255, pow(intensity, 2.2) * 255, 255));
	
<img width="400" alt="theface" src="https://user-images.githubusercontent.com/74391884/161719094-b49626d0-5ed9-4f9a-94f6-4a35cf841fda.png"><br>

    void TGAImage::triangle(Vector3f* vertex, TGAImage& image, TGAColor color)
    {
      
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
          P.z = 0;
          if (insideTriangle((float)P.x + 0.5, (float)P.y + 0.5, vertex))
          {
            Vector3f Barycentric = computeBarycentric2D((float)P.x + 0.5, (float)P.y + 0.5, vertex);
            /*for (int i = 0; i < 3; i++) P.z += vertex[i].z * Barycentric[i];*/
            P.z = vertex[0].z * Barycentric.x + vertex[1].z * Barycentric.y + vertex[2].z * Barycentric.z;//用三个顶点的z轴坐标插值出P点z值
            if (zbuffer[(P.x + P.y * width)] < P.z)
            {
              zbuffer[(P.x + P.y * width)] = P.z;
              image.set(P.x, P.y, color);
            }
          }
        }
      }
    }
## Commit 1 : Line drawing and framebuffer (included barycentric coordinate interpolation)
### Function Update Description：
#### the triangle() : find barycentric coordinates of the point P with respect to the triangle ABC. And determine whether a point is in a triangle by the barycentric coordinates.

    void TGAImage::triangle(Vec2i* vertex, TGAImage& image, TGAColor color)
    {
        Vec2i bboxleft(image.get_width() - 1, image.get_height() - 1);
        Vec2i bboxright(0, 0);
        for (int i = 0; i < 3; i++) //选取左下角与右上角的点构建包围盒
        {
            bboxleft.x = std::max(0, std::min(bboxleft.x, vertex[i].x));
            bboxleft.y = std::max(0, std::min(bboxleft.y, vertex[i].y));

            bboxright.x = std::min(image.get_width() - 1, std::max(bboxright.x, vertex[i].x));
            bboxright.y = std::min(image.get_height() - 1, std::max(bboxright.y, vertex[i].y));
        }
        Vec2i P;
        for (P.x = bboxleft.x; P.x <= bboxright.x; P.x++)
        {
            for (P.y = bboxleft.y; P.y <= bboxright.y; P.y++)
            {
                if (insideTriangle((float)P.x + 0.5, (float)P.y + 0.5, vertex))//使用像素中心点判断是否在三角形内部
                    image.set(P.x, P.y, color);//这里framebuff要用整型
            }
        }
    }
#### bool insideTriangle(float x, float y, const Vec2i* v): find barycentric coordinates of the point P with respect to the triangle ABC. And determine whether a point is in a triangle by the barycentric coordinates.
Especially, if INF is not used, there will be accuracy problems:<br>
<img width="400" alt="theface" src="https://user-images.githubusercontent.com/74391884/161389559-868fdc5f-7f6d-464e-b410-c0273977c401.png"><br>
We can see a lot of unrendered black dots and lines from the image. But there was a significant improvement with INF:<br>
<img width="651" alt="theface" src="https://user-images.githubusercontent.com/74391884/161389500-3bc44ea7-73a0-414a-8aba-8cd772b82d3d.png"><br>

    static bool insideTriangle(float x, float y, const Vec2i* v) //定义全局函数，并在头文件geometry中声明
    {   //constexpr double INF = 1e-4;
        float c2 = (x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * y + v[2].x * v[0].y - v[0].x * v[2].y) / (v[1].x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * v[1].y + v[2].x * v[0].y - v[0].x * v[2].y);
        float c3 = (x * (v[0].y - v[1].y) + (v[1].x - v[0].x) * y + v[0].x * v[1].y - v[1].x * v[0].y) / (v[2].x * (v[0].y - v[1].y) + (v[1].x - v[0].x) * v[2].y + v[0].x * v[1].y - v[1].x * v[0].y);
        float c1 = 1 - c2 - c3;
        if (c1 - 1.0 < INF  && c1 > -INF && c2 - 1.0  < INF && c2> -INF && c3 - 1.0 < INF && c3 > -INF) return true; //INF防止浮点数陷阱
        else return false;
    }

### Main effects
<img width="937" alt="f0" src="https://user-images.githubusercontent.com/74391884/161389475-08a7220e-c172-4e27-bf4c-dc0ad045e9c9.png">
<b>About back-face culling:</b><br>
In computer graphics, back-face culling determines whether a polygon of a graphical object is visible. We get normal vector of the flat through the world coordinates of the three vertices of the triangle, and judge it by the dot product of the light direction and the normal vector.

<img width="400" alt="theface" src="https://user-images.githubusercontent.com/74391884/161389414-0bc07848-b780-48e1-ac0d-d579f2f01a92.png">

But we uniformly define the vertex storage order as counter-clockwise storage (front-facing polygons have a counter-clockwise winding: v0->v1->v2 is counter-clockwise). The normal vector that we get by cross product is going to be coming out of the object. The dot product will depend on the position of the flat with respect to the light source. 
<table>
  <tbody>
    <tr>
      <th align="center">file update</th>
      <th align="center">Description</th>
    </tr>
	<tr>
      <td align="left">
      <ul>
                main.cpp<br>
	    		geometry.h<br>
                tgaimage.h/cpp<br>
	    	</ul>
      </td>
	    <td align="left">
	    	<ul>
	    		<li><b> Update Function:</b>
                <li><b>geometry.h/main.cpp</b></li>
                <ul>
                    <li><b>bool insideTriangle(float x, float y, const Vec2i* v)</b>: find barycentric coordinates of the point P with respect to the triangle ABC. And determine whether a point is in a triangle by the barycentric coordinates.
                </ul>
	    		<li><b>tgaimage.h/cpp</b>
                    <ul><li><b>void line(Vec2i t0, Vec2i t1, TGAImage& image, TGAColor color)</b>: render pixels according to linear interpolation method;
                    <li><b>void LineTriangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color)</b>:  to get the contour of the triangle by line()
                    <li><b>void triangle(Vec2i* vertex, TGAImage& image, TGAColor color)</b>: 1) select the lower-left and upper-right points to build the bounding box; 2) Scan the pixels in the bounding box to call bool insideTriangle() and image.set() (framebuffer)
                    </ul>
	    	</ul>
	    </td>
	</tr>
  </tbody>
</table>

## Commit 0 : obj resources and TGAImage file output framework
<table>
  <tbody>
    <tr>
      <th align="center">New file </th>
      <th align="center">Description</th>
    </tr>
	<tr>
      <td align="left">
      <ul>
	    		geometry.h<br>
                model.h/cpp<br>
                tgaimage.h/cpp<br>
	    	</ul>
      </td>
	    <td align="left">
	    	<ul>
	    		<li><b>geometry.h</b>: define struct Vec2f(float) / Vec2i(int) / Vec3f /Vec3i</li>
	    		<li><b>model.h/cpp</b>: obj file loading;
                    <ul><li><b>vector<int> face(int idx)</b>: return world coordinates idx; face[i] contain three points of triangles;
                    <li><b>vec3f vert(int i)</b>: return world coordinates Vec3f (represents a vertex of triangle);
                    </ul>
                <li><b>tgaimage.h/cpp</b>: TGAImage file output framework</li>
	    	</ul>
	    </td>
	</tr>
  </tbody>
</table>
