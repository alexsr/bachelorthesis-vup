__constant float4 up = (float4)(0.0f, 1.0f, 0.0f, 0.0f);
__constant float4 down = (float4)(0.0f, -1.0f, 0.0f, 0.0f);
__constant float4 left = (float4)(-1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 right = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 forth = (float4)(0.0f, 0.0f, 1.0f, 0.0f);
__constant float4 back = (float4)(0.0f, 0.0f, -1.0f, 0.0f);


__constant float radius = 0.05;

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

float4 reflect(float4 d, float4 n) {
  if (dot(d, n) > 0) {
    d = d*-1.0f;
  }
  float4 r = d - 2.0f*dot(n, d)*n;
  return r;
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

__kernel void calculateCenterOfMass(__global float4* pos, __global float4* color, __global float4* relativePos, __global float4* quat, __global float* mass, __global float4* centerOfMass, __global float* rigidMass, __global float* inertiaTensor, __global int* systemIDs, __global int* systemSizes) {
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
      float4 p = pos[s];
      massSum += mass[s];
      com += mass[s] * p;
      Ixx += mass[s] * (p.y * p.y + p.z * p.z);
      Iyy += mass[s] * (p.x * p.x + p.z * p.z);
      Izz += mass[s] * (p.x * p.x + p.y * p.y);
      Ixy -= mass[s] * (p.x * p.y);
      Ixz -= mass[s] * (p.x * p.z);
      Iyz -= mass[s] * (p.y * p.z);
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
    for (int j = id; j < id + systemSizes[id]; j++) {
      relativePos[j] = pos[j] - com;
      float4 rj = relativePos[j];
      rj.w = 0.0f;
      rj.xyz = multiplyQuat(multiplyQuat(quat[id], rj), negQuat).xyz;
      rj.w = 1.0f;
      pos[j].xyz = com.xyz + rj.xyz;
      if (length(rj.xyz) <= 0.1) {
        color[j] = (float4)(1, 0, 0, 0);
      }
    }
  }
}

__kernel void rigidCollision(__global float4* pos, __global float4* vel, __global float4* forceIntern, __global int* systemIDs, __global float* springcoefficient, __global float* dampingcoefficient) {
  int id = get_global_id(0);
  float diam = radius * 2.0f;
  float4 p = pos[id];
  float4 v = vel[id];
  float4 forceSpring = 0.0f;
  float4 forceDamping = 0.0f;
  float4 forceShear = 0.0f;
  float4 forceNormal = 0.0f;
  float4 forceTan = 0.0f;
  int sysID = systemIDs[id];
  float k = springcoefficient[id];
  float kd = dampingcoefficient[id];
  float alpha = 0.5;
  float beta = 1;
  float mu = 0.5;
  float kt = kd;
  float bounds = 1.0 - radius;
  float ybounds = 1.0 - radius;
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
  for (int j = 0; j < get_global_size(0); j++) {
    if (j == id) {
      continue;
    }
    if (sysID != systemIDs[j]) {
      float dist = distance(pos[id], pos[j]);
      float s = max(0.0f, diam - dist);
      if (dist < diam - 0.0001) {
        float4 diffPos = pos[j] - p;
        float4 vj = vel[j];
        float4 velDiff = (vj - v);
        float4 normPosDiff = diffPos / dist;
        float4 n = normPosDiff;
        float relativeVel = dot(velDiff, n);
        float fn = -kd * relativeVel * pow(s, alpha) - k * pow(s, beta);
        float4 tanVel = velDiff - relativeVel * n;
        forceNormal += fn * n;
        forceTan += -min(mu * fn, kt * length(tanVel)) * tanVel / length(tanVel);
        //printf("%d and %d collided! ", id, j);
      }
    }
  }
  forceIntern[id].xyz = forceSpring.xyz + forceDamping.xyz + forceShear.xyz + forceNormal.xyz + forceTan.xyz;
}

__kernel void calcMomenta(__global float4* forceIntern, __global int* systemIDs, __global float4* angularVel, __global float* inertiaTensor, __global int* systemSizes, __global float4* forceExtern, __global float* rigidMass, __global float4* relativePos, __global float4* momentum, __global float4* quat, __global float4* angularMomentum, float dt) {
  int id = get_global_id(0);
  if (systemIDs[id] == id) {
    float4 gravForce = (float4) (0.0f, 0.0f, 0.0f, 0.0f);
      gravForce.y = -9.81f * rigidMass[id] * 1;
    
    for (int j = id; j < id + systemSizes[id]; j++) {
      float4 force = forceIntern[j] + forceExtern[j] + gravForce;
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
    quat[id] = multiplyQuat(unitW, quat[id]);
  }
}

__kernel void integrateRigidBody(__global float4* pos, __global float4* relativePos, __global float4* quat, __global float4* momentum, __global float4* angularMomentum, __global float* rigidMass, __global float4* centerOfMass, __global float4* angularVel, __global float4* vel, __global int* systemIDs, __global int* systemSizes, float dt) {
  int id = get_global_id(0);
  int sysID = systemIDs[id];
  float4 v = momentum[sysID] / rigidMass[sysID];
  centerOfMass[sysID] = centerOfMass[sysID] + v * dt;
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
  pos[id].xyz = centerOfMass[sysID].xyz + r.xyz;
  vel[id] = v + cross(w, r);

}

__kernel void integrate(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, __global int* systemIDs, float dt, float sign) {
  int id = get_global_id(0);
  float4 gravForce = (float4) (0.0f, -9.81f * mass[id] * 1, 0.0f, 0.0f);
  //if (id != 27) {
    
    //forceExtern.xyz += -M_PI_F * radius * radius * mediumDensity * length(vel[id].xyz) * vel[id].xyz;
    vel[id].xyz += ((forceIntern[id] + gravForce).xyz / mass[id]) * dt;
    pos[id].xyz += vel[id].xyz * dt;    
  //}
    /*float bounds = 1.0 - radius;
    float ybounds = 1.0 - radius;
    if (dot(vel[id], up) < 0 && pos[id].y < -ybounds) {
      forceSpring.y += -k * 0.1 * (pos[id].y + ybounds);
      forceDamping.y += damping * vel[id].y;
    }
    else if (dot(vel[id], down) < 0 && pos[id].y > ybounds) {
      forceSpring.y += -k * 0.1 * (pos[id].y - ybounds);
      forceDamping.y += damping * vel[id].y;
    }
    if (dot(vel[id], right) < 0 && pos[id].x < -bounds) {
      forceSpring.x += -k * 0.1 * (pos[id].x + bounds);
      forceDamping.x += damping * vel[id].x;
    }
    else if (dot(vel[id], left) < 0 && pos[id].x > bounds) {
      forceSpring.x += -k * 0.1 * (pos[id].x - bounds);
      forceDamping.x += damping * vel[id].x;
    }
    if (dot(vel[id], forth) < 0 && pos[id].z < -bounds) {
      forceSpring.z += -k * 0.1 * (pos[id].z + bounds);
      forceDamping.z += damping * vel[id].z;
    }
    else if (dot(vel[id], back) < 0 && pos[id].z > bounds) {
      forceSpring.z += -k * 0.1 * (pos[id].z - bounds);
      forceDamping.z += damping * vel[id].z;
    }*/
  float bounds = 1.0 - radius;
  float ybounds = 1.0 - radius;
  if (dot(vel[id], up) < 0 && pos[id].y < -ybounds) {
    vel[id] = reflect(vel[id], up);
  }
  else if (dot(vel[id], down) < 0 && pos[id].y > ybounds) {
    vel[id] = reflect(vel[id], down);
  }
  if (dot(vel[id], right) < 0 && pos[id].x < -bounds) {
    vel[id] = reflect(vel[id], right);
  }
  else if (dot(vel[id], left) < 0 && pos[id].x > bounds) {
    vel[id] = reflect(vel[id], left);
  }
  if (dot(vel[id], forth) < 0 && pos[id].z < -bounds) {
    vel[id] = reflect(vel[id], forth);
  }
  else if (dot(vel[id], back) < 0 && pos[id].z > bounds) {
    vel[id] = reflect(vel[id], back);
  }
  pos[id].x = clamp(pos[id].x, -bounds, bounds);
  pos[id].y = clamp(pos[id].y, -ybounds, ybounds);
  pos[id].z = clamp(pos[id].z, -bounds, bounds);
}

