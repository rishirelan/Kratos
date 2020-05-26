from xmc.tools import dynamicImport
from xmc.tools import unpackedList

class ErrorEstimator():
    """
    This class is responsible for the error estimation of the multi-something MC
    estimation.
    """

    def __init__(self,**keywordArgs):
        # Attributes
        self.parameters = keywordArgs.get("parameters",None)

        # Methods
        self._errorMethod = dynamicImport(keywordArgs.get('error'))

    def error(self, *assembledEstimationsList):
        return self._errorMethod(self.parameters,*unpackedList(assembledEstimationsList))
