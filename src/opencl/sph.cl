__constant float4 up = (float4)(0.0f, 1.0f, 0.0f, 0.0f);
__constant float4 down = (float4)(0.0f, -1.0f, 0.0f, 0.0f);
__constant float4 left = (float4)(-1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 right = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 forth = (float4)(0.0f, 0.0f, 1.0f, 0.0f);
__constant float4 back = (float4)(0.0f, 0.0f, -1.0f, 0.0f);
__constant float4 gravity = (float4)(0.0f, -0.981f, 0.0f, 0.0f);

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
__kernel void integrate(__global float4* pos, __global float4* next, __global float4* vel, __global particle* particles, float dt) {
  unsigned int id = get_global_id(0);
  vel[id] = (next[id] - pos[id])/dt;
  pos[id] = next[id];
}

__kernel void move(__global float4* pos, __global float4* next, __global float4* vel, __global particle* particles, __global int* grid, __global int* counter, float cellSize, int lineSize, int cellCapacity, float dt) {
  int id = get_global_id(0);
  int current_x = (int)(pos[id].x / cellSize + lineSize / 2.0f);
  int current_y = (int)(pos[id].y / cellSize + lineSize / 2.0f);
  int current_z = (int)(pos[id].z / cellSize + lineSize / 2.0f);
  for (int x = max(current_x - 1, 0); x <= min(current_x + 1, lineSize-1); x++) {
    int x_counter_offset = x * lineSize * lineSize;
    int x_offset = x_counter_offset * cellCapacity;
    for (int y = max(current_y - 1, 0); y <= min(current_y + 1, lineSize-1); y++) {
      int y_counter_offset = y * lineSize;
      int y_offset = y_counter_offset * cellCapacity;
      for (int z = max(current_z - 1, 0); z <= min(current_z + 1, lineSize-1); z++) {
        //printf("x_counter = %d", z);
        int z_offset = z * cellCapacity;
        int n = counter[x_counter_offset + y_counter_offset + z];
        //printf("x = %d", x_counter_offset + y_counter_offset + z);
        for (int i = 0; i < n; i++) {
          //printf("otherid = %d", x_offset + y_offset + z_offset + i);
          int other = grid[x_offset + y_offset + z_offset + i];
          //printf("other = %d", other);
          if (distance(pos[id], pos[other]) < 0.2f && id != other) {
            float4 n = normalize(pos[id] - pos[other]);
            float4 v1 = vel[id];
            float4 v2 = vel[other];
            float a1 = dot(v1, n);
            float a2 = dot(v2, n);
            float m1 = 1.0f;
            float m2 = 1.0f;
            float optimizedP = (2.0f * (a1 - a2)) / (m1 + m2);
            v1 = v1 - optimizedP * m2 * n;
            v2 = v2 + optimizedP * m1 * n;
            vel[id] = v1;
            vel[other] = v2;
          }
        }
      }
    }
  }
  vel[id] = vel[id] + gravity * dt;
  next[id] = pos[id] + vel[id] * dt;
}

__kernel void resetGrid(__global int* counter) {
  unsigned int id = get_global_id(0);
  counter[id] = 0;
}

__kernel void updateGrid(__global float4* pos, __global int* grid, __global volatile int* counter, float cellSize, int lineSize, int cellCapacity) {
  unsigned int id = get_global_id(0);
  int i = (int) (pos[id].x / cellSize + lineSize / 2.0f);
  int j = (int) (pos[id].y / cellSize + lineSize / 2.0f);
  int k = (int) (pos[id].z / cellSize + lineSize / 2.0f);
  volatile int n = atomic_inc(&counter[i * lineSize * lineSize + j * lineSize + k]);
  grid[i * lineSize * lineSize * cellCapacity + j * lineSize * cellCapacity + k * cellCapacity + n] = id;
}
