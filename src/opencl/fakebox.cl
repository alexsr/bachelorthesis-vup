__constant float4 up = (float4)(0.0f, 1.0f, 0.0f, 0.0f);
__constant float4 down = (float4)(0.0f, -1.0f, 0.0f, 0.0f);
__constant float4 left = (float4)(-1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 right = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 forth = (float4)(0.0f, 0.0f, 1.0f, 0.0f);
__constant float4 back = (float4)(0.0f, 0.0f, -1.0f, 0.0f);
__constant float4 gravity = (float4)(0.0f, -9.81f, 0.0f, 0.0f);
__constant float4 fluidGrav = (float4)(1.0f, -9.81f, -1.0f, 0.0f);

float4 reflect(float4 d, float4 n) {
  if (dot(d, n) > 0) {
    d = d*-1.0f;
  }
  float4 r = d - 2.0f*dot(d, n)*n;
  return r;
}

float4 fakeCollideBounds(float4 vel, float4 pos, float damp) {
  float d = 1.0f;
  if (dot(vel, up) < 0 && pos.y < -5.0f) {
	  vel = reflect(vel, up);
    d = damp;
  }
  if (dot(vel, down) < 0 && pos.y > 5.0f) {
	  vel = reflect(vel, down);
    d = damp;
  }
  if (dot(vel, right) < 0 && pos.x < -5.0f) {
	  vel = reflect(vel, right);
    d = damp;
  }
  if (dot(vel, left) < 0 && pos.x > 5.0f) {
	  vel = reflect(vel, left);
    d = damp;
  }
  if (dot(vel, forth) < 0 && pos.z < -5.0f) {
	  vel = reflect(vel, forth);
    d = damp;
  }
  if (dot(vel, back) < 0 && pos.z > 5.0f) {
	  vel = reflect(vel, back);
    d = damp;
  }
  return vel*d;
}

__kernel void fakecollision(__global float4* pos, __global float4* vel, float dt, float particle_amount) {
  unsigned int id = get_global_id(0);
  vel[id] = fakeCollideBounds(vel[id], pos[id], 1.0f);
  for (int i = 0; i < id; i++) {
    if (distance(pos[id], pos[i]) < 0.2f) {
      float damped = 0.99f;
      float4 n = normalize(pos[id] - pos[i]);
      float4 v1 = vel[id];
      float4 v2 = vel[i];
      float a1 = dot(v1, n);
      float a2 = dot(v2, n);
      float m1 = 1.0f;
      float m2 = 1.0f;
      float optimizedP = (2.0f * (a1 - a2)) / (m1 + m2);
      v1 = v1 - optimizedP * m2 * n;
      vel[id] =  v1 * damped;
      v2 = v2 + optimizedP * m1 * n;
      vel[i] = v2 * damped;
    }
  }
  for (int i = id+1; i < particle_amount; i++) {
    if (distance(pos[id], pos[i]) < 0.2f) {
      float damped = 0.99f;
      float4 n = normalize(pos[id] - pos[i]);
      float4 v1 = vel[id];
      float4 v2 = vel[i];
      float a1 = dot(v1, n);
      float a2 = dot(v2, n);
      float m1 = 1.0f;
      float m2 = 1.0f;
      float optimizedP = (2.0f * (a1 - a2)) / (m1 + m2);
      v1 = v1 - optimizedP * m2 * n;
      vel[id] =  v1 * damped;
      v2 = v2 + optimizedP * m1 * n;
      vel[i] = v2 * damped;
    }
  }
}

__kernel void move(__global float4* pos, __global float4* vel, float dt, __global int* type) {
  unsigned int index = get_global_id(0);
  int id = type[index];
  pos[id] = pos[id] + vel[id] * dt;
}

__kernel void test(__global float4* pos, __global float4* vel, float dt) {
  unsigned int id = get_global_id(0);
  vel[id] = vel[id] + gravity * dt;
  pos[id] = pos[id] + vel[id] * dt;
}
