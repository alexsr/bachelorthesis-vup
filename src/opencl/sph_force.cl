__constant float4 up = (float4)(0.0f, 1.0f, 0.0f, 0.0f);
__constant float4 down = (float4)(0.0f, -1.0f, 0.0f, 0.0f);
__constant float4 left = (float4)(-1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 right = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 forth = (float4)(0.0f, 0.0f, 1.0f, 0.0f);
__constant float4 back = (float4)(0.0f, 0.0f, -1.0f, 0.0f);

__constant float smoothingLength = 0.2;
__constant float restDensity = 1000.0;
__constant int neighbor_amount = 20;

float polySix(float h, float r) {
  if (0 <= r && r <= h) {
    return pow(h*h - r*r, 3);
  }
  else {
    return 0.0;
  }
}

float4 spikyGradient(float h, float4 p, float4 pj) {
  float4 v = p - pj;
  float r = length(v);
  if (0 < r && r <= h) {
    return pow(h - r, 2) * v/r;
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
  float4 r = d - 2.0f*dot(n, d)*n;
  return r;
}
//
//__kernel void resetGrid(__global int* gridCounter) {
//  int id = get_global_id(0);
//  /*if (gridCounter[id] != 0) {
//    printf("c %d = %d; ", id, gridCounter[id]);
//  }*/
//  gridCounter[id] = 0;
//}
//
//__kernel void updateGrid(__global float4* pos, __global int* grid, volatile __global int* gridCounter, float cellRadius, int cellsinx, int cellsiny, int cellsinz, int cellCapacity, float4 gridMidpoint) {
//  int id = get_global_id(0);
//  float xradius = cellRadius * cellsinx;
//  float yradius = cellRadius * cellsiny;
//  float zradius = cellRadius * cellsinz;
//  int i = (pos[id].x - gridMidpoint.x + xradius) / xradius * (cellsinx / 2.0f);
//  int j = (pos[id].y - gridMidpoint.y + yradius) / yradius * (cellsiny / 2.0f);
//  int k = (pos[id].z - gridMidpoint.z + zradius) / zradius * (cellsinz / 2.0f);
//  int gridCounterIndex = i * cellsiny * cellsinz + j * cellsinz + k;
//  if (gridCounter[gridCounterIndex] < cellCapacity) {
//    int n = atomic_inc(&(gridCounter[gridCounterIndex]));
//    grid[i * cellsiny * cellsinz * cellCapacity + j * cellsinz * cellCapacity + k * cellCapacity + n] = id;
//    // int index = i * cellsiny * cellsinz * cellCapacity + j * cellsinz * cellCapacity + k * cellCapacity + 0;
//    // printf("(%f, %f, %f) -> %d, %d, %d -> %d; ", pos[id].x, pos[id].y, pos[id].z, i, j, k, index);
//  }
//}
//
//__kernel void printGrid(__global float4* pos, __global int* grid, volatile __global int* gridCounter, float cellRadius, int cellsinx, int cellsiny, int cellsinz, int cellCapacity, float4 gridMidpoint, __global float4* color) {
//  int id = get_global_id(0);
//  float xradius = cellRadius * cellsinx;
//  float yradius = cellRadius * cellsiny;
//  float zradius = cellRadius * cellsinz;
//  int i = (pos[id].x - gridMidpoint.x + xradius) / xradius * (cellsinx / 2.0f);
//  int j = (pos[id].y - gridMidpoint.y + yradius) / yradius * (cellsiny / 2.0f);
//  int k = (pos[id].z - gridMidpoint.z + zradius) / zradius * (cellsinz / 2.0f);
//  color[id].x = i / (float)(cellsinx);
//  color[id].y = j / (float)(cellsiny);
//  color[id].z = k / (float)(cellsinz);
//}
//
//__kernel void findGridNeighbors(__global float4* pos, __global int* neighbors, __global int* neighborCounter, __global int* grid, volatile __global int* gridCounter, float cellRadius, int cellsinx, int cellsiny, int cellsinz, int cellCapacity, float4 gridMidpoint, __global int* globalIndices) {
//  int id = get_global_id(0);
//  //int g_id = globalIndices[id];
//  float xradius = cellRadius * cellsinx;
//  float yradius = cellRadius * cellsiny;
//  float zradius = cellRadius * cellsinz;
//  int i = (pos[id].x - gridMidpoint.x + xradius) / xradius * (cellsinx / 2.0f);
//  int j = (pos[id].y - gridMidpoint.y + yradius) / yradius * (cellsiny / 2.0f);
//  int k = (pos[id].z - gridMidpoint.z + zradius) / zradius * (cellsinz / 2.0f);
//  neighborCounter[id] = 0;
//  for (int x = 0; x < cellsinx; x++) {
//    int x_counter_offset = x * cellsiny * cellsinz;
//    int x_offset = x_counter_offset * cellCapacity;
//    for (int y = 0; y < cellsiny; y++) {
//      int y_counter_offset = y * cellsinz;
//      int y_offset = y_counter_offset * cellCapacity;
//      for (int z = 0; z < cellsinz; z++) {
//        int z_offset = z * cellCapacity;
//        int n = gridCounter[x_counter_offset + y_counter_offset + z];
//        //printf("%d, %d, %d -> %d; ", x, y, z, n);
//        for (int o = 0; o < n; o++) {
//          int j = neighborCounter[id];
//          if (j < neighbor_amount) {
//            int other = grid[x_offset + y_offset + z_offset + o];
//            float dist = distance(pos[id].xyz, pos[other].xyz);
//            if (dist <= smoothingLength && id != other) {
//              int j_id = id * neighbor_amount + j;
//              neighbors[j_id] = other;
//              neighborCounter[id]++;
//            }
//          }
//        }
//      }
//    }
//  }
//}

 __kernel void findNeighbors(__global float4* pos, __global int* neighbors, __global int* neighborCounter, __global int* globalIndices) {
   unsigned int id = get_global_id(0);
   unsigned int g_id = globalIndices[id];

   float4 p = pos[g_id];

   neighborCounter[id] = 0;
   for (int index = 0; index < get_global_size(0); index++)
   {
     if (distance(p.xyz, pos[globalIndices[index]].xyz) <= smoothingLength)
     {
       neighbors[id * neighbor_amount + neighborCounter[id]] = index;
       neighborCounter[id]++;
     }
     if (neighborCounter[id] >= neighbor_amount - 1)
       break;
   }
 }

__kernel void calcPressure(__global float4* pos, __global int* neighbors, __global int* neighborCounter, __global float* density, __global float* pressure, __global float* mass, __global int* globalIndices) {
  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];
  float density_id = 0;
  float pressure_id = 0;
  float k = 2200.0;
  float polySix_const = 315.0f / (64.0f * M_PI_F*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength);
  for (int i = 0; i < neighborCounter[id]; i++) {
    int j = neighbors[id * neighbor_amount + i];
    int g_j = globalIndices[j];
    density_id += mass[g_id] * polySix(smoothingLength, distance(pos[g_id].xyz, pos[g_j].xyz));
  }
  density_id *= polySix_const;

  density[id] = density_id;
  //pressure_id = k * (density_id - restDensity);
  //printf("id = %d, density = %f;", id, density_id);
  pressure_id = k * (pow((density_id / restDensity), 7) - 1.0);

  pressure[id] = pressure_id;
}

__kernel void calcForces(__global float4* pos, __global int* neighbors, __global int* neighborCounter, __global float* density, __global float* pressure, __global float* mass, __global float4* vel, __global float4* forceIntern, __global int* globalIndices) {
  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];
  float spiky_const = 45.0f / (M_PI_F*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength);
  float visc_const = 0.4f;
  float4 pressureForce = 0.0f;
  float4 viscosityForce = 0.0f;
  for (int i = 0; i < neighborCounter[id]; i++) {
    int j = neighbors[id * neighbor_amount + i];
    int g_j = globalIndices[j];
    //if (density[id] != 0.0 && density[j] != 0.0) {
    pressureForce += mass[g_j] * (pressure[id] + pressure[j]) / (2.0f * density[j]) * spiky_const * spikyGradient(smoothingLength, pos[g_id], pos[g_j]);
    //}
    //if (density[j] != 0.0) {
    viscosityForce += mass[g_j] * (vel[g_j] - vel[g_id]) / density[j] * spiky_const * visc(smoothingLength, distance(pos[g_id].xyz, pos[g_j].xyz));
    //}
  }
  pressureForce *= -1.0f / density[id] * mass[g_id];
  viscosityForce *= visc_const * mass[g_id];

  //if (density[id] != 0.0) {
  forceIntern[g_id] = pressureForce + viscosityForce;
  //}
}

__kernel void integrate(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, float dt, float xleft) {
  unsigned int id = get_global_id(0);
  float4 forceExtern = 0.0f;
  forceExtern.y = -9.81f * mass[id];
 // printf("%f, ", mass[id]);
  vel[id] += ((forceIntern[id] + forceExtern) / mass[id]) * dt;
  pos[id] += vel[id] * dt;
  
  float bounds = 1.0;
  float ybounds = 1.0;
  if (dot(vel[id], up) < 0 && pos[id].y < -ybounds) {
    vel[id] = reflect(vel[id], up);
  }
  if (dot(vel[id], down) < 0 && pos[id].y > ybounds) {
    vel[id] = reflect(vel[id], down);
  }
  if (dot(vel[id], right) < 0 && pos[id].x < -xleft) {
    vel[id] = reflect(vel[id], right);
  }
  if (dot(vel[id], left) < 0 && pos[id].x > bounds) {
    vel[id] = reflect(vel[id], left);
  }
  if (dot(vel[id], forth) < 0 && pos[id].z < -bounds) {
    vel[id] = reflect(vel[id], forth);
  }
  if (dot(vel[id], back) < 0 && pos[id].z > bounds) {
    vel[id] = reflect(vel[id], back);
  }
  pos[id].x = clamp(pos[id].x, -bounds, bounds);
  pos[id].y = clamp(pos[id].y, -ybounds, ybounds);
  pos[id].z = clamp(pos[id].z, -bounds, bounds);
}
