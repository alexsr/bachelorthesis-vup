#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

__kernel void move(__global float4* pos, __global float4* vel) {
  unsigned int id = get_global_id(0);
  pos[id] = pos[id] + vel[id] * 0.01;
}