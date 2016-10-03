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

__kernel void collisionNoGrid(__global float4* pos, __global float4* vel, __global float4* forceIntern, __global int* systemIDs, __global int* globalIndices, __global float* stiffness, __global float* damping, __global float* friction, __global float* mass, float dt, float staticFriction) {
  int id = get_global_id(0);
  float m = mass[id];
  float4 v = vel[id];
  float4 p = pos[id];
  float diam = radius * 2.0f;
  float4 forceNormal = 0.0f;
  float4 forceFriction = 0.0f;
  for (int j = 0; j < get_global_size(0); j++) {
    if (j == id) {
      continue;
    }
    float4 pj = pos[j];
    float dist = distance(p.xyz, pj.xyz);
    if (dist < diam) {
      float kr = stiffness[j];
      float kd = damping[j];
      float intersect = max(0.0f, diam-dist);
      float ks = friction[j];
      float mj = mass[j];
      float4 diffPos = pj - p;
      float4 vj = vel[j];
      float4 velDiff = (v - vj);
      float4 n = diffPos / dist;
      float relativeVelocity = dot(velDiff, n);
      float4 fn = -(kd * sqrt(intersect)*relativeVelocity + kr * pow(intersect, 1.5)) * n;
      float4 v_tan = cross(n, cross(velDiff, n));
      float m_eff = m * mj / (m + mj);
      forceFriction += -staticFriction * m_eff * v_tan - min(ks * length(fn), ks * length(v_tan)) * normalize(v_tan);
      forceNormal += fn;
    }
  }
  forceIntern[id].xyz += forceFriction.xyz + forceNormal.xyz;
}

__kernel void collision(__global float4* pos, __global float4* vel, __global float4* forceIntern, __global int* systemIDs, __global int* globalIndices, __global float* stiffness, __global float* damping, __global float* friction, __global float* mass, __global int* grid, volatile __global int* gridCounter, float cellRadius, int cellsinx, int cellsiny, int cellsinz, int cellCapacity, float4 gridMidpoint, float dt, float staticFriction) {
  int id = get_global_id(0);
  int g_id = globalIndices[id];
  float xradius = cellRadius * cellsinx;
  float yradius = cellRadius * cellsiny;
  float zradius = cellRadius * cellsinz;
  float4 p = pos[g_id];
  float4 v = vel[g_id];
  float m = mass[g_id];
  int a = (p.x - gridMidpoint.x + xradius) / xradius * (cellsinx / 2.0f);
  int b = (p.y - gridMidpoint.y + yradius) / yradius * (cellsiny / 2.0f);
  int c = (p.z - gridMidpoint.z + zradius) / zradius * (cellsinz / 2.0f);
  float diam = radius * 2.0f;
  float4 forceNormal = 0.0f;
  float4 forceFriction = 0.0f;
  for (int x = max(0, a - 1); x < min(a + 2, cellsinx); x++) {
    int x_counter_offset = x * cellsiny * cellsinz;
    int x_offset = x_counter_offset * cellCapacity;
    for (int y = max(0, b - 1); y < min(b + 2, cellsiny); y++) {
      int y_counter_offset = y * cellsinz;
      int y_offset = y_counter_offset * cellCapacity;
      for (int z = max(0, c - 1); z < min(c + 2, cellsinz); z++) {
        int z_offset = z * cellCapacity;
        int n = gridCounter[x_counter_offset + y_counter_offset + z];
        for (int o = 0; o < n; o++) {
          int j = grid[x_offset + y_offset + z_offset + o];
          if (j == id) {
            continue;
          }
          float4 pj = pos[j];
          float dist = distance(p.xyz, pj.xyz);
          if (dist < diam) {
            float kr = stiffness[j];
            float kd = damping[j];
            float intersect = max(0.0f, diam-dist);
            float ks = friction[j];
            float mj = mass[j];
            float4 diffPos = pj - p;
            float4 vj = vel[j];
            float4 velDiff = (v - vj);
            float4 n = diffPos / dist;
            float relativeVelocity = dot(velDiff, n);
            float4 fn = -(kd * sqrt(intersect)*relativeVelocity + kr * pow(intersect, 1.5)) * n;
            float4 v_tan = cross(n, cross(velDiff, n));
            float m_eff = m * mj / (m + mj);
            forceFriction += -staticFriction * m_eff * v_tan - min(ks * length(fn), ks * length(v_tan)) * normalize(v_tan);
            forceNormal += fn;
          }
        }
      }
    }
  }
  forceIntern[id].xyz += forceFriction.xyz + forceNormal.xyz;
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
