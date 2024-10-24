// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2023 NeoFOAM authors

#define CATCH_CONFIG_RUNNER // Define this before including catch.hpp to create
                            // a custom main

#include "NeoFOAM/fields/field.hpp"

#include "common.hpp"

#define namespaceFoam // Suppress <using namespace Foam;>

extern Foam::Time* timePtr; // A single time object

TEST_CASE("VolumeField")
{
    NeoFOAM::Executor exec = GENERATE(
        NeoFOAM::Executor(NeoFOAM::CPUExecutor {}),
        NeoFOAM::Executor(NeoFOAM::SerialExecutor {}),
        NeoFOAM::Executor(NeoFOAM::GPUExecutor {})
    );
    std::string execName = std::visit([](auto e) { return e.print(); }, exec);

    Foam::Time& runTime = *timePtr;
    auto meshPtr = Foam::createMesh(exec, runTime);
    Foam::fvccNeoMesh& mesh = *meshPtr;
    auto nfMesh = mesh.nfMesh();

    auto ofT = randomScalarField(runTime, mesh);
    auto ofU = randomVectorField(runTime, mesh);

    SECTION("volumeScalarField " + execName)
    {
        auto nfT = constructFrom(exec, nfMesh, ofT);
        compare(nfT, ofT, ApproxScalar(1e-15));
    }

    SECTION("volumeVectorField " + execName)
    {
        auto nfU = constructFrom(exec, nfMesh, ofU);
        compare(nfU, ofU, ApproxVector(1e-15));
    }
}
