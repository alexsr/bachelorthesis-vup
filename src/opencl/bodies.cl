__constant float4 up = (float4)(0.0f, 1.0f, 0.0f, 0.0f);
__constant float4 down = (float4)(0.0f, -1.0f, 0.0f, 0.0f);
__constant float4 left = (float4)(-1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 right = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
__constant float4 forth = (float4)(0.0f, 0.0f, 1.0f, 0.0f);
__constant float4 back = (float4)(0.0f, 0.0f, -1.0f, 0.0f);


__constant float mediumDensity = 1000.0;
__constant float radius = 0.05;

float4 reflect(float4 d, float4 n) {
  if (dot(d, n) > 0) {
    d = d*-1.0f;
  }
  float4 r = d - 2.0f*dot(n, d)*n;
  return r;
}

__kernel void generateConnectionDistances(__global float4* pos, __global int* connectionCounter, __global int* connections, __global float4* connectionDistances, float maxConnections) {
  int id = get_global_id(0);
  int sysID = 0;
  int sysSize = 27;
  float maxDist = radius * 2.0f * sqrt(3.0f);
  float4 p = pos[id];
  for (int s = 0; s < get_global_size(0); s++) {
    if (connectionCounter[id] > maxConnections) {
      break;
    }
    float dist = distance(p, pos[s]);
    if (dist <= maxDist && id != s) {      
      int con_id = id * maxConnections + connectionCounter[id];
      float4 calcConnection = pos[s] - pos[id];
      connectionDistances[con_id] = calcConnection;
      connectionCounter[id]++;
      connections[con_id] = s;
    }
  }
  
}

__kernel void fakecollision(__global float4* pos, __global float4* vel, __global int* systemIDs) {
  int id = get_global_id(0);
  for (int i = 0; i < id; i++) {
    if (systemIDs[id] != systemIDs[i]) {
    float dist = distance(pos[id], pos[i]);
    if (dist < radius * 2.0f) {
      float4 n = normalize(pos[id] - pos[i]);
      float4 v1 = vel[id];
      float4 v2 = vel[i];
      float a1 = dot(v1, n);
      float a2 = dot(v2, n);
      float m1 = 4.0f;
      float m2 = 4.0f;
      float optimizedP = (2.0f * (a1 - a2)) / (m1 + m2);
      v1 = v1 - optimizedP * m2 * n;
      v2 = v2 + optimizedP * m1 * n;
      vel[id] = v1;
      vel[i] = v2;
      pos[id] += n * (radius * 2.0f - dist)/2.0f;
      pos[i] -= n * (radius * 2.0f - dist)/2.0f;
    }
    }
  }
  for (int i = id+1; i < get_global_size(0); i++) {
    if (systemIDs[id] != systemIDs[i]) {
    float dist = distance(pos[id], pos[i]);
    if (dist < radius * 2.0f) {
      float4 n = normalize(pos[id] - pos[i]);
      float4 v1 = vel[id];
      float4 v2 = vel[i];
      float a1 = dot(v1, n);
      float a2 = dot(v2, n);
      float m1 = 4.0f;
      float m2 = 4.0f;
      float optimizedP = (2.0f * (a1 - a2)) / (m1 + m2);
      v1 = v1 - optimizedP * m2 * n;
      v2 = v2 + optimizedP * m1 * n;
      vel[id] = v1;
      vel[i] = v2;
      pos[id] += n * (radius * 2.0f - dist)/2.0f;
      pos[i] -= n * (radius * 2.0f - dist)/2.0f;
    }
    }
  }
}

__kernel void computeConnectionForces(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, __global float* stiffness, __global float* dampingConstant, __global float* frictionConstant, __global int* connectionCounter, __global int* connections, __global float4* connectionDistances, float maxConnections, float dt) {
  int id = get_global_id(0);
  float4 forceB = 0.0f;
  float4 forceD = 0.0f;
  float4 forceF = 0.0f;
  //printf("%d -> %d; ", id, connectionCounter[id]);
  for (int i = 0; i < connectionCounter[id]; i++) {
    int con_id = id * maxConnections + i;
    int e_id = connections[con_id];
    float4 posDiff = pos[e_id] - pos[id];
    // float4 posDiff = pos[id] - pos[e_id];
    float4 velDiff = vel[e_id] - vel[id];
    // float4 velDiff = vel[id] - vel[e_id];
    float actualDistance = length(posDiff);
    float restDistance = length(connectionDistances[con_id]);
    float4 vel_parallel = dot(velDiff, posDiff) / dot(posDiff, posDiff) * posDiff;
    // float4 vel_ortho = velDiff - vel_parallel;
    forceB += stiffness[id] * (actualDistance - restDistance) * posDiff/actualDistance;
    forceD += dampingConstant[id] * vel_parallel;
    // forceB += stiffness[id] * (posDiff - restDistance * posDiff/actualDistance);
    // forceD += (dt * stiffness[id] /restDistance + dampingConstant[id]) * dot(velDiff, posDiff/actualDistance) * posDiff/actualDistance;
   // forceB += stiffness[id] / dot(connectionDistances[con_id], connectionDistances[con_id]) * (actualDistance - length(connectionDistances[con_id])) * posDiff/actualDistance;
    // forceD += dampingConstant[id]  / dot(connectionDistances[con_id], connectionDistances[con_id]) * velDiff;
  }
  float4 forceN = forceB + forceD;
  for (int i = 0; i < connectionCounter[id]; i++) {
    int con_id = id * maxConnections + i;
    int e_id = connections[con_id];
    float4 posDiff = pos[e_id] - pos[id];
    float4 velDiff = vel[e_id] - vel[id];
    float actualDistance = length(posDiff);
    float4 vel_parallel = dot(velDiff, posDiff) / dot(posDiff, posDiff) * posDiff;
    float4 vel_ortho = -velDiff + vel_parallel;
    forceF -= vel_ortho;
  }
  forceIntern[id] = forceN + frictionConstant[id] * length(forceN) * normalize(forceF);
 // printf("%d -> %f, %f, %f, %f; ", id, forceIntern[id].x, forceIntern[id].y, forceIntern[id].z, forceIntern[id].w);
}

__kernel void integrate(__global float4* pos, __global float4* vel, __global float* mass, __global float4* forceIntern, __global int* systemIDs, float dt, float sign) {
  int id = get_global_id(0);
  float4 forceExtern = (float4) (0.0f, 0.0f, 0.0f, 0.0f);
  forceExtern.y = -9.81f * mass[id] * 1;
  //if (id != 27) {
    
    //forceExtern.xyz += -M_PI_F * radius * radius * mediumDensity * length(vel[id].xyz) * vel[id].xyz;
    vel[id].xyz += ((forceIntern[id] + forceExtern).xyz / mass[id]) * dt;
    pos[id].xyz += vel[id].xyz * dt;    
  //}
  
  float bounds = 1.0;
  float ybounds = 1.0;
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

