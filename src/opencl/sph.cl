__constant float4 up = (float4)(0.0f, 1.0f, 0.0f, 0.0f);
__constant float4 down = (float4)(0.0f, -1.0f, 0.0f, 0.0f);
__constant float4 left = (float4)(-1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 right = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 forth = (float4)(0.0f, 0.0f, 1.0f, 0.0f);
__constant float4 back = (float4)(0.0f, 0.0f, -1.0f, 0.0f);
__constant float4 gravity = (float4)(0.0f, -9.81f, 0.0f, 0.0f);

typedef struct {
  float mass;
  float density;
  float viscosity;
  float pressure;
  float4 force;
} particle;

float4 reflect(float4 d, float4 n) {
  if (dot(d, n) > 0) {
    d = d*-1.0f;
  }
  float4 r = d - 2.0f*dot(d, n)*n;
  return r;
}

__kernel void fakecollision(__global float4* pos, __global float4* vel, float dt, float particle_amount) {
  unsigned int id = get_global_id(0);
  float4 old = pos[id];
  float radius = 0.1f;
  float bounds = 2.0f;
  /*if (dot(vel[id], up) < 0 && pos[id].y < -bounds) {
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
  }*/
  pos[id] = clamp(old, -bounds, bounds);
}

__kernel void findNeighbors(__global float4* pos, __global int* grid, __global int* counter, float gridRadius, int cellsPerLine, int cellCapacity, __global int* neighbors, __global int* neighborCounter, int neighbor_amount, float h) {
  int id = get_global_id(0);
  neighborCounter[id] = 0;
  int current_x = floor((pos[id].x + gridRadius)/gridRadius * (cellsPerLine / 2.0f));
  int current_y = floor((pos[id].y + gridRadius)/gridRadius * (cellsPerLine / 2.0f));
  int current_z = floor((pos[id].z + gridRadius)/gridRadius * (cellsPerLine / 2.0f));
  for (int x = max(current_x - 1, 0); x <= min(current_x + 1, cellsPerLine-1); x++) {
    int x_counter_offset = x * cellsPerLine * cellsPerLine;
    int x_offset = x_counter_offset * cellCapacity;
    for (int y = max(current_y - 1, 0); y <= min(current_y + 1, cellsPerLine-1); y++) {
      int y_counter_offset = y * cellsPerLine;
      int y_offset = y_counter_offset * cellCapacity;
      for (int z = max(current_z - 1, 0); z <= min(current_z + 1, cellsPerLine-1); z++) {
        int z_offset = z * cellCapacity;
        int n = counter[x_counter_offset + y_counter_offset + z];
        for (int i = 0; i < n; i++) {
          int other = grid[x_offset + y_offset + z_offset + i];
          if (distance(pos[id], pos[other]) <= h && id != other) {
            neighbors[id * neighbor_amount + neighborCounter[id]] = other;
            atomic_inc(&neighborCounter[id]);
            if (neighborCounter[id] >= neighbor_amount) {
              return;
            }
          }
        }
      }
    }
  }
  /*for (int i = 0; i < get_global_size(0); i++) {
    if (distance(pos[id], pos[i]) <= h && id != i) {
      neighbors[id * neighbor_amount + neighborCounter[id]] = i;
      neighborCounter[id]++;
      if (neighborCounter[id] >= neighbor_amount) {
        break;
      }
    }
  }*/
}

float polySix(float h, float4 p, float4 pj) {
  float r = distance(p, pj);
  if (0 <= r && r <= h) {
    float h2 = h*h;
    float r2 = r*r;
    return (h2-r2)*(h2-r2)*(h2-r2);
  }
  return 0.0f;
}

float4 spikyGradient(float h, float4 p, float4 pj) {
  float r = distance(p, pj);
  if (0 <= r && r <= h) {
    return (h - r)*(h - r) * (p - pj) / r;
  }
  return 0.0f;
}

__kernel void calcDensity(__global float4* pos, __global float* density, __global float* pressure, __global float* mass, __global int* neighbors, __global int* neighborCounter, int neighbor_amount, float h, float rest_density) {
  unsigned int id = get_global_id(0);
  float polySix_const = 315.0f/(64.0f * M_PI_F*h*h*h*h*h*h*h*h*h);
  float k = 0.00125f;
  float density_id = 0.0f;
  for (int n = 0; n < neighborCounter[id]; n++) {
    int j = neighbors[id * neighbor_amount + n];
    density_id += mass[j] * polySix(h, pos[id], pos[j]);
    //if (id == 0) {
    ////  //printf("%d -> %f, %f, %f; ", j, pos[j].x, pos[j].y, pos[j].z);
    ////  printf("%d -> %f; ", j, distance(pos[id], pos[j]));
    ////  printf("%f; ", mass[j]);
    //}
  }
  //density_id *= polySix_const;
  density[id] = polySix_const * density_id;
  if (id == 0) {
    printf("%f; ", density[id]);
  }
  pressure[id] = k*(density_id-rest_density);
}

__kernel void calcForces(__global float4* pos, __global float4* force, __global float* pressure, __global float* density, __global float* mass, __global int* neighbors, __global int* neighborCounter, int neighbor_amount, float h, float rest_density) {
  unsigned int id = get_global_id(0);
  float spiky_const = 45.0f / (M_PI_F*h*h*h*h*h*h);
  float4 pressure_force = 0.0f;
  for (int n = 0; n < neighborCounter[id]; n++) {
    int j = neighbors[id * neighbor_amount + n];
    pressure_force += (pressure[id] + pressure[j]) / density[j] * mass[j] * spikyGradient(h, pos[id], pos[j]);
  }
  pressure_force *= -0.5f*spiky_const;
  force[id] = pressure_force;
}

__kernel void integrate(__global float4* pos, __global float4* vel, __global float4* force, __global float* mass, float dt) {
  int id = get_global_id(0);
  vel[id] += force[id] / mass[id] * dt + gravity * dt;
  pos[id] += vel[id] * dt;
}

__kernel void resetGrid(__global int* counter) {
  unsigned int id = get_global_id(0);
  counter[id] = 0;
}

__kernel void updateGrid(__global float4* pos, __global int* grid, __global volatile int* counter, float gridRadius, int cellsPerLine, int cellCapacity) {
  unsigned int id = get_global_id(0);
  int i = floor((pos[id].x + gridRadius)/gridRadius * (cellsPerLine / 2.0f));
  int j = floor((pos[id].y + gridRadius)/gridRadius * (cellsPerLine / 2.0f));
  int k = floor((pos[id].z + gridRadius)/gridRadius * (cellsPerLine / 2.0f));
  volatile int n = atomic_inc(&counter[i * cellsPerLine * cellsPerLine + j * cellsPerLine + k]);
  grid[i * cellsPerLine * cellsPerLine * cellCapacity + j * cellsPerLine * cellCapacity + k * cellCapacity + n] = id;
}

__kernel void printGrid(__global float4* pos, __global int* grid, __global volatile int* counter, float gridRadius, int cellsPerLine, int cellCapacity, __global float4* color) {
  unsigned int id = get_global_id(0);
  int i = floor((pos[id].x + gridRadius)/gridRadius * (cellsPerLine / 2.0f));
  int j = floor((pos[id].y + gridRadius)/gridRadius * (cellsPerLine / 2.0f));
  int k = floor((pos[id].z + gridRadius)/gridRadius * (cellsPerLine / 2.0f));
  color[id].x = i/(float)(cellsPerLine);
  color[id].y = j/(float)(cellsPerLine);
  color[id].z = k/(float)(cellsPerLine);
}
