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

struct LightSource {
    float3 pos;
    float intensity;
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

// Scene layout

const LightSource lights[] = LightSource[](
    LightSource(
        float3(0.0, -4.0, 10.0),
        1.0
    )
);

const Sphere spheres[] = Sphere[](
    Sphere(
        float4(1.0, 1.0, 0.0, 1.0),
        float3(0.0, 0.0, -10.0),
        3.0
    ),
    Sphere(
        float4(1.0, 0.0, 1.0, 1.0),
        float3(0.0, 0.0, -5.0),
        1.0
    ),
    Sphere(
        float4(0.0, 1.0, 1.0, 0.5),
        float3(0.0, -2.0, -5.0),
        0.5
    ),
    Sphere(
        float4(0.0, 1.0, 1.0, 0.5),
        float3(0.0, 2.0, -5.0),
        0.5
    ),
    Sphere(
        float4(0.0, 1.0, 1.0, 0.5),
        float3(-2.0, 0.0, -5.0),
        0.5
    ),
    Sphere(
        float4(0.0, 1.0, 1.0, 0.5),
        float3(2.0, 0.0, -5.0),
        0.5
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
    return length(pos - sphere.center) - sphere.r;
}

float3 EstimateNormalSphere(float3 z, float eps, Sphere sphere) {
    float3 z1 = z + float3(eps, 0, 0);
    float3 z2 = z - float3(eps, 0, 0);
    float3 z3 = z + float3(0, eps, 0);
    float3 z4 = z - float3(0, eps, 0);
    float3 z5 = z + float3(0, 0, eps);
    float3 z6 = z - float3(0, 0, eps);

    float dx = IntersectSphere(z1, sphere) - IntersectSphere(z2, sphere);
    float dy = IntersectSphere(z3, sphere) - IntersectSphere(z4, sphere);
    float dz = IntersectSphere(z5, sphere) - IntersectSphere(z6, sphere);

    return normalize(float3(dx, dy, dz) / (2.0*eps));
}

float3 RayMarch(float3 ray_pos, float3 ray_dir, out int chosen_sphere) {
    float3 point = ray_pos;

    float dist = 10000.0;
    while (dist > EPS) {
        dist = 10000.0;

        for (int i = 0; i < spheres.length(); i++) {
            float tmp = IntersectSphere(point, spheres[i]);
            if (tmp < dist) {
                dist = tmp;
                chosen_sphere = i;
            }
        }

        if (dist > 100.0) {
            chosen_sphere = -1;
            break;
        }

        point += dist * ray_dir;
    }

    return point;
}

// Calculate light intensity
float RayTrace(float3 pos, float3 norm) {
    float intensity = 0.0;

    for (int i = 0; i < lights.length(); i++) {
        float3 light_direction = normalize(lights[i].pos - pos);
        intensity += lights[i].intensity * max(0.0, dot(light_direction, norm));
    }

    return intensity;
}

// Calculate color for point considering light sources
float4 CalculateColor(float3 point, int chosen_sphere) {
    float4 color;
    if (chosen_sphere == -1) {
        color = g_bgColor;
    } else {
        color = spheres[chosen_sphere].color;
        float alpha = color[3];
        color *= RayTrace(point, EstimateNormalSphere(point, EPS, spheres[chosen_sphere]));
        color[3] = alpha;
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

    int isectSphere;
    float3 isectPoint = RayMarch(ray_pos, ray_dir, isectSphere);
    fragColor = CalculateColor(isectPoint, isectSphere);
}

