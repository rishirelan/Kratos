from __future__ import print_function, absolute_import, division #makes KratosMultiphysics backward compatible with python 2.6 and 2.7

import KratosMultiphysics

def CreateSolver(main_model_part, custom_settings):

    if (type(main_model_part) != KratosMultiphysics.ModelPart):
        raise Exception("input is expected to be provided as a Kratos ModelPart object")

    if (type(custom_settings) != KratosMultiphysics.Parameters):
        raise Exception("input is expected to be provided as a Kratos Parameters object")

    parallelism = custom_settings["problem_data"]["parallel_type"].GetString()
    solver_type = custom_settings["solver_settings"]["solver_type"].GetString()

    # Solvers for OpenMP parallelism
    if (parallelism == "OpenMP"):
        if (solver_type == "Dynamic"):
            time_integration_method = custom_settings["solver_settings"]["time_integration_method"].GetString()
            if (time_integration_method == "Implicit"):
                solver_module_name = "structural_mechanics_implicit_dynamic_solver"
            else:
                raise Exception("The requested time integration method \"" + time_integration_method + "\" is not in the python solvers wrapper\n" +
                                "Available options are: \"Implicit\"")

        elif (solver_type == "Static"):
            solver_module_name = "structural_mechanics_static_solver"

        elif (solver_type == "EigenValue"):
            solver_module_name = "structural_mechanics_eigensolver"

        else:
            raise Exception("The requested solver type \"" + solver_type + "\" is not in the python solvers wrapper\n" +
                            "Available options are: \"Static\", \"Dynamic\"")

    # Solvers for MPI parallelism
    elif (parallelism == "MPI"):
        if (solver_type == "Dynamic"):
            time_integration_method = custom_settings["solver_settings"]["time_integration_method"].GetString()
            if (time_integration_method == "Implicit"):
                solver_module_name = "trilinos_structural_mechanics_implicit_dynamic_solver"
            else:
                raise Exception("The requested time integration method \"" + time_integration_method + "\" is not in the python solvers wrapper\n" +
                                "Available options are: \"Implicit\"")

        elif (solver_type == "Static"):
            solver_module_name = "trilinos_structural_mechanics_static_solver"

        else:
            raise Exception("The requested solver type \"" + solver_type + "\" is not in the python solvers wrapper\n" +
                            "Available options are: \"Static\", \"Dynamic\"")
    else:
        raise Exception("The requested parallel type \"" + parallel_type + "\" is not available!\n" +
                        "Available options are: \"OpenMP\", \"MPI\"")

    solver_module = __import__(solver_module_name)
    solver = solver_module.CreateSolver(main_model_part, custom_settings["solver_settings"])

    return solver
