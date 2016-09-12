__constant float4 up = (float4)(0.0f, 1.0f, 0.0f, 0.0f);
__constant float4 down = (float4)(0.0f, -1.0f, 0.0f, 0.0f);
__constant float4 left = (float4)(-1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 right = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 forth = (float4)(0.0f, 0.0f, 1.0f, 0.0f);
__constant float4 back = (float4)(0.0f, 0.0f, -1.0f, 0.0f);


__constant float smoothingLength = 0.2;
__constant float restDensity = 1000.0;
__constant int neighbor_amount = 30;
__constant float radius = 0.1;

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
    return pow(h - r, 2) * v / r;
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

__kernel void calculateCenterOfMass(__global float4* pos, __global float4* color, __global float4* relativePos, __global float4* quat, __global float* mass,
  __global float4* centerOfMass, __global float* rigidMass, __global float* restTensor, __global int* systemIDs, __global int* systemSizes, __global int* globalIndices) {
  int id = get_global_id(0);
  int g_id = globalIndices[id];
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
      float m = mass[g_s];
      massSum += m;
      com += m * p;
      Ixx += m * (p.y * p.y + p.z * p.z);
      Iyy += m * (p.x * p.x + p.z * p.z);
      Izz += m * (p.x * p.x + p.y * p.y);
      Ixy -= m * (p.x * p.y);
      Ixz -= m * (p.x * p.z);
      Iyz -= m * (p.y * p.z);
    }
    rigidMass[id] = massSum;
    com /= massSum;
    com.w = 1.0;
    centerOfMass[id] = com;
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
    restTensor[id] = A / detA;
    restTensor[id + 1] = D / detA;
    restTensor[id + 2] = G / detA;
    restTensor[id + 3] = B / detA;
    restTensor[id + 4] = E / detA;
    restTensor[id + 5] = H / detA;
    restTensor[id + 6] = C / detA;
    restTensor[id + 7] = F / detA;
    restTensor[id + 8] = I / detA;
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

__kernel void calcCurrentTensor(__global float4* quat, __global float* restTensor, __global float* inertiaTensor, __global int* systemIDs) {
  unsigned int id = get_global_id(0);
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

__kernel void collision(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, __global int* grid,
  volatile __global int* gridCounter, float cellRadius, int cellsinx, int cellsiny, int cellsinz, int cellCapacity, float4 gridMidpoint,
  __global float* springcoefficient, __global float* dampingcoefficient, __global int* systemIDs) {
  int id = get_global_id(0);
  float diam = radius * 2.0f;
  float4 p = pos[id];
  float4 v = vel[id];
  float4 forceSpring = 0.0f;
  float4 forceDamping = 0.0f;
  float4 forceShear = 0.0f;
  float ks = springcoefficient[id];
  float kd = dampingcoefficient[id];
  float kt = kd;
  int sysID = systemIDs[id];
  for (int j = 0; j < get_global_size(0); j++) {
    if (j == id || sysID == systemIDs[j]) {
      continue;
    }
    float dist = distance(pos[id].xyz, pos[j].xyz);
    if (dist < radius * 2.0f) {
      float4 diffPos = pos[j] - p;
      float4 vj = vel[j];
      float4 velDiff = (vj - v);
      float4 n = diffPos / dist;
      forceSpring += -ks * (diam - dist) * n;
      forceDamping += kd * velDiff;
      forceShear += kt * (velDiff - dot(velDiff, n) * n);
    }
  }
  forceIntern[id].xyz += forceSpring.xyz + forceDamping.xyz + forceShear.xyz;
}

__kernel void calcMomenta(__global float4* forceIntern, __global int* systemIDs, __global float4* angularVel, __global float4* centerOfMass,
  __global float* inertiaTensor, __global int* systemSizes, __global float4* forceExtern, __global float* rigidMass, __global float4* relativePos,
  __global float4* momentum, __global float4* quat, __global float4* vel, __global float4* angularMomentum, __global int* globalIndices, float dt) {
  int id = get_global_id(0);
  if (systemIDs[id] == id) {
    float4 q = quat[id];
    float4 negQuat = -q;
    negQuat.w *= -1.0f;
    for (int j = id; j < id + systemSizes[id]; j++) {
      int g_j = globalIndices[j];
      float4 force = forceIntern[g_j];// +forceExtern[g_j];
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
    float4 temp = sin(wlength / 2.0f) * w / wlength;
    float4 unitW = (float4)(temp.x, temp.y, temp.z, cos(wlength / 2.0));
    q += 0.5f * dt * multiplyQuat(angularVel[id], q);
    quat[id] = normalize(q);
    centerOfMass[id] = centerOfMass[id] + momentum[id] / rigidMass[id] * dt;
  }
}

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

__kernel void calcPressure(__global float4* pos, __global int* neighbors, __global int* neighborCounter, __global float* density, __global float* pressure,
  __global float* mass, __global int* globalIndices) {
  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];
  float density_id = 0;
  float pressure_id = 0;
  float k = 1000.0;
  float polySix_const = 315.0f / (64.0f * M_PI_F*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength);
  for (int i = 0; i < neighborCounter[id]; i++) {
    int j = neighbors[id * neighbor_amount + i];
    int g_j = globalIndices[j];
    density_id += mass[g_id] * polySix(smoothingLength, distance(pos[g_id].xyz, pos[g_j].xyz));
  }
  density_id *= polySix_const;
  density[id] = density_id;
  pressure_id = k * (pow((density_id / restDensity), 7) - 1.0);
  pressure[id] = pressure_id;
}

__kernel void calcForces(__global float4* pos, __global int* neighbors, __global int* neighborCounter, __global float* density, __global float* pressure, __global float* mass, __global float4* vel, __global float4* forceIntern, __global int* globalIndices) {
  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];
  float spiky_const = 45.0f / (M_PI_F*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength*smoothingLength);
  float visc_const = 0.2f;
  float4 pressureForce = 0.0f;
  float4 viscosityForce = 0.0f;
  for (int i = 0; i < neighborCounter[id]; i++) {
    int j = neighbors[id * neighbor_amount + i];
    int g_j = globalIndices[j];
    pressureForce += mass[g_j] * (pressure[id] + pressure[j]) / (2.0f * density[j]) * spiky_const * spikyGradient(smoothingLength, pos[g_id], pos[g_j]);
    viscosityForce += mass[g_j] * (vel[g_j] - vel[g_id]) / density[j] * spiky_const * visc(smoothingLength, distance(pos[g_id].xyz, pos[g_j].xyz));
  }
  pressureForce *= -1.0f / density[id];
  viscosityForce *= visc_const;
  
  forceIntern[g_id] += (pressureForce + viscosityForce) * mass[g_id];
}

__kernel void integrateFluid(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, __global int* globalIndices, float dt) {
  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];
  float4 gravForce = 0.0f;
  gravForce.x = -9.81f * mass[id];
  float4 v = vel[g_id];
  float4 p = pos[g_id];
  v += ((forceIntern[g_id] + gravForce) / mass[g_id]) * dt;
  forceIntern[g_id] = 0.0f;
  p += v * dt;
  float bounds = 2.0;
  float ybounds = 2.0;
  if (dot(v, up) < 0 && p.y < -ybounds) {
    v = reflect(v, up);
  }
  if (dot(v, down) < 0 && p.y > ybounds) {
    v = reflect(v, down);
  }
  if (dot(v, right) < 0 && p.x < -bounds) {
    v = reflect(v, right);
  }
  if (dot(v, left) < 0 && p.x > bounds) {
    v = reflect(v, left);
  }
  if (dot(v, forth) < 0 && p.z < -bounds) {
    v = reflect(v, forth);
  }
  if (dot(v, back) < 0 && p.z > bounds) {
    v = reflect(v, back);
  }
  pos[g_id].x = clamp(p.x, -bounds, bounds);
  pos[g_id].y = clamp(p.y, -ybounds, ybounds);
  pos[g_id].z = clamp(p.z, -bounds, bounds);
  vel[g_id] = v;
}

__kernel void integrateRigidBody(__global float4* pos, __global float4* relativePos, __global float* springcoefficient, __global float* dampingcoefficient,
  __global float4* quat, __global float4* momentum, __global float* rigidMass, __global float* mass, __global float4* centerOfMass,
  __global float4* angularVel, __global float4* vel, __global float4* forceIntern, __global int* systemIDs, __global int* globalIndices, float dt) {
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
  float4 v = momentum[sysID] / rigidMass[sysID] + cross(w, r);
  r.w = 1.0f;
  float4 p = 1.0f;
  p.xyz = centerOfMass[sysID].xyz + r.xyz;
  pos[g_id] = p;
  float4 gravForce = (float4) (0.0f, 0.0f, 0.0f, 0.0f);
  gravForce.x = -9.81f * mass[g_id];
  forceIntern[g_id] = gravForce;

  float4 forceSpring = 0.0f;
  float bounds = 2.0;
  float ybounds = 2.0;
  float k = springcoefficient[id];
  float kd = dampingcoefficient[id];
  bool collision = false;
  if (dot(v, up) < 0 && p.y < -ybounds) {
    forceSpring.y += -k * (p.y + ybounds);
    collision = true;
  }
  else if (dot(v, down) < 0 && p.y > ybounds) {
    forceSpring.y += -k * (p.y - ybounds);
    collision = true;
  }
  if (dot(v, right) < 0 && p.x < -bounds) {
    forceSpring.x += -k * (p.x + bounds);
    collision = true;
  }
  else if (dot(v, left) < 0 && p.x > bounds) {
    forceSpring.x += -k * (p.x - bounds);
    collision = true;
  }
  if (dot(v, forth) < 0 && p.z < -bounds) {
    forceSpring.z += -k * (p.z + bounds);
    collision = true;
  }
  else if (dot(v, back) < 0 && p.z > bounds) {
    forceSpring.z += -k * (p.z - bounds);
    collision = true;
  }
  if (collision) {
    forceSpring += -kd * v;
  }
  forceIntern[g_id].xyz += forceSpring.xyz;
  pos[g_id].x = clamp(p.x, -bounds, bounds);
  pos[g_id].y = clamp(p.y, -ybounds, ybounds);
  pos[g_id].z = clamp(p.z, -bounds, bounds);
}
