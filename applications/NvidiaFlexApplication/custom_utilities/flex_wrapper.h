#ifndef FLEX_WRAPPER_H
#define FLEX_WRAPPER_H

#include <string.h>
#include "includes/define.h"
#include "includes/model_part.h"
#include "includes/variables.h"

#include "custom_external_libraries/NvFlex.h"
#include "custom_external_libraries/NvFlexExt.h"
#include "custom_external_libraries/NvFlexDevice.h"
#define CUDA_CALLABLE
#include "custom_external_libraries/vec3.h"
#include "custom_external_libraries/vec4.h"

namespace Kratos {

    class KRATOS_API(NVIDIAFLEX_APPLICATION) FlexWrapper {

        public:

            KRATOS_CLASS_POINTER_DEFINITION(FlexWrapper);

            FlexWrapper(ModelPart& rSpheresModelPart);

            virtual ~FlexWrapper();

            void TransferDataFromKratosToFlex();
            void UpdateFlex();
            void SolveTimeSteps(double dt, int number_of_substeps);
            void SetNvFlexSolverDescParams(NvFlexSolverDesc& g_solverDesc);
            void SetNvFlexParams(NvFlexParams& g_params);
            void SetNvFlexCopyDescParams(NvFlexCopyDesc& copyDesc);
            void TransferDataFromFlexToKratos();
            void Finalize();

            virtual std::string Info() const;
            virtual void PrintInfo(std::ostream& rOStream) const;
            virtual void PrintData(std::ostream& rOStream) const;

        protected:

            NvFlexLibrary* mFlexLibrary;
            NvFlexSolverDesc mSolverDescriptor;
            NvFlexSolver* mFlexSolver;
            NvFlexParams mFlexParameters;
            NvFlexCopyDesc mFlexCopyDescriptor;
            NvFlexVector<Vec4>* mFlexPositions;
            NvFlexVector<Vec3>* mFlexVelocities;
            NvFlexVector<int>* mFlexPhases;
            NvFlexVector<int>* mActiveIndices;
            NvFlexVector<Vec4>* mFlexRestPositions;
            unsigned int mNumberOfParticles = 1;
            unsigned int mPhaseType = 1;
            double mDeltaTime = 0.0001;
            ModelPart& mrSpheresModelPart;

        private:

            FlexWrapper& operator=(FlexWrapper const& rOther);
    }; // Class FlexWrapper
} // namespace Kratos

#endif // FLEX_WRAPPER_H
