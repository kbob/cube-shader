vec3 cube_map_to_3d(vec2 pos) {
    vec3 p = vec3(0);
    if (pos.x < 128.0) {
        // top
        p = vec3(1.0 - pos.y / 128.0,
                 1.0,
                 pos.x / 128.0);
    } else if (pos.x < 256.0) {
        // back
        p = vec3(1.0 - pos.y / 128.0,
                 1.0 - (pos.x - 128.0) / 128.0,
                 1.0);
    } else if (pos.x < 384.0) {
        // bottom
        p = vec3(1.0 - pos.y / 128.0,
                 0.0,
                 1.0 - (pos.x - 256.0) / 128.0);
    } else if (pos.x < 512.0) {
        // right
        p = vec3(1.0,
                 1.0 - pos.y / 128.0,
                 1.0 - (pos.x - 384.0) / 128.0);
    } else if (pos.x < 640.0) {
        // front
        p = vec3(1.0 - (pos.x - 512.0) / 128.0,
                 1.0 - pos.y / 128.0,
                 0.0);
    } else if (pos.x < 768.0) {
        // left
        p = vec3(0,
                 1.0 - pos.y / 128.0,
                 (pos.x - 640.0) / 128.0);
    }
    return p - 0.5;
}

void mainCube(out vec4 fragColor, in vec3 fragCoord) {
     fragColor.rgb = fragCoord.xyz + .5;
}

#ifndef _EMULATOR
void mainImage(out vec4 fragColor, in vec2 fragCoord) {
     mainCube(fragColor, cube_map_to_3d(fragCoord));
}
#endif
