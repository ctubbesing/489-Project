#version 120
varying vec3 vPos; // in camera space
varying vec3 vNor; // in camera space
varying vec3 vNor_; // in object space
uniform vec3 lightPos; // in camera space
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

void main()
{
	vec3 n = normalize(vNor);
	vec3 l = normalize(lightPos - vPos);
	vec3 v = -normalize(vPos);
	vec3 h = normalize(l + v);

	//*
	vec3 ka_;
	vec3 kd_;
	vec3 norm_vNor = normalize(vNor_);
	if (norm_vNor.y < 0.8) {
		ka_ = vec3(0.1f, 0.1f, 0.1f);
		kd_ = vec3(0.4f, 0.4f, 0.4f);
	}
	else {
		ka_ = ka;
		kd_ = kd;
	}
	//*/


	vec3 colorA = ka_;
	vec3 colorD = max(dot(l, n), 0.0) * kd_;
	//vec3 colorA = ka;
	//vec3 colorD = max(dot(l, n), 0.0) * kd;
	vec3 colorS = pow(max(dot(h, n), 0.0), s) * ks;
	vec3 color = colorA + colorD + colorS;
	//gl_FragColor = vec4(0,0,1, 1.0);
	
	gl_FragColor = vec4(color.r, color.g, color.b, 1.0);







//	vec3 n = normalize(vNor);
//	vec3 color = 0.5 * (n + 1.0);
//	gl_FragColor = vec4(color.r, color.g, color.b, 1.0);
}
