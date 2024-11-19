// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2023 NeoFOAM authors

#include "FoamAdapter/setup.hpp"

namespace Foam
{

std::tuple<bool, scalar, scalar> timeControls(const Time& runTime)
{
    bool adjustTimeStep = runTime.controlDict().getOrDefault("adjustTimeStep", false);

    scalar maxCo = runTime.controlDict().getOrDefault<scalar>("maxCo", 1);

    scalar maxDeltaT = runTime.controlDict().getOrDefault<scalar>("maxDeltaT", GREAT);

    return std::make_tuple(adjustTimeStep, maxCo, maxDeltaT);
}


scalar calculateCoNum(const surfaceScalarField& phi)
{
    const fvMesh& mesh = phi.mesh();
    const Time& runTime = mesh.time();
    scalarField sumPhi(fvc::surfaceSum(mag(phi))().primitiveField());
    scalar coNum = 0.5 * gMax(sumPhi / mesh.V().field()) * runTime.deltaTValue();
    scalar meanCoNum = 0.5 * (gSum(sumPhi) / gSum(mesh.V().field())) * runTime.deltaTValue();

    Info << "Courant Number mean: " << meanCoNum << " max: " << coNum << endl;
    return coNum;
}

void setDeltaT(Time& runTime, scalar maxCo, scalar coNum, scalar maxDeltaT)
{
    scalar maxDeltaTFact = maxCo / (coNum + SMALL);
    scalar deltaTFact = min(min(maxDeltaTFact, 1.0 + 0.1 * maxDeltaTFact), 1.2);

    runTime.setDeltaT(min(deltaTFact * runTime.deltaTValue(), maxDeltaT));

    Info << "deltaT = " << runTime.deltaTValue() << endl;
}


std::unique_ptr<MeshAdapter> createMesh(const NeoFOAM::Executor& exec, const Time& runTime)
{
    word regionName(polyMesh::defaultRegion);
    IOobject io(regionName, runTime.timeName(), runTime, IOobject::MUST_READ);
    return std::make_unique<MeshAdapter>(exec, io);
}

std::unique_ptr<fvMesh> createMesh(const Time& runTime)
{
    std::unique_ptr<fvMesh> meshPtr;
    Info << "Create mesh";
    word regionName(polyMesh::defaultRegion);
    Info << " for time = " << runTime.timeName() << nl;

    meshPtr.reset(
        new fvMesh(IOobject(regionName, runTime.timeName(), runTime, IOobject::MUST_READ), false)
    );
    meshPtr->init(true); // initialise all (lower levels and current)

    return meshPtr;
}

NeoFOAM::Executor createExecutor(const dictionary& dict)
{
    auto exec_name = dict.get<Foam::word>("executor");
    Foam::Info << "Creating Executor: " << exec_name << Foam::endl;
    if (exec_name == "Serial")
    {
        Foam::Info << "Serial Executor" << Foam::endl;
        return NeoFOAM::SerialExecutor();
    }
    if (exec_name == "CPU")
    {
        Foam::Info << "CPU Executor" << Foam::endl;
        return NeoFOAM::CPUExecutor();
    }
    if (exec_name == "GPU")
    {
        Foam::Info << "GPU Executor" << Foam::endl;
        return NeoFOAM::GPUExecutor();
    }
    Foam::FatalError << "unknown Executor: " << exec_name << Foam::nl
                     << "Available executors: Serial, CPU, GPU" << Foam::nl
                     << Foam::abort(Foam::FatalError);

    return NeoFOAM::SerialExecutor();

}

}