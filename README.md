# MyTinyRender

This software rendering is my personal project following [the wiki](https://github.com/ssloy/tinyrenderer). The main use of the course given obj resources and TGAImage file output framework. I wrote the code for the render pipeline by hand, and I'm constantly adding new features.

## Description
This document is a detailed review of the significant commits to this repository from the very beginning of the project. 


## Commit 1 : Line drawing and framebuffer (included barycentric coordinate interpolation)

| New file                | Description              |
| :-----------------------|:-------------------------|
| 

## Commit 0 : obj resources and TGAImage file output framework
| New file                | Description              |
|  -----------------------|  ------------------------|
 -geometry.h              |-geometry.h: define struct Vec2f(float)/Vec2i(int)/Vec3f/Vec3i|
 -model.h/cpp             -model.h/cpp: obj file loading
 -tgaimage.h/cpp            -vector<int> face(int idx): return world coordinates idx; face[i] contain three points of triangles;
                            -vec3f vert(int i): return world coordinates Vec3f (represents a vertex of triangle)
                           -tgaimage.h/cpp: TGAImage file output framework
  
|                          |                                                             |
