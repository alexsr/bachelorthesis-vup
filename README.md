#### This project is finished and will not be updated any longer.
Please visit [VUP](https://github.com/alexsr/vup) for an ongoing rewrite of this project.

# Unified Particle Framework

This is the repository for my bachelor thesis "Development of a variable particle frameworks utilizing GPGPU".
In this bachelor thesis a framework for particle based simulation of physical
systems is created. By automating the preparation process of the simulation through usage of configuration data allows the developer to build new simulation scenarios effortlessly.
Additionally the framework’s functionality is extensible due to its overall structure. Utilizing the parallel nature of graphics hardware all calculations of the simulations are processed on the GPU.
The simulation methods described in this thesis are integrated into the particle framework. To accelerate calculations the uniform grid data structure is used resulting in an increase of simulation performance up to 90%.

## Definition of Task

Particle simulations are an important part of a lot of modern applications, e.g. in pharmaceutics or the games industry. They allow for realisitc simulation of fluids, gases, molucules, and solids and is therefore universially applicable. Additionally the parallel nature of modern GPUs enables real time simulations of particles.

The focus of this bachelor thesis is the development of a particle framework on the GPU. The goal is to create a framework which allows for both variable particle parameters and movement functions. All calculations should utilize the parallel nature of modern GPUs.

The main topic is to provide an interface to allow the user to both use already implemented particle simulation configurations and moreover to create and seemlessly integrate their own configurations.
