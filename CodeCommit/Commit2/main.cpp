#include <vector>
#include<iostream>
#include <cmath>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"


const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255); //(R,G,B,A)
const TGAColor green = TGAColor(0, 255, 0, 255);
Model *model = NULL;
const int width  = 800;
const int height = 800;





int main(int argc, char** argv) {
    TGAImage image(width, height, TGAImage::RGB);
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }
    Vector3f light_dir(0, 0, 1); // define light_dir

    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        Vector3f screen_coords[3];
        Vector3f world_coords[3];
        /*Vector2f texture[3];*/
        for (int j = 0; j < 3; j++)
        {
            Vector3f v = model->vert(face[j]);
            screen_coords[j] = Vector3f((v.x + 1.) * width / 2., (v.y + 1.) * height / 2., v.z);
            world_coords[j] = v;
            /*texture[i] = model->uv(face[2 * j + 1]);*/
        }
        Vector3f n = (world_coords[1] - world_coords[0]).cross(world_coords[2] - world_coords[0]); 
        n.normalize();
        float intensity = n * light_dir;
        if (intensity > 0) 
        {
            Vector3f vertex[3] = { screen_coords[0], screen_coords[1], screen_coords[2] };
            image.triangle(vertex, image, TGAColor(pow(intensity,2.2) * 255, pow(intensity, 2.2) * 255, pow(intensity, 2.2) * 255, 255));
        }
    }
    image.flip_vertically(); 
    image.write_tga_file("framebuffer1.tga");
    delete model;
    return 0;
}

