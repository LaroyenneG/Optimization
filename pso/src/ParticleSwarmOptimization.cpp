//
// Created by Guillaume Laroyenne on 24/02/19.
//

#include <random>
#include <iostream>
#include "ParticleSwarmOptimization.h"


ParticleSwarmOptimization::~ParticleSwarmOptimization() {

    for (int i = 0; i < swarmSize; ++i) {
        delete swarm[i];
    }

    delete[] swarm;
}


ParticleSwarmOptimization::ParticleSwarmOptimization(unsigned int _dimensionSpace, unsigned int _nbParticle,
                                                     unsigned int _nbNeighbors,
                                                     unsigned int _nbIteration, double _deltaSpeedMin,
                                                     double _deltaSpeedMax,
                                                     double _fitnessValue, double _minBound, double _maxBound,
                                                     double _indCoefficient,
                                                     double _colCoefficient, double _inertiaCoefficient,
                                                     const FitnessEvaluatorInterface *_fitnessEvaluatorInterface)
        : swarmSize(_nbParticle),
          nbNeighbors(_nbNeighbors),
          nbIteration(_nbIteration),
          fitnessDesired(_fitnessValue),
          dimensionSpace(_dimensionSpace),
          swarm(new Particle *[_nbParticle]),
          fitnessEvaluatorInterface(_fitnessEvaluatorInterface) {

    deltaSpeed[0] = _deltaSpeedMin;
    deltaSpeed[1] = _deltaSpeedMax;

    bounds[0] = _minBound;
    bounds[1] = _maxBound;

    coefficients[0] = _indCoefficient;
    coefficients[1] = _colCoefficient;
    coefficients[2] = _inertiaCoefficient;

    if (coefficients[0] + coefficients[1] != 4) {
        delete[] swarm;
        throw std::invalid_argument("invalid coefficients");
    }

    if (bounds[0] >= bounds[1]) {
        delete[] swarm;
        throw std::invalid_argument("invalid bounds");
    }

    if (deltaSpeed[0] >= deltaSpeed[1]) {
        delete[] swarm;
        throw std::invalid_argument("invalid delta speed");
    }

    for (int i = 0; i < swarmSize; ++i) {
        swarm[i] = new Particle(dimensionSpace, fitnessEvaluatorInterface);
    }

    initializeSwarm();
}

void ParticleSwarmOptimization::initializeSwarm() {

    for (int i = 0; i < swarmSize; i++) {

        Particle *particle = swarm[i];

        for (unsigned int d = 0; d < dimensionSpace; ++d) {

            double position = randDoubleBetween(bounds[0], bounds[1]);

            particle->setPosition(d, position);
            particle->setBestPosition(d, position);
            particle->setSpeed(d, randDoubleBetween(deltaSpeed[0], deltaSpeed[1]));
        }
    }
}

double ParticleSwarmOptimization::randDoubleBetween(double min, double max) {
    return min + (max - min) * random() / RAND_MAX;
}

Particle *ParticleSwarmOptimization::processing() {

    unsigned int iteration = 0;

    bool solutionFound = false;

    while (iteration < nbIteration && !solutionFound) {

        for (unsigned int i = 0; i < swarmSize; ++i) {

            Particle *particle = swarm[i];
            Particle *bestParticle = getBestParticle();

            for (unsigned int d = 0; d < particle->getDimension(); ++d) {

                double r1 = randDoubleBetween(0.0, 1.0);
                double r2 = randDoubleBetween(0.0, 1.0);

                double b1 = coefficients[0] * r1 *
                            (particle->getBestPosition(d) - particle->getPosition(d));
                double b2 = coefficients[1] * r2 * (bestParticle->getBestPosition(d) -
                                                    particle->getPosition(d));


                particle->setSpeed(d, boundedSpeed(coefficients[2] * particle->getSpeed(d) + b1 + b2));

                particle->setPosition(d, boundedPosition(particle->getPosition(d) + particle->getSpeed(d)));
            }
        }

        for (unsigned int j = 0; j < swarmSize; ++j) {

            Particle *particle = swarm[j];

            double fitness = particle->fitness();

            if (fitness < particle->bestFitness()) {
                for (unsigned int d = 0; d < particle->getDimension(); ++d) {
                    particle->setBestPosition(d, particle->getPosition(d));
                }
            }

            if (fitness == fitnessDesired) {
                solutionFound = true;
            }
        }

        iteration++;
    }

    return getBestParticle();
}

Particle *ParticleSwarmOptimization::getBestNeighbor(unsigned int index) const {

    double bestFitness = 0;
    Particle *bestNeighbor = nullptr;

    int step = nbNeighbors / 2;

    for (int i = index - step; i < static_cast<int>(index) + step; ++i) {

        if (i != index) {

            Particle *particle = swarm[i % swarmSize];

            if (bestNeighbor == nullptr || bestFitness > particle->bestFitness()) {
                bestNeighbor = particle;
                bestFitness = particle->fitness();
            }
        }
    }

    return bestNeighbor;
}

double ParticleSwarmOptimization::boundedSpeed(double value) const {

    double result;

    if (value < deltaSpeed[0]) {
        result = deltaSpeed[0];
    } else if (value > deltaSpeed[1]) {
        result = deltaSpeed[1];
    } else {
        result = value;
    }

    return result;
}

double ParticleSwarmOptimization::boundedPosition(double value) const {

    double result;

    if (value < bounds[0]) {
        result = bounds[0];
    } else if (value > bounds[1]) {
        result = bounds[1];
    } else {
        result = value;
    }
    return result;
}

Particle *ParticleSwarmOptimization::getBestParticle() const {

    Particle *best = nullptr;

    for (unsigned int i = 0; i < swarmSize; ++i) {
        if (i == 0 || best->bestFitness() > swarm[i]->bestFitness()) {
            best = swarm[i];
        }
    }

    return best;
}
