__constant float4 up = (float4)(0.0f, 1.0f, 0.0f, 0.0f);
__constant float4 down = (float4)(0.0f, -1.0f, 0.0f, 0.0f);
__constant float4 left = (float4)(-1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 right = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 forth = (float4)(0.0f, 0.0f, 1.0f, 0.0f);
__constant float4 back = (float4)(0.0f, 0.0f, -1.0f, 0.0f);

__constant float smoothingLength = 0.2;
__constant float restDensity = 1000.0;
__constant int neighbor_amount = 100;
__constant float polySixConst = 3059924.7482;
__constant float spikyConst = 223811.6387;

float polySix(float h, float r) {
  if (0 <= r && r < h) {
    return pow(h*h - r*r, 3);
  }
  else {
    return 0.0;
  }
}

float4 spikyGradient(float h, float4 p, float4 pj) {
  float4 v = p - pj;
  float r = length(v);
  if (0 < r && r < h) {
    return pow(h - r, 2) * v/r;
  }
  else {
    return 0.0;
  }
}

float visc(float h, float r) {
  if (0 <= r && r < h) {
    return h - r;
  }
  else {
    return 0.0;
  }
}

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
  float4 p = pos[id];
  int i = (p.x - gridMidpoint.x + xradius) / xradius * (cellsinx / 2.0f);
  int j = (p.y - gridMidpoint.y + yradius) / yradius * (cellsiny / 2.0f);
  int k = (p.z - gridMidpoint.z + zradius) / zradius * (cellsinz / 2.0f);
  int counterIndex = i * cellsiny * cellsinz + j * cellsinz + k;
  if (counterIndex < cellsinx*cellsiny*cellsinz) {
    int n = atomic_inc(&(gridCounter[i * cellsiny * cellsinz + j * cellsinz + k]));
    if (n < cellCapacity) {
      grid[i * cellsiny * cellsinz * cellCapacity + j * cellsinz * cellCapacity + k * cellCapacity + n] = id;
    }
  }
}

__kernel void findNeighborsNoGrid(__global float4* pos, __global int* neighbors, __global int* neighborCounter, __global int* globalIndices) {
  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];
  float4 p = pos[g_id];
  neighborCounter[id] = 0;
  for (int index = 0; index < get_global_size(0); index++) {
    if (distance(p.xyz, pos[globalIndices[index]].xyz) <= smoothingLength) {
      neighbors[id * neighbor_amount + neighborCounter[id]] = index;
      neighborCounter[id]++;
    }
    if (neighborCounter[id] >= neighbor_amount - 1) {
      break;
    }
  }
}

__kernel void findNeighbors(__global float4* pos, __global int* neighbors, __global int* neighborCounter, __global int* grid, volatile __global int* gridCounter, float cellRadius, int cellsinx, int cellsiny, int cellsinz, int cellCapacity, float4 gridMidpoint, __global int* globalIndices, __global int* typeIDs) {
  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];
  float xradius = cellRadius * cellsinx;
  float yradius = cellRadius * cellsiny;
  float zradius = cellRadius * cellsinz;
  float4 p = pos[g_id];
  int i = (p.x - gridMidpoint.x + xradius) / xradius * (cellsinx / 2.0f);
  int j = (p.y - gridMidpoint.y + yradius) / yradius * (cellsiny / 2.0f);
  int k = (p.z - gridMidpoint.z + zradius) / zradius * (cellsinz / 2.0f);
  neighborCounter[id] = 0;
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
          int other = grid[x_offset + y_offset + z_offset + o];
          float dist = distance(p.xyz, pos[other].xyz);
          if (dist <= smoothingLength && g_id != other)
          {
            neighbors[id * neighbor_amount + neighborCounter[id]] = other - typeIDs[id];
            neighborCounter[id]++;
          }
          if (neighborCounter[id] >= neighbor_amount - 1) {
            return;
          }
        }
      }
    }
  }
}

__kernel void initForces(__global float4* pos, __global float4* vel, __global float4* forceIntern, __global float4* forcePressure, __global float* mass, __global float* density, __global float* pressure, __global int* neighbors, __global int* neighborCounter, __global int* globalIndices, __global int* systemIDs) {
  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];
  float visc_const = 0.2;
  float4 viscosityForce = 0.0f;
  for (int i = 0; i < neighborCounter[id]; i++) {
    int j = neighbors[id * neighbor_amount + i];
    int g_j = globalIndices[j];
    viscosityForce += mass[g_j] * (vel[g_j] - vel[g_id]) / density[j] * visc(smoothingLength, distance(pos[g_id].xyz, pos[g_j].xyz));
  }
  viscosityForce *= visc_const * mass[g_id] * spikyConst;
  float4 forceExtern = 0.0f;
  forceExtern.y = -9.81f * mass[g_id];
  forceIntern[g_id].xyz = forceExtern.xyz + viscosityForce.xyz;
  pressure[id] = 0.0f;
  forcePressure[id] = 0.0f;
}

__kernel void predict(__global float4* pos, __global float4* vel, __global float4* predictpos, __global float4* predictvel, __global float4* forceIntern, __global float4* forcePressure, __global float* mass, __global int* globalIndices, float dt) {
  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];
  predictvel[id].xyz = vel[g_id].xyz + ((forceIntern[g_id].xyz + forcePressure[id].xyz) / mass[g_id]) * dt;
  predictpos[id].xyz = pos[g_id].xyz + predictvel[g_id].xyz * dt;
}

__kernel void updatePressure(__global float4* pos, __global float4* predictpos, __global int* neighbors, __global int* neighborCounter, __global float* density, __global float* pressure, __global float* mass, __global int* globalIndices, float dt) {
  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];
  float density_id = mass[g_id] * polySix(smoothingLength, 0.0);
  float pressure_id = 0;
  for (int i = 0; i < neighborCounter[id]; i++) {
    int j = neighbors[id * neighbor_amount + i];
    int g_j = globalIndices[j];
    density_id += mass[g_j] * polySix(smoothingLength, distance(predictpos[id].xyz, predictpos[j].xyz));
  }
  density_id *= polySixConst;

  density[id] = density_id;
  float density_err = density_id - restDensity;
  pressure[id] += -density_err * 0.02f;
}

__kernel void computePressureForce(__global float4* predictpos, __global int* neighbors, __global int* neighborCounter, __global float* density, __global float* pressure, __global float* mass, __global float4* forcePressure, __global int* globalIndices) {
  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];
  float4 pressureForce = 0.0f;
  for (int i = 0; i < neighborCounter[id]; i++) {
    int j = neighbors[id * neighbor_amount + i];
    int g_j = globalIndices[j];
    pressureForce += mass[g_j] * (pressure[id] + pressure[j]) / (2.0f * density[j]) * spikyGradient(smoothingLength, predictpos[id], predictpos[j]);
  }
  pressureForce *= mass[g_id] * spikyConst;
  forcePressure[id].xyz = pressureForce.xyz;
}

__kernel void integrate(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, __global float4* forcePressure, float dt) {
  unsigned int id = get_global_id(0);
  vel[id].xyz += ((forceIntern[id].xyz + forcePressure[id].xyz) / mass[id]) * dt;
  float bounds = 2.0;
  float ybounds = 2.0;
  float4 p = pos[id] + vel[id] * dt;
  float damping = 0.9;
  if (dot(vel[id], up) < 0 && p.y < -ybounds) {
    vel[id] = reflect(vel[id], up) * damping;
  }
  if (dot(vel[id], down) < 0 && p.y > ybounds) {
    vel[id] = reflect(vel[id], down) * damping;
  }
  if (dot(vel[id], right) < 0 && p.x < -bounds) {
    vel[id] = reflect(vel[id], right) * damping;
  }
  if (dot(vel[id], left) < 0 && p.x > bounds) {
    vel[id] = reflect(vel[id], left) * damping;
  }
  if (dot(vel[id], forth) < 0 && p.z < -bounds) {
    vel[id] = reflect(vel[id], forth) * damping;
  }
  if (dot(vel[id], back) < 0 && p.z > bounds) {
    vel[id] = reflect(vel[id], back) * damping;
  }
  pos[id].x = clamp(p.x, -bounds, bounds);
  pos[id].y = clamp(p.y, -bounds, bounds);
  pos[id].z = clamp(p.z, -bounds, bounds);
}
