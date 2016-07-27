__constant float4 up = (float4)(0.0f, 1.0f, 0.0f, 0.0f);
__constant float4 down = (float4)(0.0f, -1.0f, 0.0f, 0.0f);
__constant float4 left = (float4)(-1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 right = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 forth = (float4)(0.0f, 0.0f, 1.0f, 0.0f);
__constant float4 back = (float4)(0.0f, 0.0f, -1.0f, 0.0f);

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

__kernel void findNeighbors(__global float4* pos, __global int* grid, __global int* counter, float gridRadius, int cellsPerLine, int cellCapacity,
  __global int* neighbors, __global int* neighborCounter, int neighbor_amount, float h) {
  int id = get_global_id(0);
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
  }
  /*if (neighborCounter[id] != 0) {
    //printf("c %d = %d; ", id, neighborCounter[id]);
  }*/

  //unsigned int i = get_global_id(0);
  //printf("n = %d;", neighborCounter[i]);

  //float4 p = pos[i];

  //neighborCounter[i] = 0;
  ////save neighbours of THIS particle in an array 
  ////array size is 50(n) times bigger than pos[]
  //for (int index = 0; index < get_global_size(0); index++)
  //{
  //  if (distance(p.xyz, pos[index].xyz) <= h) // < smoothingLength
  //  {
  //    neighbors[i * neighbor_amount + neighborCounter[i]] = index;
  //    neighborCounter[i]++;
  //    //only saves values with distance < smoothing Lenght --> [0,smoothingLength]
  //  }
  //  //stop when 50(n) neighbours of i are found
  //  if (neighborCounter[i] >= neighbor_amount - 1)
  //    break;
  //}
}

__kernel void calcPressure(__global float4* pos, __global int* neighbor, __global int* counter, __global float* density, __global float* pressure,
  __global float* mass, float h, int neighbor_amount, float defaultDensity) {
  unsigned int id = get_global_id(0);
  float density_id = 0;
  float pressure_id = 0;
  float k = .04030142;
  float k2 = 0.5301f;
  float polySix_const = 315.0f / (64.0f * M_PI_F*h*h*h*h*h*h*h*h*h);

  for (int i = 0; i < counter[id]; i++) {
    int j = neighbor[id * neighbor_amount + i];
    density_id += polySix(h, distance(pos[id].xyz, pos[j].xyz));
  }
  density_id *= polySix_const * mass[id];

  density[id] = density_id;
  pressure_id = k * defaultDensity / k2 * (pow((density_id / defaultDensity), k2) - 1.0f);

  pressure[id] = pressure_id;
}

__kernel void calcForces(__global float4* pos, __global int* neighbor, __global int* counter, __global float* density, __global float* pressure,
  __global float* mass, float h, int neighbor_amount, __global float4* vel, __global float4* forceIntern) {
  unsigned int id = get_global_id(0);
  float spiky_const = 45.0f / (M_PI_F*h*h*h*h*h*h);
  float visc_const = 0.000005;
  float4 pressureForce = 0.0f;
  float4 viscosityForce = 0.0f;

  for (int i = 0; i < counter[id]; i++) {
    int j = neighbor[id * neighbor_amount + i];
    //if (density[id] != 0.0 && density[j] != 0.0) {
      pressureForce += mass[j] * (pressure[id] / (density[id] * density[id]) + pressure[j] / (density[j] * density[j])) * spikyGradient(h, pos[id], pos[j]);
    //}
    //if (density[j] != 0.0) {
      viscosityForce += (vel[j] - vel[id]) / density[j] * visc(h, distance(pos[id].xyz, pos[j].xyz));
    //}
  }
  pressureForce *= -mass[id] * spiky_const;
  viscosityForce *= visc_const * mass[id] * spiky_const;

  //if (density[id] != 0.0) {
    forceIntern[id] = pressureForce + viscosityForce;
    forceIntern[id] /= density[id];
  //}
}

__kernel void integrate(__global float4* pos, __global float4* vel, __global float* density, __global float* mass, __global float4* force, float defaultDensity, float dt) {
  unsigned int id = get_global_id(0);

  float4 forceExtern;
  forceExtern.y = -9.81 * mass[id];

  vel[id] += ((force[id] + forceExtern) / mass[id]) * dt;
  pos[id] += vel[id] * dt;
  
  float bounds = 1.0;
  if (dot(vel[id], up) < 0 && pos[id].y < -bounds) {
    vel[id] = reflect(vel[id], up);
  }
  if (dot(vel[id], down) < 0 && pos[id].y > bounds) {
    vel[id] = reflect(vel[id], down);
  }
  if (dot(vel[id], right) < 0 && pos[id].x < -bounds) {
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
  pos[id] = clamp(pos[id], -bounds, bounds);
}
