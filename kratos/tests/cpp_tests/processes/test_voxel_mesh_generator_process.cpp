//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Pooyan Dadvand
//					 Ruben Zorrilla
//

// Project includes
#include "testing/testing.h"
#include "containers/model.h"
#include "includes/checks.h"
// #include "includes/gid_io.h"
#include "geometries/hexahedra_3d_8.h"
#include "geometries/tetrahedra_3d_4.h"
#include "processes/voxel_mesh_generator_process.h"
#include "processes/voxel_mesh_coloring_process.h"

namespace Kratos {
  namespace Testing {


	KRATOS_TEST_CASE_IN_SUITE(VoxelMeshGeneratorTetrahedraSkinProcess, KratosCoreFastSuite)
	{
		Parameters mesher_parameters(R"(
		{
			"number_of_divisions":   10,
			"element_name":     "Element3D4N"
		})");

        Model current_model;
		ModelPart &volume_part = current_model.CreateModelPart("Volume");
		volume_part.AddNodalSolutionStepVariable(VELOCITY);
		volume_part.AddNodalSolutionStepVariable(DISTANCE);
		volume_part.AddNodalSolutionStepVariable(EMBEDDED_VELOCITY);

		// Generate the skin
		ModelPart &skin_model_part = current_model.CreateModelPart("Skin");
        ModelPart &skin_part = skin_model_part.CreateSubModelPart("SkinPart");

		skin_part.AddNodalSolutionStepVariable(VELOCITY);
		skin_part.CreateNewNode(901, 2.0, 2.0, 2.0);
		skin_part.CreateNewNode(902, 6.0, 2.0, 2.0);
		skin_part.CreateNewNode(903, 4.0, 6.0, 2.0);
		skin_part.CreateNewNode(904, 4.0, 4.0, 7.0);
		Properties::Pointer p_properties(new Properties(0));
		skin_part.CreateNewElement("Element3D3N", 901, { 901,902,903 }, p_properties);
		skin_part.CreateNewElement("Element3D3N", 902, { 901,904,903 }, p_properties);
		skin_part.CreateNewElement("Element3D3N", 903, { 902,903,904 }, p_properties);
		skin_part.CreateNewElement("Element3D3N", 904, { 901,902,904 }, p_properties);

		// Generating the mesh
		VoxelMeshGeneratorProcess(Point{0.00, 0.00, 0.00}, Point{10.00, 10.00, 10.00}, volume_part, skin_model_part, mesher_parameters).Execute();
		// Compute distance
		VoxelMeshColoringProcess(Point{0.00, 0.00, 0.00}, Point{10.00, 10.00, 10.00}, volume_part, skin_model_part, mesher_parameters).Execute();


		Tetrahedra3D4<Node<3>> tetrahedra(skin_part.pGetNode(901), skin_part.pGetNode(902), skin_part.pGetNode(903), skin_part.pGetNode(904));

        Point dummy(0.00,0.00,0.00);
        for(auto& element : volume_part.Elements()){
            if(tetrahedra.IsInside(element.GetGeometry().Center(),dummy)){
                KRATOS_CHECK_NEAR(element.GetValue(DISTANCE), -1.00, 1e-6);
            }
	}

		//GidIO<> gid_io_fluid("C:/Temp/Tests/distance_test_fluid", GiD_PostAscii, SingleFile, WriteDeformed, WriteConditions);
		//gid_io_fluid.InitializeMesh(0.00);
		//gid_io_fluid.WriteMesh(volume_part.GetMesh());
		//gid_io_fluid.FinalizeMesh();
		//gid_io_fluid.InitializeResults(0, volume_part.GetMesh());
		//gid_io_fluid.WriteNodalResults(DISTANCE, volume_part.Nodes(), 0, 0);
		//gid_io_fluid.FinalizeResults();

	}


}
}  // namespace Kratos.
