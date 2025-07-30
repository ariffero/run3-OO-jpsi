# Fits to the invariant mass distribution

This folder contains the macro used to perform the fit to the invariant mass distribution.

Content:

1. **fitJPsi.cpp** is the macro that perform the fits to the mass distribution. It takes the kinematic cuts from the configuration `config.cfg`. It also needs the header `../library/savedVarInMassFits.h` to save the results in a tree.

2. **config.cfg** contains the kinematic cuts used in the fits. It can caontain different sets of cuts, arranged by name.

Note: at the moment, the parameters are estimated only using the data. The fits need a lot of improvments. The machinery is ok, but the parameters would need some change.
