__constant float4 up = (float4)(0.0f, 1.0f, 0.0f, 0.0f);
__constant float4 down = (float4)(0.0f, -1.0f, 0.0f, 0.0f);
__constant float4 left = (float4)(-1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 right = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 forth = (float4)(0.0f, 0.0f, 1.0f, 0.0f);
__constant float4 back = (float4)(0.0f, 0.0f, -1.0f, 0.0f);


__constant float radius = 0.1;

float4 reflect(float4 d, float4 n) {
  if (dot(d, n) > 0) {
    d = d*-1.0f;
  }
  float4 r = d - 2.0f*dot(n, d)*n;
  return r;
}

__kernel void resetGrid(__global int* gridCounter) {
  int id = get_global_id(0);
  gridCounter[id] = 0;
}

__kernel void updateGrid(__global float4* pos, __global int* grid, volatile __global int* gridCounter, float cellRadius, int cellsinx, int cellsiny, int cellsinz, int cellCapacity, float4 gridMidpoint) {
  int id = get_global_id(0);
  float xradius = cellRadius * cellsinx;
  float yradius = cellRadius * cellsiny;
  float zradius = cellRadius * cellsinz;
  int i = (pos[id].x - gridMidpoint.x + xradius) / xradius * (cellsinx / 2.0f);
  int j = (pos[id].y - gridMidpoint.y + yradius) / yradius * (cellsiny / 2.0f);
  int k = (pos[id].z - gridMidpoint.z + zradius) / zradius * (cellsinz / 2.0f);
  int counterIndex = i * cellsiny * cellsinz + j * cellsinz + k;
  if (counterIndex < cellsinx*cellsiny*cellsinz) {
    int n = atomic_inc(&(gridCounter[i * cellsiny * cellsinz + j * cellsinz + k]));
    if (n < cellCapacity) {
      grid[i * cellsiny * cellsinz * cellCapacity + j * cellsinz * cellCapacity + k * cellCapacity + n] = id;
    }
  }
}

__kernel void rigidCollision(__global float4* pos, __global float4* vel, __global float4* forceIntern, __global int* systemIDs, __global float* springcoefficient, __global float* dampingcoefficient, __global float* mass, __global int* grid, volatile __global int* gridCounter, float cellRadius, int cellsinx, int cellsiny, int cellsinz, int cellCapacity, float4 gridMidpoint, float dt) {
  int id = get_global_id(0);
  float xradius = cellRadius * cellsinx;
  float yradius = cellRadius * cellsiny;
  float zradius = cellRadius * cellsinz;
  int i = (pos[id].x - gridMidpoint.x + xradius) / xradius * (cellsinx / 2.0f);
  int j = (pos[id].y - gridMidpoint.y + yradius) / yradius * (cellsiny / 2.0f);
  int k = (pos[id].z - gridMidpoint.z + zradius) / zradius * (cellsinz / 2.0f);
  float diam = radius * 2.0f;
  float4 p = pos[id];
  float4 v = vel[id];
  float4 forceSpring = 0.0f;
  float4 forceDamping = 0.0f;
  float4 forceShear = 0.0f;
  for (int x = max(0, i - 1); x < min(i + 2, cellsinx); x++) {
    int x_counter_offset = x * cellsiny * cellsinz;
    int x_offset = x_counter_offset * cellCapacity;
    for (int y = max(0, j - 1); y < min(j + 2, cellsiny); y++) {
      int y_counter_offset = y * cellsinz;
      int y_offset = y_counter_offset * cellCapacity;
      for (int z = max(0, k - 1); z < min(k + 2, cellsinz); z++) {
        int z_offset = z * cellCapacity;
        int n = gridCounter[x_counter_offset + y_counter_offset + z];
        for (int o = 0; o < n; o++) {
          int j = grid[x_offset + y_offset + z_offset + o];
          if (j == id) {
            continue;
          }
          float dist = distance(pos[id].xyz, pos[j].xyz);
          if (dist < diam) {
            float k = springcoefficient[j];
            float kd = dampingcoefficient[j];
            float kt = kd * 0.1f;
            float4 diffPos = pos[j] - p;
            float4 vj = vel[j];
            float4 velDiff = (vj - v);
            float4 n = diffPos / dist;
            forceSpring += -k * (diam - dist) * n;
            forceDamping += kd * velDiff;
            forceShear += kt * (velDiff - dot(velDiff, n) * n);
          }
        }
      }
    }
  }
  forceIntern[id].xyz += forceSpring.xyz + forceDamping.xyz + forceShear.xyz;
}

__kernel void integrate(__global float4* pos, __global float* mass, __global float4* vel, __global float4* forceIntern, __global int* globalIndices, float dt) {
  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];
  float4 gravForce = 0.0f;
  gravForce.y = -9.81f * mass[g_id];
  float4 v = vel[g_id];
  float4 p = pos[g_id];
  v += ((forceIntern[g_id] + gravForce) / mass[g_id]) * dt;
  forceIntern[g_id] = 0.0f;
  p += v * dt;
  float bounds = 2.0;
  float ybounds = 2.0;
  if (dot(v, up) < 0 && p.y < -ybounds) {
    v = reflect(v, up);
  }
  if (dot(v, down) < 0 && p.y > ybounds) {
    v = reflect(v, down);
  }
  if (dot(v, right) < 0 && p.x < -bounds) {
    v = reflect(v, right);
  }
  if (dot(v, left) < 0 && p.x > bounds) {
    v = reflect(v, left);
  }
  if (dot(v, forth) < 0 && p.z < -bounds) {
    v = reflect(v, forth);
  }
  if (dot(v, back) < 0 && p.z > bounds) {
    v = reflect(v, back);
  }
  pos[g_id].x = clamp(p.x, -bounds, bounds);
  pos[g_id].y = clamp(p.y, -ybounds, ybounds);
  pos[g_id].z = clamp(p.z, -bounds, bounds);
  vel[g_id] = v;
}
