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

### Building
Use CMake (not qmake). Key dependencies: Qt6, GSL (GNU Scientific Library), Qwt (plotting).
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DADVANCED_FEATURES=ON
make -j2
```

### Testing
- Tests in `tests/` directory using custom CMake test framework
- Examples in `example/` directory (`.strata`, `.json` files)
- No comprehensive unit test suite - validation relies on comparison with SHAKE and analytical solutions

### Code Formatting
CI enforces clang-format. Run locally: `clang-format -i source/*.{cpp,h}`

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
