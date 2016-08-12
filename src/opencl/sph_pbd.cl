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

__kernel void predict(__global float4* pos, __global float4* next, __global float4* vel, __global float* mass, float dt, float sign) {
  unsigned int id = get_global_id(0);
  float4 grav = (float4) (0.0, -9.81, 0.0, 0.0);
  vel[id] = vel[id] + dt * grav * 1;
  next[id] = pos[id] + vel[id] * dt;
}

__kernel void findNeighbors(__global float4* next, __global int* neighbors, __global int* neighborCounter) {
  /*int id = get_global_id(0);
  neighborCounter[id] = 0;
  int current_x = floor((next) / gridRadius * ((cellsPerLine - 1) / 2.0f));
  int current_y = floor((next[id].y + gridRadius) / gridRadius * ((cellsPerLine - 1) / 2.0f));
  int current_z = floor((next[id].z + gridRadius) / gridRadius * ((cellsPerLine - 1) / 2.0f));
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

  float4 p = next[i];

  neighborCounter[i] = 0;
  //save neighbours of THIS particle in an array %
  //array size is 50(n) times bigger than pos[]
  for (int index = 0; index < get_global_size(0); index++)
  {
    if (distance(p.xyz, next[index].xyz) <= smoothingLength) // < smoothingLength
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
  // if (id == 0) {
    // printf("%d -> %f;", id, density_id);
  // }
  //float C = density[id] / restDensity - 1.0f;
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
  float n = 4.0f;
  float k = 0.001;
  for (int i = 0; i < neighborCounter[id]; i++) {
    int j = neighbors[id * neighbor_amount + i];
    //if (density[id] != 0.0 && density[j] != 0.0) {
      float corr = -k * pow((polySix(smoothingLength, distance(next[id].xyz, next[j].xyz)))/(polySix(smoothingLength, 0.2 * smoothingLength)), n);
      posDelta += (lambda[id] + lambda[j] + corr) * (spiky_const * spikyGradient(smoothingLength, next[id], next[j]));
    //}
  }
  next[id].xyz += posDelta.xyz / restDensity;
}

__kernel void integrate(__global float4* pos, __global float4* next, __global float4* vel, __global float* mass, float dt, float xleft) {
  unsigned int id = get_global_id(0);
 // printf("%f, ", mass[id]);
  vel[id] = (next[id] - pos[id])/dt;
  vel[id].w = 0.0f;
  // float velclamp = 0.0001;
  // vel[id].x = clamp(vel[id].x, -velclamp, velclamp);
  // vel[id].y = clamp(vel[id].y, -velclamp, velclamp);
  // vel[id].z = clamp(vel[id].z, -velclamp, velclamp);
  pos[id] = next[id];
  
  float bounds = 2.0;
  float ybounds = 5.0;
  if (dot(vel[id], up) < 0 && pos[id].y < -ybounds) {
    vel[id] = reflect(vel[id], up);
  }
  if (dot(vel[id], down) < 0 && pos[id].y > ybounds) {
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
  pos[id].x = clamp(pos[id].x, -bounds, bounds);
  pos[id].y = clamp(pos[id].y, -ybounds, ybounds);
  pos[id].z = clamp(pos[id].z, -bounds, bounds);
}

__kernel void viscosity(__global float4* pos, __global float4* next, __global float4* vel, __global int* neighborCounter, __global int* neighbors) {
  unsigned int id = get_global_id(0);
  // printf("%f, ", mass[id]);
  float4 velsum = (0, 0, 0, 0);
  float spiky_const = 45.0f / (M_PI_F*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength);
  float c = 0.01;
  for (int i = 0; i < neighborCounter[id]; i++) {
    int j = neighbors[id * neighbor_amount + i];
    velsum += (vel[id] - vel[j]) * spiky_const * polySix(smoothingLength, distance(pos[id].xyz, pos[j].xyz));
  }
  velsum *= c;
  //vel[id] += velsum;
}
