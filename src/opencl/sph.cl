__constant float4 up = (float4)(0.0f, 1.0f, 0.0f, 0.0f);
__constant float4 down = (float4)(0.0f, -1.0f, 0.0f, 0.0f);
__constant float4 left = (float4)(-1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 right = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 forth = (float4)(0.0f, 0.0f, 1.0f, 0.0f);
__constant float4 back = (float4)(0.0f, 0.0f, -1.0f, 0.0f);
__constant float4 gravity = (float4)(0.0f, -9.81f, 0.0f, 0.0f);

typedef struct {
  int id;
  int type;
  float mass;
  float density;
  float viscosity;
} particle;

float4 reflect(float4 d, float4 n) {
  if (dot(d, n) > 0) {
    d = d*-1.0f;
  }
  float4 r = d - 2.0f*dot(d, n)*n;
  return r;
}

__kernel void fakecollision(__global float4* pos,  __global float4* next, __global float4* vel, float dt, float particle_amount) {
  unsigned int id = get_global_id(0);
  float4 old = pos[id];
  float4 newpos = next[id];
  float radius = 0.1f;
  if (dot(vel[id], up) < 0 && next[id].y < -5.0f+radius) {
	  newpos.y = -10.0f+2*radius - next[id].y;
    old.y = -10.0f+2*radius - pos[id].y;
  }
  if (dot(vel[id], down) < 0 && next[id].y > 5.0f-radius) {
	  newpos.y = 10.0f-2*radius - next[id].y;
    old.y = 10.0f-2*radius - pos[id].y;
  }
  if (dot(vel[id], right) < 0 && next[id].x < -5.0f+radius) {
	  newpos.x = -10.0f+2*radius - next[id].x;
    old.x = -10.0f+2*radius - pos[id].x;
  }
  if (dot(vel[id], left) < 0 && next[id].x > 5.0f-radius) {
	  newpos.x = 10.0f-2*radius - next[id].x;
    old.x = 10.0f-2*radius - pos[id].x;
  }
  if (dot(vel[id], forth) < 0 && next[id].z < -5.0f+radius) {
	  newpos.z = -10.0f+2*radius - next[id].z;
    old.z = -10.0f+2*radius - pos[id].z;
  }
  if (dot(vel[id], back) < 0 && next[id].z > 5.0f-radius) {
	  newpos.z = 10.0f-2*radius - next[id].z;
    old.z = 10.0f-2*radius - pos[id].z;
  }
  pos[id] = old;
  next[id] = newpos;
}
__kernel void integrate(__global float4* pos, __global float4* next, __global float4* vel, __global particle* p, float dt) {
  unsigned int id = get_global_id(0);
  vel[id] = (next[id] - pos[id])/dt;
  pos[id] = next[id];
}

__kernel void move(__global float4* pos, __global float4* next, __global float4* vel, __global particle* p, float dt) {
  unsigned int id = get_global_id(0);
  vel[id] = vel[id] + gravity * dt;
  next[id] = pos[id] + vel[id] * dt;
}
