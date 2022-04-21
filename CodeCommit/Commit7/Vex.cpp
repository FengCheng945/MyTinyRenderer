#include "Vex.h"


void Vex::set_TBN()
{
    float u1 = texture_coords[0].x;
    float v1 = texture_coords[0].y;
    float u2 = texture_coords[1].x;
    float v2 = texture_coords[1].y;
    float u3 = texture_coords[2].x;
    float v3 = texture_coords[2].y;
    float deltaU1 = u2 - u1;
    float deltaU2 = u3 - u1;
    float deltaV1 = v2 - v1;
    float deltaV2 = v3 - v1;
    Matrix<float, 2, 3> E; //edge
    E << std::vector<float>{
        world_coords[1].x - world_coords[0].x, world_coords[1].y - world_coords[0].y, world_coords[1].z - world_coords[0].z,
            world_coords[2].x - world_coords[0].x, world_coords[2].y - world_coords[0].y, world_coords[2].z - world_coords[0].z
    };
    Matrix2f UV_inv;
    UV_inv << std::vector<float>{
        deltaV2, -deltaV1,
            -deltaU2, deltaU1
    };
    UV_inv *= 1.f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);
    Matrix<float, 2, 3> TB;
    TB = UV_inv * E;
    Vector3f T(TB.m[0][0], TB.m[0][1], TB.m[0][2]);
    Vector3f B(TB.m[1][0], TB.m[1][1], TB.m[1][2]);

    T.normalize();
    B.normalize();
    TBN << std::vector<float>{
        T[0], B[0], 0,
            T[1], B[1], 0,
            T[2], B[2], 0
    };
}

void Vex::set_TBN(Vector3f& N)
{
    N.normalize();
    TBN.m[0][2] = N[0];
    TBN.m[1][2] = N[1];
    TBN.m[2][2] = N[2];
}
