#include <iostream>
#include <fstream>
#include <random>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <cuda_runtime.h>

double G = 6.674*std::pow(10,-11);
//double G = 1;

struct simulation {
  size_t nbpart;
  
  std::vector<double> mass;

  //position
  std::vector<double> x;
  std::vector<double> y;
  std::vector<double> z;

  //velocity
  std::vector<double> vx;
  std::vector<double> vy;
  std::vector<double> vz;

  //force
  std::vector<double> fx;
  std::vector<double> fy;
  std::vector<double> fz;

  
  simulation(size_t nb)
    :nbpart(nb), mass(nb),
     x(nb), y(nb), z(nb),
     vx(nb), vy(nb), vz(nb),
     fx(nb), fy(nb), fz(nb) 
  {}
};


void random_init(simulation& s) {
  std::random_device rd;  
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dismass(0.9, 1.);
  std::normal_distribution<> dispos(0., 1.);
  std::normal_distribution<> disvel(0., 1.);

  for (size_t i = 0; i<s.nbpart; ++i) {
    s.mass[i] = dismass(gen);

    s.x[i] = dispos(gen);
    s.y[i] = dispos(gen);
    s.z[i] = dispos(gen);
    s.z[i] = 0.;
    
    s.vx[i] = disvel(gen);
    s.vy[i] = disvel(gen);
    s.vz[i] = disvel(gen);
    s.vz[i] = 0.;
    s.vx[i] = s.y[i]*1.5;
    s.vy[i] = -s.x[i]*1.5;
  }

  return;
  //normalize velocity (using normalization found on some physicis blog)
  double meanmass = 0;
  double meanmassvx = 0;
  double meanmassvy = 0;
  double meanmassvz = 0;
  for (size_t i = 0; i<s.nbpart; ++i) {
    meanmass += s.mass[i];
    meanmassvx += s.mass[i] * s.vx[i];
    meanmassvy += s.mass[i] * s.vy[i];
    meanmassvz += s.mass[i] * s.vz[i];
  }
  for (size_t i = 0; i<s.nbpart; ++i) {
    s.vx[i] -= meanmassvx/meanmass;
    s.vy[i] -= meanmassvy/meanmass;
    s.vz[i] -= meanmassvz/meanmass;
  }
  
}

void init_solar(simulation& s) {
  enum Planets {SUN, MERCURY, VENUS, EARTH, MARS, JUPITER, SATURN, URANUS, NEPTUNE, MOON};
  s = simulation(10);

  // Masses in kg
  s.mass[SUN] = 1.9891 * std::pow(10, 30);
  s.mass[MERCURY] = 3.285 * std::pow(10, 23);
  s.mass[VENUS] = 4.867 * std::pow(10, 24);
  s.mass[EARTH] = 5.972 * std::pow(10, 24);
  s.mass[MARS] = 6.39 * std::pow(10, 23);
  s.mass[JUPITER] = 1.898 * std::pow(10, 27);
  s.mass[SATURN] = 5.683 * std::pow(10, 26);
  s.mass[URANUS] = 8.681 * std::pow(10, 25);
  s.mass[NEPTUNE] = 1.024 * std::pow(10, 26);
  s.mass[MOON] = 7.342 * std::pow(10, 22);

  // Positions (in meters) and velocities (in m/s)
  double AU = 1.496 * std::pow(10, 11); // Astronomical Unit

  s.x = {0, 0.39*AU, 0.72*AU, 1.0*AU, 1.52*AU, 5.20*AU, 9.58*AU, 19.22*AU, 30.05*AU, 1.0*AU + 3.844*std::pow(10, 8)};
  s.y = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  s.z = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  s.vx = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  s.vy = {0, 47870, 35020, 29780, 24130, 13070, 9680, 6800, 5430, 29780 + 1022};
  s.vz = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
}

void dump_state(simulation& s) {
  std::cout<<s.nbpart<<'\t';
  for (size_t i=0; i<s.nbpart; ++i) {
    std::cout<<s.mass[i]<<'\t';
    std::cout<<s.x[i]<<'\t'<<s.y[i]<<'\t'<<s.z[i]<<'\t';
    std::cout<<s.vx[i]<<'\t'<<s.vy[i]<<'\t'<<s.vz[i]<<'\t';
    std::cout<<s.fx[i]<<'\t'<<s.fy[i]<<'\t'<<s.fz[i]<<'\t';
  }
  std::cout<<'\n';
}

void load_from_file(simulation& s, std::string filename) {
  std::ifstream in (filename);
  size_t nbpart;
  in>>nbpart;
  s = simulation(nbpart);
  for (size_t i=0; i<s.nbpart; ++i) {
    in>>s.mass[i];
    in >>  s.x[i] >>  s.y[i] >>  s.z[i];
    in >> s.vx[i] >> s.vy[i] >> s.vz[i];
    in >> s.fx[i] >> s.fy[i] >> s.fz[i];
  }
  if (!in.good())
    throw "kaboom";
}

void check_cuda(cudaError_t status) {
  if (status != cudaSuccess) {
    std::cerr<<cudaGetErrorString(status)<<std::endl;
    std::exit(1);
  }
}

__global__ void compute_force_kernel(size_t nbpart,
                                     double* mass,
                                     double* x,
                                     double* y,
                                     double* z,
                                     double* fx,
                                     double* fy,
                                     double* fz) {
  size_t to = blockIdx.x * blockDim.x + threadIdx.x;

  if (to >= nbpart)
    return;

  double softening = .1;
  double local_fx = 0.;
  double local_fy = 0.;
  double local_fz = 0.;
  double cuda_G = 6.674e-11;

  for (size_t from = 0; from<nbpart; ++from) {
    if (from != to) {
      double dx = x[from]-x[to];
      double dy = y[from]-y[to];
      double dz = z[from]-z[to];

      double dist_sq = dx*dx + dy*dy + dz*dz;
      double F = cuda_G * mass[from]*mass[to]/(dist_sq+softening);

      double norm = sqrt(dx*dx+dy*dy+dz*dz);

      dx = dx/norm;
      dy = dy/norm;
      dz = dz/norm;

      local_fx += dx*F;
      local_fy += dy*F;
      local_fz += dz*F;
    }
  }

  fx[to] = local_fx;
  fy[to] = local_fy;
  fz[to] = local_fz;
}

__global__ void update_kernel(size_t nbpart,
                              double dt,
                              double* mass,
                              double* x,
                              double* y,
                              double* z,
                              double* vx,
                              double* vy,
                              double* vz,
                              double* fx,
                              double* fy,
                              double* fz) {
  size_t i = blockIdx.x * blockDim.x + threadIdx.x;

  if (i >= nbpart)
    return;

  vx[i] += fx[i]/mass[i]*dt;
  vy[i] += fy[i]/mass[i]*dt;
  vz[i] += fz[i]/mass[i]*dt;

  x[i] += vx[i]*dt;
  y[i] += vy[i]*dt;
  z[i] += vz[i]*dt;
}

void copy_device_to_host(simulation& s,
                         double* d_mass,
                         double* d_x,
                         double* d_y,
                         double* d_z,
                         double* d_vx,
                         double* d_vy,
                         double* d_vz,
                         double* d_fx,
                         double* d_fy,
                         double* d_fz) {
  size_t bytes = s.nbpart * sizeof(double);

  check_cuda(cudaMemcpy(&(s.mass[0]), d_mass, bytes, cudaMemcpyDeviceToHost));
  check_cuda(cudaMemcpy(&(s.x[0]), d_x, bytes, cudaMemcpyDeviceToHost));
  check_cuda(cudaMemcpy(&(s.y[0]), d_y, bytes, cudaMemcpyDeviceToHost));
  check_cuda(cudaMemcpy(&(s.z[0]), d_z, bytes, cudaMemcpyDeviceToHost));
  check_cuda(cudaMemcpy(&(s.vx[0]), d_vx, bytes, cudaMemcpyDeviceToHost));
  check_cuda(cudaMemcpy(&(s.vy[0]), d_vy, bytes, cudaMemcpyDeviceToHost));
  check_cuda(cudaMemcpy(&(s.vz[0]), d_vz, bytes, cudaMemcpyDeviceToHost));
  check_cuda(cudaMemcpy(&(s.fx[0]), d_fx, bytes, cudaMemcpyDeviceToHost));
  check_cuda(cudaMemcpy(&(s.fy[0]), d_fy, bytes, cudaMemcpyDeviceToHost));
  check_cuda(cudaMemcpy(&(s.fz[0]), d_fz, bytes, cudaMemcpyDeviceToHost));
}

int main(int argc, char* argv[]) {
  if (argc != 6) {
    std::cerr
      <<"usage: "<<argv[0]<<" <input> <dt> <nbstep> <printevery> <blocksize>"<<"\n"
      <<"input can be:"<<"\n"
      <<"a number (random initialization)"<<"\n"
      <<"planet (initialize with solar system)"<<"\n"
      <<"a filename (load from file in singleline tsv)"<<"\n";
    return -1;
  }
  
  double dt = std::atof(argv[2]); //in seconds
  size_t nbstep = std::atol(argv[3]);
  size_t printevery = std::atol(argv[4]);
  int blocksize = std::atoi(argv[5]);
  
  
  simulation s(1);

  //parse command line
  {
    size_t nbpart = std::atol(argv[1]); //return 0 if not a number
    if ( nbpart > 0) {
      s = simulation(nbpart);
      random_init(s);
    } else {
      std::string inputparam = argv[1];
      if (inputparam == "planet") {
	init_solar(s);
      } else{
	load_from_file(s, inputparam);
      }
    }    
  }

  size_t bytes = s.nbpart * sizeof(double);

  double* d_mass;
  double* d_x;
  double* d_y;
  double* d_z;
  double* d_vx;
  double* d_vy;
  double* d_vz;
  double* d_fx;
  double* d_fy;
  double* d_fz;

  check_cuda(cudaMalloc(&d_mass, bytes));
  check_cuda(cudaMalloc(&d_x, bytes));
  check_cuda(cudaMalloc(&d_y, bytes));
  check_cuda(cudaMalloc(&d_z, bytes));
  check_cuda(cudaMalloc(&d_vx, bytes));
  check_cuda(cudaMalloc(&d_vy, bytes));
  check_cuda(cudaMalloc(&d_vz, bytes));
  check_cuda(cudaMalloc(&d_fx, bytes));
  check_cuda(cudaMalloc(&d_fy, bytes));
  check_cuda(cudaMalloc(&d_fz, bytes));

  check_cuda(cudaMemcpy(d_mass, &(s.mass[0]), bytes, cudaMemcpyHostToDevice));
  check_cuda(cudaMemcpy(d_x, &(s.x[0]), bytes, cudaMemcpyHostToDevice));
  check_cuda(cudaMemcpy(d_y, &(s.y[0]), bytes, cudaMemcpyHostToDevice));
  check_cuda(cudaMemcpy(d_z, &(s.z[0]), bytes, cudaMemcpyHostToDevice));
  check_cuda(cudaMemcpy(d_vx, &(s.vx[0]), bytes, cudaMemcpyHostToDevice));
  check_cuda(cudaMemcpy(d_vy, &(s.vy[0]), bytes, cudaMemcpyHostToDevice));
  check_cuda(cudaMemcpy(d_vz, &(s.vz[0]), bytes, cudaMemcpyHostToDevice));
  check_cuda(cudaMemcpy(d_fx, &(s.fx[0]), bytes, cudaMemcpyHostToDevice));
  check_cuda(cudaMemcpy(d_fy, &(s.fy[0]), bytes, cudaMemcpyHostToDevice));
  check_cuda(cudaMemcpy(d_fz, &(s.fz[0]), bytes, cudaMemcpyHostToDevice));

  int gridsize = (s.nbpart + blocksize - 1) / blocksize;

  for (size_t step = 0; step< nbstep; step++) {
    if (step %printevery == 0) {
      copy_device_to_host(s, d_mass, d_x, d_y, d_z, d_vx, d_vy, d_vz, d_fx, d_fy, d_fz);
      dump_state(s);
    }

    compute_force_kernel<<<gridsize, blocksize>>>(s.nbpart, d_mass, d_x, d_y, d_z, d_fx, d_fy, d_fz);
    check_cuda(cudaGetLastError());

    update_kernel<<<gridsize, blocksize>>>(s.nbpart, dt, d_mass, d_x, d_y, d_z, d_vx, d_vy, d_vz, d_fx, d_fy, d_fz);
    check_cuda(cudaGetLastError());
  }
  
  //dump_state(s);  

  check_cuda(cudaFree(d_mass));
  check_cuda(cudaFree(d_x));
  check_cuda(cudaFree(d_y));
  check_cuda(cudaFree(d_z));
  check_cuda(cudaFree(d_vx));
  check_cuda(cudaFree(d_vy));
  check_cuda(cudaFree(d_vz));
  check_cuda(cudaFree(d_fx));
  check_cuda(cudaFree(d_fy));
  check_cuda(cudaFree(d_fz));

  return 0;
}