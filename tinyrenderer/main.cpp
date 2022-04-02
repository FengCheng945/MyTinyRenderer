#include <vector>
#include<iostream>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

constexpr double INF = 1e-9;
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255); //(R,G,B,A)
const TGAColor green = TGAColor(0, 255, 0, 255);
Model *model = NULL;
const int width  = 800;
const int height = 800;

bool insideTriangle(float x, float y, const Vec2i* v) //定义全局函数，并在头文件geometry中声明
{

    float c2 = (x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * y + v[2].x * v[0].y - v[0].x * v[2].y) / (v[1].x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * v[1].y + v[2].x * v[0].y - v[0].x * v[2].y);
    float c3 = (x * (v[0].y - v[1].y) + (v[1].x - v[0].x) * y + v[0].x * v[1].y - v[1].x * v[0].y) / (v[2].x * (v[0].y - v[1].y) + (v[1].x - v[0].x) * v[2].y + v[0].x * v[1].y - v[1].x * v[0].y);
    float c1 = 1 - c2 - c3;
  
    if (c1 - 1.0 < INF  && c1 > -INF && c2 - 1.0  < INF && c2> -INF && c3 - 1.0 < INF && c3 > -INF) return true; //INF防止浮点数陷阱
    else return false;
}

int main(int argc, char** argv) {
    TGAImage image(width, height, TGAImage::RGB);
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }
    //LineFace:
    //for (int i=0; i<model->nfaces(); i++) {
    //    std::vector<int> face = model->face(i);
    //    for (int j=0; j<3; j++) {
    //        Vec3f v0 = model->vert(face[j]);
    //        Vec3f v1 = model->vert(face[(j+1)%3]);
    //        int x0 = (v0.x+1.)*width/2.;
    //        int y0 = (v0.y+1.)*height/2.;
    //        int x1 = (v1.x+1.)*width/2.;
    //        int y1 = (v1.y+1.)*height/2.;
    //        Vec2i t0(x0,y0);
    //        Vec2i t1(x1,y1);
    //        image.line(t0, t1, image, white);
    //    }
    //}
    //LineTriangle:
    //Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
    //Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
    //Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };
    //image.LineTriangle(t0[0], t0[1], t0[2], image, red);
    //image.LineTriangle(t1[0], t1[1], t1[2], image, white);
    //image.LineTriangle(t2[0], t2[1], t2[2], image, green);
    //Triangle render
    //Vec2i vertex[3] = { Vec2i(10,10), Vec2i(100, 30), Vec2i(190, 160) };
    //image.triangle(vertex, image,red);
    //Flat shading render
    //for (int i = 0; i < model->nfaces(); i++)
    //{
    //    std::vector<int> face = model->face(i);
    //    Vec2i screen_coords[3];
    //    for (int j = 0; j < 3; j++)
    //    {
    //        Vec3f world_coords = model->vert(face[j]);
    //        screen_coords[j] = Vec2i((world_coords.x + 1.) * width / 2., (world_coords.y + 1.) * height / 2.); //视口变换
    //    }
    //    Vec2i vertex[3] = { screen_coords[0], screen_coords[1], screen_coords[2] };
    //    image.triangle(vertex, image, TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
    //}
    //
    Vec3f light_dir(0, 0, 1); // define light_dir

    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);

        Vec2i screen_coords[3];
        Vec3f world_coords[3];
        for (int j = 0; j < 3; j++)
        {
            Vec3f v = model->vert(face[j]);
            screen_coords[j] = Vec2i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2.);
            world_coords[j] = v;
        }
        Vec3f n = (world_coords[1] - world_coords[0]) ^ (world_coords[2] - world_coords[0]);
        n.normalize();
        float intensity = n * light_dir;
        if (intensity > 0) //背面剔除操作，相机到物体向量点乘<0就代表在背面，不渲染。
        {
            Vec2i vertex[3] = { screen_coords[0], screen_coords[1], screen_coords[2] };
            image.triangle(vertex, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
        }
    }
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("framebuffer1.tga");
    delete model;
    return 0;
}

