float pVarPoly(float h, float r)
{
  if (0 <= r && r <= h)
    return pow(h*h - r*r, 3);
  else return 0;
}

float4 pVarSpiky(float h, float4 p, float4 pn)
{
  float4 r = p - pn;
  float r_length = length(r);
  if (0< r_length && r_length <= h)
    return  r / r_length * pow(h - r_length, 2);
  else return 0;
}

float pVarVisc(float h, float4 p, float4 pn)
{
  float r_length = length(p - pn);
  if (0 <= r_length && r_length < h)
    return h - r_length;
  else return 0;
}

__kernel void resetGrid(__global int* counter) {
  unsigned int id = get_global_id(0);
  counter[id] = 0;
}

__kernel void updateGrid(__global float4* pos, __global int* grid, __global volatile int* counter, float gridRadius, int cellsPerLine, int cellCapacity) {
  unsigned int id = get_global_id(0);
  int i = floor((pos[id].x + gridRadius) / gridRadius * (cellsPerLine / 2.0f));
  int j = floor((pos[id].y + gridRadius) / gridRadius * (cellsPerLine / 2.0f));
  int k = floor((pos[id].z + gridRadius) / gridRadius * (cellsPerLine / 2.0f));
  volatile int n = atomic_inc(&counter[i * cellsPerLine * cellsPerLine + j * cellsPerLine + k]);
  grid[i * cellsPerLine * cellsPerLine * cellCapacity + j * cellsPerLine * cellCapacity + k * cellCapacity + n] = id;
}

__kernel void printGrid(__global float4* pos, __global int* grid, __global volatile int* counter, float gridRadius, int cellsPerLine, int cellCapacity, __global float4* color) {
  unsigned int id = get_global_id(0);
  int i = floor((pos[id].x + gridRadius) / gridRadius * (cellsPerLine / 2.0f));
  int j = floor((pos[id].y + gridRadius) / gridRadius * (cellsPerLine / 2.0f));
  int k = floor((pos[id].z + gridRadius) / gridRadius * (cellsPerLine / 2.0f));
  color[id].x = i / (float)(cellsPerLine);
  color[id].y = j / (float)(cellsPerLine);
  color[id].z = k / (float)(cellsPerLine);
}

__kernel void neighbours(__global float4* pos, __global int* neighbour, __global int* counter, float smoothingLength, __global float* mass, int neighbor_amount)
{
  unsigned int i = get_global_id(0);

  float4 p = pos[i];

  counter[i] = 0;
  //save neighbours of THIS particle in an array 
  //array size is 50(n) times bigger than pos[]
  for (int index = 0; index < get_global_size(0); index++)
  {
    if (distance(p.xyz, pos[index].xyz) <= smoothingLength) // < smoothingLength
    {
      neighbour[i * neighbor_amount + counter[i]] = index;
      counter[i]++;
      //only saves values with distance < smoothing Lenght --> [0,smoothingLength]
    }
    //stop when 50(n) neighbours of i are found
    if (counter[i] >= neighbor_amount-1)
      break;
  }
}

__kernel void densityPressureCalc(__global float4* pos, __global int* neighbour, __global int* counter, __global float* density, __global float* pressure,
  __global float* mass, float smoothingLength, int neighbor_amount, float rho0)
{
  unsigned int i = get_global_id(0);

  float4 p = pos[i];
  float rho = 0;
  float pressure_new = 0;
  float k = .04030142;
  float k2 = 0.5301f;
  float h = smoothingLength;
  float polySix_const = 315.0f / (64.0f * M_PI_F*h*h*h*h*h*h*h*h*h);

  for (int index = 0; index < counter[i]; index++)
  {
    int j = neighbour[i * neighbor_amount + index];
    rho += pVarPoly(smoothingLength, distance(p.xyz, pos[j].xyz));
  }
  rho *= polySix_const * mass[i];

  density[i] = rho;
  pressure_new = k * rho0/k2 * (pow((rho/rho0), k2) - 1.0f);

  pressure[i] = pressure_new;
}

__kernel void SPH(__global float4* pos, __global float4* vel, __global int* neighbour, __global int* counter, __global float* density, __global float* pressure,
  __global float* mass, __global float4* forceIntern, float smoothingLength, int neighbor_amount)
{
  unsigned int i = get_global_id(0);
  float h = smoothingLength;
  float spiky_const = 45.0f / (M_PI_F*h*h*h*h*h*h);
  float4 p = pos[i];
  float mue = 0.000004;

  float4 f_pressure = 0.0f;
  float4 f_viscosity = 0.0f;

  //force calculation
  for (int index = 0; index < counter[i]; index++)
  {
    int j = neighbour[i * neighbor_amount + index];
    f_pressure += mass[j] * (pressure[i] / (density[i] * density[i]) + pressure[j] / (density[j] * density[j])) * pVarSpiky(smoothingLength, p, pos[j]);

    f_viscosity += (vel[j] - vel[i]) / density[j] * pVarVisc(smoothingLength, p, pos[j]);
  }

  f_pressure *= -mass[i] * spiky_const;

  f_viscosity *= mue * spiky_const * mass[i];

  forceIntern[i] = f_pressure + f_viscosity;
  forceIntern[i] /= density[i];
}

__kernel void integration(__global float4* pos, __global float4* vel, __global float* density, __global float* mass, __global float4* force, float rho0, float dt, float xleft)
{
  unsigned int i = get_global_id(0);

  float4 p_old = pos[i];
  float4 v_old = vel[i];
  float4 p_new = p_old;
  float4 v_new = v_old;

  float gravityForce = -9.81f * mass[i];
  
  float3 forceExtern;
  forceExtern.x = 0.0f;
  forceExtern.y = gravityForce;
  forceExtern.z = 0.0f;

  //integration of velocity
  v_new.xyz = v_old.xyz + ((force[i].xyz + forceExtern.xyz) / mass[i]) * dt;

  //integration of position
  p_new.xyz = p_old.xyz + v_new.xyz * dt;

  //boundary damping
  float bDamp = -0.9;
  //boundarys
  if (p_new.y < -1.0f)
  {
    v_new.y *= bDamp;
    p_new.y = -1.0f;
  }

  if (p_new.y > 1.0f)
  {
    v_new.y *= bDamp;
    p_new.y = 1.0f;
  }

  if (p_new.x > 1.0f) {
    v_new.x *= bDamp;
    p_new.x = 1.0f;
  }

  if (p_new.x < xleft) {
    v_new.x *= bDamp;
    p_new.x = xleft;
  }

  if (p_new.z > 1.0f) {
    v_new.z *= bDamp;
    p_new.z = 1.0f;
  }

  if (p_new.z < -0.5f) {
    v_new.z *= bDamp;
    p_new.z = -0.5f;
  }


  //update the arrays with computed values
  pos[i].xyz = p_new.xyz;
  vel[i].xyz = v_new.xyz;
}
