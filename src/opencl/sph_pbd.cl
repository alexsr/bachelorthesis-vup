__constant float4 up = (float4)(0.0f, 1.0f, 0.0f, 0.0f);
__constant float4 down = (float4)(0.0f, -1.0f, 0.0f, 0.0f);
__constant float4 left = (float4)(-1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 right = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 forth = (float4)(0.0f, 0.0f, 1.0f, 0.0f);
__constant float4 back = (float4)(0.0f, 0.0f, -1.0f, 0.0f);

__constant float smoothingLength = 0.2;
__constant float restDensity = 1000.0;
__constant int neighbor_amount = 20;
__constant float polySixConst = 3059924.7482;
__constant float spikyConst = 223811.6387;

float polySix(float h, float r) {
  if (0 <= r && r <= h) {
    return pow(h*h - r*r, 3);
  }
  else {
    return 0.0;
  }
}

float4 spikyGradient(float h, float4 p, float4 pj) {
  float4 v = 0.0f;
  v.xyz = p.xyz - pj.xyz;
  float r = length(v);
  if (0 < r && r <= h) {
    return -pow(h - r, 2) * v/r;
  }
  else {
    return 0;
  }
}

float visc(float h, float r) {
  if (0 <= r && r <= h) {
    return h - r;
  }
  else {
    return 0;
  }
}

float4 reflect(float4 d, float4 n) {
  if (dot(d, n) > 0) {
    d = d*-1.0f;
  }
  float4 r = d - 2.0f*dot(d, n)*n;
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

__kernel void predict(__global float4* pos, __global float4* next, __global float4* vel, __global float* mass, float dt, float sign) {
  unsigned int id = get_global_id(0);
  float4 grav = (float4) (0.0, -9.81, 0.0, 0.0);
  vel[id] = vel[id] + dt * grav * 1;
  next[id] = pos[id] + vel[id] * dt;
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

__kernel void calcLambda(__global float4* next, __global int* neighbors, __global int* neighborCounter, __global float* density, __global float* lambda, __global float* mass) {
  unsigned int id = get_global_id(0);
  float pressure_id = 0;
  float polySix_const = 315.0f / (64.0f * M_PI_F*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength);
  float spiky_const = 45.0f / (M_PI_F*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength);
  float grad_sum_k = 0.0f;
  float4 grad_sum_k_i = 0.0f;
  float density_id = 0.0f;
  for (int i = 0; i < neighborCounter[id]; i++) {
    int j = neighbors[id * neighbor_amount + i];
    float4 grad = spiky_const * spikyGradient(smoothingLength, next[id], next[j]);
    grad_sum_k += length(grad/restDensity) * length(grad/restDensity);
    grad_sum_k_i += grad;
    density_id += mass[j] * polySix_const * polySix(smoothingLength, distance(next[id].xyz, next[j].xyz));
  }
  grad_sum_k += length(grad_sum_k_i/restDensity) * length(grad_sum_k_i/restDensity);
  density[id] = density_id;
  float C = max(density[id] / restDensity - 1.0f, 0.0f);
  if(C != 0.0f) {
    float eps = 0.0001;
    lambda[id] = -C / (grad_sum_k + eps);
   }
   else {
    lambda[id] = 0.0f;
   }
}

__kernel void calcDelta(__global float4* pos, __global float4* next, __global int* neighbors, __global int* neighborCounter, __global float* lambda, __global float* mass) {
  unsigned int id = get_global_id(0);
  float spiky_const = 45.0f / (M_PI_F*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength);
  float polySix_const = 315.0f / (64.0f * M_PI_F*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength);
  float4 posDelta = 0.0f;
  float n = 0.1f;
  float k = 0.001;
  for (int i = 0; i < neighborCounter[id]; i++) {
    int j = neighbors[id * neighbor_amount + i];
    float corr = -k * pow((polySix(smoothingLength, distance(next[id].xyz, next[j].xyz)))/(polySix(smoothingLength, 0.2 * smoothingLength)), n);
    posDelta += (lambda[id] + lambda[j] + corr) * (spiky_const * spikyGradient(smoothingLength, next[id], next[j]));
  }
  next[id].xyz += posDelta.xyz / restDensity;
}

__kernel void integrate(__global float4* pos, __global float4* next, __global float4* vel, __global float* mass, float dt) {
  unsigned int id = get_global_id(0);
  vel[id] = (next[id] - pos[id])/dt;
  vel[id].w = 0.0f;
  pos[id] = next[id];

  float bounds = 2.0;
  float ybounds = 2.0;
  if (dot(vel[id], up) < 0 && pos[id].y < -ybounds) {
    vel[id] = reflect(vel[id], up);
    vel[id] *= 0.8f;
  }
  if (dot(vel[id], down) < 0 && pos[id].y > ybounds) {
    vel[id] = reflect(vel[id], down);
    vel[id] *= 0.8f;
  }
  if (dot(vel[id], right) < 0 && pos[id].x < -bounds) {
    vel[id] = reflect(vel[id], right);
    vel[id] *= 0.8f;
  }
  if (dot(vel[id], left) < 0 && pos[id].x > bounds) {
    vel[id] = reflect(vel[id], left);
    vel[id] *= 0.8f;
  }
  if (dot(vel[id], forth) < 0 && pos[id].z < -bounds) {
    vel[id] = reflect(vel[id], forth);
    vel[id] *= 0.8f;
  }
  if (dot(vel[id], back) < 0 && pos[id].z > bounds) {
    vel[id] = reflect(vel[id], back);
    vel[id] *= 0.8f;
  }
  pos[id].x = clamp(pos[id].x, -bounds, bounds);
  pos[id].y = clamp(pos[id].y, -ybounds, ybounds);
  pos[id].z = clamp(pos[id].z, -bounds, bounds);
}

__kernel void viscosity(__global float4* pos, __global float4* next, __global float4* vel, __global int* neighborCounter, __global int* neighbors) {
  unsigned int id = get_global_id(0);
  float4 velsum = (0, 0, 0, 0);
  float spiky_const = 45.0f / (M_PI_F*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength);
  float c = 0.01;
  for (int i = 0; i < neighborCounter[id]; i++) {
    int j = neighbors[id * neighbor_amount + i];
    velsum += (vel[id] - vel[j]) * spiky_const * polySix(smoothingLength, distance(pos[id].xyz, pos[j].xyz));
  }
  velsum *= c;
}
