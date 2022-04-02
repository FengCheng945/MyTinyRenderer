# MyTinyRender

This software rendering is my personal project following [the wiki](https://github.com/ssloy/tinyrenderer). The main use of the course given obj resources and TGAImage file output framework. I wrote the code for the render pipeline by hand, and I'm constantly adding new features.

## Description
This document is a detailed review of the significant commits to this repository from the very beginning of the project. 


## Commit 1 : Line drawing and framebuffer (included barycentric coordinate interpolation)
#### Function Update Description：
##### the triangle() : find barycentric coordinates of the point P with respect to the triangle ABC. And determine whether a point is in a triangle by the barycentric coordinates.

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
##### bool insideTriangle(float x, float y, const Vec2i* v): find barycentric coordinates of the point P with respect to the triangle ABC. And determine whether a point is in a triangle by the barycentric coordinates.
especially, if INF is not used, there will be accuracy problems:
tupian
We can see a lot of unrendered black dots and lines from the image. But there was a significant improvement with INF:
tupian

    bool insideTriangle(float x, float y, const Vec2i* v) //定义全局函数，并在头文件geometry中声明
    {   //constexpr double INF = 1e-9;
        float c2 = (x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * y + v[2].x * v[0].y - v[0].x * v[2].y) / (v[1].x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * v[1].y + v[2].x * v[0].y - v[0].x * v[2].y);
        float c3 = (x * (v[0].y - v[1].y) + (v[1].x - v[0].x) * y + v[0].x * v[1].y - v[1].x * v[0].y) / (v[2].x * (v[0].y - v[1].y) + (v[1].x - v[0].x) * v[2].y + v[0].x * v[1].y - v[1].x * v[0].y);
        float c1 = 1 - c2 - c3;
        if (c1 - 1.0 < INF  && c1 > -INF && c2 - 1.0  < INF && c2> -INF && c3 - 1.0 < INF && c3 > -INF) return true; //INF防止浮点数陷阱
        else return false;
    }

#### Main effects
tupian
<b>About back-face culling:</b>
In computer graphics, back-face culling determines whether a polygon of a graphical object is visible. We get normal vector of the flat through the world coordinates of the three vertices of the triangle, and judge it by the dot product of the light direction and the normal vector.

![image](https://user-images.githubusercontent.com/74391884/161389414-0bc07848-b780-48e1-ac0d-d579f2f01a92.png)

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
