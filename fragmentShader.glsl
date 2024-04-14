#version 410

uniform vec2 screen; // Screen resolution
uniform vec3 camPos; // Camera Position
uniform float seed;

vec2 uv = (gl_FragCoord.xy * 2.0 - screen) / min(screen.x, screen.y);
vec3 finalColor;
vec3 camDir = normalize(vec3(uv, -1.0));
float SDF = 10000;
int SDF_T = 0;
float dotP;
float lerp(float a, float b, float t) {
    return a * (1.0 - t) + b * t;
}

vec3 color;

float hash(vec2 p) {
//     float seed = 43758.5343;
//     float seed = 162123.659;
    return fract(sin(dot(p, vec2(12.9898,78.233))) * seed);
}

float noise2D(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);

    // Four corners in 2D of a tile
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    // Smooth interpolation
    float u = f.x * f.x * (3.0 - 2.0 * f.x);
    float v = f.y * f.y * (3.0 - 2.0 * f.y);

    return lerp(a, b, u) + (c - a) * v * (1.0 - u) +(d - b) * u * v;
}

float noise(vec2 p, float magn, float height) {
    return noise2D(p / magn) * height;
}

float terrain(vec2 p) {
    float tempNoise = 0;

    if(noise(p, 150, 1) < 0.2) {
        tempNoise += 13 + noise(p, 25, 15);

        return tempNoise;
    }

    tempNoise += noise(p, 35, 15) * (noise(p, 200, 2.5));
    tempNoise += noise(p, 20, 9);
    tempNoise += noise(p, 10, 4);
    tempNoise += noise(p, 5, 2);
    tempNoise += noise(p, 2, 1);
    tempNoise += noise(p, 0.5, 0.5);

    return tempNoise;
}

vec3 calcColor(vec3 pos) {
    float groundHeight = terrain(vec2(pos.x, pos.z));

    if (pos.y <= groundHeight + 0.2) {
        // pattern
        int checkX = int(floor(pos.x));
        int checkZ = int(floor(pos.z));
        int checker = (checkX + checkZ) % 2;
        if (checker == 0) {
            if(noise(vec2(pos.x, pos.z), 150, 1) < 0.2) {
                if(groundHeight > 15) return vec3(1.0, 0.9, 0.0);
                return vec3(0.0, 1.0, 0.0);
            }
            if(groundHeight > 38.0 - groundHeight / 8.0) return vec3(1.0);
            if(groundHeight > 28.0 - groundHeight / 8.0) return vec3(0.65);
            if(groundHeight > 20.0 - groundHeight / 8.0) return vec3(0.0, 0.6, 0.0);
            if(groundHeight > 17.0 - groundHeight / 8.0) return vec3(0.0, 1.0, 0.0);
            return vec3(1.0, 1.0, 0.0); // lighter
        } else {
            if(noise(vec2(pos.x, pos.z), 150, 1) < 0.2) {
                if(groundHeight > 15) return vec3(0.7, 0.7, 0.0);
                return vec3(0.0, 0.7, 0.0);
            }
            if(groundHeight > 38.0 - groundHeight / 8.0) return vec3(0.7);
            if(groundHeight > 28.0 - groundHeight / 8.0) return vec3(0.3);
            if(groundHeight > 20.0 - groundHeight / 8.0) return vec3(0.0, 0.3, 0.0);
            if(groundHeight > 17.0 - groundHeight / 8.0) return vec3(0.0, 0.5, 0.0);

            return vec3(0.7, 0.7, 0.0); // darker
        }
    } else if(pos.y <= 22) {
        return vec3(0.0, 0.0, 0.7);
    } else {
        return vec3(0.05, 0.3, 0.7 + gl_FragCoord * 0.3); // sky
    }
}

int hit(vec3 pos) {
    if(pos.y <= 14) {return 2;} // hit water, return type 2
    else if (pos.y < terrain(vec2(pos.x, pos.z))) {return 1;} // hit land, return type 1
    else{return 0;} // hit nothing, return 0.
}

void planeSDF(float d, vec3 position) {
    dotP = dot(position, vec3(0.0, 1.0, 0.0)) - d;
    if(dotP < SDF) {
        SDF = dotP;
        SDF_T = 0;
    }
}

void terrainSDF(vec3 position) {
    float tempNoise = terrain(vec2(position.x, position.z));
    if(position.y - tempNoise < SDF) {
        SDF = position.y - tempNoise;
        SDF_T = 1;
    }
}

void sceneSDF(vec3 rayPos) {
    SDF = 100000;
    SDF_T = 0;
    planeSDF(14, rayPos);
    terrainSDF(rayPos);
}

vec3 marchRay(vec3 camPos, vec3 camDir) {
    vec3 rayPos = camPos;
    bool reflections = false;
    SDF = 100000.0;
    float rayLen = 0;
    int i;
    for(i = 1; i < 8000 && rayLen < 700 + ((camPos.y - 10) * 2); i++) {
        sceneSDF(rayPos);
        if(SDF < 0.01) {
            if(SDF_T == 0 || rayPos.y < 14 && !reflections) {
                if(reflections) break;
                camDir = reflect(camDir, vec3(sin(rayPos.x)/180, 1.0, sin(rayPos.z)/180));
                i = 0;
                rayPos.y = 14.1;
                reflections = true;
            } else {
                break;
            }
        }
        rayPos += camDir * SDF;
        rayLen += SDF;
    }
    if(rayLen > 1500 && !reflections) return vec3(0.05, 0.1, 0.7);
    if(rayLen > 1500) SDF_T = 3;
    if(reflections) {color = mix(calcColor(rayPos), vec3(0.0, 0.0, 0.7), 0.4);}
    else {color = calcColor(rayPos);}

    if(SDF_T < 3) {
    return mix(color, vec3(0.4, 0.4, 1.0), rayLen/(1200 + ((camPos.y - 10) * 2)));}
    else {return color;}
}

/*vec3 castRay(vec3 camPos, vec3 camDir) {
    vec3 rayPos = camPos;

    for(int i = 1; i < 225; i++) { // maxiterk
        float stepSize;
        if(i < 150) {
            stepSize = 0.4;
        } else {
            stepSize = 2.5;
        }
        int hitType = hit(rayPos);
        if (hitType == 1) {
            return calcColor(rayPos); // ray hits terrain
        } else if (hitType == 2) {
            vec3 reflectDir = reflect(camDir, vec3(0.0, 1.0, 0.0));
            for(int j = 0; j < 100; j++) {
                rayPos += reflectDir * stepSize;
                int hitType = hit(rayPos);
                if (hitType == 1) {
                    return mix(calcColor(rayPos), vec3(0.0, 0.0, 0.7), 0.3);
                }
                if (hitType == 2) {
                    return vec3(0.0, 0.0, 0.7);
                }
            }
            return vec3(0.0, 0.0, 0.7);
        }
        rayPos += camDir * stepSize; // move along the ray
    }
    return calcColor(rayPos);
}*/


void main() {
//     color = castRay(camPos, camDir);
    color = marchRay(camPos, camDir);
    gl_FragColor = vec4(color, 1.0);
}
