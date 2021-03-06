//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Riccardo Rossi
//                   Vicente Mataix Ferrandiz
//
//

/* System includes */

/* External includes */

/* Project includes */
#include "utilities/variable_utils.h"
#include "utilities/geometry_utilities.h"
#include "processes/compute_nodal_gradient_process.h"

namespace Kratos
{
template<bool THistorical>
void ComputeNodalGradientProcess<THistorical>::Execute()
{
    KRATOS_TRY;

    // Set to zero
    ClearGradient();

    // Auxiliar containers
    Matrix DN_DX, J0;
    Vector N;

    // First element iterator
    const auto it_element_begin = mrModelPart.ElementsBegin();

    // Current domain size
    const std::size_t dimension = mrModelPart.GetProcessInfo()[DOMAIN_SIZE];

    // Iterate over the elements
    #pragma omp parallel for firstprivate(DN_DX,  N, J0)
    for(int i_elem=0; i_elem<static_cast<int>(mrModelPart.Elements().size()); ++i_elem) {
        auto it_elem = it_element_begin + i_elem;
        auto& r_geometry = it_elem->GetGeometry();

        // Current geometry information
        const std::size_t local_space_dimension = r_geometry.LocalSpaceDimension();
        const std::size_t number_of_nodes = r_geometry.PointsNumber();

        // Resize if needed
        if (DN_DX.size1() != number_of_nodes || DN_DX.size2() != dimension)
            DN_DX.resize(number_of_nodes, dimension);
        if (N.size() != number_of_nodes)
            N.resize(number_of_nodes);
        if (J0.size1() != dimension || J0.size2() != local_space_dimension)
            J0.resize(dimension, local_space_dimension);

        // The integration points
        const auto& r_integration_method = r_geometry.GetDefaultIntegrationMethod();
        const auto& r_integration_points = r_geometry.IntegrationPoints(r_integration_method);
        const std::size_t number_of_integration_points = r_integration_points.size();

        Vector values(number_of_nodes);
        if (!mNonHistoricalVariable) {
            for(std::size_t i_node=0; i_node<number_of_nodes; ++i_node)
                values[i_node] = r_geometry[i_node].FastGetSolutionStepValue(*mpOriginVariable);
        } else {
            for(std::size_t i_node=0; i_node<number_of_nodes; ++i_node)
                values[i_node] = r_geometry[i_node].GetValue(*mpOriginVariable);
        }

        // The containers of the shape functions and the local gradients
        const Matrix& rNcontainer = r_geometry.ShapeFunctionsValues(r_integration_method);
        const auto& rDN_DeContainer = r_geometry.ShapeFunctionsLocalGradients(r_integration_method);

        for ( IndexType point_number = 0; point_number < number_of_integration_points; ++point_number ) {
            // Getting the shape functions
            noalias(N) = row(rNcontainer, point_number);

            // Getting the jacobians and local gradients
            GeometryUtils::JacobianOnInitialConfiguration(r_geometry, r_integration_points[point_number], J0);
            double detJ0;
            Matrix InvJ0;
            MathUtils<double>::GeneralizedInvertMatrix(J0, InvJ0, detJ0);
            const Matrix& rDN_De = rDN_DeContainer[point_number];
            GeometryUtils::ShapeFunctionsGradients(rDN_De, InvJ0, DN_DX);

            const Vector grad = prod(trans(DN_DX), values);
            const double gauss_point_volume = r_integration_points[point_number].Weight() * detJ0;

            for(std::size_t i_node=0; i_node<number_of_nodes; ++i_node) {
                array_1d<double, 3>& r_gradient = GetGradient(r_geometry, i_node);
                for(std::size_t k=0; k<dimension; ++k) {
                    #pragma omp atomic
                    r_gradient[k] += N[i_node] * gauss_point_volume*grad[k];
                }

                double& vol = r_geometry[i_node].GetValue(*mpAreaVariable);

                #pragma omp atomic
                vol += N[i_node] * gauss_point_volume;
            }
        }
    }

    PonderateGradient();

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

template<>
ComputeNodalGradientProcess<ComputeNodalGradientProcessSettings::SaveAsHistoricalVariable>::ComputeNodalGradientProcess(
    ModelPart& rModelPart,
    Parameters ThisParameters
    ) : mrModelPart(rModelPart)
{
    KRATOS_TRY

    // We check the parameters
    const Parameters default_parameters = GetDefaultParameters();
    ThisParameters.RecursivelyValidateAndAssignDefaults(default_parameters);

    // We get the gradient variable
    const std::string& r_origin_variable_name = ThisParameters["origin_variable"].GetString();

    // We push the list of double variables
    if (KratosComponents<Variable<double>>::Has(r_origin_variable_name)) {
        mpOriginVariable = &KratosComponents<Variable<double>>::Get(r_origin_variable_name);
    } else {
        KRATOS_ERROR << "Only doubles are allowed as variables" << std::endl;
    }

    // Setting the non-historical flag
    mNonHistoricalVariable = ThisParameters["non_historical_gradient_variable"].GetBool();

    // Additional checks
    if (!mNonHistoricalVariable) {
        VariableUtils().CheckVariableExists(*mpOriginVariable, mrModelPart.Nodes());
    } else {
        KRATOS_ERROR_IF_NOT(mrModelPart.Nodes().begin()->Has(*mpOriginVariable)) << "Variable " << r_origin_variable_name << " not defined on non-historial database" << std::endl;
    }
    VariableUtils().CheckVariableExists(*mpGradientVariable, mrModelPart.Nodes());
    // In case the area or gradient variable is not initialized we initialize it
    auto& r_nodes = rModelPart.Nodes();
    if (!r_nodes.begin()->Has( *mpAreaVariable )) {
        VariableUtils().SetNonHistoricalVariable(*mpAreaVariable, 0.0, r_nodes);
    }

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

template<>
ComputeNodalGradientProcess<ComputeNodalGradientProcessSettings::SaveAsNonHistoricalVariable>::ComputeNodalGradientProcess(
    ModelPart& rModelPart,
    Parameters ThisParameters
    ) : mrModelPart(rModelPart)
{
    KRATOS_TRY

    // We check the parameters
    const Parameters default_parameters = GetDefaultParameters();
    ThisParameters.RecursivelyValidateAndAssignDefaults(default_parameters);

    // We get the gradient variable
    const std::string& r_origin_variable_name = ThisParameters["origin_variable"].GetString();

    // We push the list of double variables
    if (KratosComponents<Variable<double>>::Has(r_origin_variable_name)) {
        mpOriginVariable = &KratosComponents<Variable<double>>::Get(r_origin_variable_name);
    } else {
        KRATOS_ERROR << "Only doubles are allowed as variables" << std::endl;
    }

    // Setting the non-historical flag
    mNonHistoricalVariable = ThisParameters["non_historical_origin_variable"].GetBool();

    // Additional checks
    if (!mNonHistoricalVariable) {
        VariableUtils().CheckVariableExists(*mpOriginVariable, mrModelPart.Nodes());
    } else {
        KRATOS_ERROR_IF_NOT(mrModelPart.Nodes().begin()->Has(*mpOriginVariable)) << "Variable " << r_origin_variable_name << " not defined on non-historial database" << std::endl;
    }
    // In case the area or gradient variable is not initialized we initialize it
    auto& r_nodes = rModelPart.Nodes();
    if (!r_nodes.begin()->Has( *mpGradientVariable )) {
        const array_1d<double,3> zero_vector = ZeroVector(3);
        VariableUtils().SetNonHistoricalVariable(*mpGradientVariable, zero_vector, r_nodes);
    }
    if (!r_nodes.begin()->Has( *mpAreaVariable )) {
        VariableUtils().SetNonHistoricalVariable(*mpAreaVariable, 0.0, r_nodes);
    }

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

template<>
ComputeNodalGradientProcess<ComputeNodalGradientProcessSettings::SaveAsHistoricalVariable>::ComputeNodalGradientProcess(
    ModelPart& rModelPart,
    const Variable<double>& rOriginVariable,
    const Variable<array_1d<double,3> >& rGradientVariable,
    const Variable<double>& rAreaVariable,
    const bool NonHistoricalVariable
    ) : mrModelPart(rModelPart),
        mpOriginVariable(&rOriginVariable),
        mpGradientVariable(&rGradientVariable),
        mpAreaVariable(&rAreaVariable),
        mNonHistoricalVariable(NonHistoricalVariable)
{
    KRATOS_TRY

    // Doing several checks
    if (!mNonHistoricalVariable) {
        VariableUtils().CheckVariableExists(rOriginVariable, mrModelPart.Nodes());
    } else {
        KRATOS_ERROR_IF_NOT(mrModelPart.Nodes().begin()->Has(rOriginVariable)) << "Variable " << rOriginVariable.Name() << " not defined on non-historial database" << std::endl;
    }
    VariableUtils().CheckVariableExists(rGradientVariable, mrModelPart.Nodes());
    // In case the area or gradient variable is not initialized we initialize it
    auto& r_nodes = rModelPart.Nodes();
    if (!r_nodes.begin()->Has( rAreaVariable )) {
        VariableUtils().SetNonHistoricalVariable(rAreaVariable, 0.0, r_nodes);
    }

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

template<>
ComputeNodalGradientProcess<ComputeNodalGradientProcessSettings::SaveAsNonHistoricalVariable>::ComputeNodalGradientProcess(
    ModelPart& rModelPart,
    const Variable<double>& rOriginVariable,
    const Variable<array_1d<double,3> >& rGradientVariable,
    const Variable<double>& rAreaVariable,
    const bool NonHistoricalVariable
    ) : mrModelPart(rModelPart),
        mpOriginVariable(&rOriginVariable),
        mpGradientVariable(&rGradientVariable),
        mpAreaVariable(&rAreaVariable),
        mNonHistoricalVariable(NonHistoricalVariable)
{
    KRATOS_TRY

    // Doing several checks
    if (!mNonHistoricalVariable) {
        VariableUtils().CheckVariableExists(rOriginVariable, mrModelPart.Nodes());
    } else {
        KRATOS_ERROR_IF_NOT(mrModelPart.Nodes().begin()->Has(rOriginVariable)) << "Variable " << rOriginVariable.Name() << " not defined on non-historial database" << std::endl;
    }
    // In case the area or gradient variable is not initialized we initialize it
    auto& r_nodes = rModelPart.Nodes();
    if (!r_nodes.begin()->Has( rGradientVariable )) {
        const array_1d<double,3> zero_vector = ZeroVector(3);
        VariableUtils().SetNonHistoricalVariable(rGradientVariable, zero_vector, r_nodes);
    }
    if (!r_nodes.begin()->Has( rAreaVariable )) {
        VariableUtils().SetNonHistoricalVariable(rAreaVariable, 0.0, r_nodes);
    }

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

template<>
void ComputeNodalGradientProcess<ComputeNodalGradientProcessSettings::SaveAsHistoricalVariable>::ClearGradient()
{
    const auto it_node_begin = mrModelPart.NodesBegin();

    #pragma omp parallel for
    for(int i = 0; i < static_cast<int>(mrModelPart.Nodes().size()); ++i) {
        auto it_node=it_node_begin + i;
        it_node->SetValue(*mpAreaVariable, 0.0);
        it_node->FastGetSolutionStepValue(*mpGradientVariable).clear();
    }
}

/***********************************************************************************/
/***********************************************************************************/

template <>
void ComputeNodalGradientProcess<ComputeNodalGradientProcessSettings::SaveAsNonHistoricalVariable>::ClearGradient()
{
    const array_1d<double, 3> aux_zero_vector = ZeroVector(3);
    const auto it_node_begin = mrModelPart.NodesBegin();

    #pragma omp parallel for
    for(int i = 0; i < static_cast<int>(mrModelPart.Nodes().size()); ++i) {
        auto it_node= it_node_begin + i;
        it_node->SetValue(*mpAreaVariable, 0.0);
        it_node->SetValue(*mpGradientVariable, aux_zero_vector);
    }
}

/***********************************************************************************/
/***********************************************************************************/

template <>
array_1d<double, 3>& ComputeNodalGradientProcess<ComputeNodalGradientProcessSettings::SaveAsHistoricalVariable>::GetGradient(
    Element::GeometryType& rThisGeometry,
    unsigned int i
    )
{
    return rThisGeometry[i].FastGetSolutionStepValue(*mpGradientVariable);
}

/***********************************************************************************/
/***********************************************************************************/

template <>
array_1d<double, 3>& ComputeNodalGradientProcess<ComputeNodalGradientProcessSettings::SaveAsNonHistoricalVariable>::GetGradient(
    Element::GeometryType& rThisGeometry,
    unsigned int i
    )
{
    return rThisGeometry[i].GetValue(*mpGradientVariable);
}

/***********************************************************************************/
/***********************************************************************************/

template <>
void ComputeNodalGradientProcess<ComputeNodalGradientProcessSettings::SaveAsHistoricalVariable>::PonderateGradient()
{
    const auto it_node_begin = mrModelPart.NodesBegin();

    #pragma omp parallel for
    for(int i = 0; i < static_cast<int>(mrModelPart.Nodes().size()); ++i) {
        auto it_node = it_node_begin + i;
        it_node->FastGetSolutionStepValue(*mpGradientVariable) /= it_node->GetValue(*mpAreaVariable);
    }
}

/***********************************************************************************/
/***********************************************************************************/

template <>
void ComputeNodalGradientProcess<ComputeNodalGradientProcessSettings::SaveAsNonHistoricalVariable>::PonderateGradient()
{
    const auto it_node_begin = mrModelPart.NodesBegin();

    #pragma omp parallel for
    for(int i = 0; i < static_cast<int>(mrModelPart.Nodes().size()); ++i) {
        auto it_node = it_node_begin + i;
        it_node->GetValue(*mpGradientVariable) /= it_node->GetValue(*mpAreaVariable);
    }
}

/***********************************************************************************/
/***********************************************************************************/

template<bool THistorical>
const Parameters ComputeNodalGradientProcess<THistorical>::GetDefaultParameters() const
{
    Parameters default_parameters = Parameters(R"(
    {
        "origin_variable"                : "PLEASE_DEFINE_A_VARIABLE",
        "non_historical_origin_variable" :  false
    })" );

    return default_parameters;
}

/***********************************************************************************/
/***********************************************************************************/

template class ComputeNodalGradientProcess<ComputeNodalGradientProcessSettings::SaveAsHistoricalVariable>;
template class ComputeNodalGradientProcess<ComputeNodalGradientProcessSettings::SaveAsNonHistoricalVariable>;

} /* namespace Kratos.*/
