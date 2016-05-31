__kernel void move(__global float4* pos, __global float4* vel, float dt) {
  unsigned int id = get_global_id(0);
  pos[id] = pos[id] + vel[id] * dt;
}
