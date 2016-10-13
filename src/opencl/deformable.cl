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

__kernel void generateConnectionDistances(__global float4* pos, __global int* connectionCounter, __global int* connections, __global float4* connectionDistances, float maxConnections, __global int* systemIDs) {
  int id = get_global_id(0);
  int sysID = systemIDs[id];
  float maxDist = radius * 3.46410162; // ~ 2.0 * sqrt(3.0)
  float4 p = pos[id];
  for (int s = 0; s < get_global_size(0); s++) {
    if (connectionCounter[id] > maxConnections) {
      break;
    }
    float dist = distance(p, pos[s]);
    if (dist <= maxDist && id != s && sysID == systemIDs[s]) {
      int con_id = id * maxConnections + connectionCounter[id];
      float4 calcConnection = pos[s] - pos[id];
      connectionDistances[con_id] = calcConnection;
      connectionCounter[id]++;
      connections[con_id] = s;
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

__kernel void collision(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, __global float* epsilon, __global int* grid, volatile __global int* gridCounter, float cellRadius, int cellsinx, int cellsiny, int cellsinz, int cellCapacity, float4 gridMidpoint, __global int* systemIDs, float dt) {
  int id = get_global_id(0);
  float xradius = cellRadius * cellsinx;
  float yradius = cellRadius * cellsiny;
  float zradius = cellRadius * cellsinz;
  float4 p = pos[id];
  float4 v = vel[id];
  float m = mass[id];
  float e = epsilon[id];
  int i = (p.x - gridMidpoint.x + xradius) / xradius * (cellsinx / 2.0f);
  int j = (p.y - gridMidpoint.y + yradius) / yradius * (cellsiny / 2.0f);
  int k = (p.z - gridMidpoint.z + zradius) / zradius * (cellsinz / 2.0f);
  for (int x = i - 1; x < i + 2; x++) {
    int x_counter_offset = x * cellsiny * cellsinz;
    int x_offset = x_counter_offset * cellCapacity;
    for (int y = j - 1; y < j + 2; y++) {
      int y_counter_offset = y * cellsinz;
      int y_offset = y_counter_offset * cellCapacity;
      for (int z = k - 1; z < k + 2; z++) {
        int z_offset = z * cellCapacity;
        int n = gridCounter[x_counter_offset + y_counter_offset + z];
        for (int o = 0; o < min(n, cellCapacity); o++) {
          int other = grid[x_offset + y_offset + z_offset + o];
          float4 pj = pos[other];
          float dist = distance(p.xyz, pj.xyz);
          if (systemIDs[other] != systemIDs[id] && dist < radius * 2.0f && id != other) {
            float4 n = normalize(p - pj);
            float4 vj = vel[other];
            float mj = mass[other];
            float jr = -(e + epsilon[other]) * dot(vj - v, n) / (1.0/m + 1.0/mj);
            //vel[id] += -jr / m * n;
            //vel[other] += jr / mj * n;
            forceIntern[id] += -jr * n / dt;
            //forceIntern[other] += jr * n / dt;
            pos[id] += n * (radius * 2.0f - dist)/2.0f;
            //pos[other] -= n * (radius * 2.0f - dist)/2.0f;
          }
        }
      }
    }
  }
}

__kernel void selfcollisionNoGrid(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, __global int* systemIDs, __global int* systemSizes, float dt, __global int* globalIndices) {
  int id = get_global_id(0);
  float m1 = mass[id];
  float4 v1 = vel[id];
  float4 p = pos[id];
  for (int j = 0; j < get_global_size(0); j++) {
    float dist = distance(p.xyz, pos[j].xyz);
    if (systemIDs[id] == systemIDs[j] && dist < radius * 2.0f && id != j) {
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

__kernel void selfcollision(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, __global int* grid, volatile __global int* gridCounter, float cellRadius, int cellsinx, int cellsiny, int cellsinz, int cellCapacity, float4 gridMidpoint, __global int* systemIDs, __global int* systemSizes, float dt, __global int* globalIndices) {
  int id = get_global_id(0);
  int g_id = globalIndices[id];
  float xradius = cellRadius * cellsinx;
  float yradius = cellRadius * cellsiny;
  float zradius = cellRadius * cellsinz;
  float4 p = pos[g_id];
  float4 v = vel[g_id];
  float m = mass[g_id];
  int i = (p.x - gridMidpoint.x + xradius) / xradius * (cellsinx / 2.0f);
  int j = (p.y - gridMidpoint.y + yradius) / yradius * (cellsiny / 2.0f);
  int k = (p.z - gridMidpoint.z + zradius) / zradius * (cellsinz / 2.0f);
  for (int x = i - 1; x < i + 2; x++) {
    int x_counter_offset = x * cellsiny * cellsinz;
    int x_offset = x_counter_offset * cellCapacity;
    for (int y = j - 1; y < j + 2; y++) {
      int y_counter_offset = y * cellsinz;
      int y_offset = y_counter_offset * cellCapacity;
      for (int z = k - 1; z < k + 2; z++) {
        int z_offset = z * cellCapacity;
        int n = gridCounter[x_counter_offset + y_counter_offset + z];
        for (int o = 0; o < n; o++) {
          int other = grid[x_offset + y_offset + z_offset + o];
          float4 pj = pos[other];
          float dist = distance(p.xyz, pj.xyz);
          if (other >= systemIDs[id] && other < systemIDs[id] + systemSizes[id] && dist < radius * 2.0f && g_id != other) {
            float4 n = normalize(p - pj);
            float4 vj = vel[other];
            float mj = mass[other];
            float jr = -2.0 * dot(vj - v, n) / (1.0/m + 1.0/mj);
            //vel[id] += -jr / m * n;
            //vel[other] += jr / mj * n;
            forceIntern[g_id] += -jr * n / dt;
            //forceIntern[other] += jr * n / dt;
            pos[g_id] += n * (radius * 2.0f - dist)/2.0f;
            //pos[other] -= n * (radius * 2.0f - dist)/2.0f;
          }
        }
      }
    }
  }
}

__kernel void computeConnectionForces(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, __global float* stiffness, __global float* damping, __global int* connectionCounter, __global int* connections, __global float4* connectionDistances, float maxConnections, float dt) {
  int id = get_global_id(0);
  float4 forceB = 0.0f;
  float4 forceD = 0.0f;
  for (int i = 0; i < connectionCounter[id]; i++) {
    int con_id = id * maxConnections + i;
    int e_id = connections[con_id];
    float4 posDiff = pos[e_id] - pos[id];
    float4 velDiff = vel[e_id] - vel[id];
    float actualDistance = length(posDiff);
    float restDistance = length(connectionDistances[con_id]);
    float4 vel_parallel = dot(velDiff, posDiff) / dot(posDiff, posDiff) * posDiff;
    forceB += stiffness[id] * (actualDistance - restDistance) * posDiff/actualDistance;
    forceD += damping[id] * vel_parallel;
  }
  forceIntern[id] += forceB + forceD;
}

__kernel void integrate(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, __global int* systemIDs, float dt) {
  int id = get_global_id(0);
  float4 gravForce = (float4) (0.0f, 0.0f, 0.0f, 0.0f);
  gravForce.y = -9.81f * mass[id];
  vel[id].xyz += ((forceIntern[id] + gravForce).xyz / mass[id]) * dt;
  pos[id].xyz += vel[id].xyz * dt;
  forceIntern[id] = 0.0f;

  float bounds = 1.0;
  float ybounds = 1.0;
  float damping = 0.9f;
  if (dot(vel[id], up) < 0 && pos[id].y < -ybounds) {
    vel[id] = reflect(vel[id], up) * damping;
  }
  if (dot(vel[id], down) < 0 && pos[id].y > ybounds) {
    vel[id] = reflect(vel[id], down) * damping;
  }
  if (dot(vel[id], right) < 0 && pos[id].x < -bounds) {
    vel[id] = reflect(vel[id], right) * damping;
  }
  if (dot(vel[id], left) < 0 && pos[id].x > bounds) {
    vel[id] = reflect(vel[id], left) * damping;
  }
  if (dot(vel[id], forth) < 0 && pos[id].z < -bounds) {
    vel[id] = reflect(vel[id], forth) * damping;
  }
  if (dot(vel[id], back) < 0 && pos[id].z > bounds) {
    vel[id] = reflect(vel[id], back) * damping;
  }
  pos[id].x = clamp(pos[id].x, -bounds, bounds);
  pos[id].y = clamp(pos[id].y, -ybounds, ybounds);
  pos[id].z = clamp(pos[id].z, -bounds, bounds);
}
