﻿__constant float4 up = (float4)(0.0f, 1.0f, 0.0f, 0.0f);
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
  float4 r = d - 2.0f*dot(d, n)*n;
  return r;
}

__kernel void resetGrid(__global int* counter) {
  int id = get_global_id(0);
  /*if (counter[id] != 0) {
    printf("c %d = %d; ", id, counter[id]);
  }*/
  counter[id] = 0;
}

__kernel void updateGrid(__global float4* pos, __global int* grid, volatile __global int* counter, float gridRadius, int cellsPerLine, int cellCapacity) {
  int id = get_global_id(0);
  int i = floor((pos[id].x + gridRadius) / gridRadius * ((cellsPerLine - 1) / 2.0f));
  int j = floor((pos[id].y + gridRadius) / gridRadius * ((cellsPerLine - 1) / 2.0f));
  int k = floor((pos[id].z + gridRadius) / gridRadius * ((cellsPerLine - 1) / 2.0f));
  int n = atomic_inc(&(counter[i * cellsPerLine * cellsPerLine + j * cellsPerLine + k]));
  grid[i * cellsPerLine * cellsPerLine * cellCapacity + j * cellsPerLine * cellCapacity + k * cellCapacity + n] = id;
}

__kernel void printGrid(__global float4* pos, __global int* grid, volatile __global int* counter, float gridRadius, int cellsPerLine, int cellCapacity, __global float4* color) {
  if (get_global_id(0) == 0) {
    for (int id = 0; id < get_global_size(0); id++) {
      int i = floor((pos[id].x + gridRadius) / gridRadius * (cellsPerLine / 2.0f));
      int j = floor((pos[id].y + gridRadius) / gridRadius * (cellsPerLine / 2.0f));
      int k = floor((pos[id].z + gridRadius) / gridRadius * (cellsPerLine / 2.0f));
      //printf("\nid %d\n", id);
      //printf("cell %d, %d, %d;\n------\n", i, j, k);
      //printf("others in the cell:\n");
      volatile int n = counter[i * cellsPerLine * cellsPerLine + j * cellsPerLine + k];
      for (int o = 0; o < n; o++) {
        int other = grid[i * cellsPerLine * cellsPerLine * cellCapacity + j * cellsPerLine * cellCapacity + k * cellCapacity + o];
        //printf("%d; ", other);
      }
    }
  }
  //unsigned int id = get_global_id(0);
  ///*color[id].x = i / (float)(cellsPerLine);
  //color[id].y = j / (float)(cellsPerLine);
  //color[id].z = k / (float)(cellsPerLine);*/
  //if (id == 0) {
  /*}*/
}

__kernel void findNeighbors(__global float4* pos, __global int* neighbors, __global int* neighborCounter) {
  /*int id = get_global_id(0);
  neighborCounter[id] = 0;
  int current_x = floor((pos[id].x + gridRadius) / gridRadius * ((cellsPerLine - 1) / 2.0f));
  int current_y = floor((pos[id].y + gridRadius) / gridRadius * ((cellsPerLine - 1) / 2.0f));
  int current_z = floor((pos[id].z + gridRadius) / gridRadius * ((cellsPerLine - 1) / 2.0f));
  for (int x = 0; x < cellsPerLine; x++) {
    int x_counter_offset = x * cellsPerLine * cellsPerLine;
    int x_offset = x_counter_offset * cellCapacity;
    for (int y = 0; y < cellsPerLine; y++) {
      int y_counter_offset = y * cellsPerLine;
      int y_offset = y_counter_offset * cellCapacity;
      for (int z = 0; z < cellsPerLine; z++) {
        int z_offset = z * cellCapacity;
        int n = counter[x_counter_offset + y_counter_offset + z];
        for (int i = 0; i < n; i++) {
          int other = grid[x_offset + y_offset + z_offset + i];
          if (distance(pos[id].xyz, pos[other].xyz) <= h && id != other) {
            neighbors[id * neighbor_amount + neighborCounter[id]] = other;
            neighborCounter[id] = neighborCounter[id] + 1;
          }
          if (neighborCounter[id] >= neighbor_amount - 1)
            break;
        }
        if (neighborCounter[id] >= neighbor_amount - 1)
          break;
      }
      if (neighborCounter[id] >= neighbor_amount - 1)
        break;
    }
    if (neighborCounter[id] >= neighbor_amount - 1)
      break;
  }*/

  unsigned int i = get_global_id(0);

  float4 p = pos[i];

  neighborCounter[i] = 0;
  //save neighbours of THIS particle in an array %
  //array size is 50(n) times bigger than pos[]
  for (int index = 0; index < get_global_size(0); index++)
  {
    if (distance(p.xyz, pos[index].xyz) <= smoothingLength) // < smoothingLength
    {
      neighbors[i * neighbor_amount + neighborCounter[i]] = index;
      neighborCounter[i]++;
      //only saves values with distance < smoothing Lenght --> [0,smoothingLength]
    }
    //stop when 50(n) neighbours of i are found
    if (neighborCounter[i] >= neighbor_amount - 1)
      break;
  }
}

__kernel void calcPressure(__global float4* pos, __global int* neighbors, __global int* neighborCounter, __global float* density, __global float* pressure, __global float* mass) {
  unsigned int id = get_global_id(0);
  float density_id = 0;
  float pressure_id = 0;
  float k = 2000.0;
  float polySix_const = 315.0f / (64.0f * M_PI_F*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength);
  for (int i = 0; i < neighborCounter[id]; i++) {
    int j = neighbors[id * neighbor_amount + i];
    density_id += polySix(smoothingLength, distance(pos[id].xyz, pos[j].xyz));
  }
  density_id *= polySix_const * mass[id];

  density[id] = density_id;
  //pressure_id = k * (density_id - restDensity);
  pressure_id = k * (pow((density_id / restDensity), 7) - 1.0);

  pressure[id] = pressure_id;
}

__kernel void calcForces(__global float4* pos, __global int* neighbors, __global int* neighborCounter, __global float* density, __global float* pressure, __global float* mass, __global float4* vel, __global float4* forceIntern) {
  unsigned int id = get_global_id(0);
  float spiky_const = 45.0f / (M_PI_F*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength);
  float visc_const = 0.02f;
  float4 pressureForce = 0.0f;
  float4 viscosityForce = 0.0f;
  for (int i = 0; i < neighborCounter[id]; i++) {
    int j = neighbors[id * neighbor_amount + i];
    //if (density[id] != 0.0 && density[j] != 0.0) {
      pressureForce += mass[j] * (pressure[id] + pressure[j]) / (2.0f * density[j]) * spikyGradient(smoothingLength, pos[id], pos[j]);
    //}
    //if (density[j] != 0.0) {
      viscosityForce += mass[j] * (vel[j] - vel[id]) / density[j] * visc(smoothingLength, distance(pos[id].xyz, pos[j].xyz));
    //}
  }
  pressureForce *= -mass[id] * spiky_const;
  viscosityForce *= visc_const * spiky_const;

  //if (density[id] != 0.0) {
  forceIntern[id] = pressureForce + viscosityForce;
  forceIntern[id] /= density[id];
  //}
}

__kernel void integrate(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, float dt, float xleft) {
  unsigned int id = get_global_id(0);
  float4 forceExtern = 0.0f;
  forceExtern.y = -9.81f * mass[id];
 // printf("%f, ", mass[id]);
  vel[id] += ((forceIntern[id] + forceExtern) / mass[id]) * dt;
  pos[id] += vel[id] * dt;
  
  float bounds = 2.0;
  float ybounds = 3.0;
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
  pos[id].x = clamp(pos[id].x, -xleft, bounds);
  pos[id].y = clamp(pos[id].y, -ybounds, ybounds);
  pos[id].z = clamp(pos[id].z, -bounds, bounds);
}