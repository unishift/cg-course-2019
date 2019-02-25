#version 330

#define float2 vec2
#define float3 vec3
#define float4 vec4
#define float4x4 mat4
#define float3x3 mat3

struct LightSource {
    float3 pos;
    float intensity;
};

in float2 fragmentTexCoord;

layout(location = 0) out vec4 fragColor;

uniform int g_screenWidth;
uniform int g_screenHeight;

uniform float4x4 g_rayMatrix;

uniform int g_time;

uniform float4 g_bgColor = float4(0.5, 0.5, 0.5, 1.0);

const float EPS = 1e-2;


// Materials

struct Material {
    float4 color;
    float3 albedo;
    float exponent;
};

const Material ivory = Material(
    float4(0.4, 0.4, 0.4, 1.0),
    float3(0.6, 0.3, 0.1),
    50
);

const Material red_rubber = Material(
    float4(0.3, 0.1, 0.1, 1.0),
    float3(0.9, 0.1, 0.0),
    10
);

const Material gold = Material(
    4 * float4(0.24725, 0.2245, 0.0645, 1.0),
    float3(0.3, 0.8, 0.2),
    83.2
);

const Material mirror = Material(
    float4(1.0, 1.0, 1.0, 1.0),
    float3(0.0, 10.0, 0.8),
    1425.0
);


// Primitives

struct Sphere {
    float3 center;
    float r;

    Material material;
};

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

struct Box {
    float3 center;
    float3 size;

    Material material;
};

float IntersectBox(float3 pos, Box box) {
    return length(max(abs(pos - box.center) - box.size, 0.0));
}

float3 EstimateNormalBox(float3 z, float eps, Box box) {
    float3 z1 = z + float3(eps, 0, 0);
    float3 z2 = z - float3(eps, 0, 0);
    float3 z3 = z + float3(0, eps, 0);
    float3 z4 = z - float3(0, eps, 0);
    float3 z5 = z + float3(0, 0, eps);
    float3 z6 = z - float3(0, 0, eps);

    float dx = IntersectBox(z1, box) - IntersectBox(z2, box);
    float dy = IntersectBox(z3, box) - IntersectBox(z4, box);
    float dz = IntersectBox(z5, box) - IntersectBox(z6, box);

    return normalize(float3(dx, dy, dz) / (2.0*eps));
}

struct Torus {
    float3 center;
    float2 size;

    Material material;
};

float IntersectTorus(float3 pos, Torus torus) {
    float2 q = float2(length(pos.xz - torus.center.xz) - torus.size.x, pos.y- torus.center.y);
    return length(q)-torus.size.y;
}

float3 EstimateNormalTorus(float3 z, float eps, Torus torus) {
    float3 z1 = z + float3(eps, 0, 0);
    float3 z2 = z - float3(eps, 0, 0);
    float3 z3 = z + float3(0, eps, 0);
    float3 z4 = z - float3(0, eps, 0);
    float3 z5 = z + float3(0, 0, eps);
    float3 z6 = z - float3(0, 0, eps);

    float dx = IntersectTorus(z1, torus) - IntersectTorus(z2, torus);
    float dy = IntersectTorus(z3, torus) - IntersectTorus(z4, torus);
    float dz = IntersectTorus(z5, torus) - IntersectTorus(z6, torus);

    return normalize(float3(dx, dy, dz) / (2.0*eps));
}


// Scene layout

float time_mod = float(g_time - 50) / 50 * 3.14159265 * 4;

LightSource lights[] = LightSource[](
    LightSource(
        float3(0.0, 4.0, 10.0),
        1.0 + sin(time_mod / 4)
    ),
    LightSource(
        float3(10.0, 20.0, 1.0),
        1.0
    )
);

Sphere spheres[] = Sphere[](
    Sphere(
        float3(0.0, cos(time_mod / 2), 0.0),
        3.0,

        gold
    ),
    Sphere(
//        float3(4.0, 5.0, 1.5),
        6.5765 * float3(0.707 * cos(time_mod / 4), 0.707 * cos(time_mod / 4), -2.5 * sin(time_mod / 4)),
        2.0,

        mirror
    )
);

uniform Box boxes[] = Box[](
    Box(
        float3(8.0, 5.0, -10.0),
        float3(3.0, 1.0, 1.0),

        red_rubber
    ),
    Box(
        float3(-8.0, 5.0, -10.0),
        float3(1.0, 3.0, 1.0),

        gold
    ),
    Box(
        float3(0.0, -10.0, 0.0),
        float3(30.0, 0.1, 30.0),

        ivory
    )
);

Torus toruses[] = Torus[](
    Torus(
        float3(0.0, cos(time_mod / 2 - 0.4), 0.0),
        float2(10.0, 0.4),

        red_rubber
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

const int SPHERE = 0;
const int BOX = 1;
const int TORUS = 2;
const float MAX_DIST = 1000.0;
// Find first ray intersection with object
// Return intersection point, type and index of object
float GetMinimalDistance(float3 point, out int type, out int index) {

    float dist = MAX_DIST;
    type = -1;
    // Check spheres
    for (int i = 0; i < spheres.length(); i++) {
        float tmp = IntersectSphere(point, spheres[i]);
        if (tmp < dist) {
            dist = tmp;
            type = SPHERE;
            index = i;
        }
    }

    // Check boxes
    for (int i = 0; i < boxes.length(); i++) {
        float tmp = IntersectBox(point, boxes[i]);
        if (tmp < dist) {
            dist = tmp;
            type = BOX;
            index = i;
        }
    }

    // Check toruses
    for (int i = 0; i < toruses.length(); i++) {
        float tmp = IntersectTorus(point, toruses[i]);
        if (tmp < dist) {
            dist = tmp;
            type = TORUS;
            index = i;
        }
    }

    return dist;
}

// Return position, norm to object and material of object
bool GetIntersectionParameters(float3 ray_pos, float3 ray_dir, out float3 point, out float3 norm, out Material material) {
    int object_type;
    int object_index;
    float step = 0.0;
    while (true) {
        float min_dist = GetMinimalDistance(ray_pos + step * ray_dir, object_type, object_index);
        if (min_dist < EPS) {
            break;
        }

        step += min_dist;

        if (step > MAX_DIST || object_type == -1) {
            object_type = -1;
            break;
        }
    }
    point = ray_pos + step * ray_dir;

    switch (object_type) {
        case SPHERE:
            norm = EstimateNormalSphere(point, EPS, spheres[object_index]);
            material = spheres[object_index].material;
            break;
        case BOX:
            norm = EstimateNormalBox(point, EPS, boxes[object_index]);
            material = boxes[object_index].material;
            break;
        case TORUS:
            norm = EstimateNormalTorus(point, EPS, toruses[object_index]);
            material = toruses[object_index].material;
            break;
        case -1:
            return false;
    }

    return true;
}

float GetShadowCoefficient(float3 ray_pos, float3 ray_dir) {
    int object_type;
    int object_index;

    float step = 0.0;
    float shadow_coef = 1.0;
    while (step < MAX_DIST) {
        float min_dist = GetMinimalDistance(ray_pos + step * ray_dir, object_type, object_index);
        if (min_dist < EPS) {
            return 0.0;
        }

        const float k = 16;
        shadow_coef = min(shadow_coef, k * min_dist / step);
        step += min_dist;
    }

    return shadow_coef;
}

float4 CalculateBackground(float3 ray_dir) {
    return g_bgColor;
}

// Calculate color for point considering light sources
float4 CalculateColor(float3 ray_dir, float3 point, float3 norm, Material material) {
    float ref_modifier = 1.0;
    float4 color = float4(0.0, 0.0, 0.0, 1.0);
    for (int depth = 0; depth < 2; depth++) {
        float3 albedo = material.albedo;

        float intensity = 0.0;
        float specularity = 0.0;
        for (int i = 0; i < lights.length(); i++) {
            float light_distance = length(lights[i].pos - point);
            float3 light_direction = normalize(lights[i].pos - point);
            float shadow_coef = GetShadowCoefficient(dot(light_direction, norm) < 0 ? point - 2 * EPS * norm : point + 2 * EPS * norm, light_direction);

            intensity += shadow_coef * lights[i].intensity * max(0.0, dot(light_direction, norm));
            specularity += shadow_coef * lights[i].intensity * pow(max(0.0, -dot(reflect(-light_direction, norm), ray_dir)), material.exponent);
        }


        color += ref_modifier * (material.color * intensity * albedo[0] + float4(1.0, 1.0, 1.0, 0.0) * specularity * albedo[1]);
        float3 ref_dir = reflect(ray_dir, norm);
        float3 ref_start = dot(ref_dir, norm) < 0 ? point - 2 * EPS * norm : point + 2 * EPS * norm;

        float3 ref_pos;
        float3 ref_norm;
        Material ref_material;
        bool isForeground = GetIntersectionParameters(ref_start, ref_dir, ref_pos, ref_norm, ref_material);
        ref_modifier *= albedo[2];
        if (!isForeground) {
            color += ref_modifier * CalculateBackground(ref_dir);
            break;
        } else {
            ray_dir = ref_dir;
            point = ref_pos;
            norm = ref_norm;
            material = ref_material;
        }
    }

    color[3] = 1.0;
    return color;
}

void main(void)
{
    float w = float(g_screenWidth);
    float h = float(g_screenHeight);

    // get curr pixelcoordinates
    float x = fragmentTexCoord.x*w;
    float y = fragmentTexCoord.y*h;

    // generate initial ray
    float3 ray_pos = float3(0,0,0);
    float3 ray_dir = EyeRayDir(x,y,w,h);

    // transorm ray with matrix
    ray_pos = (g_rayMatrix*float4(ray_pos,1)).xyz;
    ray_dir = float3x3(g_rayMatrix)*ray_dir;

    float3 isectPoint;
    float3 isectNorm;
    Material isectMaterial;
    bool isForeground = GetIntersectionParameters(ray_pos, ray_dir, isectPoint, isectNorm, isectMaterial);
    if (!isForeground) {
        fragColor = CalculateBackground(ray_dir);
    } else {
        fragColor = CalculateColor(ray_dir, isectPoint, isectNorm, isectMaterial);
    }
}

