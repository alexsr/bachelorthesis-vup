__kernel void move(__global float4* pos, __global float4* vel, float dt, __global int* type) {
  unsigned int index = get_global_id(0);
  int id = type[index];
  pos[id] = pos[id] + vel[id] * dt;
}

__kernel void test(__global float4* pos, __global float4* vel, float dt, __global int* type) {
  unsigned int index = get_global_id(0);
  int id = type[index];
  vel[id] = vel[id] + gravity * dt;
  pos[id] = pos[id] + vel[id] * dt;
  if (dot(vel[id], down) < 0 && pos[id].y < -5.0f) {
	vel[id].y = vel[id].y*-1.0f;
  }
}
