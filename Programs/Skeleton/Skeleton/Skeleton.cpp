//=============================================================================================
// Orbifold vizualizáció
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Juhasz Benedek Laszlo
// Neptun : C8B5CT
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

vec3 const NODES[20] = {
	{0,			0.618034,	1.61803},
	{0,			-0.618034,	1.61803},
	{0,			-0.618034,	-1.61803},
	{0,			0.618034,	-1.61803},
	{1.61803,	0,			0.618034},
	{-1.61803,	0,			0.618034},
	{-1.61803,	0,			-0.618034},
	{1.61803,	0,			-0.618034},
	{0.618034,	1.61803,	0},
	{-0.618034, 1.61803,	0},
	{-0.618034,	-1.61803,	0},
	{0.618034,	-1.61803,	0},
	{1,			1,			1},
	{-1,		1,			1},
	{-1,		-1,			1},
	{1,			-1,			1},
	{1,			-1,			-1},
	{1,			1,			-1},
	{-1,		1,			-1},
	{-1,		-1,			-1},
};

int const SIDES[12][5] = {
	{1,		2,		16,		5,		13},
	{1,		13,		9,		10,		14},
	{1,		14,		6,		15,		2},
	{2,		15,		11,		12,		16},
	{3,		4,		18,		8,		17},
	{3,		17,		12,		11,		20},
	{3,		20,		7,		19,		4},
	{19,	10,		9,		18,		4},
	{16,	12,		17,		8,		5},
	{5,		8,		18,		9,		13},
	{14,	10,		19,		7,		6},
	{6,		7,		20,		11,		15},
};

struct Material {
	vec3 ka, kd, ks, n, kappa, f0;
	float shine;
	int rough;

	Material(vec3 ka, vec3 kd, vec3 ks, vec3 n, vec3 kappa, float shine, int rough)
		: ka(ka), kd(kd), ks(ks), n(n), kappa(kappa), shine(shine), rough(rough) {
		if (rough == 0) {
			vec3 nminus = n - vec3(1.0f, 1.0f, 1.0f);
			vec3 nplus = n + vec3(1.0f, 1.0f, 1.0f);
			vec3 kappasquare = kappa * kappa;
			vec3 numerator = nminus * nminus + kappasquare;
			vec3 denominator = nplus * nplus + kappasquare;
			f0 = vec3(
				numerator.x / denominator.x,
				numerator.y / denominator.y,
				numerator.z / denominator.z
			);
		}
	}
};

vec3 rotateZ(vec3 v, float angle) {
	return vec3(
		v.x * cos(angle) - v.y * sin(angle),
		v.x * sin(angle) + v.y * cos(angle),
		v.z
	);
}

struct Camera {
	vec3 eye, lookat, right, up;
	float fov;

	void set(vec3 _eye, vec3 _lookat, vec3 vup, float _fov) {
		eye = _eye;
		lookat = _lookat;
		fov = _fov;
		vec3 w = eye - lookat;
		float f = length(w);
		right = normalize(cross(vup, w)) * f * tanf(fov / 2);
		up = normalize(cross(w, right)) * f * tanf(fov / 2);
	}

	Camera(vec3 eye, vec3 lookat, vec3 up, float fov) {
		set(eye, lookat, up, fov);
	}

	void animate(float dt) {
		eye = rotateZ(eye, dt);
		lookat = rotateZ(lookat, dt);
		right = rotateZ(right, dt);
		up = rotateZ(up, dt);
	}
};

struct Light {
	vec3 pos;
	vec3 Le;
};

struct Pentagon {
	vec3 nodes[5];
	vec3 norm;
	vec3 center;

	void set(int i) {
		center = vec3(0.0f, 0.0f, 0.0f);
		for (int j = 0; j != 5; j++) {
			nodes[j] = NODES[SIDES[i][j] - 1];
			center = center + nodes[j] / 5.0f;
		}
		norm = normalize(cross(nodes[1] - nodes[0], nodes[2] - nodes[1]));
	}
};

struct OminousThing {
	float a, b, c, r;
	vec3 center;
};

struct FullScreenTexturedQuad {
	unsigned int vao = 0;

	void create() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		unsigned int vbo;
		glGenBuffers(1, &vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		float vertexCoords[] = { -1, -1,  1, -1,  1, 1,  -1, 1 };
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), vertexCoords, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	void draw() {
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}
};

Pentagon sides[12];
OminousThing o = OminousThing{
	1.0f,
	1.0f,
	0.1f,
	0.3f,
	vec3(0.0f, 0.0f, 0.0f)
};
Material materials[2] = {
	Material(
	vec3(0.8f, 0.8f, 0.6f),
	vec3(0.8f, 0.8f, 0.6f),
	vec3(0.8f, 0.8f, 0.6f),
	vec3(0.0f, 0.0f, 0.0f),
	vec3(0.0f, 0.0f, 0.0f),
	0.9f,
	1),
	Material(
	vec3(1.0f, 1.0f, 1.0f),
	vec3(1.0f, 1.0f, 1.0f),
	vec3(1.0f, 1.0f, 1.0f),
	vec3(0.17f, 0.35f, 1.5f),
	vec3(3.1f, 2.7f, 1.9f),
	0.0f,
	0
	)
};
Camera camera = Camera(
	vec3(1.0f, 0.0f, 0.5f),
	vec3(0.0f, 0.0f, 0.0f),
	vec3(0.0f, 0.0f, 1.0f),
	M_PI / 2.0f
);
Light light = {
	vec3(0.7f, 0.7f, 0.7f),
	vec3(20.0f, 20.0f, 20.0f)
};
vec3 La = vec3(0.1f, 0.1f, 0.5f);

GPUProgram gpuProgram;
FullScreenTexturedQuad fq;

/////////////////////////////////////////////////////////////////

const char* vertexSource = R"(
	#version 330
    precision highp float;

	uniform vec3 lookat, right, up;

	layout(location = 0) in vec2 c;
	out vec3 p;

	void main() {
		gl_Position = vec4(c, 0, 1);
		p = lookat + right * c.x + up * c.y;
	}
)";

const char* const fragmentSource = R"(
#version 330
precision highp float;

struct Material {
		vec3 ka, kd, ks, f0;
		float shine;
		int rough;
};

struct Pentagon {
		vec3 nodes[5];
		vec3 norm;
		vec3 center;
};

struct Ray {
	vec3 start;
	vec3 dir;
};
  
struct Hit {
	float t;
	vec3 pos, norm;
	Material material;
	bool isPortal;
	vec3 portalCenter;
};

struct Light {
	vec3 pos;
	vec3 Le;
};

struct OminousThing {
	float a, b, c, r;
	vec3 center;
};

uniform vec3 eye;
uniform Material materials[2];
uniform vec3 La;
uniform Light light;
uniform OminousThing o;
uniform Pentagon sides[12];
  
const float edgeWidth = 0.1;
const int  maxDepth = 5;
const float eps = 0.0001;
const float M_PI = 3.141592653;

	
in vec3 p;
out vec4 fragmentColor;

vec3 Rodriguez(vec3 d, vec3 r, float angle) {
	return r * cos(angle) + d * dot(r, d) * (1.0 - cos(angle)) + cross(d, r) * sin(angle);
}

Hit intersectPentagon(Pentagon p, Ray ray) {
	Hit hit;
	hit.t = -1.0;
	float t = dot(p.nodes[0] - ray.start, p.norm) / dot(ray.dir, p.norm);
	if (t < 0.0)
		return hit;
	vec3 pos = ray.start + t * ray.dir;
	bool portal = true;
	for (int j = 0; j != 5; j++) {
		vec3 a = p.nodes[j];
		vec3 b = j == 4 ? p.nodes[0] : p.nodes[j + 1];
		vec3 crossV = cross(b - a, pos - a);
		if (!(dot(crossV, p.norm) >= 0.0))
			return hit;
		if (portal)
			if (length(crossV) < edgeWidth * length(b - a))
				portal = false;
	}
	hit.t = t;
	hit.pos = pos;
	hit.norm = p.norm;
	hit.material = materials[0];
	hit.isPortal = portal;
	if (portal)
		hit.portalCenter = p.center;
	return hit;
}

vec3 Fresnel(vec3 L, vec3 N, vec3 f0) {
	float onemincosalpha = 1.0 + dot(L, N);
	return f0 + (vec3(1.0, 1.0, 1.0) - f0) * pow(onemincosalpha,5.0);
}

vec2 quadratic(float a, float b, float c) {
	if (a < eps) {
		if (b < eps)
			return vec2(-1.0, -1.0);
		else
			return vec2(-c / b, -1.0);
	}
	float discr = b * b - 4.0 * a * c;
	if (discr < 0.0)
		return vec2(-1.0, -1.0);
	float root = sqrt(discr);
	float denominator = 2.0 * a;
	float x1 = (-b - root) / denominator;
	float x2 = (-b + root) / denominator;
	if (x1 < x2)
		return vec2(x1, x2);
	else
		return vec2(x2, x1);
}

Ray getPortal(Hit hit, Ray ray) {
	vec3 norm = hit.norm;
	vec3 center = hit.portalCenter;

	vec3 reflectDir = reflect(ray.dir, norm);

	return Ray(
		center + Rodriguez(norm, hit.pos - center, 0.4 * M_PI) + norm * eps,
		Rodriguez(norm, reflectDir, 0.4 * M_PI)
	);
}

Hit intersectOminousThing(Ray ray) {
	Hit hit;
	hit.t = -1.0;

	vec3 s = ray.start;
	vec3 d = ray.dir;
	vec2 t = quadratic(
		o.a * d.x * d.x + o.b * d.y * d.y,
		2.0 * (o.a * s.x * d.x + o.b * s.y * d.y) - o.c * d.z,
		o.a * s.x * s.x + o.b * s.y * s.y - o.c * s.z
	);
	vec3 posx;
	vec3 posy;
	if (t.x < 0.0 || length((posx = s + t.x * d) - o.center) > o.r) {
		if (t.y < 0.0 || length((posy = s + t.y * d) - o.center) > o.r) {
			return hit;
		}
		else {
			hit.t = t.y;
			hit.pos = posy;
		}
	}
	else {
		hit.t = t.x;
		hit.pos = posx;
	}

	hit.norm = normalize(vec3(
		o.a * 2.0 * hit.pos.x,
		o.b * 2.0 * hit.pos.y,
		-o.c
	));
	hit.material = materials[1];
	hit.isPortal = false;
	return hit;
}

Hit firstIntersect(Ray ray) {
	Hit bestHit;
	bestHit.t = -1.0;
	bestHit.isPortal = false;
	for (int i = 0; i != 12; i++) {
		Hit hit = intersectPentagon(sides[i], ray);
		if (hit.t > 0.0)
			if (bestHit.t < 0.0 || hit.t < bestHit.t)
				bestHit = hit;
	}
	Hit ominousHit = intersectOminousThing(ray);
	if (ominousHit.t > 0.0)
		if (bestHit.t < 0.0 || ominousHit.t < bestHit.t)
			bestHit = ominousHit;
	if (dot(ray.dir, bestHit.norm) > 0.0)
		bestHit.norm *= -1.0;
	bestHit.pos += eps * bestHit.norm;
	return bestHit;
}

bool shadowIntersect(Ray ray, float dist) {
	for (int i = 0; i != 12; i++) {
		Hit hit = intersectPentagon(sides[i], ray);
		if (hit.t > 0.0 && hit.t < dist)
			return true;
	}
	Hit ominousHit = intersectOminousThing(ray);
	if (ominousHit.t > 0.0 && ominousHit.t < dist)
		return true;
	return false;
}

vec3 trace(Ray ray) {
	vec3 weight = vec3(1.0, 1.0, 1.0);
	vec3 outRad = vec3(0.0, 0.0, 0.0);
	float allDist = 0.0;
	Hit hit;
	for (int d = 0; d < maxDepth + 1; d++) {
		hit = firstIntersect(ray);
		if (hit.t < 0.0)
			return weight * La;
		allDist += hit.t;
		
		if (hit.isPortal) {
			allDist += hit.t;
			ray = getPortal(hit, ray);
		}
		else if (hit.material.rough == 0) {
			weight *= Fresnel(ray.dir, hit.norm, hit.material.f0);
			vec3 reflectionDir = reflect(ray.dir, hit.norm);
			ray = Ray( hit.pos, reflectionDir );
		}
		else{
			outRad += weight * hit.material.ka * La;
			vec3 dirLight = normalize(light.pos - hit.pos);
			float lengthLight = length(light.pos - hit.pos);
			Ray shadowRay = Ray( hit.pos, dirLight );
			float cosTheta = dot(dirLight, hit.norm);
			if (cosTheta > 0.0 && !shadowIntersect(shadowRay, lengthLight)) {
				outRad += weight * light.Le * hit.material.kd * cosTheta / (allDist + lengthLight) / (allDist + lengthLight);
				vec3 LplusV = shadowRay.dir - ray.dir;
				vec3 halfway = normalize(LplusV);
				float cosDelta = dot(LplusV, hit.norm);
				if (cosDelta > 0.0)
					outRad += weight * light.Le * hit.material.ks * pow(cosDelta, hit.material.shine) * cosTheta / dot(LplusV, LplusV) / (allDist + lengthLight) / (allDist + lengthLight);
			}
			return outRad;
		}
	}
	if (hit.isPortal)
		outRad += weight * La;
	return outRad;
}

void main(){
	Ray ray = Ray(eye, normalize(p - eye));
	fragmentColor = vec4(trace(ray), 1); 
}
)";

void setUniforms() {
	gpuProgram.setUniform(camera.eye, "eye");
	gpuProgram.setUniform(La, "La");
	gpuProgram.setUniform(camera.lookat, "lookat");
	gpuProgram.setUniform(camera.right, "right");
	gpuProgram.setUniform(camera.up, "up");
	for (int i = 0; i != 2; i++) {
		std::string s = "materials[" + std::to_string(i) + "].";
		gpuProgram.setUniform(materials[i].ka, s + "ka");
		gpuProgram.setUniform(materials[i].kd, s + "kd");
		gpuProgram.setUniform(materials[i].ks, s + "ks");
		gpuProgram.setUniform(materials[i].f0, s + "f0");
		gpuProgram.setUniform(materials[i].shine, s + "shine");
		gpuProgram.setUniform(materials[i].rough, s + "rough");
	}
	for (int i = 0; i != 12; i++) {
		std::string s = "sides[" + std::to_string(i) + "].";
		gpuProgram.setUniform(sides[i].center, s + "center");
		gpuProgram.setUniform(sides[i].norm, s + "norm");
		for (int j = 0; j != 5; j++)
			gpuProgram.setUniform(sides[i].nodes[j], s + "nodes[" + std::to_string(j) + "]");
	}
	gpuProgram.setUniform(light.Le, "light.Le");
	gpuProgram.setUniform(light.pos, "light.pos");
	gpuProgram.setUniform(o.a, "o.a");
	gpuProgram.setUniform(o.b, "o.b");
	gpuProgram.setUniform(o.c, "o.c");
	gpuProgram.setUniform(o.r, "o.r");
	gpuProgram.setUniform(o.center, "o.center");
}


void onInitialization() {
	for (int i = 0; i != 12; i++)
		sides[i].set(i);

	glViewport(0, 0, windowWidth, windowHeight);
	fq.create();
	gpuProgram.create(vertexSource, fragmentSource, "fragmentColor");

}

void onDisplay() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setUniforms();
	fq.draw();

	glutSwapBuffers();
}

void onKeyboard(unsigned char key, int pX, int pY) {}

void onKeyboardUp(unsigned char key, int pX, int pY) {}

void onMouseMotion(int pX, int pY) {}

void onMouse(int button, int state, int pX, int pY) {}

void onIdle() {
	camera.animate(0.005f);
	glutPostRedisplay();
}