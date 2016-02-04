#version 150

uniform sampler2DRect texA, texB;
uniform float blend;
uniform vec2 size;

in vec2 texCoordVarying;

out vec4 fragColor;

void main(){
    fragColor = mix(texture(texA, texCoordVarying * size), texture(texB, texCoordVarying * size), blend);
//    fragColor = vec4(texCoordVarying.x, texCoordVarying.y, 0, 1);
}
