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
uniform samplerCube skybox;

uniform float4 g_bgColor = float4(0.5, 0.5, 0.5, 1.0);

const float EPS = 1e-2;

// Settings
uniform bool g_softShadows;
uniform bool g_reflect;
uniform bool g_refract;
uniform bool g_ambient;
uniform bool g_antiAlias;


// Materials

struct Material {
    float4 color;
    float4 albedo;
    float exponent;
    float refraction_index;
};

const Material ivory = Material(
    float4(0.4, 0.4, 0.4, 1.0),
    float4(0.6, 0.3, 0.1, 0.0),
    50,
    1.0
);

const Material red_rubber = Material(
    float4(0.3, 0.1, 0.1, 1.0),
    float4(0.9, 0.1, 0.0, 0.0),
    10,
    1.0
);

const Material gold = Material(
    4 * float4(0.24725, 0.2245, 0.0645, 1.0),
    float4(0.3, 0.8, 0.2, 0.0),
    83.2,
    1.0
);

const Material mirror = Material(
    float4(1.0, 1.0, 1.0, 1.0),
    float4(0.0, 10.0, 0.8, 0.0),
    1425.0,
    1.0
);

const Material glass = Material(
    float4(0.6, 0.7, 0.8, 1.0),
    float4(0.01, 0.5, 0.1, 0.8),
    125.0,
    1.5
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
  float3 d = abs(pos - box.center) - box.size;
  return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
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

struct MSponge {
    float3 center;
    float3 size;

    Material material;
};

float IntersectMSponge(float3 pos, MSponge msponge) {
   pos -= msponge.center;
   float d = IntersectBox(pos, Box(vec3(0.0), msponge.size, gold));

   float s = 1.0;
   for(int m = 0; m < 3; m++)
   {
      vec3 a = mod(pos * s, 2.0) - 1.0;
      s *= 3.0;
      vec3 r = abs(1.0 - 3.0 * abs(a));

      float da = max(r.x, r.y);
      float db = max(r.y, r.z);
      float dc = max(r.z, r.x);
      float c = (min(da, min(db, dc)) - 1.0) / s;

      d = max(d, c);
   }

   return d;
}

float3 EstimateNormalMSponge(float3 z, float eps, MSponge msponge) {
    float3 z1 = z + float3(eps, 0, 0);
    float3 z2 = z - float3(eps, 0, 0);
    float3 z3 = z + float3(0, eps, 0);
    float3 z4 = z - float3(0, eps, 0);
    float3 z5 = z + float3(0, 0, eps);
    float3 z6 = z - float3(0, 0, eps);

    float dx = IntersectMSponge(z1, msponge) - IntersectMSponge(z2, msponge);
    float dy = IntersectMSponge(z3, msponge) - IntersectMSponge(z4, msponge);
    float dz = IntersectMSponge(z5, msponge) - IntersectMSponge(z6, msponge);

    return normalize(float3(dx, dy, dz) / (2.0*eps));
}


// Scene layout

float time_mod = g_time / 5.0;

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

        glass
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

MSponge sponges[] = MSponge[](
    MSponge(
        float3(20.0, 0.0, 0.0),
        float3(4.0 + 2 * cos(time_mod / 2), 4.0 + 2 * sin(time_mod / 4), 2.0),

        gold
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
const int MSPONGE = 3;
const float MAX_DIST = 1000.0;
// Find first ray intersection with object
// Return intersection point, type and index of object
float GetMinimalDistance(float3 point, out int type, out int index) {

    float dist = MAX_DIST;
    type = -1;
    // Check spheres
    for (int i = 0; i < spheres.length(); i++) {
        float tmp = abs(IntersectSphere(point, spheres[i]));
        if (tmp < dist) {
            dist = tmp;
            type = SPHERE;
            index = i;
        }
    }

    // Check boxes
    for (int i = 0; i < boxes.length(); i++) {
        float tmp = abs(IntersectBox(point, boxes[i]));
        if (tmp < dist) {
            dist = tmp;
            type = BOX;
            index = i;
        }
    }

    // Check toruses
    for (int i = 0; i < toruses.length(); i++) {
        float tmp = abs(IntersectTorus(point, toruses[i]));
        if (tmp < dist) {
            dist = tmp;
            type = TORUS;
            index = i;
        }
    }

    // Check sponges
    for (int i = 0; i < sponges.length(); i++) {
        float tmp = abs(IntersectMSponge(point, sponges[i]));
        if (tmp < dist) {
            dist = tmp;
            type = MSPONGE;
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
        case MSPONGE:
            norm = EstimateNormalMSponge(point, EPS, sponges[object_index]);
            material = sponges[object_index].material;
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
        if (g_softShadows) {
            shadow_coef = min(shadow_coef, k * min_dist / step);
        }
        step += min_dist;
    }

    return shadow_coef;
}

float4 CalculateBackground(float3 ray_dir) {
    return texture(skybox, -ray_dir);
}

float AmbientOcclusion(float3 point, float3 norm) {
    int type, index;
    if (g_ambient) {
        return GetMinimalDistance(point + 0.5 * norm, type, index) / 0.5;
    } else {
        return 1.0;
    }
}

// Calculate color for point considering light sources
float4 CalculateColor(float3 ray_dir, float3 point) {
    float3 norm;
    Material material;
    float ref_modifier = 1.0;
    float4 color = float4(0.0, 0.0, 0.0, 1.0);
    for (int depth = 0; depth < 4; depth++) {
        float3 ref_point;
        bool isForeground = GetIntersectionParameters(point, ray_dir, ref_point, norm, material);
        if (!isForeground) {
            color += ref_modifier * CalculateBackground(ray_dir);
            break;
        } else {
            point = ref_point;
        }

        float4 albedo = material.albedo;
        float intensity = AmbientOcclusion(point, norm);
        float specularity = 0.0;
        for (int i = 0; i < lights.length(); i++) {
            float light_distance = length(lights[i].pos - point);
            float3 light_direction = normalize(lights[i].pos - point);
            float shadow_coef = GetShadowCoefficient(dot(light_direction, norm) < 0 ? point - 2 * EPS * norm : point + 2 * EPS * norm, light_direction);

            intensity += shadow_coef * lights[i].intensity * max(0.0, dot(light_direction, norm));
            specularity += shadow_coef * lights[i].intensity * pow(max(0.0, -dot(reflect(-light_direction, norm), ray_dir)), material.exponent);
        }


        color += ref_modifier * (material.color * intensity * albedo[0] + float4(1.0, 1.0, 1.0, 0.0) * specularity * albedo[1]);

        float3 ref_dir;
        float3 ref_start;
        if (albedo[3] == 0.0) {
            if (!g_reflect) {
                break;
            }
            ref_dir = reflect(ray_dir, norm);
            ref_modifier *= albedo[2];
        } else {
            if (!g_refract) {
                break;
            }
            if (dot(ray_dir, norm) < 0) {
                ref_dir = refract(ray_dir, norm, 1.0 / material.refraction_index);
            } else {
                ref_dir = refract(ray_dir, -norm, material.refraction_index);
            }
            if (ref_dir == float3(0.0)) {
                if (!g_reflect) {
                    break;
                }
                ref_dir = reflect(ray_dir, norm);
                ref_modifier *= albedo[2];
            } else {
                ref_modifier *= albedo[3];
            }
        }
        ref_start = dot(ref_dir, norm) < 0 ? point - 2 * EPS * norm : point + 2 * EPS * norm;

        ray_dir = normalize(ref_dir);
        point = ref_start;
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

    float3 ray_pos = (g_rayMatrix * float4(0.0, 0.0, 0.0, 1.0)).xyz;
    float3x3 rot_mat = float3x3(g_rayMatrix);

    if (g_antiAlias) {
        float3 ray_dir0 = rot_mat * EyeRayDir(x - 0.25, y - 0.25, w, h);
        float3 ray_dir1 = rot_mat * EyeRayDir(x + 0.25, y - 0.25, w, h);
        float3 ray_dir2 = rot_mat * EyeRayDir(x - 0.25, y + 0.25, w, h);
        float3 ray_dir3 = rot_mat * EyeRayDir(x + 0.25, y + 0.25, w, h);

        fragColor = (CalculateColor(ray_dir0, ray_pos) +
                     CalculateColor(ray_dir1, ray_pos) +
                     CalculateColor(ray_dir2, ray_pos) +
                     CalculateColor(ray_dir3, ray_pos)) / 4.0;
    } else {
        float3 ray_dir = rot_mat * EyeRayDir(x, y, w, h);

        fragColor = CalculateColor(ray_dir, ray_pos);
    }

}

