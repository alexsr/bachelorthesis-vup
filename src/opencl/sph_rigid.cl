__constant float4 up = (float4)(0.0f, 1.0f, 0.0f, 0.0f);
__constant float4 down = (float4)(0.0f, -1.0f, 0.0f, 0.0f);
__constant float4 left = (float4)(-1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 right = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 forth = (float4)(0.0f, 0.0f, 1.0f, 0.0f);
__constant float4 back = (float4)(0.0f, 0.0f, -1.0f, 0.0f);

__constant float smoothingLength = 0.2;
__constant float restDensity = 1000.0;
__constant int neighbor_amount = 20;


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

__kernel void findNeighbors(__global float4* pos, __global int* neighbors, __global int* neighborCounter, __global int* globalIndices) {
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

  unsigned int id = get_global_id(0);
  unsigned int g_id = globalIndices[id];

  float4 p = pos[g_id];

  neighborCounter[id] = 0;
  //save neighbours of THIS particle in an array %
  //array size is 50(n) times bigger than pos[]
  for (int index = 0; index < get_global_size(0); index++)
  {
    if (distance(p.xyz, pos[globalIndices[index]].xyz) <= smoothingLength) // < smoothingLength
    {
      neighbors[id * neighbor_amount + neighborCounter[id]] = index;
      neighborCounter[id]++;
      //only saves values with distance < smoothing Lenght --> [0,smoothingLength]
    }
    //stop when 50(n) neighbours of i are found
    if (neighborCounter[id] >= neighbor_amount - 1)
      break;
  }
}


__kernel void generateConnectionDistances(__global float4* pos, __global int* connectionCounter, __global int* connections, __global float4* connectionDistances, __global int* systemIDs, float maxConnections) {
  int id = get_global_id(0);
  float maxDist = radius * 2.0f * sqrt(3.0f);
  float4 p = pos[id];
  for (int s = 0; s < get_global_size(0); s++) {
    if (systemIDs[id] != systemIDs[s] || id == s) {
      continue;
    }
    if (connectionCounter[id] > maxConnections) {
      break;
    }
    float dist = distance(p, pos[s]);
    if (dist <= maxDist) {      
      int con_id = id * maxConnections + connectionCounter[id];
      float4 calcConnection = pos[s] - pos[id];
      connectionDistances[con_id] = calcConnection;
      connectionCounter[id]++;
      connections[con_id] = s;
    }
  }
}

__kernel void calculateCenterOfMass(__global float4* pos, __global float4* color, __global float4* relativePos, __global float4* quat, __global float* mass, __global float4* centerOfMass, __global float* rigidMass, __global float* inertiaTensor, __global int* systemIDs, __global int* systemSizes, __global int* globalIndices) {
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
    inertiaTensor[id] = A;
    inertiaTensor[id + 1] = D;
    inertiaTensor[id + 2] = G;
    inertiaTensor[id + 3] = B;
    inertiaTensor[id + 4] = E;
    inertiaTensor[id + 5] = H;
    inertiaTensor[id + 6] = C;
    inertiaTensor[id + 7] = F;
    inertiaTensor[id + 8] = I;
    float4 negQuat = -quat[id];
    negQuat.w *= -1.0f;
    centerOfMass[id] = (float4)(0.0, 3.0, 0.0, 1.0);
    for (int j = id; j < id + systemSizes[id]; j++) {
      int g_j = globalIndices[j];
      relativePos[j] = pos[g_j] - com;
      float4 rj = relativePos[j];
      rj.w = 0.0f;
      rj.xyz = multiplyQuat(multiplyQuat(quat[id], rj), negQuat).xyz;
      rj.w = 1.0f;
      pos[g_j].xyz = centerOfMass[id].xyz + rj.xyz;
      if (length(rj.xyz) <= 0.001) {
        color[g_j] = (float4)(1, 0, 0, 0);
      }
    }
  }
}

__kernel void rigidCollision(__global float4* pos, __global float4* vel, __global float4* forceIntern, __global float* mass, __global int* systemIDs, __global float* springcoefficient, __global float* dampingcoefficient, float dt) {
  int id = get_global_id(0);
  forceIntern[id] = 0.0f;
  float diam = radius * 2.0f;
  float4 p = pos[id];
  float4 v = vel[id];
  float m1 = mass[id];
  int sysID = systemIDs[id];
  for (int j = 0; j < get_global_size(0); j++) {
    if (j == id) {
      continue;
    }
    if (sysID != systemIDs[j]) {
      float dist = distance(pos[id], pos[j]);
      float s = max(0.0f, diam - dist);
      if (dist < diam) {
        float4 n = normalize(pos[id] - pos[j]);
        float4 v2 = vel[j];
        float m2 = mass[j];
        float jr = -1.0 * dot(v2 - v, n) / (1.0 / m1 + 1.0 / m2);
        //v1 = v1 - jr / m1 * n;
        //v2 = v2 + jr / m2 * n;
        /*vel[id] = v1;
        vel[other] = v2;*/
        forceIntern[id] = -jr * n / dt;
        forceIntern[j] = jr * n / dt;
        p += n * (radius * 2.0f - dist) / 2.0f;
        pos[id] = p;
        pos[j] -= n * (radius * 2.0f - dist) / 2.0f;
        //printf("%d and %d collided! dist %f sysIDs: %d, %d; ", id, j, dist, sysID, systemIDs[j]);
      }
    }
  }
}

__kernel void rigidBounds(__global float4* pos, __global float4* vel, __global float4* forceIntern, __global float* springcoefficient, __global float* dampingcoefficient, __global int* globalIndices) {
  int id = get_global_id(0);
  id = globalIndices[id];
  float4 forceSpring = 0.0f;
  float4 forceDamping = 0.0f;
  float k = springcoefficient[id];
  float kd = dampingcoefficient[id];
  float bounds = 1.0 - radius;
  float ybounds = 2.0 - radius;
  bool collision = false;
  if (dot(vel[id], up) < 0 && pos[id].y < -ybounds) {
    forceSpring.y += -k * (pos[id].y + ybounds);
    collision = true;
  }
  else if (dot(vel[id], down) < 0 && pos[id].y > ybounds) {
    forceSpring.y += -k * (pos[id].y - ybounds);
    collision = true;
  }
  if (dot(vel[id], right) < 0 && pos[id].x < -bounds) {
    forceSpring.x += -k * (pos[id].x + bounds);
    collision = true;
  }
  else if (dot(vel[id], left) < 0 && pos[id].x > bounds) {
    forceSpring.x += -k * (pos[id].x - bounds);
    collision = true;
  }
  if (dot(vel[id], forth) < 0 && pos[id].z < -bounds) {
    forceSpring.z += -k * (pos[id].z + bounds);
    collision = true;
  }
  else if (dot(vel[id], back) < 0 && pos[id].z > bounds) {
    forceSpring.z += -k * (pos[id].z - bounds);
    collision = true;
  }
  if (collision) {
    forceDamping = -kd * vel[id];
  }
  forceIntern[id] += forceDamping + forceSpring;
}

__kernel void calcMomenta(__global float4* forceIntern, __global int* systemIDs, __global float4* angularVel, __global float* inertiaTensor, __global int* systemSizes, __global float4* forceExtern, __global float* rigidMass, __global float4* relativePos, __global float4* momentum, __global float4* quat, __global float4* angularMomentum, __global float4* centerOfMass, __global int* globalIndices, float dt) {
  int id = get_global_id(0);
  int g_id = globalIndices[id];
  if (systemIDs[id] == id) {
    float4 gravForce = (float4) (0.0f, 0.0f, 0.0f, 0.0f);
      gravForce.y = -9.81f * rigidMass[id] * 1;
    
    for (int j = id; j < id + systemSizes[id]; j++) {
      int g_j = globalIndices[j];
      float4 force = forceIntern[g_j] + forceExtern[g_j] + gravForce;
      momentum[id] += force * dt;
      angularMomentum[id] += cross(relativePos[j], force) * dt;
    }

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
    float I00 = inertiaTensor[id];
    float I01 = inertiaTensor[id + 1];
    float I02 = inertiaTensor[id + 2];
    float I10 = inertiaTensor[id + 3];
    float I11 = inertiaTensor[id + 4];
    float I12 = inertiaTensor[id + 5];
    float I20 = inertiaTensor[id + 6];
    float I21 = inertiaTensor[id + 7];
    float I22 = inertiaTensor[id + 8];
    struct mat3 tensorNull;
    tensorNull.x = (float3)(I00, I01, I02);
    tensorNull.y = (float3)(I10, I11, I12);
    tensorNull.z = (float3)(I20, I21, I22);
    float4 w = multiplyMatVec(multiplyMat(r, multiplyMat(tensorNull, rt)), angularMomentum[id]);
    w.w = 0.0f;
    angularVel[id] = w;
    w = w * dt;
    float wlength = length(w);
    float4 temp = sin(wlength/2.0f) * w/wlength;
    float4 unitW = (float4)(temp.x, temp.y, temp.z, cos(wlength/2.0));
    quat[id] =multiplyQuat(unitW, quat[id]);
    float4 v = momentum[id] / rigidMass[id];
    centerOfMass[id] = centerOfMass[id] + v * dt;
  }
}

__kernel void integrateRigidBody(__global float4* pos, __global float4* relativePos, __global float4* quat, __global float4* momentum, __global float4* angularMomentum, __global float* rigidMass, __global float4* centerOfMass, __global float4* angularVel, __global float4* vel, __global int* systemIDs, __global int* systemSizes, __global int* globalIndices, float dt) {
  int id = get_global_id(0);
  int g_id = globalIndices[id];
  int sysID = systemIDs[id];
  float4 v = momentum[sysID] / rigidMass[sysID];
  float4 w = angularVel[sysID];
  float4 q = quat[sysID];
  //printf("id %d rot:\n(%f, %f, %f)\n(%f, %f, %f)\n(%f, %f, %f)\n", id, r.x.x, r.x.y, r.x.z, r.y.x, r.y.y, r.y.z, r.z.x, r.z.y, r.z.z);
  
  // printf("id %d quat: (%f, %f, %f, %f)\n", id, quat[id].w, quat[id].x, quat[id].y, quat[id].z);
  float4 negQuat = -q;
  negQuat.w *= -1.0f;
  
  float4 r = relativePos[id];
  r.w = 0.0f;
  r.xyz = multiplyQuat(multiplyQuat(q, r), negQuat).xyz;
  r.w = 1.0f;
  pos[g_id].xyz = centerOfMass[sysID].xyz + r.xyz;
  vel[g_id] = v + cross(w, r);
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
  forceIntern[g_id] += pressureForce + viscosityForce;
  //}
}

__kernel void integrateFluid(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, float dt, float xleft, __global int* globalIndices) {
  unsigned int id = get_global_id(0);
  id = globalIndices[id];
  float4 forceExtern = 0.0f;
  forceExtern.y = -9.81f * mass[id];
 // printf("%f, ", mass[id]);
  vel[id] += ((forceIntern[id] + forceExtern) / mass[id]) * dt;
  pos[id] += vel[id] * dt;
  float4 p = pos[id];
  float4 v = vel[id];
  float m1 = mass[id];
  float bounds = 1.0 - radius;
  float ybounds = 1.0 - radius;
  if (dot(v, up) < 0 && p.y < -ybounds) {
    vel[id] = reflect(v, up);
  }
  if (dot(v, down) < 0 && p.y > ybounds) {
    vel[id] = reflect(v, down);
  }
  if (dot(v, right) < 0 && p.x < -bounds) {
    vel[id] = reflect(v, right);
  }
  if (dot(v, left) < 0 && p.x > bounds) {
    vel[id] = reflect(v, left);
  }
  if (dot(v, forth) < 0 && p.z < -bounds) {
    vel[id] = reflect(v, forth);
  }
  if (dot(v, back) < 0 && p.z > bounds) {
    vel[id] = reflect(v, back);
  }
  pos[id].x = clamp(p.x, -bounds, bounds);
  pos[id].y = clamp(p.y, -ybounds, ybounds);
  pos[id].z = clamp(p.z, -bounds, bounds);
}
