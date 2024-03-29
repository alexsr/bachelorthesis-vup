__constant float4 up = (float4)(0.0f, 1.0f, 0.0f, 0.0f);
__constant float4 down = (float4)(0.0f, -1.0f, 0.0f, 0.0f);
__constant float4 left = (float4)(-1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 right = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 forth = (float4)(0.0f, 0.0f, 1.0f, 0.0f);
__constant float4 back = (float4)(0.0f, 0.0f, -1.0f, 0.0f);

__constant float radius = 0.1;
__constant float smoothingLength = 0.2;
__constant float polySixConst = 3059924.7482;
__constant float spikyConst = 223811.6387;
__constant float restDensity = 1000.0;
__constant int neighbor_amount = 30;

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

__kernel void generateConnectionDistances(__global float4* pos, __global int* globalIndices, __global int* connectionCounter, __global int* connections, __global float4* connectionDistances, float maxConnections, __global int* systemIDs) {
  int id = get_global_id(0);
  int g_id = globalIndices[id];
  int sysID = systemIDs[id];
  float maxDist = radius * 2.0f * sqrt(3.0f);
  float4 p = pos[g_id];
  for (int s = 0; s < get_global_size(0); s++) {
    if (connectionCounter[id] > maxConnections) {
      break;
    }
    int g_s = globalIndices[s];
    float dist = distance(p, pos[g_s]);
    if (dist <= maxDist && id != s && sysID == systemIDs[s]) {
      int con_id = id * maxConnections + connectionCounter[id];
      float4 calcConnection = pos[g_s] - p;
      connectionDistances[con_id] = calcConnection;
      connectionCounter[id]++;
      connections[con_id] = g_s;
    }
  }
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

__kernel void printGrid(__global float4* pos, __global int* grid, volatile __global int* gridCounter, float cellRadius, int cellsinx, int cellsiny, int cellsinz, int cellCapacity, float4 gridMidpoint, __global float4* color) {
  int id = get_global_id(0);
  float xradius = cellRadius * cellsinx;
  float yradius = cellRadius * cellsiny;
  float zradius = cellRadius * cellsinz;
  int i = (pos[id].x - gridMidpoint.x + xradius) / xradius * (cellsinx / 2.0f);
  int j = (pos[id].y - gridMidpoint.y + yradius) / yradius * (cellsiny / 2.0f);
  int k = (pos[id].z - gridMidpoint.z + zradius) / zradius * (cellsinz / 2.0f);
  color[id].x = i / (float)(cellsinx);
  color[id].y = j / (float)(cellsiny);
  color[id].z = k / (float)(cellsinz);
}

__kernel void collisionNoGrid(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, __global int* systemIDs, float dt) {
  int id = get_global_id(0);
  float m1 = mass[id];
  float4 v1 = vel[id];
  float4 p = pos[id];
  for (int j = 0; j < get_global_size(0); j++) {
    float dist = distance(p.xyz, pos[j].xyz);
    if (systemIDs[id] != systemIDs[j] && dist < radius * 2.0f && id != j) {
      float4 n = normalize(p - pos[j]);
      float4 v2 = vel[j];
      float m2 = mass[j];
      float jr = -2.0 * dot(v2 - v1, n) / (1.0/m1 + 1.0/m2);
      // v2 = v2 + jr / m2 * n;
      // vel[id] += - jr / m1 * n;
      // vel[other] = v2;
      forceIntern[id] += -jr * n / dt;
      // forceIntern[other] = jr * n / dt;
      pos[id] += n * (radius * 2.0f - dist)/2.0f;
      // pos[other] -= n * (radius * 2.0f - dist)/2.0f;
    }
  }
}

__kernel void collision(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, __global int* grid, volatile __global int* gridCounter, float cellRadius, int cellsinx, int cellsiny, int cellsinz, int cellCapacity, float4 gridMidpoint, __global int* systemIDs, float dt) {
  int id = get_global_id(0);
  float xradius = cellRadius * cellsinx;
  float yradius = cellRadius * cellsiny;
  float zradius = cellRadius * cellsinz;
  int i = (pos[id].x - gridMidpoint.x + xradius) / xradius * (cellsinx / 2.0f);
  int j = (pos[id].y - gridMidpoint.y + yradius) / yradius * (cellsiny / 2.0f);
  int k = (pos[id].z - gridMidpoint.z + zradius) / zradius * (cellsinz / 2.0f);
  float m1 = mass[id];
  float4 v1 = vel[id];
  float4 p = pos[id];
  for (int x = max(0, i - 1); x < min(i + 2, cellsinx); x++) {
    int x_counter_offset = x * cellsiny * cellsinz;
    int x_offset = x_counter_offset * cellCapacity;
    for (int y = max(0, j - 1); y < min(j + 2, cellsiny); y++) {
      int y_counter_offset = y * cellsinz;
      int y_offset = y_counter_offset * cellCapacity;
      for (int z = max(0, k - 1); z < min(k + 2, cellsinz); z++) {
        int z_offset = z * cellCapacity;
        int n = gridCounter[x_counter_offset + y_counter_offset + z];
        for (int o = 0; o < min(n, cellCapacity); o++) {
          int other = grid[x_offset + y_offset + z_offset + o];
          float dist = distance(pos[id].xyz, pos[other].xyz);
          if (systemIDs[other] != systemIDs[id] && dist < radius * 2.0f && id != other) {
            float4 n = normalize(p - pos[other]);
            float4 v2 = vel[other];
            float m2 = mass[other];
            float jr = -2.0 * dot(v2 - v1, n) / (1.0/m1 + 1.0/m2);
            // v2 = v2 + jr / m2 * n;
            // vel[id] += - jr / m1 * n;
            // vel[other] = v2;
            forceIntern[id] += -jr * n / dt;
            // forceIntern[other] = jr * n / dt;
            pos[id] += n * (radius * 2.0f - dist)/2.0f;
            // pos[other] -= n * (radius * 2.0f - dist)/2.0f;
          }
        }
      }
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

__kernel void findNeighbors(__global float4* pos, __global int* neighbors, __global int* neighborCounter, __global int* grid, volatile __global int* gridCounter, float cellRadius, int cellsinx, int cellsiny, int cellsinz, int cellCapacity, float4 gridMidpoint, __global int* globalIndices, __global int* typeIDs, __global int* typeSizes) {
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
        for (int o = 0; o < min(n, cellCapacity); o++) {
          int other = grid[x_offset + y_offset + z_offset + o];
          float dist = distance(p.xyz, pos[other].xyz);
          if (dist <= smoothingLength) {
            if (other >= typeIDs[id] && other < typeIDs[id] + typeSizes[id]) {
              neighbors[id * neighbor_amount + neighborCounter[id]] = other - typeIDs[id];
              neighborCounter[id]++;
            }
          }
          if (neighborCounter[id] >= neighbor_amount - 1) {
            return;
          }
        }
      }
    }
  }
}

__kernel void initForces(__global float4* pos, __global float4* vel, __global float4* forceIntern, __global float4* forcePressure, __global float* mass, __global float* density, __global float* pressure, __global int* neighbors, __global int* neighborCounter, __global int* globalIndices) {
  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];
  float visc_const = 0.2;
  float4 viscosityForce = 0.0f;
  for (int i = 0; i < neighborCounter[id]; i++) {
    int j = neighbors[id * neighbor_amount + i];
    int g_j = globalIndices[j];
    viscosityForce += mass[g_j] * (vel[g_j] - vel[g_id]) / density[j] * visc(smoothingLength, distance(pos[g_id].xyz, pos[g_j].xyz));
  }
  viscosityForce *= spikyConst * visc_const * mass[g_id];
  float4 forceExtern = 0.0f;
  forceExtern.y = -9.81f * mass[g_id];
  forceIntern[g_id].xyz += forceExtern.xyz + viscosityForce.xyz;
  pressure[id] = 0.0f;
  forcePressure[id] = 0.0f;
}

__kernel void predict(__global float4* pos, __global float4* vel, __global float4* predictpos, __global float4* predictvel, __global float4* forceIntern, __global float4* forcePressure, __global float* mass, __global int* globalIndices, float dt) {
  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];
  predictvel[id].xyz = vel[g_id].xyz + ((forceIntern[g_id].xyz + forcePressure[id].xyz) / mass[g_id]) * dt;
  predictpos[id].xyz = pos[g_id].xyz + predictvel[id].xyz * dt;
}

__kernel void updatePressure(__global float4* pos, __global float4* predictpos, __global int* neighbors, __global int* neighborCounter, __global float* density, __global float* pressure, __global float* mass, __global int* globalIndices, float dt) {
  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];
  float density_id = 0.0f;
  float pressure_id = 0;
  for (int i = 0; i < neighborCounter[id]; i++) {
    int j = neighbors[id * neighbor_amount + i];
    int g_j = globalIndices[j];
    density_id += mass[g_j] * polySix(smoothingLength, distance(predictpos[id].xyz, predictpos[j].xyz));
  }
  density_id *= polySixConst;

  density[id] = density_id;
  float density_err = density_id - restDensity;
  pressure[id] += -density_err * 0.009f;
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

__kernel void computeConnectionForces(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, __global float* stiffness, __global float* dampingConstant,
__global int* connectionCounter, __global int* connections, __global float4* connectionDistances, float maxConnections, __global int* globalIndices, float dt) {
  int id = get_global_id(0);
  int g_id = globalIndices[id];
  float4 forceB = 0.0f;
  float4 forceD = 0.0f;
  for (int i = 0; i < connectionCounter[id]; i++) {
    int con_id = id * maxConnections + i;
    int e_id = connections[con_id];
    float4 posDiff = pos[e_id] - pos[g_id];
    float4 velDiff = vel[e_id] - vel[g_id];
    float actualDistance = length(posDiff);
    float restDistance = length(connectionDistances[con_id]);
    float4 vel_parallel = dot(velDiff, posDiff) / dot(posDiff, posDiff) * posDiff;
    forceB += stiffness[id] * (actualDistance - restDistance) * posDiff/actualDistance;
    forceD += dampingConstant[id] * vel_parallel;
  }
  float4 forceExtern = 0.0f;
  forceExtern.y = -9.81f * mass[g_id];
  forceIntern[g_id] += forceExtern + forceB + forceD;
}

__kernel void updateFluidForce(__global float4* forceIntern, __global float4* forcePressure, __global int* globalIndices) {
  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];
  forceIntern[g_id] += forcePressure[id];
}

__kernel void integrate(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, float dt) {
  unsigned int id = get_global_id(0);
  vel[id].xyz += forceIntern[id].xyz / mass[id] * dt;
  forceIntern[id] = 0.0;
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
