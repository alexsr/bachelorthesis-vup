__constant float4 up = (float4)(0.0f, 1.0f, 0.0f, 0.0f);
__constant float4 down = (float4)(0.0f, -1.0f, 0.0f, 0.0f);
__constant float4 left = (float4)(-1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 right = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 forth = (float4)(0.0f, 0.0f, 1.0f, 0.0f);
__constant float4 back = (float4)(0.0f, 0.0f, -1.0f, 0.0f);

__constant float radius = 0.1;
__constant float smoothingLength = 0.2;
__constant float restDensity = 1000.0;
__constant int neighbor_amount = 30;
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


struct mat3 {
  float3 x;
  float3 y;
  float3 z;
};

struct mat3 multiplyMat(struct mat3 m1, struct mat3 m2) {
  float3 m2x = (float3) (m2.x.x, m2.y.x, m2.z.x);
  float3 m2y = (float3) (m2.x.y, m2.y.y, m2.z.y);
  float3 m2z = (float3) (m2.x.z, m2.y.z, m2.z.z);
  struct mat3 m;
  m.x.x = dot(m2x, m1.x);
  m.x.y = dot(m2y, m1.x);
  m.x.z = dot(m2z, m1.x);
  m.y.x = dot(m2x, m1.y);
  m.y.y = dot(m2y, m1.y);
  m.y.z = dot(m2z, m1.y);
  m.z.x = dot(m2x, m1.z);
  m.z.y = dot(m2y, m1.z);
  m.z.z = dot(m2z, m1.z);
  return m;
}

float4 multiplyMatVec(struct mat3 m, float4 v) {
  float4 result = 0.0f;
  result.x = dot(m.x, v.xyz);
  result.y = dot(m.y, v.xyz);
  result.z = dot(m.z, v.xyz);
  return result;
}

float4 multiplyQuat(float4 q1, float4 q2) {
  float w1 = q1.w;
  float x1 = q1.x;
  float y1 = q1.y;
  float z1 = q1.z;
  float w2 = q2.w;
  float x2 = q2.x;
  float y2 = q2.y;
  float z2 = q2.z;
  float4 q = 0.0f;
  q.w = w1*w2 - x1*x2 - y1*y2 - z1*z2;
  q.x = w1*x2 + w2*x1 + y1*z2 - z1*y2;
  q.y = w1*y2 + w2 * y1 + z1 * x2 - x1*z2;
  q.z = w1*z2 + w2*z1 + x1*y2 - y1*x2;
  return q;
}

__kernel void calculateCenterOfMass(__global float4* pos, __global float4* color, __global float4* relativePos, __global float4* quat, __global float* mass, __global float4* centerOfMass, __global float* rigidMass, __global float* restTensor, __global int* systemIDs, __global int* systemSizes, __global int* globalIndices) {
  int id = get_global_id(0);
  if (id == systemIDs[id]) {
    float4 com = (float4)(0.0f, 0.0f, 0.0f, 1.0f);
    float massSum = 0.0f;
    float Ixx = 0.0f;
    float Iyy = 0.0f;
    float Izz = 0.0f;
    float Ixy = 0.0f;
    float Ixz = 0.0f;
    float Iyz = 0.0f;
    for (int s = id; s < id + systemSizes[id]; s++) {
      int g_s = globalIndices[s];
      float4 p = pos[g_s];
      massSum += mass[g_s];
      com += mass[g_s] * p;
      Ixx += mass[g_s] * (p.y * p.y + p.z * p.z);
      Iyy += mass[g_s] * (p.x * p.x + p.z * p.z);
      Izz += mass[g_s] * (p.x * p.x + p.y * p.y);
      Ixy -= mass[g_s] * (p.x * p.y);
      Ixz -= mass[g_s] * (p.x * p.z);
      Iyz -= mass[g_s] * (p.y * p.z);
    }
    rigidMass[id] = massSum;
    com /= massSum;
    com.w = 1.0;
    centerOfMass[id] = com;
    float A = (Iyy * Izz - Iyz * Iyz);
    float B = -(Ixy * Izz - Iyz * Ixz);
    float C = (Ixy * Iyz - Iyy * Ixz);
    float D = -(Ixy * Izz - Ixz * Iyz);
    float E = (Ixx * Izz - Ixz * Ixz);
    float F = -(Ixx * Iyz - Ixy * Ixz);
    float G = (Ixy * Iyz - Ixz * Iyy);
    float H = -(Ixx * Iyz - Ixz * Ixy);
    float I = (Ixx * Iyy - Ixy * Ixy);
    float detA = Ixx * A + Ixy * B + Ixz * C;
    A /= detA;
    B /= detA;
    C /= detA;
    D /= detA;
    E /= detA;
    F /= detA;
    G /= detA;
    H /= detA;
    I /= detA;
    restTensor[id] = A;
    restTensor[id + 1] = D;
    restTensor[id + 2] = G;
    restTensor[id + 3] = B;
    restTensor[id + 4] = E;
    restTensor[id + 5] = H;
    restTensor[id + 6] = C;
    restTensor[id + 7] = F;
    restTensor[id + 8] = I;
    float4 negQuat = -quat[id];
    negQuat.w *= -1.0f;
    for (int j = id; j < id + systemSizes[id]; j++) {
      int g_j = globalIndices[j];
      relativePos[j] = pos[g_j] - com;
      float4 rj = relativePos[j];
      rj.w = 0.0f;
      rj.xyz = multiplyQuat(multiplyQuat(quat[id], rj), negQuat).xyz;
      rj.w = 1.0f;
      pos[g_j].xyz = com.xyz + rj.xyz;
      if (length(rj.xyz) <= 0.1) {
        color[g_j] = (float4)(1, 0, 0, 0);
      }
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
  int i = (pos[g_id].x - gridMidpoint.x + xradius) / xradius * (cellsinx / 2.0f);
  int j = (pos[g_id].y - gridMidpoint.y + yradius) / yradius * (cellsiny / 2.0f);
  int k = (pos[g_id].z - gridMidpoint.z + zradius) / zradius * (cellsinz / 2.0f);
  float4 p = pos[g_id];
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

__kernel void calcPressure(__global float4* pos, __global int* neighbors, __global int* neighborCounter, __global float* density, __global float* pressure, __global float* mass, __global int* globalIndices) {
  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];
  float density_id = mass[g_id] * polySix(smoothingLength, 0.0);
  float pressure_id = 0;
  float k = 1000.0;
  for (int i = 0; i < neighborCounter[id]; i++) {
    int j = neighbors[id * neighbor_amount + i];
    int g_j = globalIndices[j];
    density_id += mass[g_j] * polySix(smoothingLength, distance(pos[g_id].xyz, pos[g_j].xyz));
  }
  density_id *= polySixConst;

  density[id] = density_id;
  //pressure_id = k * (density_id - restDensity);
  pressure_id = k * (pow((density_id / restDensity), 7) - 1.0);

  pressure[id] = pressure_id;
}

__kernel void calcForces(__global float4* pos, __global int* neighbors, __global int* neighborCounter, __global float* density, __global float* pressure, __global float* mass, __global float4* vel, __global float4* forceIntern, __global int* globalIndices) {
  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];
  float visc_const = 0.2f;
  float4 pressureForce = 0.0f;
  float4 viscosityForce = 0.0f;
  for (int i = 0; i < neighborCounter[id]; i++) {
    int j = neighbors[id * neighbor_amount + i];
    int g_j = globalIndices[j];
    pressureForce += mass[g_j] * (pressure[id] + pressure[j]) / (2.0f * density[j]) * spikyGradient(smoothingLength, pos[g_id], pos[g_j]);
    viscosityForce += mass[g_j] * (vel[g_j] - vel[g_id]) / density[j] * visc(smoothingLength, distance(pos[g_id].xyz, pos[g_j].xyz));
  }
  pressureForce *= -1.0f / density[id] * spikyConst;
  viscosityForce *= visc_const * spikyConst;

  forceIntern[g_id].xyz += (pressureForce + viscosityForce).xyz * mass[g_id];
}
__kernel void calcCurrentTensor(__global float4* pos, __global float4* vel, __global float4* forceIntern, __global float* mass, __global float4* quat, __global float* restTensor, __global float* inertiaTensor, __global int* systemIDs, __global int* globalIndices, __global float* springcoefficient, __global float* dampingcoefficient) {
  unsigned int id = get_global_id(0);
  int g_id = globalIndices[id];
  float4 forceSpring = 0.0f;
  float bounds = 2.0;
  float ybounds = 4.0;
  float k = springcoefficient[g_id];
  float kd = dampingcoefficient[g_id];
  bool collision = false;
  if (dot(vel[g_id], up) < 0 && pos[g_id].y < -bounds) {
    forceSpring.y += -k * (pos[g_id].y + bounds);
    collision = true;
  }
  else if (dot(vel[g_id], down) < 0 && pos[g_id].y > ybounds) {
    forceSpring.y += -k * (pos[g_id].y - ybounds);
    collision = true;
  }
  if (dot(vel[g_id], right) < 0 && pos[g_id].x < -bounds) {
    forceSpring.x += -k * (pos[g_id].x + bounds);
    collision = true;
  }
  else if (dot(vel[g_id], left) < 0 && pos[g_id].x > bounds) {
    forceSpring.x += -k * (pos[g_id].x - bounds);
    collision = true;
  }
  if (dot(vel[g_id], forth) < 0 && pos[g_id].z < -bounds) {
    forceSpring.z += -k * (pos[g_id].z + bounds);
    collision = true;
  }
  else if (dot(vel[g_id], back) < 0 && pos[g_id].z > bounds) {
    forceSpring.z += -k * (pos[g_id].z - bounds);
    collision = true;
  }
  if (collision) {
    //forceSpring += -kd * vel[g_id];
  }
  float4 gravForce = 0.0f;
  gravForce.y = -9.81f * mass[g_id];
  forceIntern[g_id].xyz = gravForce.xyz + forceSpring.xyz;
  if (id == systemIDs[id]) {
    float4 q = quat[id];
    float R00 = 1.0 - 2 * q.y * q.y - 2 * q.z * q.z;
    float R01 = 2 * q.x * q.y - 2 * q.w * q.z;
    float R02 = 2 * q.x * q.z + 2 * q.w * q.y;
    float R10 = 2 * q.x*q.y + 2 * q.w*q.z;
    float R11 = 1 - 2 * q.x*q.x - q.z*q.z;
    float R12 = 2 * q.y*q.z - 2 * q.w*q.x;
    float R20 = 2 * q.x*q.z - 2 * q.w*q.y;
    float R21 = 2 * q.y*q.z + 2 * q.w*q.x;
    float R22 = 1 - 2 * q.x*q.x - 2 * q.y*q.y;
    struct mat3 r;
    r.x = (float3)(R00, R01, R02);
    r.y = (float3)(R10, R11, R12);
    r.z = (float3)(R20, R21, R22);
    struct mat3 rt;
    rt.x = (float3)(R00, R10, R20);
    rt.y = (float3)(R01, R11, R21);
    rt.z = (float3)(R02, R12, R22);
    float I00 = restTensor[id];
    float I01 = restTensor[id + 1];
    float I02 = restTensor[id + 2];
    float I10 = restTensor[id + 3];
    float I11 = restTensor[id + 4];
    float I12 = restTensor[id + 5];
    float I20 = restTensor[id + 6];
    float I21 = restTensor[id + 7];
    float I22 = restTensor[id + 8];
    struct mat3 tensor;
    tensor.x = (float3)(I00, I01, I02);
    tensor.y = (float3)(I10, I11, I12);
    tensor.z = (float3)(I20, I21, I22);
    tensor = multiplyMat(r, multiplyMat(tensor, rt));
    inertiaTensor[id] = tensor.x.x;
    inertiaTensor[id + 1] = tensor.x.y;
    inertiaTensor[id + 2] = tensor.x.z;
    inertiaTensor[id + 3] = tensor.y.x;
    inertiaTensor[id + 4] = tensor.y.y;
    inertiaTensor[id + 5] = tensor.y.z;
    inertiaTensor[id + 6] = tensor.z.x;
    inertiaTensor[id + 7] = tensor.z.y;
    inertiaTensor[id + 8] = tensor.z.z;
  }
}

__kernel void collisionNoGrid(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, __global int* systemIDs,__global float* springcoefficient, __global float* dampingcoefficient) {
  int id = get_global_id(0);
  float m = mass[id];
  float4 v = vel[id];
  float4 p = pos[id];
  float4 forceSpring = 0.0f;
  float4 forceDamping = 0.0f;
  float4 forceShear = 0.0f;
  float diam = 2.0f * radius;
  for (int j = 0; j < get_global_size(0); j++) {
    float dist = distance(p.xyz, pos[j].xyz);
    if (systemIDs[id] != systemIDs[j] && dist < radius * 2.0f && id != j) {
      float ks = springcoefficient[j];
      float kd = dampingcoefficient[j];
      float4 diffPos = pos[j] - p;
      float4 vj = vel[j];
      float4 velDiff = (vj - v);
      float4 n = diffPos / dist;
      forceSpring += -ks * (diam - dist) * n;
      forceDamping += kd * velDiff;
      forceShear += 0.0f * (velDiff - dot(velDiff, n) * n);
    }
  }
  float4 gravForce = 0.0f;
  gravForce.y = -9.81f * mass[id];
  forceIntern[id].xyz += gravForce.xyz + forceSpring.xyz + forceDamping.xyz + forceShear.xyz;
}

__kernel void collision(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, __global int* grid, volatile __global int* gridCounter, float cellRadius, int cellsinx, int cellsiny, int cellsinz, int cellCapacity, float4 gridMidpoint, __global int* systemIDs, __global float* springcoefficient, __global float* dampingcoefficient) {
  int id = get_global_id(0);
  float4 p = pos[id];
  float4 v = vel[id];
  float4 forceSpring = 0.0f;
  float4 forceDamping = 0.0f;
  float4 forceShear = 0.0f;
  float diam = 2.0f * radius;
  float ks = springcoefficient[id];
  float kd = dampingcoefficient[id];
  float xradius = cellRadius * cellsinx;
  float yradius = cellRadius * cellsiny;
  float zradius = cellRadius * cellsinz;
  int i = (p.x - gridMidpoint.x + xradius) / xradius * (cellsinx / 2.0f);
  int j = (p.y - gridMidpoint.y + yradius) / yradius * (cellsiny / 2.0f);
  int k = (p.z - gridMidpoint.z + zradius) / zradius * (cellsinz / 2.0f);
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
          if (systemIDs[other] != systemIDs[id] && dist < diam && id != other) {
            float4 diffPos = pos[other] - p;
            float4 vj = vel[other];
            float4 velDiff = (vj - v);
            float4 n = diffPos / dist;
            forceSpring += -ks * (diam - dist) * n;
            forceDamping += kd * velDiff;
            forceShear += 0.0f * (velDiff - dot(velDiff, n) * n);
          }
        }
      }
    }
  }
  float4 gravForce = 0.0f;
  gravForce.y = -9.81f * mass[id];
  forceIntern[id].xyz += gravForce.xyz + forceSpring.xyz + forceDamping.xyz + forceShear.xyz;
}
__kernel void collisionNonrigid(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, __global int* grid, volatile __global int* gridCounter, float cellRadius, int cellsinx, int cellsiny, int cellsinz, int cellCapacity, float4 gridMidpoint, __global int* systemIDs) {
  float dt = 0.001;
  int id = get_global_id(0);
  float xradius = cellRadius * cellsinx;
  float yradius = cellRadius * cellsiny;
  float zradius = cellRadius * cellsinz;
  int i = (pos[id].x - gridMidpoint.x + xradius) / xradius * (cellsinx / 2.0f);
  int j = (pos[id].y - gridMidpoint.y + yradius) / yradius * (cellsiny / 2.0f);
  int k = (pos[id].z - gridMidpoint.z + zradius) / zradius * (cellsinz / 2.0f);
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
          float dist = distance(pos[id].xyz, pos[other].xyz);
          if (systemIDs[other] != systemIDs[id] && dist < radius * 2.0f && id != other) {
            float4 n = normalize(pos[id] - pos[other]);
            float4 v1 = vel[id];
            float4 v2 = vel[other];
            float m1 = mass[id];
            float m2 = mass[other];
            float jr = -2.0 * dot(v2 - v1, n) / (1.0/m1 + 1.0/m2);
            forceIntern[id] += -jr * n / dt;
            //forceIntern[other] = jr * n / dt;
            pos[id] += n * (radius * 2.0f - dist)/2.0f;
            //pos[other] -= n * (radius * 2.0f - dist)/2.0f;
          }
        }
      }
    }
  }
}

__kernel void calcMomenta(__global float4* forceIntern, __global int* systemIDs, __global float4* angularVel, __global float4* centerOfMass, __global float* inertiaTensor, __global int* systemSizes, __global float4* forceExtern, __global float* rigidMass, __global float4* relativePos, __global float4* momentum, __global float4* quat, __global float4* vel, __global float4* angularMomentum, __global int* globalIndices, float dt) {
  int id = get_global_id(0);
  if (systemIDs[id] == id) {
    float4 q = quat[id];
    float4 negQuat = -q;
    negQuat.w *= -1.0f;
    for (int j = id; j < id + systemSizes[id]; j++) {
      int g_j = globalIndices[j];
      float4 force = forceIntern[g_j];
      momentum[id] += force * dt;
      float4 r = relativePos[j];
      r.w = 0.0f;
      r.xyz = multiplyQuat(multiplyQuat(q, r), negQuat).xyz;
      angularMomentum[id] += cross(r, force) * dt;
      r.w = 1.0f;
    }
    float I00 = inertiaTensor[id];
    float I01 = inertiaTensor[id + 1];
    float I02 = inertiaTensor[id + 2];
    float I10 = inertiaTensor[id + 3];
    float I11 = inertiaTensor[id + 4];
    float I12 = inertiaTensor[id + 5];
    float I20 = inertiaTensor[id + 6];
    float I21 = inertiaTensor[id + 7];
    float I22 = inertiaTensor[id + 8];
    struct mat3 tensor;
    tensor.x = (float3)(I00, I01, I02);
    tensor.y = (float3)(I10, I11, I12);
    tensor.z = (float3)(I20, I21, I22);
    float4 w = multiplyMatVec(tensor, angularMomentum[id]);
    w.w = 0.0f;
    angularVel[id] = w;
    w = w * dt;
    float wlength = length(w);
    float4 temp = sin(wlength/2.0f) * w/wlength;
    float4 unitW = (float4)(temp.x, temp.y, temp.z, cos(wlength/2.0));
    q += 0.5f * dt * multiplyQuat(angularVel[id], q);
    quat[id] = normalize(q);
    centerOfMass[id] = centerOfMass[id] + momentum[id] / rigidMass[id] * dt;
  }
}

__kernel void integrateFluid(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, __global int* globalIndices, float dt) {
  unsigned int id = get_global_id(0);
  int g_id = globalIndices[id];
  float4 forceExtern = 0.0f;
  forceExtern.z = 4.0f * mass[g_id];
  float4 gravForce = 0.0f;
  gravForce.y = -9.81f * mass[g_id];
  vel[g_id].xyz += ((forceIntern[g_id] + gravForce + forceExtern).xyz / mass[g_id]) * dt;
  pos[g_id] += vel[g_id] * dt;
  float4 p = pos[g_id];
  float bounds = 2.0;
  float ybounds = 4.0;
  float damping = 0.9;
  if (dot(vel[g_id], up) < 0 && p.y < -bounds) {
    vel[g_id] = reflect(vel[g_id], up) * damping;
  }
  if (dot(vel[g_id], down) < 0 && p.y > ybounds) {
    vel[g_id] = reflect(vel[g_id], down) * damping;
  }
  if (dot(vel[g_id], right) < 0 && p.x < -bounds) {
    vel[g_id] = reflect(vel[g_id], right) * damping;
  }
  if (dot(vel[g_id], left) < 0 && p.x > bounds) {
    vel[g_id] = reflect(vel[g_id], left) * damping;
  }
  if (dot(vel[g_id], forth) < 0 && p.z < -bounds) {
    vel[g_id] = reflect(vel[g_id], forth) * damping;
  }
  if (dot(vel[g_id], back) < 0 && p.z > bounds) {
    vel[g_id] = reflect(vel[g_id], back) * damping;
  }
  forceIntern[g_id] = 0.0f;
  pos[g_id].x = clamp(pos[g_id].x, -bounds, bounds);
  pos[g_id].y = clamp(pos[g_id].y, -bounds, ybounds);
  pos[g_id].z = clamp(pos[g_id].z, -bounds, bounds);
}

__kernel void integrateRigidBody(__global float4* pos, __global float4* relativePos, __global float* springcoefficient, __global float* dampingcoefficient, __global float4* quat, __global float4* momentum, __global float* rigidMass, __global float* mass, __global float4* centerOfMass, __global float4* angularVel, __global float4* vel, __global float4* forceIntern, __global int* systemIDs, __global int* globalIndices, float dt) {
  int id = get_global_id(0);
  int g_id = globalIndices[id];
  int sysID = systemIDs[id];
  float4 w = angularVel[sysID];
  float4 q = quat[sysID];
  float4 negQuat = -q;
  negQuat.w *= -1.0f;

  float4 r = relativePos[id];
  r.w = 0.0f;
  r.xyz = multiplyQuat(multiplyQuat(q, r), negQuat).xyz;
  r.w = 1.0f;
  pos[g_id].xyz = centerOfMass[sysID].xyz + r.xyz;
  vel[g_id] = momentum[sysID]/rigidMass[sysID] + cross(w, r);
}
