attribute vec3 aVertexPosition;
attribute mat3 aPrecomputeLT;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

uniform mat3 uPrecomputeLR;
uniform mat3 uPrecomputeLG;
uniform mat3 uPrecomputeLB;

varying highp vec3 vColor;

void main() {
    gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aVertexPosition, 1.0);

    vColor = 
    // (2.0/3.1415926)*
    vec3(dot(uPrecomputeLR[0], aPrecomputeLT[0]) + dot(uPrecomputeLR[1], aPrecomputeLT[1]) + dot(uPrecomputeLR[2], aPrecomputeLT[2]), dot(uPrecomputeLG[0], aPrecomputeLT[0]) + dot(uPrecomputeLG[1], aPrecomputeLT[1]) + dot(uPrecomputeLG[2], aPrecomputeLT[2]), dot(uPrecomputeLB[0], aPrecomputeLT[0]) + dot(uPrecomputeLB[1], aPrecomputeLT[1]) + dot(uPrecomputeLB[2], aPrecomputeLT[2]));
}