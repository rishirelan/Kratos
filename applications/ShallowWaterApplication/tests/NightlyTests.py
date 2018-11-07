from shallow_water_test_factory import ShallowWaterTestFactory as TestFactory

class Pfem2ConservedVariables(TestFactory):
    file_name = "element_tests/pfem2_conserved_variables"

class EulerianPrimitiveVariables(TestFactory):
    file_name = "element_tests/euler_primitive_variables"

class EulerianConservedVariables(TestFactory):
    file_name = "element_tests/euler_conserved_variables"
