# Strata AI Coding Guidelines

Strata is a seismic site response analysis application that performs equivalent linear analysis using Qt6/C++17. It calculates how earthquake ground motions are modified as they propagate through layered soil deposits.

## Architecture Overview

### Core Components
- **AbstractCalculator hierarchy**: Three calculation methods
  - `LinearElasticCalculator`: Simple linear analysis
  - `EquivalentLinearCalculator`: Iterative SHAKE-type analysis (most common)
  - `FrequencyDependentCalculator`: Advanced frequency-dependent analysis (requires `ADVANCED_FEATURES`)
- **Motion types**: Time series, RVT (Random Vibration Theory), and source theory motions
- **SiteResponseModel**: Main model coordinating all components (inherits QThread for background calculations)
- **SoilProfile**: Layered soil column with auto-discretization into sublayers
- **OutputCatalog**: Manages calculation results and statistical analysis

### Key Patterns

#### Abstract Factory Pattern
All major components use abstract base classes with concrete implementations:
```cpp
// Calculator selection in SiteResponseModel::setMethod()
case EquivalentLinear:
    setCalculator(new EquivalentLinearCalculator(this));
    break;
```

#### Qt Model/View Pattern
Most data structures inherit from Qt's abstract models (`QAbstractListModel`, `QAbstractTableModel`) for GUI integration.

#### Modern Qt6 Signal/Slot Connections
Use type-safe function pointer syntax, not old SIGNAL/SLOT macros:
```cpp
// Correct Qt6 style
connect(model, &SiteResponseModel::progressChanged, progressBar, &QProgressBar::setValue);

// Old style - DO NOT USE
connect(model, SIGNAL(progressChanged(int)), progressBar, SLOT(setValue(int)));
```

#### Iterative Convergence
Equivalent linear analysis uses strain-compatible iterations:
```cpp
// Maximum iterations typically 10, convergence threshold typically 1-5%
while (maxError > tolerance && iterations < maxIterations)
```

## Development Workflows

### Branching and Release Workflow
- Feature and bugfix branches should be merged into `dev` prior to merging into `main`.
- Pushing to `dev` triggers the CI release workflow (`.github/workflows/release.yml`) which builds and packages platform-native installers (Windows, macOS, Linux) as downloadable workflow artifacts for verification.
- Once verified on `dev`, changes can be merged into `main` and tagged/released.

### Building
Uses CMake Presets + vcpkg (not qmake) for reproducible builds across platforms. Dependencies
(Qt6, GSL, Qwt) are fetched/built by the vcpkg submodule — do not assume they're preinstalled.
```bash
git submodule update --init --recursive   # if vcpkg/ submodule is missing
./vcpkg/bootstrap-vcpkg.sh                # one-time, Linux/macOS

cmake --preset linux-release              # or macos-release / windows-release / linux-debug
cmake --build --preset linux-release
```
`ADVANCED_FEATURES` (frequency-dependent calculator) is a CMake option, `ON` by default.
Compiled binary: `build/<preset-name>/source/strata`.

### Testing
- No unit test framework; regression tests compare full `strata -b <file>` runs against
  reference outputs using `scripts/compare_examples.py`.
- Registered as a single CTest test (`example_regression`) that runs the script over every
  case in `example/`:
  ```bash
  cd build/linux-release
  ctest -R example_regression --output-on-failure
  ```
- To test one example directly (faster iteration on a single case), invoke the script or the
  binary yourself:
  ```bash
  python3 scripts/compare_examples.py build/linux-release/source/strata example/<case-dir>
  # or run the batch mode directly and inspect output
  build/linux-release/source/strata -b example/<case-dir>/example.json
  ```
- `test/` contains legacy SHAKE2000 comparison data (`test/shake2000b/`) and standalone
  `.strata` fixtures, not part of the automated CTest suite.

### Code Formatting
No `.clang-format` file is checked in, so formatting follows clang-format's default (LLVM)
style. Enforced two ways — match whichever tool you have available:
- `pre-commit` hook (`.pre-commit-config.yaml`, clang-format v22) run on `source/`.
- CI workflow (`.github/workflows/clang-format.yml`) that auto-commits formatting fixes on push.
```bash
clang-format -i source/*.{cpp,h}
```

## Project-Specific Conventions

### File Organization
- All source in `source/` (no subdirectories)
- Headers and implementations co-located
- UI files (`.ui`) auto-processed by Qt's MOC
- Resources embedded via `resources/resources.qrc`

### Error Handling
Uses Qt's approach with signals/slots rather than exceptions:
```cpp
emit wasModified();  // Standard modification signal
_okToContinue = false;  // Calculation cancellation pattern
```

### Memory Management
Qt's parent-child ownership model. Objects with QObject parents auto-delete:
```cpp
// Proper pattern - parent takes ownership
auto *motion = new RvtMotion(_motionLibrary);
_motionLibrary->addMotion(motion);
```

### Coordinate Systems
- Depth increases downward (standard geotechnical convention)
- Input motions at bottom (bedrock), outputs computed upward
- Location class handles within/outcrop motion types

## Critical Implementation Details

### Binary File Format (Qt6 Migration)
Native `.strata` files use QDataStream serialization:
- **Qt6 files**: Use `QDataStream::Qt_6_0` with version marker for forward compatibility
- **Legacy files**: Automatically detected and loaded using `Qt_4_0` format
- Version detection based on checking if value after magic number is valid QDataStream version
- Increment serialization version when making breaking changes

### Layer Discretization
Unlike SHAKE, users define velocity layers that Strata auto-discretizes into sublayers based on wavelength:
```cpp
h_max = v_s / (f_max * discretization_factor)
```

### Property Randomization
Monte Carlo simulations vary:
- Layer thickness (lognormal distribution)
- Shear wave velocity (correlation with depth)
- Nonlinear curves (Darendeli models with uncertainty)

### Performance Optimization
Critical calculation loops are optimized for speed:
- Cache frequently accessed values in inner loops
- Use references to avoid repeated `at()` calls
- Pre-calculate complex values like `exp(cTerm)` when used multiple times
- Minimize function call overhead in tight loops

### Advanced Features
Code conditionally compiled with `ADVANCED_FEATURES`:
```cpp
#ifdef ADVANCED_FEATURES
    // Frequency-dependent equivalent linear analysis
#endif
```

## Integration Points

### External Dependencies
- **GSL**: Numerical routines, random number generation
- **Qwt**: Scientific plotting widgets
- **FFTW**: Optional, faster FFT than Qt's built-in

### Data Exchange
- Motion import: Text files (time series) or JSON (RVT parameters)
- Results export: CSV, JSON, or binary formats
- No direct SHAKE file compatibility (different sublayer approach)

## Common Pitfalls

1. **Threading**: Calculations run in separate threads via QThread inheritance - use Qt's threading patterns
2. **Units**: Mixed metric/imperial - check unit consistency in calculations
3. **Frequency Domain**: All calculations in frequency domain, even for time series input
4. **Auto-discretization**: Don't assume user-defined layers match calculation sublayers
5. **Qt6 Compatibility**: Always use modern signal/slot syntax and Qt6-compatible APIs
6. **Performance**: Avoid repeated `.at()` calls in tight loops - cache values or use direct indexing
