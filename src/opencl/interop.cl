__kernel void move(__global float4* pos, __global float4* vel, float dt, __global int* type) {
  unsigned int index = get_global_id(0);
  int id = type[index];
  pos[id] = pos[id] + vel[id] * dt;
}
