// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2023 NeoFOAM authors

#define CATCH_CONFIG_RUNNER // Define this before including catch.hpp to create
                            // a custom main
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include "catch2/common.hpp"

#include "NeoFOAM/fields/field.hpp"
#include "NeoFOAM/core/tokenList.hpp"

#include "FoamAdapter/readers/foamDictionary.hpp"
#include "FoamAdapter/writers.hpp"

#include "common.hpp"

extern Foam::Time* timePtr; // A single time object

TEST_CASE("read dict")
{
    Foam::Info << "\nReading testDict" << Foam::endl;

    Foam::dictionary testDict;
    testDict.add("label", 1);
    testDict.add("scalar", 2.1);
    testDict.add("scalar2", 2.0);
    testDict.add("vector", Foam::vector(1.0, 2.0, 3.0));
    testDict.add("word", "word");

    Foam::dictionary subDict;
    subDict.add("subScalar", 4.1);
    subDict.add("subVector", Foam::vector(5.0, 6.0, 7.0));
    subDict.add("subWord", "subWord");
    testDict.add("subDict", subDict);

    NeoFOAM::Dictionary neoDict = Foam::readFoamDictionary(testDict);

    REQUIRE(neoDict.get<NeoFOAM::label>("label") == 1);
    REQUIRE(neoDict.get<NeoFOAM::scalar>("scalar") == 2.1);
    REQUIRE(neoDict.get<NeoFOAM::scalar>("scalar2") == 2.0);
    REQUIRE(neoDict.get<NeoFOAM::Vector>("vector") == NeoFOAM::Vector(1.0, 2.0, 3.0));
    REQUIRE(neoDict.get<std::string>("word") == "word");


    NeoFOAM::Dictionary& subNeoDict = neoDict.subDict("subDict");
    REQUIRE(subNeoDict.get<NeoFOAM::scalar>("subScalar") == 4.1);
    REQUIRE(subNeoDict.get<NeoFOAM::Vector>("subVector") == NeoFOAM::Vector(5.0, 6.0, 7.0));
}


TEST_CASE("read fvSchemes")
{
    Foam::Time& runTime = *timePtr;
    auto meshPtr = Foam::createMesh(NeoFOAM::SerialExecutor {}, runTime);
    Foam::MeshAdapter& mesh = *meshPtr;

    Foam::dictionary fvSchemes = mesh.schemesDict();
    Foam::Info << "reading fvSchemes" << fvSchemes << Foam::endl;

    NeoFOAM::Dictionary fvSchemesDict = Foam::readFoamDictionary(mesh.schemesDict());
    auto keys = fvSchemesDict.keys();

    REQUIRE(fvSchemesDict.subDict("ddtSchemes").get<std::string>("ddt(T)") == "Euler");
    auto gradSchemeKeys = fvSchemesDict.subDict("gradSchemes").keys();
    NeoFOAM::TokenList limitedToken =
        fvSchemesDict.subDict("gradSchemes").get<NeoFOAM::TokenList>("limited");
    REQUIRE(limitedToken.size() == 4);
    REQUIRE(limitedToken.get<std::string>(0) == "cellLimited");
    REQUIRE(limitedToken.get<std::string>(1) == "Gauss");
    REQUIRE(limitedToken.get<std::string>(2) == "linear");
    REQUIRE(limitedToken.get<NeoFOAM::label>(3) == 1);

    NeoFOAM::TokenList gradU =
        fvSchemesDict.subDict("gradSchemes").get<NeoFOAM::TokenList>("grad(U)");
    REQUIRE(gradU.size() == 2);
    REQUIRE(gradU.get<std::string>(0) == "Gauss");
    REQUIRE(gradU.get<std::string>(1) == "linear");
}