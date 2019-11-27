//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Suneth Warnakulasuriya (https://github.com/sunethwarna)
//

#if !defined(KRATOS_RANS_APPLICATION_VARIABLES_H_INCLUDED)
#define KRATOS_RANS_APPLICATION_VARIABLES_H_INCLUDED

// System includes

// External includes

// Project includes
#include "includes/define.h"
#include "containers/variable.h"

namespace Kratos
{
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, TURBULENT_KINETIC_ENERGY)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, TURBULENT_ENERGY_DISSIPATION_RATE)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, TURBULENT_KINETIC_ENERGY_RATE)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, TURBULENT_ENERGY_DISSIPATION_RATE_2)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, bool, IS_CO_SOLVING_PROCESS_ACTIVE)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, RANS_Y_PLUS)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, RANS_AUXILIARY_VARIABLE_1)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, RANS_AUXILIARY_VARIABLE_2)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, WALL_SMOOTHNESS_BETA)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, WALL_VON_KARMAN)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, TURBULENCE_RANS_C_MU)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, TURBULENCE_RANS_C1)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, TURBULENCE_RANS_C2)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, TURBULENT_KINETIC_ENERGY_SIGMA)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, TURBULENT_ENERGY_DISSIPATION_RATE_SIGMA)

KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, int, NUMBER_OF_NEIGHBOUR_CONDITIONS)

//for the k-omega element
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, TURBULENCE_RANS_SIGMA_K)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, TURBULENCE_RANS_SIGMA_OMEGA)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, TURBULENCE_RANS_GAMMA)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, TURBULENCE_RANS_BETA_ZERO_STAR)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, TURBULENCE_RANS_BETA_ZERO)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, TURBULENT_SPECIFIC_ENERGY_DISSIPATION_RATE)
KRATOS_DEFINE_APPLICATION_VARIABLE(RANS_APPLICATION, double, TURBULENT_SPECIFIC_ENERGY_DISSIPATION_RATE_2)

} // namespace Kratos

#endif /* KRATOS_RANS_APPLICATION_VARIABLES_H_INCLUDED */
