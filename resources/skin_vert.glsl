#version 120

attribute vec4 aPos;
attribute vec3 aNor;
attribute vec2 aTex;

attribute vec4 weights0;
attribute vec4 weights1;
attribute vec4 weights2;
attribute vec4 bones0;
attribute vec4 bones1;
attribute vec4 bones2;
attribute float numInfl;

uniform mat4 P;
uniform mat4 MV;
uniform mat3 T;

uniform mat4 M[82];

varying vec3 vPos;
varying vec3 vNor;
varying vec2 vTex;

void main()
{
	vec4 posCam = MV * aPos;
	vec3 norCam = (MV * vec4(aNor, 0.0)).xyz;
	gl_Position = P * posCam;
	vPos = posCam.xyz;
	vNor = norCam;
	vTex = vec2(T * vec3(aTex, 1.0));
}
