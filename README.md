# MyTinyRenderer

This software rendering is my personal project following [the wiki](https://github.com/ssloy/tinyrenderer). The main use of the course given obj resources and TGAImage file output framework. I wrote the code for the render pipeline by hand, and I'm constantly adding new features.

## Description
This document is a detailed review of the significant commits to this repository from the very beginning of the project. 

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

overloading triangle:

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
          if (insideTriangle((float)P.x + 0.5, (float)P.y + 0.5, vertex))
          {
            Vector3f Barycentric = computeBarycentric2D((float)P.x + 0.5, (float)P.y + 0.5, vertex);
            P.z = interpolate(Barycentric.x, Barycentric.y, Barycentric.z, vertex[0], vertex[1], vertex[2], 1).z;//用三个顶点的z轴坐标插值出P点z值
            Vector2i tex_coords = interpolate(Barycentric.x, Barycentric.y, Barycentric.z, tex[0], tex[1], tex[2], 1);
            if (zbuffer[(P.x + P.y * width)] < P.z)
            {
              TGAColor tex_color = model->diffuse(tex_coords);
              zbuffer[(P.x + P.y * width)] = P.z;
              image.set(P.x, P.y, TGAColor(pow(intensity, 2.2) * tex_color.r, pow(intensity, 2.2) * tex_color.g, pow(intensity, 2.2) * tex_color.b, tex_color.a));
            }
          }
        }
      }
    }

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

    TGAImage::TGAImage() : data(nullptr), zbuffer(nullptr), width(0), height(0), bytespp(0) {
      }
    TGAImage::TGAImage(int w, int h, int bpp) : data(nullptr), zbuffer(nullptr), width(w), height(h), bytespp(bpp) {
          unsigned long nbytes = width*height*bytespp;
          data = new unsigned char[nbytes];
          memset(data, 0, nbytes);
          zbuffer = new float[width * height];
          memset(zbuffer, std::numeric_limits<float>::infinity(), sizeof zbuffer);
      }
    TGAImage::TGAImage(const TGAImage &img) {
          width = img.width;
          height = img.height;
          bytespp = img.bytespp;
          unsigned long nbytes = width*height*bytespp;
          data = new unsigned char[nbytes];
          memcpy(data, img.data, nbytes);
          zbuffer = new float[width * height];
          memcpy(zbuffer, img.zbuffer, sizeof zbuffer);
      }
    TGAImage::~TGAImage() {
          if (data) delete [] data;
          if (zbuffer) delete[] zbuffer;
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
