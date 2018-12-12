//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Inigo Lopez and Riccardo Rossi
//

#include "incompressible_potential_flow_element.h"

namespace Kratos
{

///////////////////////////////////////////////////////////////////////////////////////////////////
// Public Operations

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::CalculateLocalSystem(
    MatrixType &rLeftHandSideMatrix,
    VectorType &rRightHandSideVector,
    ProcessInfo &rCurrentProcessInfo)
{
    const IncompressiblePotentialFlowElement &r_this = *this;
    const int &wake = r_this.GetValue(WAKE);

    if (wake == 0) // Normal element (non-wake) - eventually an embedded
        CalculateLocalSystemNormalElement(rLeftHandSideMatrix, rRightHandSideVector);
    else // Wake element
        CalculateLocalSystemWakeElement(rLeftHandSideMatrix, rRightHandSideVector);
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::CalculateRightHandSide(
    VectorType &rRightHandSideVector,
    ProcessInfo &rCurrentProcessInfo)
{
    //TODO: improve speed
    Matrix tmp;
    CalculateLocalSystem(tmp, rRightHandSideVector, rCurrentProcessInfo);
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::EquationIdVector(
    EquationIdVectorType &rResult,
    ProcessInfo &CurrentProcessInfo)
{
    const IncompressiblePotentialFlowElement &r_this = *this;
    const int &wake = r_this.GetValue(WAKE);

    if (wake == 0) // Normal element
    {
        if (rResult.size() != NumNodes)
            rResult.resize(NumNodes, false);

        const int &kutta = r_this.GetValue(KUTTA);

        if (kutta == 0)
            GetEquationIdVectorNormalElement(rResult);
        else
            GetEquationIdVectorKuttaElement(rResult);
    }
    else // Wake element
    {
        if (rResult.size() != 2 * NumNodes)
            rResult.resize(2 * NumNodes, false);

        GetEquationIdVectorWakeElement(rResult);
    }
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::GetDofList(
    DofsVectorType &rElementalDofList,
    ProcessInfo &CurrentProcessInfo)
{
    const IncompressiblePotentialFlowElement &r_this = *this;
    const int &wake = r_this.GetValue(WAKE);

    if (wake == 0) //Normal element
    {
        if (rElementalDofList.size() != NumNodes)
            rElementalDofList.resize(NumNodes);

        const int &kutta = r_this.GetValue(KUTTA);

        if (kutta == 0)
            GetDofListNormalElement(rElementalDofList);
        else
            GetDofListKuttaElement(rElementalDofList);
    }
    else // wake element
    {
        if (rElementalDofList.size() != 2 * NumNodes)
            rElementalDofList.resize(2 * NumNodes);

        GetDofListWakeElement(rElementalDofList);
    }
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::FinalizeSolutionStep(
    ProcessInfo &rCurrentProcessInfo)
{
    bool active = true;
    if ((this)->IsDefined(ACTIVE))
        active = (this)->Is(ACTIVE);

    const IncompressiblePotentialFlowElement &r_this = *this;
    const int &wake = r_this.GetValue(WAKE);

    if (wake != 0 && active == true)
    {
        CheckWakeCondition();
        ComputePotentialJump(rCurrentProcessInfo);
    }
    ComputeElementInternalEnergy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Inquiry

template <int Dim, int NumNodes>
int IncompressiblePotentialFlowElement<Dim, NumNodes>::Check(const ProcessInfo &rCurrentProcessInfo)
{
    KRATOS_TRY

    // Generic geometry check
    int out = Element::Check(rCurrentProcessInfo);
    if (out != 0) {
        return out;
    }

    KRATOS_ERROR_IF(GetGeometry().Area() <= 0.0) << this->Id() << "Area cannot be less than or equal to 0" << std::endl;

    for (unsigned int i = 0; i < this->GetGeometry().size(); i++){
        KRATOS_ERROR_IF(this->GetGeometry()[i].SolutionStepsDataHas(VELOCITY_POTENTIAL) == false)
            << "missing variable VELOCITY_POTENTIAL on node " << this->GetGeometry()[i].Id() << std::endl;
    }

    return out;

    KRATOS_CATCH("");
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::GetValueOnIntegrationPoints(
    const Variable<double> &rVariable,
    std::vector<double> &rValues,
    const ProcessInfo &rCurrentProcessInfo)
{
    if (rValues.size() != 1)
        rValues.resize(1);

        if (rVariable == PRESSURE)
    {
        double p = 0.0;
        p = ComputePressureUpper(rCurrentProcessInfo);
        rValues[0] = p;
    }
    else if (rVariable == PRESSURE_LOWER)
    {
        double p = 0.0;
        p = ComputePressureLower(rCurrentProcessInfo);
        rValues[0] = p;
    }
    else if (rVariable == WAKE)
    {
        const IncompressiblePotentialFlowElement &r_this = *this;
        rValues[0] = r_this.GetValue(WAKE);
    }
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::GetValueOnIntegrationPoints(
    const Variable<int> &rVariable,
    std::vector<int> &rValues,
    const ProcessInfo &rCurrentProcessInfo)
{
    if (rValues.size() != 1)
        rValues.resize(1);
    if (rVariable == TRAILING_EDGE)
        rValues[0] = this->GetValue(TRAILING_EDGE);
    else if (rVariable == KUTTA)
        rValues[0] = this->GetValue(KUTTA);
    else if (rVariable == ALL_TRAILING_EDGE)
        rValues[0] = this->GetValue(ALL_TRAILING_EDGE);
    else if (rVariable == ZERO_VELOCITY_CONDITION)
        rValues[0] = this->GetValue(ZERO_VELOCITY_CONDITION);
    else if (rVariable == TRAILING_EDGE_ELEMENT)
        rValues[0] = this->GetValue(TRAILING_EDGE_ELEMENT);
    else if (rVariable == DECOUPLED_TRAILING_EDGE_ELEMENT)
        rValues[0] = this->GetValue(DECOUPLED_TRAILING_EDGE_ELEMENT);
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::GetValueOnIntegrationPoints(
    const Variable<array_1d<double, 3>> &rVariable,
    std::vector<array_1d<double, 3>> &rValues,
    const ProcessInfo &rCurrentProcessInfo)
{
    if (rValues.size() != 1)
        rValues.resize(1);
    if (rVariable == VELOCITY)
    {
        array_1d<double, 3> v(3, 0.0);
        array_1d<double, Dim> vaux;
        ComputeVelocityUpper(vaux);
        for (unsigned int k = 0; k < Dim; k++)
            v[k] = vaux[k];
        rValues[0] = v;
    }
    else if (rVariable == VELOCITY_LOWER)
    {
        array_1d<double, 3> v(3, 0.0);
        array_1d<double, Dim> vaux;
        ComputeVelocityLower(vaux);
        for (unsigned int k = 0; k < Dim; k++)
            v[k] = vaux[k];
        rValues[0] = v;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Input and output

template <int Dim, int NumNodes>
std::string IncompressiblePotentialFlowElement<Dim, NumNodes>::Info() const
{
    std::stringstream buffer;
    buffer << "IncompressiblePotentialFlowElement #" << Id();
    return buffer.str();
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::PrintInfo(std::ostream &rOStream) const
{
    rOStream << "IncompressiblePotentialFlowElement #" << Id();
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::PrintData(std::ostream &rOStream) const
{
    pGetGeometry()->PrintData(rOStream);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Protected functions
///////////////////////////////////////////////////////////////////////////////////////////////////

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::GetWakeDistances(array_1d<double, NumNodes> &distances)
{
    noalias(distances) = GetValue(ELEMENTAL_DISTANCES);
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::GetEquationIdVectorNormalElement(
    EquationIdVectorType &rResult)
{
    for (unsigned int i = 0; i < NumNodes; i++)
        rResult[i] = GetGeometry()[i].GetDof(VELOCITY_POTENTIAL).EquationId();
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::GetEquationIdVectorKuttaElement(
    EquationIdVectorType &rResult)
{
    // Kutta elements have only negative part
    for (unsigned int i = 0; i < NumNodes; i++)
    {
        if (!GetGeometry()[i].FastGetSolutionStepValue(TRAILING_EDGE))
            rResult[i] = GetGeometry()[i].GetDof(VELOCITY_POTENTIAL).EquationId();
        else
            rResult[i] = GetGeometry()[i].GetDof(AUXILIARY_VELOCITY_POTENTIAL).EquationId();
    }
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::GetEquationIdVectorWakeElement(
    EquationIdVectorType &rResult)
{
    array_1d<double, NumNodes> distances;
    GetWakeDistances(distances);

    // Positive part
    for (unsigned int i = 0; i < NumNodes; i++)
    {
        if (distances[i] > 0.0)
            rResult[i] = GetGeometry()[i].GetDof(VELOCITY_POTENTIAL).EquationId();
        else
            rResult[i] = GetGeometry()[i].GetDof(AUXILIARY_VELOCITY_POTENTIAL, 0).EquationId();
    }

    // Negative part - sign is opposite to the previous case
    for (unsigned int i = 0; i < NumNodes; i++)
    {
        if (distances[i] < 0.0)
            rResult[NumNodes + i] = GetGeometry()[i].GetDof(VELOCITY_POTENTIAL).EquationId();
        else
            rResult[NumNodes + i] = GetGeometry()[i].GetDof(AUXILIARY_VELOCITY_POTENTIAL).EquationId();
    }
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::GetDofListNormalElement(
    DofsVectorType &rElementalDofList)
{
    for (unsigned int i = 0; i < NumNodes; i++)
        rElementalDofList[i] = GetGeometry()[i].pGetDof(VELOCITY_POTENTIAL);
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::GetDofListKuttaElement(
    DofsVectorType &rElementalDofList)
{
    // Kutta elements have only negative part
    for (unsigned int i = 0; i < NumNodes; i++)
    {
        if (!GetGeometry()[i].FastGetSolutionStepValue(TRAILING_EDGE))
            rElementalDofList[i] = GetGeometry()[i].pGetDof(VELOCITY_POTENTIAL);
        else
            rElementalDofList[i] = GetGeometry()[i].pGetDof(AUXILIARY_VELOCITY_POTENTIAL);
    }
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::GetDofListWakeElement(
    DofsVectorType &rElementalDofList)
{
    array_1d<double, NumNodes> distances;
    GetWakeDistances(distances);

    // Positive part
    for (unsigned int i = 0; i < NumNodes; i++)
    {
        if (distances[i] > 0)
            rElementalDofList[i] = GetGeometry()[i].pGetDof(VELOCITY_POTENTIAL);
        else
            rElementalDofList[i] = GetGeometry()[i].pGetDof(AUXILIARY_VELOCITY_POTENTIAL);
    }

    // Negative part - sign is opposite to the previous case
    for (unsigned int i = 0; i < NumNodes; i++)
    {
        if (distances[i] < 0)
            rElementalDofList[NumNodes + i] = GetGeometry()[i].pGetDof(VELOCITY_POTENTIAL);
        else
            rElementalDofList[NumNodes + i] = GetGeometry()[i].pGetDof(AUXILIARY_VELOCITY_POTENTIAL);
    }
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::CalculateLocalSystemNormalElement(
    MatrixType &rLeftHandSideMatrix,
    VectorType &rRightHandSideVector)
{
    if (rLeftHandSideMatrix.size1() != NumNodes || rLeftHandSideMatrix.size2() != NumNodes)
        rLeftHandSideMatrix.resize(NumNodes, NumNodes, false);
    if (rRightHandSideVector.size() != NumNodes)
        rRightHandSideVector.resize(NumNodes, false);
    rLeftHandSideMatrix.clear();

    ElementalData<NumNodes, Dim> data;

    // Calculate shape functions
    GeometryUtils::CalculateGeometryData(GetGeometry(), data.DN_DX, data.N, data.vol);

    ComputeLHSGaussPointContribution(data.vol, rLeftHandSideMatrix, data);

    GetPotentialOnNormalElement(data.phis);
    noalias(rRightHandSideVector) = -prod(rLeftHandSideMatrix, data.phis);
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::CalculateLocalSystemWakeElement(
    MatrixType &rLeftHandSideMatrix,
    VectorType &rRightHandSideVector)
{
    // Note that the lhs and rhs have double the size
    if (rLeftHandSideMatrix.size1() != 2 * NumNodes || rLeftHandSideMatrix.size2() != 2 * NumNodes)
        rLeftHandSideMatrix.resize(2 * NumNodes, 2 * NumNodes, false);
    if (rRightHandSideVector.size() != 2 * NumNodes)
        rRightHandSideVector.resize(2 * NumNodes, false);
    rLeftHandSideMatrix.clear();
    rRightHandSideVector.clear();

    ElementalData<NumNodes, Dim> data;

    // Calculate shape functions
    GeometryUtils::CalculateGeometryData(GetGeometry(), data.DN_DX, data.N, data.vol);

    GetWakeDistances(data.distances);

    Matrix lhs_total = ZeroMatrix(NumNodes, NumNodes);

    ComputeLHSGaussPointContribution(data.vol, lhs_total, data);

    if (this->Is(STRUCTURE))
    {
        Matrix lhs_positive = ZeroMatrix(NumNodes, NumNodes);
        Matrix lhs_negative = ZeroMatrix(NumNodes, NumNodes);

        CalculateLocalSystemSubdividedElement(lhs_positive, lhs_negative);
        AssignLocalSystemSubdividedElement(rLeftHandSideMatrix, lhs_positive, lhs_negative, lhs_total, data);
    }
    else
        AssignLocalSystemWakeElement(rLeftHandSideMatrix, lhs_total, data);
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::CalculateLocalSystemSubdividedElement(
    Matrix &lhs_positive,
    Matrix &lhs_negative)
{
    ElementalData<NumNodes, Dim> data;

    // Calculate shape functions
    GeometryUtils::CalculateGeometryData(GetGeometry(), data.DN_DX, data.N, data.vol);

    GetWakeDistances(data.distances);

    // Subdivide the element
    constexpr unsigned int nvolumes = 3 * (Dim - 1);
    bounded_matrix<double, NumNodes, Dim> Points;
    array_1d<double, nvolumes> PartitionsSign;
    bounded_matrix<double, nvolumes, NumNodes> GPShapeFunctionValues;
    array_1d<double, nvolumes> Volumes;
    std::vector<Matrix> GradientsValue(nvolumes);
    bounded_matrix<double, nvolumes, 2> NEnriched;
    for (unsigned int i = 0; i < GradientsValue.size(); ++i)
        GradientsValue[i].resize(2, Dim, false);
    for (unsigned int i = 0; i < NumNodes; ++i)
    {
        const array_1d<double, 3> &coords = GetGeometry()[i].Coordinates();
        for (unsigned int k = 0; k < Dim; ++k)
        {
            Points(i, k) = coords[k];
        }
    }

    const unsigned int nsubdivisions = EnrichmentUtilities::CalculateEnrichedShapeFuncions(Points,
                                                                                           data.DN_DX,
                                                                                           data.distances,
                                                                                           Volumes,
                                                                                           GPShapeFunctionValues,
                                                                                           PartitionsSign,
                                                                                           GradientsValue,
                                                                                           NEnriched);

    // Compute the lhs and rhs that would correspond to it being divided
    for (unsigned int i = 0; i < nsubdivisions; ++i)
    {
        if (PartitionsSign[i] > 0)
            ComputeLHSGaussPointContribution(Volumes[i], lhs_positive, data);
        else
            ComputeLHSGaussPointContribution(Volumes[i], lhs_negative, data);
    }
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::ComputeLHSGaussPointContribution(
    const double weight,
    Matrix &lhs,
    const ElementalData<NumNodes, Dim> &data)
{
    noalias(lhs) += weight * prod(data.DN_DX, trans(data.DN_DX));
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::AssignLocalSystemSubdividedElement(
    MatrixType &rLeftHandSideMatrix,
    Matrix &lhs_positive,
    Matrix &lhs_negative,
    Matrix &lhs_total,
    const ElementalData<NumNodes, Dim> &data)
{
    for (unsigned int i = 0; i < NumNodes; ++i)
    {
        // The TE node takes the contribution of the subdivided element and
        // we do not apply the wake condition on the TE node
        if (GetGeometry()[i].FastGetSolutionStepValue(TRAILING_EDGE))
        {
            for (unsigned int j = 0; j < NumNodes; ++j)
            {
                rLeftHandSideMatrix(i, j) = lhs_positive(i, j);
                rLeftHandSideMatrix(i + NumNodes, j + NumNodes) = lhs_negative(i, j);
            }
        }
        else
            AssignLocalSystemWakeNode(rLeftHandSideMatrix, lhs_total, data, i);
    }
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::AssignLocalSystemWakeElement(
    MatrixType &rLeftHandSideMatrix,
    Matrix &lhs_total,
    const ElementalData<NumNodes, Dim> &data)
{
    for (unsigned int row = 0; row < NumNodes; ++row)
        AssignLocalSystemWakeNode(rLeftHandSideMatrix, lhs_total, data, row);
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::AssignLocalSystemWakeNode(
    MatrixType &rLeftHandSideMatrix,
    Matrix &lhs_total,
    const ElementalData<NumNodes, Dim> &data,
    unsigned int &row)
{
    // Filling the diagonal blocks (i.e. decoupling upper and lower dofs)
    for (unsigned int column = 0; column < NumNodes; ++column)
    {
        rLeftHandSideMatrix(row, column) = lhs_total(row, column);
        rLeftHandSideMatrix(row + NumNodes, column + NumNodes) = lhs_total(row, column);
    }

    // Applying wake condition on the AUXILIARY_VELOCITY_POTENTIAL dofs
    if (data.distances[row] < 0.0)
        for (unsigned int column = 0; column < NumNodes; ++column)
            rLeftHandSideMatrix(row, column + NumNodes) = -lhs_total(row, column); //Side 1
    else if (data.distances[row] > 0.0)
        for (unsigned int column = 0; column < NumNodes; ++column)
            rLeftHandSideMatrix(row + NumNodes, column) = -lhs_total(row, column); //Side 2
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::CheckWakeCondition()
{
    array_1d<double, Dim> upper_wake_velocity;
    ComputeVelocityUpperWakeElement(upper_wake_velocity);
    const double vupnorm = inner_prod(upper_wake_velocity, upper_wake_velocity);

    array_1d<double, Dim> lower_wake_velocity;
    ComputeVelocityLowerWakeElement(lower_wake_velocity);
    const double vlownorm = inner_prod(lower_wake_velocity, lower_wake_velocity);

    if (std::abs(vupnorm - vlownorm) > 0.1)
        std::cout << "WAKE CONDITION NOT FULFILLED IN ELEMENT # " << this->Id() << std::endl;
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::ComputePotentialJump(
    ProcessInfo &rCurrentProcessInfo)
{
    const array_1d<double, 3> vinfinity = rCurrentProcessInfo[VELOCITY_INFINITY];
    const double vinfinity_norm = sqrt(inner_prod(vinfinity, vinfinity));

    array_1d<double, NumNodes> distances;
    GetWakeDistances(distances);

    for (unsigned int i = 0; i < NumNodes; i++)
        if (distances[i] > 0)
            GetGeometry()[i].GetSolutionStepValue(POTENTIAL_JUMP) = 2.0 / vinfinity_norm * (GetGeometry()[i].FastGetSolutionStepValue(AUXILIARY_VELOCITY_POTENTIAL) - GetGeometry()[i].FastGetSolutionStepValue(VELOCITY_POTENTIAL));
        else
            GetGeometry()[i].GetSolutionStepValue(POTENTIAL_JUMP) = 2.0 / vinfinity_norm * (GetGeometry()[i].FastGetSolutionStepValue(VELOCITY_POTENTIAL) - GetGeometry()[i].FastGetSolutionStepValue(AUXILIARY_VELOCITY_POTENTIAL));
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::ComputeElementInternalEnergy()
{
    double internal_energy = 0.0;
    array_1d<double, Dim> velocity;

    const IncompressiblePotentialFlowElement &r_this = *this;
    const int &wake = r_this.GetValue(WAKE);

    if (wake == 0) // Normal element (non-wake) - eventually an embedded
        ComputeVelocityNormalElement(velocity);
    else // Wake element
        ComputeVelocityUpperWakeElement(velocity);

    internal_energy = 0.5 * inner_prod(velocity, velocity);
    this->SetValue(INTERNAL_ENERGY, abs(internal_energy));
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::GetPotentialOnNormalElement(
    array_1d<double, NumNodes> &phis)
{
    const IncompressiblePotentialFlowElement &r_this = *this;
    const int &kutta = r_this.GetValue(KUTTA);

    if (kutta == 0)
        for (unsigned int i = 0; i < NumNodes; i++)
            phis[i] = GetGeometry()[i].FastGetSolutionStepValue(VELOCITY_POTENTIAL);
    else
        for (unsigned int i = 0; i < NumNodes; i++)
            if (!GetGeometry()[i].FastGetSolutionStepValue(TRAILING_EDGE))
                phis[i] = GetGeometry()[i].FastGetSolutionStepValue(VELOCITY_POTENTIAL);
            else
                phis[i] = GetGeometry()[i].FastGetSolutionStepValue(AUXILIARY_VELOCITY_POTENTIAL);
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::GetPotentialOnWakeElement(
    Vector &split_element_values,
    const array_1d<double, NumNodes> &distances)
{
    array_1d<double, NumNodes> upper_phis;
    GetPotentialOnUpperWakeElement(upper_phis, distances);

    array_1d<double, NumNodes> lower_phis;
    GetPotentialOnLowerWakeElement(lower_phis, distances);

    for (unsigned int i = 0; i < NumNodes; i++)
    {
        split_element_values[i] = upper_phis[i];
        split_element_values[NumNodes + i] = lower_phis[i];
    }
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::GetPotentialOnUpperWakeElement(
    array_1d<double, NumNodes> &upper_phis,
    const array_1d<double, NumNodes> &distances)
{
    for (unsigned int i = 0; i < NumNodes; i++)
        if (distances[i] > 0)
            upper_phis[i] = GetGeometry()[i].FastGetSolutionStepValue(VELOCITY_POTENTIAL);
        else
            upper_phis[i] = GetGeometry()[i].FastGetSolutionStepValue(AUXILIARY_VELOCITY_POTENTIAL);
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::GetPotentialOnLowerWakeElement(
    array_1d<double, NumNodes> &lower_phis,
    const array_1d<double, NumNodes> &distances)
{
    for (unsigned int i = 0; i < NumNodes; i++)
        if (distances[i] < 0)
            lower_phis[i] = GetGeometry()[i].FastGetSolutionStepValue(VELOCITY_POTENTIAL);
        else
            lower_phis[i] = GetGeometry()[i].FastGetSolutionStepValue(AUXILIARY_VELOCITY_POTENTIAL);
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::ComputeVelocityUpper(array_1d<double, Dim> &velocity)
{
    velocity.clear();

    const IncompressiblePotentialFlowElement &r_this = *this;
    const int &wake = r_this.GetValue(WAKE);

    if (wake == 0)
        ComputeVelocityNormalElement(velocity);
    else
        ComputeVelocityUpperWakeElement(velocity);
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::ComputeVelocityLower(array_1d<double, Dim> &velocity)
{
    velocity.clear();

    const IncompressiblePotentialFlowElement &r_this = *this;
    const int &wake = r_this.GetValue(WAKE);

    if (wake == 0)
        ComputeVelocityNormalElement(velocity);
    else
        ComputeVelocityLowerWakeElement(velocity);
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::ComputeVelocityNormalElement(array_1d<double, Dim> &velocity)
{
    ElementalData<NumNodes, Dim> data;

    // Calculate shape functions
    GeometryUtils::CalculateGeometryData(GetGeometry(), data.DN_DX, data.N, data.vol);

    GetPotentialOnNormalElement(data.phis);

    noalias(velocity) = prod(trans(data.DN_DX), data.phis);
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::ComputeVelocityUpperWakeElement(array_1d<double, Dim> &velocity)
{
    ElementalData<NumNodes, Dim> data;

    // Calculate shape functions
    GeometryUtils::CalculateGeometryData(GetGeometry(), data.DN_DX, data.N, data.vol);

    array_1d<double, NumNodes> distances;
    GetWakeDistances(distances);

    GetPotentialOnUpperWakeElement(data.phis, distances);

    noalias(velocity) = prod(trans(data.DN_DX), data.phis);
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::ComputeVelocityLowerWakeElement(array_1d<double, Dim> &velocity)
{
    ElementalData<NumNodes, Dim> data;

    // Calculate shape functions
    GeometryUtils::CalculateGeometryData(GetGeometry(), data.DN_DX, data.N, data.vol);

    array_1d<double, NumNodes> distances;
    GetWakeDistances(distances);

    GetPotentialOnLowerWakeElement(data.phis, distances);

    noalias(velocity) = prod(trans(data.DN_DX), data.phis);
}

template <int Dim, int NumNodes>
double IncompressiblePotentialFlowElement<Dim, NumNodes>::ComputePressureUpper(const ProcessInfo &rCurrentProcessInfo)
{
    double pressure = 0.0;

    const IncompressiblePotentialFlowElement &r_this = *this;
    const int &wake = r_this.GetValue(WAKE);

    if (wake == 0)
        pressure = ComputePressureNormalElement(rCurrentProcessInfo);
    else
        pressure = ComputePressureUpperWakeElement(rCurrentProcessInfo);

    return pressure;
}

template <int Dim, int NumNodes>
double IncompressiblePotentialFlowElement<Dim, NumNodes>::ComputePressureLower(const ProcessInfo &rCurrentProcessInfo)
{
    double pressure = 0.0;

    const IncompressiblePotentialFlowElement &r_this = *this;
    const int &wake = r_this.GetValue(WAKE);

    if (wake == 0)
        pressure = ComputePressureNormalElement(rCurrentProcessInfo);
    else
        pressure = ComputePressureLowerWakeElement(rCurrentProcessInfo);

    return pressure;
}

template <int Dim, int NumNodes>
double IncompressiblePotentialFlowElement<Dim, NumNodes>::ComputePressureNormalElement(const ProcessInfo &rCurrentProcessInfo)
{
    double pressure = 0.0;

    const array_1d<double, 3> vinfinity = rCurrentProcessInfo[VELOCITY_INFINITY];
    const double vinfinity_norm2 = inner_prod(vinfinity, vinfinity);

    KRATOS_ERROR_IF(vinfinity_norm2 < std::numeric_limits<double>::epsilon())
        << "Error on element -> " << this->Id() << "\n"
        << "vinfinity_norm2 must be larger than zero." << std::endl;

    array_1d<double, Dim> v;
    ComputeVelocityNormalElement(v);

    pressure = (vinfinity_norm2 - inner_prod(v, v)) / vinfinity_norm2; //0.5*(norm_2(vinfinity) - norm_2(v));
    return pressure;
}

template <int Dim, int NumNodes>
double IncompressiblePotentialFlowElement<Dim, NumNodes>::ComputePressureUpperWakeElement(const ProcessInfo &rCurrentProcessInfo)
{
    double pressure = 0.0;

    const array_1d<double, 3> vinfinity = rCurrentProcessInfo[VELOCITY_INFINITY];
    const double vinfinity_norm2 = inner_prod(vinfinity, vinfinity);

    KRATOS_ERROR_IF(vinfinity_norm2 < std::numeric_limits<double>::epsilon())
        << "Error on element -> " << this->Id() << "\n"
        << "vinfinity_norm2 must be larger than zero." << std::endl;

    array_1d<double, Dim> v;
    ComputeVelocityUpperWakeElement(v);

    pressure = (vinfinity_norm2 - inner_prod(v, v)) / vinfinity_norm2; //0.5*(norm_2(vinfinity) - norm_2(v));

    return pressure;
}

template <int Dim, int NumNodes>
double IncompressiblePotentialFlowElement<Dim, NumNodes>::ComputePressureLowerWakeElement(const ProcessInfo &rCurrentProcessInfo)
{
    double pressure = 0.0;

    const array_1d<double, 3> vinfinity = rCurrentProcessInfo[VELOCITY_INFINITY];
    const double vinfinity_norm2 = inner_prod(vinfinity, vinfinity);

    KRATOS_ERROR_IF(vinfinity_norm2 < std::numeric_limits<double>::epsilon())
        << "Error on element -> " << this->Id() << "\n"
        << "vinfinity_norm2 must be larger than zero." << std::endl;

    array_1d<double, Dim> v;
    ComputeVelocityLowerWakeElement(v);

    pressure = (vinfinity_norm2 - inner_prod(v, v)) / vinfinity_norm2; //0.5*(norm_2(vinfinity) - norm_2(v));

    return pressure;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Private functions
///////////////////////////////////////////////////////////////////////////////////////////////////

// serializer

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::save(Serializer &rSerializer) const
{
    KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, Element);
}

template <int Dim, int NumNodes>
void IncompressiblePotentialFlowElement<Dim, NumNodes>::load(Serializer &rSerializer)
{
    KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, Element);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Template class instantiation

template class IncompressiblePotentialFlowElement<2, 3>;

} // namespace Kratos
