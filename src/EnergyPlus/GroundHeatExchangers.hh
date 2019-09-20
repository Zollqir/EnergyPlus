// EnergyPlus, Copyright (c) 1996-2019, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef GroundHeatExchangers_hh_INCLUDED
#define GroundHeatExchangers_hh_INCLUDED

// C++ headers
#include <deque>
#include <utility>

// ObjexxFCL Headers
//#include <ObjexxFCL/Array1D.hh>

// JSON Headers
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// EnergyPlus Headers
#include <DataGlobals.hh>
#include <EnergyPlus.hh>
#include <UtilityRoutines.hh>
//#include <GroundTemperatureModeling/GroundTemperatureModelManager.hh>
//#include <PlantComponent.hh>

namespace EnergyPlus {

namespace GroundHeatExchangers {

    // Using/Aliasing
    using DataGlobals::Pi;
    // using namespace GroundTemperatureManager;

    // Data
    // DERIVED TYPE DEFINITIONS

    // MODULE PARAMETER DEFINITIONS
    // extern Real64 const hrsPerDay;   // Number of hours in a day
    // extern Real64 const hrsPerMonth; // Number of hours in month
    // extern int const maxTSinHr;      // Max number of time step in a hour

    // MODULE VARIABLE DECLARATIONS:
    // na

    struct BaseProps
    {
        // member variables
        Real64 k = 0.0;           // Thermal conductivity [W/m-K]
        Real64 rho = 0.0;         // Density [kg/m3]
        Real64 cp = 0.0;          // Specific heat [J/kg-K]
        Real64 rhoCp = 0.0;       // Heat capacity [J/m3-K]
        Real64 diffusivity = 0.0; // Thermal diffusivity [m2/s]

        // constructor
        explicit BaseProps(const json &j)
        {
            this->k = j["conductivity"];
            this->rho = j["density"];
            this->cp = j["specific-heat"];
            this->rhoCp = rho * cp;
            this->diffusivity = k / this->rhoCp;
        }

        // default constructor
        BaseProps() = default;

        // destructor
        ~BaseProps() = default;
    };

    struct FluidWorker
    {
        // E+ member variables
        int loopNum = 0;

        // constructor
        explicit FluidWorker(const json &j) {
            this->loopNum = j["loop-num"];
        }

        // default constructor
        FluidWorker() = default;

        // destructor
        ~FluidWorker() = default;

        // member functions
        Real64 get_cp(Real64 &temperature, const std::string &routineName);
        Real64 get_k(Real64 &temperature, const std::string &routineName);
        Real64 get_mu(Real64 &temperature, const std::string &routineName);
        Real64 get_rho(Real64 &temperature, const std::string &routineName);
        Real64 get_Pr(Real64 &temperature, const std::string &routineName);
    };

    struct Pipe : public BaseProps, FluidWorker
    {
        // E+ member variables
        int loopNum = 0;

        // parent classes
        FluidWorker fluid;

        // model member variables
        Real64 outDia = 0.0;        // Outer diameter [m]
        Real64 innerDia = 0.0;      // Inner diameter [m]
        Real64 length = 0.0;        // Length [m]
        Real64 outRadius = 0.0;     // Outer radius [m]
        Real64 innerRadius = 0.0;   // Inner radius [m]
        Real64 wallThickness = 0.0; // Pipe wall thickness [m]
        Real64 areaCrOuter = 0.0;   // Outer cross-sectional area [m2]
        Real64 areaCrInner = 0.0;   // Inner cross-sectional area [m2]
        Real64 areaCrPipe = 0.0;    // Pipe wall cross-sectional area [m2]
        Real64 areaSurfOuter = 0.0; // Pipe outer surface area [m2]
        Real64 areaSurfInner = 0.0; // Pipe inner surface area [m2]
        Real64 volTotal = 0.0;      // Total pipe volume [m3]
        Real64 volFluid = 0.0;      // Fluid volume [m3]
        Real64 volPipeWall = 0.0;   // Pipe wall volume [m3]
        Real64 friction = 0.0;      // Friction factor [-]
        Real64 resistPipe = 0.0;    // Total pipe resistance [K/(W/m)]
        Real64 resistConv = 0.0;    // Pipe convection resistance [K/(W/m)]
        int const numCells = 16;    // Number of pipe elements
        std::vector<Real64> cellTemps = {
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // Pipe temperature for each node
        std::deque<Real64> inletTemps = {0.0};                                              // Inlet temperature history [C]
        std::deque<Real64> inletTempTimes = {0.0};                                           // Times for respective inlet temperatures [s]
        Real64 outletTemp = 0.0;                                                             // Pipe outlet temperature [C]
        bool applyTransitDelay = true;

        // constructor
        explicit Pipe(const json &j)
        {
            // properties
            BaseProps tmpProps(j);
            this->fluid = FluidWorker(j);
            this->k = tmpProps.k;
            this->rho = tmpProps.rho;
            this->cp = tmpProps.cp;
            this->rhoCp = tmpProps.rhoCp;
            this->diffusivity = tmpProps.diffusivity;

            // geometry
            this->outDia = j["outer-diameter"];
            this->innerDia = j["inner-diameter"];
            this->length = j["length"];
            this->outRadius = this->outDia / 2;
            this->innerRadius = this->innerDia / 2;
            this->wallThickness = this->outRadius - this->innerRadius;

            // areas
            this->areaCrOuter = (Pi / 4) * pow(this->outDia, 2);
            this->areaCrInner = (Pi / 4) * pow(this->innerDia, 2);
            this->areaCrPipe = this->areaCrOuter - this->areaCrInner;
            this->areaSurfOuter = Pi * this->outDia * this->length;
            this->areaSurfInner = Pi * this->innerDia * this->length;

            // volumes
            this->volTotal = this->areaCrOuter * this->length;
            this->volFluid = this->areaCrInner * this->length;
            this->volPipeWall = this->volTotal - this->volFluid;

            Real64 initTemp = j["initial-temperature"];
            std::replace(this->cellTemps.begin(), this->cellTemps.end(), 0.0, initTemp);
            std::replace(this->inletTemps.begin(), this->inletTemps.end(), 0.0, initTemp);
        }

        // default constructor
        Pipe() = default;

        // destructor
        ~Pipe() = default;

        // members functions
        Real64 calcTransitTime(Real64 flowRate, Real64 temperature);
        void simulate(Real64 time, Real64 timeStep, Real64 flowRate, Real64 inletTemp);
        Real64 plugFlowOutletTemp(Real64 time);
        void logInletTemps(Real64 inletTemp, Real64 time);
        Real64 mdotToRe(Real64 flowRate, Real64 temperature);
        Real64 calcFrictionFactor(Real64 Re);
        Real64 calcConductionResistance();
        Real64 calcConvectionResistance(Real64 flowRate, Real64 temperature);
        Real64 calcResistance(Real64 flowRate, Real64 temperature);
        Real64 turbulentNusselt(Real64 Re, Real64 temperature);

        static Real64 laminarNusselt()
        {
            // laminar Nusselt number for smooth pipes
            // mean(4.36, 3.66)

            return 4.01;
        }

        static Real64 laminarFrictionFactor(Real64 Re)
        {
            // laminar friction factor

            // @param Re: Reynolds number

            return 64 / Re;
        }

        static Real64 turbulentFrictionFactor(Real64 Re)
        {
            // turbulent friction factor

            // Petukhov, B. S. (1970). Advances in Heat Transfer, volume 6, chapter Heat transfer and
            // friction in turbulent pipe flow with variable physical properties, pages 503–564.
            // Academic Press, Inc., New York, NY.

            // @param Re: Reynolds number

            return std::pow(0.79 * std::log(Re) - 1.64, -2.0);
        }
    };

    struct Interp1D
    {
        std::vector<Real64> x_data;
        std::vector<Real64> y_data;
        std::string routineName;
        std::vector<std::pair<Real64, Real64> > table;
        bool extrapolate;

        // constructor
        Interp1D(std::vector<Real64> &x_data, std::vector<Real64> &y_data,
                 std::string &routineName, bool extrapolate = false) {

            this->x_data = x_data;
            this->y_data = y_data;
            this->routineName = routineName;
            this->extrapolate = extrapolate;

            if (this->x_data.size() == this->y_data.size()) {
                for (unsigned i : this->x_data) {
                    table.emplace_back(std::pair<Real64, Real64> {this->x_data[i], this->y_data[i]});
                }
            } else {
                ShowFatalError(routineName + ": Number of X and Y data must be equal.");
            }
            // add option later to ask if the data needs to be sorted
            // std::sort(table.begin(), table.end());
        }

        // default constructor
        Interp1D() = default;

        // destructor
        ~Interp1D() = default;

        // member functions
        Real64 interpolate(Real64 &x);
    };

    static Real64 smoothingFunc(Real64 x, Real64 a, Real64 b)
    {
        //  Sigmoid smoothing function
        //
        //  https://en.wikipedia.org/wiki/Sigmoid_function
        //
        //  @param x: independent variable
        //  @param a: fitting parameter 1
        //  @param b: fitting parameter 2
        //  @return float between 0-1

        return 1 / (1 + std::exp(-(x - a) / b));
    }

    static Real64 linInterp(Real64 x, Real64 x_l, Real64 x_h, Real64 y_l, Real64 y_h)
    {
        //  Simple linear interpolation
        //
        //  @param x: independent input variable
        //  @param x_l: low independent interval bound
        //  @param x_h: high independent interval bound
        //  @param y_l: low dependent interval bound
        //  @param y_h: high dependent interval bound
        //  @return interpolated value

        return (x - x_l) / (x_h - x_l) * (y_h - y_l) + y_l;
    }

    static std::vector<Real64> TDMA(std::vector<Real64> a, std::vector<Real64> b, std::vector<Real64> c, std::vector<Real64> d)
    {
        // Tri-diagonal matrix solver

        // This solver expects the ghost points at a(0) and c(n) to be present

        // a(0) = 0
        // c(n) = 0

        // len(a) = len(b) = len(c) = len(d)

        // Adapted from: https://en.wikibooks.org/wiki/Algorithm_Implementation/Linear_Algebra/Tridiagonal_matrix_algorithm#C++

        // param a: west diagonal vector from coefficient matrix
        // param b: center diagonal vector from coefficient matrix
        // param c: east diagonal vector from coefficient matrix
        // param d: column vector
        // returns solution vector

        u_int n = d.size() - 1;

        c[0] /= b[0];
        d[0] /= b[0];

        for (u_int i = 1; i < n; ++i) {
            c[i] /= b[i] - a[i] * c[i - 1];
            d[i] = (d[i] - a[i] * d[i - 1]) / (b[i] - a[i] * c[i - 1]);
        }

        d[n] = (d[n] - a[n] * d[n - 1]) / (b[n] - a[n] * c[n - 1]);

        for (int i = n; i-- > 0;) {
            d[i] -= c[i] * d[i + 1];
        }

        return d;
    }

    // struct GLHEVertPropsStruct
    //{
    //    // Destructor
    //    ~GLHEVertPropsStruct()
    //    {
    //    }

    //    // Members
    //    std::string name;   // Name
    //    Real64 bhTopDepth;  // Depth of top of borehole {m}
    //    Real64 bhLength;    // Length of borehole from top of borehole {m}
    //    Real64 bhDiameter;  // Diameter of borehole {m}
    //    BaseProps grout;    // Grout properties
    //    Pipe pipe;          // Pipe properties
    //    Real64 bhUTubeDist; // U-tube, shank-to-shank spacking {m}

    //    GLHEVertPropsStruct() : bhTopDepth(0.0), bhLength(0.0), bhDiameter(0.0), bhUTubeDist(0.0)
    //    {
    //    }
    //};

    // struct MyCartesian
    //{
    //    // Destructor
    //    ~MyCartesian()
    //    {
    //    }

    //    Real64 x;
    //    Real64 y;
    //    Real64 z;

    //    MyCartesian() : x(0.0), y(0.0), z(0.0)
    //    {
    //    }
    //};

    // struct GLHEVertSingleStruct
    //{
    //    // Destructor
    //    ~GLHEVertSingleStruct()
    //    {
    //    }

    //    // Members
    //    std::string name;                           // Name
    //    Real64 xLoc;                                // X-direction location {m}
    //    Real64 yLoc;                                // Y-direction location {m}
    //    Real64 dl_i;                                // Discretized bh length between points
    //    Real64 dl_ii;                               // Discretized bh length between points
    //    Real64 dl_j;                                // Discretized bh length between points
    //    std::shared_ptr<GLHEVertPropsStruct> props; // Properties
    //    std::vector<MyCartesian>
    //        pointLocations_i; // Discretized point locations for when computing temperature response of other boreholes on this bh
    //    std::vector<MyCartesian> pointLocations_ii; // Discretized point locations for when computing temperature response of this bh on itself
    //    std::vector<MyCartesian>
    //        pointLocations_j; // Discretized point locations for when other bh are computing the temperature response of this bh on themselves

    //    GLHEVertSingleStruct() : xLoc(0.0), yLoc(0.0), dl_i(0.0), dl_ii(0.0), dl_j(0.0)
    //    {
    //    }
    //};

    // struct GLHEVertArrayStruct
    //{
    //    // Destructor
    //    ~GLHEVertArrayStruct()
    //    {
    //    }

    //    // Members
    //    std::string name;                           // Name
    //    int numBHinXDirection;                      // Number of boreholes in X direction
    //    int numBHinYDirection;                      // Number of boreholes in Y direction
    //    Real64 bhSpacing;                           // Borehole center-to-center spacing {m}
    //    std::shared_ptr<GLHEVertPropsStruct> props; // Properties

    //    GLHEVertArrayStruct() : numBHinXDirection(0), numBHinYDirection(0), bhSpacing(0.0)
    //    {
    //    }
    //};

    // struct GLHEResponseFactorsStruct
    //{
    //    // Destructor
    //    ~GLHEResponseFactorsStruct()
    //    {
    //    }

    //    // Members
    //    std::string name;                                              // Name
    //    int numBoreholes;                                              // Number of boreholes
    //    int numGFuncPairs;                                             // Number of g-function pairs
    //    Real64 gRefRatio;                                              // Reference ratio of g-function set
    //    Real64 maxSimYears;                                            // Maximum length of simulation in years
    //    Array1D<Real64> time;                                          // response time in seconds
    //    Array1D<Real64> LNTTS;                                         // natural log of Non Dimensional Time Ln(t/ts)
    //    Array1D<Real64> GFNC;                                          // G-function ( Non Dimensional temperature response factors)
    //    std::shared_ptr<GLHEVertPropsStruct> props;                    // Properties
    //    std::vector<std::shared_ptr<GLHEVertSingleStruct>> myBorholes; // Boreholes used by this response factors object

    //    GLHEResponseFactorsStruct() : numBoreholes(0), numGFuncPairs(0), gRefRatio(0.0), maxSimYears(0.0)
    //    {
    //    }
    //};

    // struct GLHEBase : PlantComponent
    //{
    //    // Destructor
    //    virtual ~GLHEBase()
    //    {
    //    }

    //    // Members
    //    bool available;   // need an array of logicals--load identifiers of available equipment
    //    bool on;          // simulate the machine at it's operating part load ratio
    //    std::string name; // user identifier
    //    int loopNum;
    //    int loopSideNum;
    //    int branchNum;
    //    int compNum;
    //    int inletNodeNum;  // Node number on the inlet side of the plant
    //    int outletNodeNum; // Node number on the outlet side of the plant
    //    BaseProps soil;
    //    Pipe pipe;
    //    BaseProps grout;
    //    std::shared_ptr<GLHEResponseFactorsStruct> myRespFactors;
    //    Real64 designFlow;            // Design volumetric flow rate			[m3/s]
    //    Real64 designMassFlow;        // Design mass flow rate				[kg/s]
    //    Real64 tempGround;            // The far field temperature of the ground   [degC]
    //    Array1D<Real64> QnMonthlyAgg; // Monthly aggregated normalized heat extraction/rejection rate [W/m]
    //    Array1D<Real64> QnHr;         // Hourly aggregated normalized heat extraction/rejection rate [W/m]
    //    Array1D<Real64> QnSubHr; // Contains the sub-hourly heat extraction/rejection rate normalized by the total active length of bore holes [W/m]
    //    int prevHour;
    //    int AGG;               // Minimum Hourly History required
    //    int SubAGG;            // Minimum sub-hourly History
    //    Array1D_int LastHourN; // Stores the Previous hour's N for past hours until the minimum sub-hourly history
    //    Real64 bhTemp;         // [degC]
    //    Real64 massFlowRate;   // [kg/s]
    //    Real64 outletTemp;     // [degC]
    //    Real64 inletTemp;      // [degC]
    //    Real64 aveFluidTemp;   // [degC]
    //    Real64 QGLHE;          // [W] heat transfer rate
    //    bool myFlag;
    //    bool myEnvrnFlag;
    //    bool gFunctionsExist;
    //    Real64 lastQnSubHr;
    //    Real64 HXResistance;    // The thermal resistance of the GHX, (K per W/m)
    //    Real64 totalTubeLength; // The total length of pipe. NumBoreholes * BoreholeDepth OR Pi * Dcoil * NumCoils
    //    Real64 timeSS;          // Steady state time
    //    Real64 timeSSFactor;    // Steady state time factor for calculation
    //    std::shared_ptr<BaseGroundTempsModel> groundTempModel;

    //    // some statics pulled out into member variables
    //    bool firstTime;
    //    int numErrorCalls;
    //    Real64 ToutNew;
    //    int PrevN;             // The saved value of N at previous time step
    //    bool updateCurSimTime; // Used to reset the CurSimTime to reset after WarmupFlag
    //    bool triggerDesignDayReset;

    //    GLHEBase()
    //        : available(false), on(false), loopNum(0), loopSideNum(0), branchNum(0), compNum(0), inletNodeNum(0), outletNodeNum(0), designFlow(0.0),
    //          designMassFlow(0.0), tempGround(0.0), prevHour(1), AGG(0), SubAGG(0), bhTemp(0.0), massFlowRate(0.0), outletTemp(0.0), inletTemp(0.0),
    //          aveFluidTemp(0.0), QGLHE(0.0), myFlag(true), myEnvrnFlag(true), gFunctionsExist(false), lastQnSubHr(0.0), HXResistance(0.0),
    //          timeSS(0.0), timeSSFactor(0.0), firstTime(true), numErrorCalls(0), ToutNew(19.375), PrevN(1), updateCurSimTime(true),
    //          triggerDesignDayReset(false)
    //    {
    //    }

    //    virtual void calcGFunctions() = 0;

    //    void calcAggregateLoad();

    //    void updateGHX();

    //    void calcGroundHeatExchanger();

    //    inline bool isEven(int const val);

    //    Real64 interpGFunc(Real64);

    //    void makeThisGLHECacheAndCompareWithFileCache();

    //    virtual void makeThisGLHECacheStruct() = 0;

    //    virtual void readCacheFileAndCompareWithThisGLHECache() = 0;

    //    void onInitLoopEquip(const PlantLocation &calledFromLocation) override;

    //    void simulate(const PlantLocation &calledFromLocation, bool const FirstHVACIteration, Real64 &CurLoad, bool const RunFlag) override;

    //    static PlantComponent *factory(int const objectType, std::string objectName);

    //    virtual Real64 getGFunc(Real64) = 0;

    //    virtual void initGLHESimVars() = 0;

    //    virtual Real64 calcHXResistance() = 0;

    //    virtual void getAnnualTimeConstant() = 0;
    //};

    // struct GLHEVert : GLHEBase
    //{
    //    // Destructor
    //    ~GLHEVert()
    //    {
    //    }

    //    // Members
    //    Real64 bhDiameter;  // Diameter of borehole {m}
    //    Real64 bhRadius;    // Radius of borehole {m}
    //    Real64 bhLength;    // Length of borehole {m}
    //    Real64 bhUTubeDist; // Distance between u-tube legs {m}

    //    // Parameters for the multipole method
    //    Real64 theta_1;
    //    Real64 theta_2;
    //    Real64 theta_3;
    //    Real64 sigma;

    //    nlohmann::json myCacheData;

    //    std::vector<Real64> GFNC_shortTimestep;
    //    std::vector<Real64> LNTTS_shortTimestep;

    //    GLHEVert() : bhDiameter(0.0), bhRadius(0.0), bhLength(0.0), bhUTubeDist(0.0), theta_1(0.0), theta_2(0.0), theta_3(0.0), sigma(0.0)
    //    {
    //    }

    //    std::vector<Real64> distances(MyCartesian const &point_i, MyCartesian const &point_j);

    //    Real64 calcResponse(std::vector<Real64> const &dists, Real64 const &currTime);

    //    Real64 integral(MyCartesian const &point_i, std::shared_ptr<GLHEVertSingleStruct> const &bh_j, Real64 const &currTime);

    //    Real64
    //    doubleIntegral(std::shared_ptr<GLHEVertSingleStruct> const &bh_i, std::shared_ptr<GLHEVertSingleStruct> const &bh_j, Real64 const
    //    &currTime);

    //    void calcShortTimestepGFunctions();

    //    void calcLongTimestepGFunctions();

    //    void calcGFunctions();

    //    Real64 calcHXResistance();

    //    void initGLHESimVars();

    //    void getAnnualTimeConstant();

    //    Real64 getGFunc(Real64 const time);

    //    void makeThisGLHECacheStruct();

    //    void readCacheFileAndCompareWithThisGLHECache();

    //    void writeGLHECacheToFile();

    //    Real64 calcBHAverageResistance();

    //    Real64 calcBHTotalInternalResistance();

    //    Real64 calcBHGroutResistance();

    //    Real64 calcPipeConductionResistance();

    //    Real64 calcPipeConvectionResistance();

    //    Real64 frictionFactor(Real64 const reynoldsNum);

    //    Real64 calcPipeResistance();

    //    void combineShortAndLongTimestepGFunctions();
    //};

    // struct GLHESlinky : GLHEBase
    //{

    //    // Destructor
    //    ~GLHESlinky()
    //    {
    //    }

    //    // Members
    //    bool verticalConfig;  // HX Configuration Flag
    //    Real64 coilDiameter;  // Diameter of the slinky coils [m]
    //    Real64 coilPitch;     // Center-to-center slinky coil spacing [m]
    //    Real64 coilDepth;     // Average depth of the coil [m]
    //    Real64 trenchDepth;   // Trench depth from ground surface to trench bottom [m]
    //    Real64 trenchLength;  // Length of single trench [m]
    //    int numTrenches;      // Number of parallel trenches [m]
    //    Real64 trenchSpacing; // Spacing between parallel trenches [m]
    //    int numCoils;         // Number of coils
    //    int monthOfMinSurfTemp;
    //    Real64 maxSimYears;
    //    Real64 minSurfTemp;
    //    Array1D<Real64> X0;
    //    Array1D<Real64> Y0;
    //    Real64 Z0;

    //    GLHESlinky()
    //        : verticalConfig(false), coilDiameter(0.0), coilPitch(0.0), coilDepth(0.0), trenchDepth(0.0), trenchLength(0.0), numTrenches(0),
    //          trenchSpacing(0.0), numCoils(0), monthOfMinSurfTemp(0), maxSimYears(0.0), minSurfTemp(0.0)
    //    {
    //    }

    //    Real64 calcHXResistance();

    //    void calcGFunctions();

    //    void initGLHESimVars();

    //    void getAnnualTimeConstant();

    //    Real64 doubleIntegral(int const m, int const n, int const m1, int const n1, Real64 const t, int const I0, int const J0);

    //    Real64 integral(int const m, int const n, int const m1, int const n1, Real64 const t, Real64 const eta, Real64 const J0);

    //    Real64 distance(int const m, int const n, int const m1, int const n1, Real64 const eta, Real64 const theta);

    //    Real64 distanceToFictRing(int const m, int const n, int const m1, int const n1, Real64 const eta, Real64 const theta);

    //    Real64 distToCenter(int const m, int const n, int const m1, int const n1);

    //    Real64 nearFieldResponseFunction(int const m, int const n, int const m1, int const n1, Real64 const eta, Real64 const theta, Real64 const
    //    t);

    //    Real64 midFieldResponseFunction(int const m, int const n, int const m1, int const n1, Real64 const t);

    //    Real64 getGFunc(Real64 const time);

    //    void makeThisGLHECacheStruct();

    //    void readCacheFileAndCompareWithThisGLHECache();
    //};

    void clear_state();

    // void GetGroundHeatExchangerInput();

    // std::shared_ptr<GLHEResponseFactorsStruct> BuildAndGetResponseFactorObjectFromArray(std::shared_ptr<GLHEVertArrayStruct> const
    // &arrayObjectPtr);

    // std::shared_ptr<GLHEResponseFactorsStruct>
    // BuildAndGetResponseFactorsObjectFromSingleBHs(std::vector<std::shared_ptr<GLHEVertSingleStruct>> const &singleBHsForRFVect);

    // void SetupBHPointsForResponseFactorsObject(std::shared_ptr<GLHEResponseFactorsStruct> &thisRF);

    // std::shared_ptr<GLHEResponseFactorsStruct> GetResponseFactor(std::string const &objectName);

    // std::shared_ptr<GLHEVertSingleStruct> GetSingleBH(std::string const &objectName);

    // std::shared_ptr<GLHEVertPropsStruct> GetVertProps(std::string const &objectName);

    // std::shared_ptr<GLHEVertArrayStruct> GetVertArray(std::string const &objectName);

    // std::vector<Real64> TDMA(std::vector<Real64> a, std::vector<Real64> b, std::vector<Real64> c, std::vector<Real64> d);

    //// Object Data
    // extern std::vector<GLHEVert> verticalGLHE;                                            // Vertical GLHEs
    // extern std::vector<GLHESlinky> slinkyGLHE;                                            // Slinky GLHEs
    // extern std::vector<std::shared_ptr<GLHEVertArrayStruct>> vertArraysVector;            // Vertical Arrays
    // extern std::vector<std::shared_ptr<GLHEVertPropsStruct>> vertPropsVector;             // Vertical Properties
    // extern std::vector<std::shared_ptr<GLHEResponseFactorsStruct>> responseFactorsVector; // Vertical Response Factors
    // extern std::vector<std::shared_ptr<GLHEVertSingleStruct>> singleBoreholesVector;      // Vertical Single Boreholes

} // namespace GroundHeatExchangers

} // namespace EnergyPlus

#endif
