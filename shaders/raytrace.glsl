#version 330

#define float2 vec2
#define float3 vec3
#define float4 vec4
#define float4x4 mat4
#define float3x3 mat3

struct Sphere {
    float4 color;

    float3 center;
    float r;
};

in float2 fragmentTexCoord;

layout(location = 0) out vec4 fragColor;

uniform int g_screenWidth;
uniform int g_screenHeight;

uniform float3 g_bBoxMin   = float3(-1,-1,-1);
uniform float3 g_bBoxMax   = float3(+1,+1,+1);

uniform float4x4 g_rayMatrix;

uniform float4 g_bgColor = float4(0.0, 0.0, 0.0, 1.0);

const float EPS = 1e-4;
const int NUM_OF_SPHERES = 2;

const Sphere spheres[NUM_OF_SPHERES] = Sphere[NUM_OF_SPHERES](
    Sphere(
        float4(1.0, 1.0, 0.0, 1.0),
        float3(0.0, 0.0, -10.0),
        3.0
    ),
    Sphere(
        float4(1.0, 0.0, 1.0, 1.0),
        float3(0.0, 0.0, -5.0),
        1.0
    )
);

float3 EyeRayDir(float x, float y, float w, float h)
{
	float fov = 3.141592654f/(2.0f);
    float3 ray_dir;

	ray_dir.x = x+0.5f - (w/2.0f);
	ray_dir.y = y+0.5f - (h/2.0f);
	ray_dir.z = -(w)/tan(fov/2.0f);

    return normalize(ray_dir);
}

float IntersectSphere(float3 pos, Sphere sphere) {
    return abs(length(pos - sphere.center) - sphere.r);
}

float4 RayMarch(float3 ray_pos, float3 ray_dir) {
    float4 color;
    float3 point = ray_pos;

    float dist = 10000.0;
    while (dist > EPS) {
        dist = 10000.0;

        for (int i = 0; i < NUM_OF_SPHERES; i++) {
            float tmp = IntersectSphere(point, spheres[i]);
            if (tmp < dist) {
                dist = tmp;
                color = spheres[i].color;
            }
        }

        if (dist > 100.0) {
            color = g_bgColor;
            break;
        }

        point += dist * ray_dir;
    }

    return color;
}

void main(void)
{
    float w = float(g_screenWidth);
    float h = float(g_screenHeight);

    // get curr pixelcoordinates
    //
    float x = fragmentTexCoord.x*w;
    float y = fragmentTexCoord.y*h;

    // generate initial ray
    //
    float3 ray_pos = float3(0,0,0);
    float3 ray_dir = EyeRayDir(x,y,w,h);

    // transorm ray with matrix
    //
    ray_pos = (g_rayMatrix*float4(ray_pos,1)).xyz;
    ray_dir = float3x3(g_rayMatrix)*ray_dir;

    // intersect bounding box of the whole scene, if no intersection found return background color
    //
    float tmin = 1e38f;
    float tmax = 0;

    fragColor = RayMarch(ray_pos, ray_dir);
}

