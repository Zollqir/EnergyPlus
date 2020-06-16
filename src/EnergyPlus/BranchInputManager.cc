// EnergyPlus, Copyright (c) 1996-2020, The Board of Trustees of the University of Illinois,
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

// C++ Headers
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchInputManager.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace BranchInputManager {

    // Module containing the routines dealing with the BRANCH and CONNECTOR
    // lists input.

    // MODULE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 1999
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To Get the IDD objects "BranchList", "Branch", "ConnectorList",
    // "Connector:Splitter", and "Connector:Mixer".  Also, to supply other modules/routines with
    // information about these objects.

    // Using/Aliasing
    using DataGlobals::DisplayExtraWarnings;
    using namespace DataLoopNode;
    using namespace DataBranchAirLoopPlant;
    using namespace NodeInputManager;
    using namespace BranchNodeConnections;

    // MODULE PARAMETER DEFINITIONS
    const char * cMIXER("Connector:Mixer");
    const char * cSPLITTER("Connector:Splitter");

    void ManageBranchInput(BranchInputManagerData &data)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   Nov 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is called from HVACManager to make sure that branch input is
        // gathered prior to need.

        if (data.GetBranchInputFlag) {
            GetBranchInput(data);
            if (data.GetBranchListInputFlag) {
                data.GetBranchListInputFlag = false;
                GetBranchListInput(data);
            }
            AuditBranches(data, false);
            data.GetBranchInputFlag = false;
        }
    }

    //==================================================================================
    //   Routines that "get" data from internal branch management structure
    //==================================================================================

    void GetBranchList(BranchInputManagerData &data,
                       std::string const &LoopName,       // Name of Loop Branch List is on
                       std::string const &BranchListName, // Branch List Name from Input
                       int &NumBranchNames,               // Number of Branches for this Branch List
                       Array1D_string &BranchNames,       // Names of Branches on this Branch List
                       std::string const &LoopType        // Type of Loop Branch list is on
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   October 1999
        //       MODIFIED       October 2001, Automatic Extensibility
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine "gets" the branch list specified in a Plant or Condenser loop and
        // returns number and names to the outside calling routine.

        // Using/Aliasing
        using General::TrimSigDigits;

        int Found;     // Points to correct Branch List/Branch
        bool ErrFound; // True when error has occurred (cannot find Branch List)

        ErrFound = false;

        if (data.GetBranchListInputFlag) {
            data.GetBranchListInputFlag = false;
            GetBranchListInput(data);
        }

        //  Find this BranchList in the master BranchList Names
        Found = UtilityRoutines::FindItemInList(BranchListName, data.BranchList);
        if (Found == 0) {
            ShowFatalError("GetBranchList: BranchList Name not found=" + BranchListName);
        }

        // Set data
        if (data.BranchList(Found).LoopName.empty()) {
            data.BranchList(Found).LoopName = LoopName;
            data.BranchList(Found).LoopType = LoopType;
        } else if (data.BranchList(Found).LoopName != LoopName) {
            ShowSevereError("GetBranchList: BranchList Loop Name already assigned");
            ShowContinueError("BranchList=" + data.BranchList(Found).Name + ", already assigned to loop=" + data.BranchList(Found).LoopName);
            ShowContinueError("Now requesting assignment to Loop=" + LoopName);
            ErrFound = true;
        }

        // Return data
        NumBranchNames = data.BranchList(Found).NumOfBranchNames;
        if (isize(BranchNames) < NumBranchNames) {
            ShowSevereError("GetBranchList: Branch Names array not big enough to hold Branch Names");
            ShowContinueError("Input BranchListName=" + BranchListName + ", in Loop=" + LoopName);
            ShowContinueError("BranchName Array size=" + TrimSigDigits(size(BranchNames)) + ", but input size=" + TrimSigDigits(NumBranchNames));
            ErrFound = true;
        } else {
            BranchNames = "";
            BranchNames({1, NumBranchNames}) = data.BranchList(Found).BranchNames({1, NumBranchNames});
        }

        if (ErrFound) {
            ShowFatalError("GetBranchList: preceding condition(s) causes program termination.");
        }
    }

    int NumBranchesInBranchList(BranchInputManagerData &data, std::string const &BranchListName)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   July 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function returns the number of branches in a branch list so that the calling
        // routine can allocate arrays before calling GetBranchList.

        // Return value
        int NumBranchesInBranchList;

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int Found;

        if (data.GetBranchListInputFlag) {
            data.GetBranchListInputFlag = false;
            GetBranchListInput(data);
        }

        //  Find this BranchList in the master BranchList Names
        Found = UtilityRoutines::FindItemInList(BranchListName, data.BranchList);
        if (Found == 0) {
            ShowFatalError("NumBranchesInBranchList: BranchList Name not found=" + BranchListName);
        }

        NumBranchesInBranchList = data.BranchList(Found).NumOfBranchNames;

        return NumBranchesInBranchList;
    }

    void GetBranchData(BranchInputManagerData &data,
                       std::string const &LoopName,         // Loop Name of this Branch
                       std::string const &BranchName,       // Requested Branch Name
                       int &PressCurveType,                 // Index of a pressure curve object
                       int &PressCurveIndex,                // Index of a pressure curve object
                       int &NumComps,                       // Number of Components on Branch
                       Array1D_string &CompType,            // Component Type for each item on Branch
                       Array1D_string &CompName,            // Component Name for each item on Branch
                       Array1D_string &CompInletNodeNames,  // Component Inlet Node IDs for each item on Branch
                       Array1D_int &CompInletNodeNums,      // Component Inlet Node Numbers for each item on Branch
                       Array1D_string &CompOutletNodeNames, // Component Outlet Node IDs for each item on Branch
                       Array1D_int &CompOutletNodeNums,     // Component Outlet Node Numbers for each item on Branch
                       bool &ErrorsFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   October 1999
        //       MODIFIED       October 2001, Automatic Extensibility
        //                      September 2012, B. Griffith, removed component control types
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine gets the Branch Data (internal structure) for the requested
        // Branch Name and returns it in "list structure" to the calling routine.

        // Using/Aliasing
        using General::TrimSigDigits;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Count; // Loop Counter
        int MinCompsAllowed;

        // Object Data
        static Array1D<ComponentData> BComponents; // Component data to be returned

        // NumComps now defined on input

        BComponents.allocate(NumComps);

        GetInternalBranchData(data, LoopName, BranchName, PressCurveType, PressCurveIndex, NumComps, BComponents, ErrorsFound);

        MinCompsAllowed = min(
            size(CompType), size(CompName), size(CompInletNodeNames), size(CompInletNodeNums), size(CompOutletNodeNames), size(CompOutletNodeNums));
        if (MinCompsAllowed < NumComps) {
            ShowSevereError("GetBranchData: Component List arrays not big enough to hold Number of Components");
            ShowContinueError("Input BranchName=" + BranchName + ", in Loop=" + LoopName);
            ShowContinueError("Max Component Array size=" + TrimSigDigits(MinCompsAllowed) + ", but input size=" + TrimSigDigits(NumComps));
            ShowFatalError("Program terminates due to preceding conditions.");
        }

        for (Count = 1; Count <= NumComps; ++Count) {
            CompType(Count) = BComponents(Count).CType;
            CompName(Count) = BComponents(Count).Name;
            CompInletNodeNames(Count) = BComponents(Count).InletNodeName;
            CompInletNodeNums(Count) = BComponents(Count).InletNode;
            CompOutletNodeNames(Count) = BComponents(Count).OutletNodeName;
            CompOutletNodeNums(Count) = BComponents(Count).OutletNode;
        }
        BComponents.deallocate();
    }

    int NumCompsInBranch(BranchInputManagerData &data, std::string const &BranchName)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   July 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function returns the number of components in a branch so that the calling
        // routine can allocate arrays before calling GetBranchData.

        // Return value
        int NumCompsInBranch;

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int Found;

        if (data.GetBranchInputFlag) {
            data.GetBranchInputFlag = false;
            GetBranchInput(data);
        }

        Found = UtilityRoutines::FindItemInList(BranchName, data.Branch);
        if (Found == 0) {
            ShowSevereError("NumCompsInBranch:  Branch not found=" + BranchName);
            NumCompsInBranch = 0;
        } else {
            NumCompsInBranch = data.Branch(Found).NumOfComponents;
        }

        return NumCompsInBranch;
    }

    int GetAirBranchIndex(BranchInputManagerData &data, std::string const &CompType, std::string const &CompName)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   April 2013
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function returns the branch index so that the calling
        // routine can search for a fan on this branch or use branch flow for sizing.

        // Return value
        int GetAirBranchIndex(0);

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int BranchNum;
        int CompNum;
        int NumBranches;

        if (data.GetBranchInputFlag) {
            data.GetBranchInputFlag = false;
            GetBranchInput(data);
        }

        NumBranches = size(data.Branch);

        if (NumBranches == 0) {
            ShowSevereError("GetAirBranchIndex:  Branch not found with component = " + CompType + " \"" + CompName + "\"");
        } else {
            for (BranchNum = 1; BranchNum <= NumBranches; ++BranchNum) {
                for (CompNum = 1; CompNum <= data.Branch(BranchNum).NumOfComponents; ++CompNum) {
                    if (UtilityRoutines::SameString(CompType, data.Branch(BranchNum).Component(CompNum).CType) &&
                        UtilityRoutines::SameString(CompName, data.Branch(BranchNum).Component(CompNum).Name)) {
                        GetAirBranchIndex = BranchNum;
                        goto BranchLoop_exit;
                    }
                }
            }
        BranchLoop_exit:;
        }

        return GetAirBranchIndex;
    }

    void GetBranchFanTypeName(BranchInputManagerData &data,
                              int const BranchNum,
                              std::string &FanType,
                              std::string &FanName,
                              bool &ErrFound // Set to true if error found, false otherwise
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   April 2013
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function returns the branch fan flow rate so that the calling
        // routine can either use this flow or use then branch flow for sizing.

        // Using/Aliasing
        using General::TrimSigDigits;

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int CompNum;
        int NumBranches;

        if (data.GetBranchInputFlag) {
            data.GetBranchInputFlag = false;
            GetBranchInput(data);
        }

        ErrFound = false;
        NumBranches = size(data.Branch);

        FanType = std::string();
        FanName = std::string();

        if (NumBranches == 0) {
            ShowSevereError("GetBranchFanTypeName:  Branch index not found = " + TrimSigDigits(BranchNum));
            ErrFound = true;
        } else {
            if (BranchNum > 0 && BranchNum <= NumBranches) {
                for (CompNum = 1; CompNum <= data.Branch(BranchNum).NumOfComponents; ++CompNum) {
                    if (UtilityRoutines::SameString("Fan:OnOff", data.Branch(BranchNum).Component(CompNum).CType) ||
                        UtilityRoutines::SameString("Fan:ConstantVolume", data.Branch(BranchNum).Component(CompNum).CType) ||
                        UtilityRoutines::SameString("Fan:VariableVolume", data.Branch(BranchNum).Component(CompNum).CType) ||
                        UtilityRoutines::SameString("Fan:SystemModel", data.Branch(BranchNum).Component(CompNum).CType)) {
                        FanType = data.Branch(BranchNum).Component(CompNum).CType;
                        FanName = data.Branch(BranchNum).Component(CompNum).Name;
                        break;
                    }
                }
                if (FanType.empty()) ErrFound = true;
            } else {
                ShowSevereError("GetBranchFanTypeName:  Branch index not found = " + TrimSigDigits(BranchNum));
                ErrFound = true;
            }
        }
    }

    void GetInternalBranchData(BranchInputManagerData &data,
                               std::string const &LoopName,         // Loop Name for Branch
                               std::string const &BranchName,       // Requested Branch Name
                               int &PressCurveType,                 // Index of pressure curve object
                               int &PressCurveIndex,                // Index of pressure curve object
                               int &NumComps,                       // Number of Components on Branch
                               Array1D<ComponentData> &BComponents, // Component data returned
                               bool &ErrorsFound                    // True when Loop Name is already assigned and this not same loop
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   October 1999
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine gets the Branch Data (internal structure) for the requested
        // Branch Name and returns it to the calling routine.  This is used internally
        // in the module.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Found; // Pointer to requested Branch Name

        if (data.GetBranchInputFlag) {
            GetBranchInput(data);
            data.GetBranchInputFlag = false;
        }

        Found = UtilityRoutines::FindItemInList(BranchName, data.Branch);
        if (Found == 0) {
            ShowSevereError("GetInternalBranchData:  Branch not found=" + BranchName);
            ErrorsFound = true;
            NumComps = 0;
        } else {
            if (data.Branch(Found).AssignedLoopName.empty()) {
                data.Branch(Found).AssignedLoopName = LoopName;
                PressCurveType = data.Branch(Found).PressureCurveType;
                PressCurveIndex = data.Branch(Found).PressureCurveIndex;
                NumComps = data.Branch(Found).NumOfComponents;
                BComponents({1, NumComps}) = data.Branch(Found).Component({1, NumComps});
            } else if (data.Branch(Found).AssignedLoopName != LoopName) {
                ShowSevereError("Attempt to assign branch to two different loops, Branch=" + BranchName);
                ShowContinueError("Branch already assigned to loop=" + data.Branch(Found).AssignedLoopName);
                ShowContinueError("New attempt to assign to loop=" + LoopName);
                ErrorsFound = true;
                NumComps = 0;
            } else {
                PressCurveType = data.Branch(Found).PressureCurveType;
                PressCurveIndex = data.Branch(Found).PressureCurveIndex;
                NumComps = data.Branch(Found).NumOfComponents;
                BComponents({1, NumComps}) = data.Branch(Found).Component({1, NumComps});
            }
        }
    }

    void GetNumSplitterMixerInConntrList(BranchInputManagerData &data,
                                         std::string const &LoopName,          // Loop Name for this Splitter (used in error message)
                                         std::string const &ConnectorListName, // Requested Connector List Name
                                         int &numSplitters,                    // Number of splitters in the loop
                                         int &numMixers,                       // Number of mixers in the loop
                                         bool &ErrorsFound                     // if no connector list
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Sankaranarayanan K P
        //       DATE WRITTEN   April 2005
        //       MODIFIED       Linda Lawrie - September 2005
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine returns the number of splitter and mixers in a connector list item
        // The data is filled from the idd object 'ConnectorList'

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ConnNum;

        if (data.GetConnectorListInputFlag) {
            GetConnectorListInput(data);
            data.GetConnectorListInputFlag = false;
        }

        numSplitters = 0;
        numMixers = 0;
        ConnNum = UtilityRoutines::FindItemInList(ConnectorListName, data.ConnectorLists);

        if (ConnNum > 0) {
            numSplitters = data.ConnectorLists(ConnNum).NumOfSplitters;
            numMixers = data.ConnectorLists(ConnNum).NumOfMixers;
        } else {
            ShowSevereError("Ref: Loop=" + LoopName + ", Connector List not found=" + ConnectorListName);
            ErrorsFound = true;
        }
    }

    void GetConnectorList(BranchInputManagerData &data,
                          std::string const &ConnectorListName, // Requested Connector List
                          ConnectorData &Connectoid,            // Returned Connector Data
                          Optional_int_const NumInList          // Number of the current connector in the list of connectors
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   October 1999
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains connector data for requested connector list.  Also,
        // this subroutine gets the input for the following IDD structure:
        // ConnectorList,
        //         \memo only two connectors allowed per loop
        //         \memo if two entered, one must be Connector:Splitter and one must be Connector:Mixer
        //     A1, \field Name
        //         \required-field
        //         \reference ConnectorLists
        //     A2, \field Connector 1 Object Type
        //         \required-field
        //         \key Connector:Splitter
        //         \key Connector:Mixer
        //     A3, \field Connector 1 Name
        //         \required-field
        //     A4, \field Connector 2 Object Type
        //         \key Connector:Splitter
        //         \key Connector:Mixer
        //     A5; \field Connector 2 Name

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Count; // Loop Counter

        if (data.GetConnectorListInputFlag) {
            GetConnectorListInput(data);
            data.GetConnectorListInputFlag = false;
        }

        if (not_blank(ConnectorListName)) {
            Count = UtilityRoutines::FindItemInList(ConnectorListName, data.ConnectorLists);
            if (Count == 0) {
                ShowFatalError("GetConnectorList: Connector List not found=" + ConnectorListName);
            }
            Connectoid = data.ConnectorLists(Count);
            if (present(NumInList)) {
                Connectoid.ConnectorType(1) = data.ConnectorLists(Count).ConnectorType(NumInList);
                Connectoid.ConnectorName(1) = data.ConnectorLists(Count).ConnectorName(NumInList);
                Connectoid.ConnectorType(2) = "";
                Connectoid.ConnectorName(2) = "";
            }
        } else {
            Connectoid.Name = "";
            Connectoid.NumOfConnectors = 0;
            Connectoid.ConnectorType(1) = "";
            Connectoid.ConnectorType(2) = "";
            Connectoid.ConnectorName(1) = "";
            Connectoid.ConnectorName(2) = "";
        }
    }

    void GetLoopMixer(BranchInputManagerData &data,
                      std::string const &LoopName,          // Loop Name for Mixer
                      std::string const &ConnectorListName, // Requested Connector List Name
                      std::string &MixerName,               // Name of Mixer
                      bool &IsMixer,                        // True when Mixer is on this connector, false otherwise
                      std::string &OutletNodeName,          // Outlet Node ID
                      int &OutletNodeNum,                   // Outlet Node Number
                      int &NumInletNodes,                   // Number of Inlet Nodes
                      Array1D_string &InletNodeNames,       // Inlet Node IDs
                      Array1D_int &InletNodeNums,           // Inlet Node Numbers
                      bool &ErrorsFound,
                      Optional_int_const ConnectorNumber, // number of the current item in connector list
                      Optional_int MixerNumber            // Mixer number for this specific splitter
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   October 1999
        //       MODIFIED       October 2001, Automatic Extensibility
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine gets the data for the requested Connector List and returns values indicating
        // if this connector list name is a mixer or not.

        // Using/Aliasing
        using General::TrimSigDigits;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Count;    // Loop Counter
        int Loop;     // Loop Counter
        int NumComps; // Number of Components on this Branch
        int PressCurveType;
        int PressCurveIndex;
        bool errFlag; // Error flag from RegisterNodeConnection
        int NumParams;
        int NumAlphas;
        int NumNumbers;

        // Object Data
        ConnectorData Connectoid;           // Connector Data
        Array1D<ComponentData> BComponents; // Branch Component Data

        if (data.GetMixerInputFlag) {
            GetMixerInput(data);
            data.GetMixerInputFlag = false;
        }

        GetConnectorList(data, ConnectorListName, Connectoid, ConnectorNumber);
        if (UtilityRoutines::SameString(Connectoid.ConnectorType(1), cMIXER)) {
            Count = UtilityRoutines::FindItemInList(Connectoid.ConnectorName(1), data.Mixers);
            if (present(MixerNumber)) ++MixerNumber;
            if (Count == 0) {
                ShowFatalError("GetLoopMixer: No Mixer Found=" + Connectoid.ConnectorName(1));
            }
        } else if (UtilityRoutines::SameString(Connectoid.ConnectorType(2), cMIXER)) {
            Count = UtilityRoutines::FindItemInList(Connectoid.ConnectorName(2), data.Mixers);
            if (Count == 0) {
                ShowFatalError("GetLoopMixer: No Mixer Found=" + Connectoid.ConnectorName(2));
            }
        } else {
            Count = 0;
        }

        // Set defaults for later error potential
        IsMixer = false;
        MixerName = std::string();
        OutletNodeName = std::string();
        OutletNodeNum = 0;
        NumInletNodes = 0;
        InletNodeNames = "";
        InletNodeNums = 0;

        if (Count != 0) { // Build up Output list(s). For each component(?)

            // The inlet nodes for the mixer will be the last "outlet" node of
            // each corresponding inlet branch.  The outlet node for the mixer
            // will be the first "inlet" node of the outlet branch since that
            // would be the first node on the branch.
            MixerName = data.Mixers(Count).Name;
            IsMixer = true;
            // The number of "components" on a Mixer is the number of branches.  This is the number of alpha arguments -1.
            inputProcessor->getObjectDefMaxArgs("Branch", NumParams, NumAlphas, NumNumbers);
            BComponents.allocate(NumAlphas - 1);
            errFlag = false;
            GetInternalBranchData(data, LoopName, data.Mixers(Count).OutletBranchName, PressCurveType, PressCurveIndex, NumComps, BComponents, errFlag);
            if (errFlag) {
                ShowContinueError("..occurs for Connector:Mixer Name=" + data.Mixers(Count).Name);
                ErrorsFound = true;
            }
            if (NumComps > 0) {
                OutletNodeName = BComponents(1).InletNodeName;
                OutletNodeNum = BComponents(1).InletNode;
                NumInletNodes = data.Mixers(Count).NumInletBranches;
                // Register this node connection because the mixer gets node information indirectly from the branch
                errFlag = false;
                RegisterNodeConnection(OutletNodeNum,
                                       NodeID(OutletNodeNum),
                                       "Connector:Mixer",
                                       MixerName,
                                       ValidConnectionTypes(NodeConnectionType_Outlet),
                                       1,
                                       ObjectIsNotParent,
                                       errFlag);

                if (NumInletNodes > isize(InletNodeNames) || NumInletNodes > isize(InletNodeNums)) {
                    ShowSevereError("GetLoopMixer: Connector:Mixer=" + MixerName + " contains too many inlets for size of Inlet Array.");
                    ShowContinueError("Max array size=" + TrimSigDigits(size(InletNodeNames)) +
                                      ", Mixer statement inlets=" + TrimSigDigits(NumInletNodes));
                    ShowFatalError("Program terminates due to preceding condition.");
                }
                InletNodeNums = 0;
                InletNodeNames = "";

                for (Loop = 1; Loop <= data.Mixers(Count).NumInletBranches; ++Loop) {
                    GetInternalBranchData(data,
                        LoopName, data.Mixers(Count).InletBranchNames(Loop), PressCurveType, PressCurveIndex, NumComps, BComponents, ErrorsFound);
                    if (NumComps > 0) {
                        InletNodeNames(Loop) = BComponents(NumComps).OutletNodeName;
                        InletNodeNums(Loop) = BComponents(NumComps).OutletNode;
                        // Register this node connection because the mixer gets node information indirectly from the branch
                        errFlag = false;
                        RegisterNodeConnection(InletNodeNums(Loop),
                                               NodeID(InletNodeNums(Loop)),
                                               "Connector:Mixer",
                                               MixerName,
                                               ValidConnectionTypes(NodeConnectionType_Inlet),
                                               1,
                                               ObjectIsNotParent,
                                               errFlag);
                    }
                }
            } else {
                // Set so cascading errors don't happen?
                IsMixer = false;
            }
            BComponents.deallocate();
        }
    }

    void GetLoopSplitter(BranchInputManagerData &data,
                         std::string const &LoopName,          // Loop Name for this Splitter
                         std::string const &ConnectorListName, // Requested Connector List Name
                         std::string &SplitterName,            // Name of Splitter
                         bool &IsSplitter,                     // True if splitter on this connector list, false otherwise
                         std::string &InletNodeName,           // Inlet Node ID
                         int &InletNodeNum,                    // Inlet Node Number
                         int &NumOutletNodes,                  // Number of Outlet Nodes
                         Array1D_string &OutletNodeNames,      // Outlet Node IDs
                         Array1D_int &OutletNodeNums,          // Outlet Node Numbers
                         bool &ErrorsFound,
                         Optional_int_const ConnectorNumber, // number of the current item in connector list
                         Optional_int SplitterNumber         // splitter number for this specific splitter
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   October 1999
        //       MODIFIED       October 2001, Automatic Extensibility
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine gets the data for the requested Connector List and returns values indicating
        // if this connector list name is a splitter or not.

        // Using/Aliasing
        using General::TrimSigDigits;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Count;    // Loop Counter
        int Loop;     // Loop Counter
        int NumComps; // Number of Components on this Branch
        int PressCurveType;
        int PressCurveIndex;
        bool errFlag; // Error flag from RegisterNodeConnection
        int NumParams;
        int NumAlphas;
        int NumNumbers;

        // Object Data
        ConnectorData Connectoid;           // Connector Data
        Array1D<ComponentData> BComponents; // Branch Component Data

        if (data.GetSplitterInputFlag) {
            GetSplitterInput(data);
            data.GetSplitterInputFlag = false;
        }

        if (ConnectorListName.empty()) {
            ShowSevereError("GetLoopSplitter: ConnectorListName is blank.  LoopName=" + LoopName);
            ShowFatalError("Program terminates due to previous condition.");
        }
        GetConnectorList(data, ConnectorListName, Connectoid, ConnectorNumber);
        if (UtilityRoutines::SameString(Connectoid.ConnectorType(1), cSPLITTER)) {
            Count = UtilityRoutines::FindItemInList(Connectoid.ConnectorName(1), data.Splitters);
            if (present(SplitterNumber)) ++SplitterNumber;
            if (Count == 0) {
                ShowFatalError("GetLoopSplitter: No Splitter Found=" + Connectoid.ConnectorName(1));
            }
        } else if (UtilityRoutines::SameString(Connectoid.ConnectorType(2), cSPLITTER)) {
            Count = UtilityRoutines::FindItemInList(Connectoid.ConnectorName(2), data.Splitters);
            if (Count == 0) {
                ShowFatalError("GetLoopSplitter: No Splitter Found=" + Connectoid.ConnectorName(2));
            }
        } else {
            Count = 0;
        }

        // Default for any errors
        SplitterName = std::string();
        IsSplitter = false;
        InletNodeName = std::string();
        InletNodeNum = 0;
        NumOutletNodes = 0;
        OutletNodeNames = "";
        OutletNodeNums = 0;

        if (Count != 0) { // Build up Output list(s). For each component(?)

            // The inlet node for the splitter will be the last "outlet" node of the inlet
            // branch. The outlet nodes for the splitter will be the first "inlet" node of
            // each corresponding outlet branch since that would be the first node on the branch.

            SplitterName = data.Splitters(Count).Name;
            IsSplitter = true;
            // The number of "components" on a Splitter is the number of branches.  This is the number of alpha arguments -1.
            inputProcessor->getObjectDefMaxArgs("Branch", NumParams, NumAlphas, NumNumbers);
            BComponents.allocate(NumAlphas - 1);
            errFlag = false;
            GetInternalBranchData(data, LoopName, data.Splitters(Count).InletBranchName, PressCurveType, PressCurveIndex, NumComps, BComponents, errFlag);
            if (errFlag) {
                ShowContinueError("..occurs for Splitter Name=" + data.Splitters(Count).Name);
                ErrorsFound = true;
            }
            if (NumComps > 0) {
                InletNodeName = BComponents(NumComps).OutletNodeName;
                InletNodeNum = BComponents(NumComps).OutletNode;
                NumOutletNodes = data.Splitters(Count).NumOutletBranches;
                // Register this node connection because the splitter gets node information indirectly from the branch
                errFlag = false;
                RegisterNodeConnection(InletNodeNum,
                                       NodeID(InletNodeNum),
                                       "Connector:Splitter",
                                       SplitterName,
                                       ValidConnectionTypes(NodeConnectionType_Inlet),
                                       1,
                                       ObjectIsNotParent,
                                       errFlag);

                if (NumOutletNodes > isize(OutletNodeNames) || NumOutletNodes > isize(OutletNodeNums)) {
                    ShowSevereError("GetLoopSplitter: Connector:Splitter=" + SplitterName + " contains too many outlets for size of Outlet Array.");
                    ShowContinueError("Max array size=" + TrimSigDigits(size(OutletNodeNames)) +
                                      ", Splitter statement outlets=" + TrimSigDigits(NumOutletNodes));
                    ShowFatalError("Program terminates due to preceding condition.");
                }
                OutletNodeNums = 0;
                OutletNodeNames = "";

                for (Loop = 1; Loop <= data.Splitters(Count).NumOutletBranches; ++Loop) {
                    GetInternalBranchData(data,
                        LoopName, data.Splitters(Count).OutletBranchNames(Loop), PressCurveType, PressCurveIndex, NumComps, BComponents, ErrorsFound);
                    if (NumComps > 0) {
                        OutletNodeNames(Loop) = BComponents(1).InletNodeName;
                        OutletNodeNums(Loop) = BComponents(1).InletNode;
                        // Register this node connection because the splitter gets node information indirectly from the branch
                        errFlag = false;
                        RegisterNodeConnection(OutletNodeNums(Loop),
                                               NodeID(OutletNodeNums(Loop)),
                                               "Connector:Splitter",
                                               SplitterName,
                                               ValidConnectionTypes(NodeConnectionType_Outlet),
                                               1,
                                               ObjectIsNotParent,
                                               errFlag);
                    }
                }
            } else {
                //  Set so cascading errors don't happen
                IsSplitter = false;
            }
            BComponents.deallocate();
        }
    }

    std::string GetFirstBranchInletNodeName(BranchInputManagerData &data, std::string const &BranchListName) // Branch List name to search
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   November 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function uses the branch structure to obtain the inlet node
        // of the first branch from referenced Branch List.

        // Return value
        std::string InletNodeName; // Inlet node name of first branch in branch list

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int Found1; // Pointer to Branch List Name
        int Found2; // Pointer to Branch data

        if (data.GetBranchListInputFlag) {
            data.GetBranchListInputFlag = false;
            GetBranchListInput(data);
        }

        Found1 = UtilityRoutines::FindItemInList(BranchListName, data.BranchList);
        if (Found1 == 0) {
            ShowSevereError("GetFirstBranchInletNodeName: BranchList=\"" + BranchListName + "\", not a valid BranchList Name");
            InletNodeName = "Invalid Node Name";
        } else {
            Found2 = UtilityRoutines::FindItemInList(data.BranchList(Found1).BranchNames(1), data.Branch);
            if (Found2 == 0) {
                ShowSevereError("GetFirstBranchInletNodeName: BranchList=\"" + BranchListName + "\", Branch=\"" + data.BranchList(Found1).BranchNames(1) +
                                "\" not a valid Branch Name");
                InletNodeName = "Invalid Node Name";
            } else {
                InletNodeName = data.Branch(Found2).Component(1).InletNodeName;
            }
        }

        return InletNodeName;
    }

    std::string GetLastBranchOutletNodeName(BranchInputManagerData &data, std::string const &BranchListName) // Branch List name to search
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   August 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function uses the branch structure to obtain the outlet node
        // of the last branch from referenced Branch List.

        // Return value
        std::string OutletNodeName; // Outlet node name of last branch in branch list

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int Found1; // Pointer to Branch List Name
        int Found2; // Pointer to Branch data

        if (data.GetBranchListInputFlag) {
            data.GetBranchListInputFlag = false;
            GetBranchListInput(data);
        }

        Found1 = UtilityRoutines::FindItemInList(BranchListName, data.BranchList);
        if (Found1 == 0) {
            ShowSevereError("GetLastBranchOutletNodeName: BranchList=\"" + BranchListName + "\", not a valid BranchList Name");
            OutletNodeName = "Invalid Node Name";
        } else {
            Found2 = UtilityRoutines::FindItemInList(data.BranchList(Found1).BranchNames(data.BranchList(Found1).NumOfBranchNames), data.Branch);
            if (Found2 == 0) {
                ShowSevereError("GetLastBranchOutletNodeName: BranchList=\"" + BranchListName + "\", Branch=\"" +
                                data.BranchList(Found1).BranchNames(data.BranchList(Found1).NumOfBranchNames) + "\" not a valid Branch Name");
                OutletNodeName = "Invalid Node Name";
            } else {
                OutletNodeName = data.Branch(Found2).Component(data.Branch(Found2).NumOfComponents).OutletNodeName;
            }
        }

        return OutletNodeName;
    }

    //==================================================================================
    //   Routines that get the input for the internal branch management structure
    //==================================================================================

    void GetBranchInput(BranchInputManagerData &data)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   October 1999
        //       MODIFIED       October 2001, Automatic Extensibility
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the input for the following IDD structure:
        // Branch,
        //         \extensible:4 Just duplicate last 4 fields and \ comments (changing numbering, please)
        //         \memo List components on the branch in simulation and connection order
        //         \memo Note: this should NOT include splitters or mixers which define
        //         \memo endpoints of branches
        //    A1,  \field Name
        //         \required-field
        //         \reference Branches
        //    N1, \field Maximum Flow Rate
        //         \default 0
        //         \units m3/s
        //         \minimum 0
        //         \autosizable
        //    A2, \field Pressure Curve Name
        //         \type object-list
        //         \reference AllCurves
        //    A3, \field Component 1 Object Type
        //         \required-field
        //    A4, \field Component 1 Name
        //         \required-field
        //    A5, \field Component 1 Inlet Node Name
        //         \required-field
        //    A6, \field Component 1 Outlet Node Name
        //         \required-field

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const RoutineName("GetBranchInput: ");

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        int BCount;              // Actual Num of Branches
        bool ErrFound;           // Flag for error detection
        int NumAlphas;           // Used to retrieve names from IDF
        Array1D_string Alphas;   // Used to retrieve names from IDF
        Array1D_int NodeNums;    // Possible Array of Node Numbers (only 1 allowed)
        int NumNumbers;          // Used to retrieve numbers from IDF
        Array1D<Real64> Numbers; // Used to retrieve numbers from IDF
        Array1D_string cAlphaFields;
        Array1D_string cNumericFields;
        Array1D_bool lNumericBlanks;
        Array1D_bool lAlphaBlanks;
        int IOStat; // Could be used in the Get Routines, not currently checked
        int NumParams;

        if (data.GetBranchInputOneTimeFlag) {
            std::string CurrentModuleObject = "Branch";
            data.NumOfBranches = inputProcessor->getNumObjectsFound(CurrentModuleObject);
            if (data.NumOfBranches > 0) {
                data.Branch.allocate(data.NumOfBranches);
                for (auto &e : data.Branch)
                    e.AssignedLoopName.clear();
                ErrFound = false;
                inputProcessor->getObjectDefMaxArgs("NodeList", NumParams, NumAlphas, NumNumbers);
                NodeNums.dimension(NumParams, 0);
                inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, NumParams, NumAlphas, NumNumbers);
                Alphas.allocate(NumAlphas);
                Numbers.dimension(NumNumbers, 0.0);
                cAlphaFields.allocate(NumAlphas);
                cNumericFields.allocate(NumNumbers);
                lAlphaBlanks.dimension(NumAlphas, true);
                lNumericBlanks.dimension(NumNumbers, true);
                BCount = 0;
                for (int Count = 1; Count <= data.NumOfBranches; ++Count) {
                    inputProcessor->getObjectItem(CurrentModuleObject,
                                                  Count,
                                                  Alphas,
                                                  NumAlphas,
                                                  Numbers,
                                                  NumNumbers,
                                                  IOStat,
                                                  lNumericBlanks,
                                                  lAlphaBlanks,
                                                  cAlphaFields,
                                                  cNumericFields);
                    if (UtilityRoutines::IsNameEmpty(Alphas(1), CurrentModuleObject, ErrFound)) continue;
                    ++BCount;
                    GetSingleBranchInput(data, RoutineName, BCount, Alphas, cAlphaFields, NumAlphas, NodeNums, lAlphaBlanks);
                }

                data.NumOfBranches = BCount;
                NodeNums.deallocate();
                Alphas.deallocate();
                Numbers.deallocate();
                cAlphaFields.deallocate();
                cNumericFields.deallocate();
                lAlphaBlanks.deallocate();
                lNumericBlanks.deallocate();
                if (ErrFound) {
                    ShowSevereError(RoutineName + " Invalid " + CurrentModuleObject +
                                    " Input, preceding condition(s) will likely cause termination.");
                    data.InvalidBranchDefinitions = true;
                }
                TestInletOutletNodes(ErrFound);
                data.GetBranchInputOneTimeFlag = false;
            }
        }
    }

    void GetSingleBranchInput(BranchInputManagerData &data,
                              std::string const &RoutineName,
                              int const BCount,
                              Array1D_string &Alphas,
                              Array1D_string &cAlphaFields,
                              int const NumAlphas,
                              Array1D_int &NodeNums,
                              Array1D_bool &lAlphaBlanks)
    {
        // Using
        using CurveManager::GetPressureCurveTypeAndIndex;
        using General::RoundSigDigits;

        // Locals
        int PressureCurveType;
        int PressureCurveIndex;
        bool ErrFound;      // Flag for error detection
        int Comp;           // Loop Counter
        bool IsNotOK;       // Flag to verify name
        int NumInComps;     // Number of components actually verified (no SPLITTER or MIXER allowed)
        int ConnectionType; // Used to pass variable node connection type to GetNodeNums
        int NumNodes;       // Number of Nodes from NodeInputManager

        std::string CurrentModuleObject = "Branch";

        data.Branch(BCount).Name = Alphas(1);
        GetPressureCurveTypeAndIndex(Alphas(2), PressureCurveType, PressureCurveIndex);
        if (PressureCurveType == PressureCurve_Error) {
            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + Alphas(1) + "\", invalid data.");
            ShowContinueError("..Invalid " + cAlphaFields(2) + "=\"" + Alphas(2) + "\".");
            ShowContinueError("This curve could not be found in the input deck.  Ensure that this curve has been entered");
            ShowContinueError(" as either a Curve:Functional:PressureDrop or one of Curve:{Linear,Quadratic,Cubic,Exponent}");
            ShowContinueError("This error could be caused by a misspelled curve name");
            ErrFound = true;
        }
        data.Branch(BCount).PressureCurveType = PressureCurveType;
        data.Branch(BCount).PressureCurveIndex = PressureCurveIndex;
        data.Branch(BCount).NumOfComponents = (NumAlphas - 2) / 4;
        if (data.Branch(BCount).NumOfComponents * 4 != (NumAlphas - 2)) ++data.Branch(BCount).NumOfComponents;
        NumInComps = data.Branch(BCount).NumOfComponents;
        data.Branch(BCount).Component.allocate(data.Branch(BCount).NumOfComponents);
        Comp = 1;
        for (int Loop = 3; Loop <= NumAlphas; Loop += 4) {
            if (UtilityRoutines::SameString(Alphas(Loop), cSPLITTER) || UtilityRoutines::SameString(Alphas(Loop), cMIXER)) {
                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + Alphas(1) + "\", invalid data.");
                ShowContinueError("Connector:Splitter/Connector:Mixer not allowed in object " + CurrentModuleObject);
                ErrFound = true;
                continue;
            }
            if (Comp > NumInComps) {
                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + Alphas(1) + "\", invalid data.");
                ShowContinueError("...Number of Arguments indicate [" + RoundSigDigits(NumInComps) + "], but count of fields indicates [" +
                                  RoundSigDigits(Comp) + ']');
                ShowContinueError("...examine " + CurrentModuleObject + " carefully.");
                continue;
            }
            data.Branch(BCount).Component(Comp).CType = Alphas(Loop);
            data.Branch(BCount).Component(Comp).Name = Alphas(Loop + 1);
            ValidateComponent(Alphas(Loop), Alphas(Loop + 1), IsNotOK, CurrentModuleObject);
            if (IsNotOK) {
                ShowContinueError("Occurs on " + CurrentModuleObject + '=' + Alphas(1));
                ErrFound = true;
            }
            data.Branch(BCount).Component(Comp).InletNodeName = Alphas(Loop + 2);
            // If first component on branch, then inlet node is inlet to branch, otherwise node is internal
            if (Loop == 3) {
                ConnectionType = NodeConnectionType_Inlet;
            } else {
                ConnectionType = NodeConnectionType_Internal;
            }
            if (!lAlphaBlanks(Loop + 2)) {
                GetNodeNums(data.Branch(BCount).Component(Comp).InletNodeName,
                            NumNodes,
                            NodeNums,
                            ErrFound,
                            NodeType_Unknown,
                            CurrentModuleObject,
                            data.Branch(BCount).Name,
                            ConnectionType,
                            1,
                            ObjectIsParent,
                            _,
                            cAlphaFields(Loop + 2));
                if (NumNodes > 1) {
                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + Alphas(1) + "\", invalid data.");
                    ShowContinueError("..invalid " + cAlphaFields(Loop + 2) + "=\"" + data.Branch(BCount).Component(Comp).InletNodeName +
                                      "\" must be a single node - appears to be a list.");
                    ShowContinueError("Occurs on " + cAlphaFields(Loop) + "=\"" + Alphas(Loop) + "\", " + cAlphaFields(Loop + 1) + "=\"" +
                                      Alphas(Loop + 1) + "\".");
                    ErrFound = true;
                } else {
                    data.Branch(BCount).Component(Comp).InletNode = NodeNums(1);
                }
            } else {
                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + Alphas(1) + "\", invalid data.");
                ShowContinueError("blank required field: " + cAlphaFields(Loop + 2));
                ShowContinueError("Occurs on " + cAlphaFields(Loop) + "=\"" + Alphas(Loop) + "\", " + cAlphaFields(Loop + 1) + "=\"" +
                                  Alphas(Loop + 1) + "\".");
                ErrFound = true;
            }
            data.Branch(BCount).Component(Comp).OutletNodeName = Alphas(Loop + 3);
            // If last component on branch, then outlet node is outlet from branch, otherwise node is internal
            if (Loop == NumAlphas - 3) {
                ConnectionType = NodeConnectionType_Outlet;
            } else {
                ConnectionType = NodeConnectionType_Internal;
            }
            if (!lAlphaBlanks(Loop + 3)) {
                GetNodeNums(data.Branch(BCount).Component(Comp).OutletNodeName,
                            NumNodes,
                            NodeNums,
                            ErrFound,
                            NodeType_Unknown,
                            CurrentModuleObject,
                            data.Branch(BCount).Name,
                            ConnectionType,
                            1,
                            ObjectIsParent,
                            _,
                            cAlphaFields(Loop + 3));
                if (NumNodes > 1) {
                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + Alphas(1) + "\", invalid data.");
                    ShowContinueError("..invalid " + cAlphaFields(Loop + 2) + "=\"" + data.Branch(BCount).Component(Comp).InletNodeName +
                                      "\" must be a single node - appears to be a list.");
                    ShowContinueError("Occurs on " + cAlphaFields(Loop) + "=\"" + Alphas(Loop) + "\", " + cAlphaFields(Loop + 1) + "=\"" +
                                      Alphas(Loop + 1) + "\".");
                    ErrFound = true;
                } else {
                    data.Branch(BCount).Component(Comp).OutletNode = NodeNums(1);
                }
            } else {
                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + Alphas(1) + "\", invalid data.");
                ShowContinueError("blank required field: " + cAlphaFields(Loop + 3));
                ShowContinueError("Occurs on " + cAlphaFields(Loop) + "=\"" + Alphas(Loop) + "\", " + cAlphaFields(Loop + 1) + "=\"" +
                                  Alphas(Loop + 1) + "\".");
                ErrFound = true;
            }

            if (!lAlphaBlanks(Loop) && !lAlphaBlanks(Loop + 1) && !lAlphaBlanks(Loop + 2) && !lAlphaBlanks(Loop + 3))
                SetUpCompSets(CurrentModuleObject,
                              data.Branch(BCount).Name,
                              Alphas(Loop),
                              Alphas(Loop + 1),
                              Alphas(Loop + 2),
                              Alphas(Loop + 3)); // no blanks in required field set

            ++Comp;
        }
        data.Branch(BCount).NumOfComponents = NumInComps;
    }

    void GetBranchListInput(BranchInputManagerData &data)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   July 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the branch list input and fills up the structures for
        // branch lists.
        // This subroutine gets the input for the following IDD structure:
        // BRANCH LIST,
        //  \extensible:1 Just duplicate last field and \ comments (changing numbering, please)
        //  \memo Branches MUST be listed in flow order: inlet branch, then parallel branches, then outlet branch.
        //  \memo Branches are simulated in the order listed.  Branch names cannot be duplicated within a single branch list.
        //    A1, \field Branch List Name
        //        \required-field
        //        \reference BranchLists
        //    A2, \field Branch Name 1
        //        \required-field
        //        \type object-list
        //        \object-list Branches
        //    A3, \field Branch Name 2
        //        \type object-list
        //        \object-list Branches

        // Using/Aliasing
        using General::TrimSigDigits;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const RoutineName("GetBranchListInput: ");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Count;     // Loop Counter
        int BCount;    // Actual Branch List Count
        int Loop;      // Loop Counter
        int Found;     // Points to correct Branch List/Branch
        bool ErrFound; // True when error has occurred (cannot find Branch List)
        // Following are needed because routine calls GetBranchInput
        // which would overwrite the module Alphas and NumAlphas
        int NumAlphas;         // Used to retrieve Branch list from IDF
        Array1D_string Alphas; // Used to retrieve names from IDF
        int NumNumbers;
        Array1D<Real64> Numbers; // Not used in this object
        Array1D_string cAlphaFields;
        Array1D_string cNumericFields;
        Array1D_bool lNumericBlanks;
        Array1D_bool lAlphaBlanks;
        int IOStat; // Could be used in the Get Routines, not currently checked
        int NumParams;
        std::string TestName;

        ErrFound = false;
        std::string CurrentModuleObject = "BranchList";
        data.NumOfBranchLists = inputProcessor->getNumObjectsFound(CurrentModuleObject);
        data.BranchList.allocate(data.NumOfBranchLists);
        for (auto &e : data.BranchList) {
            e.LoopName.clear();
            e.LoopType.clear();
        }
        inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, NumParams, NumAlphas, NumNumbers);
        Alphas.allocate(NumAlphas);
        Numbers.dimension(NumNumbers, 0.0);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);

        if (NumNumbers > 0) {
            ShowSevereError(RoutineName + CurrentModuleObject +
                            " Object definition contains numbers, cannot be decoded by GetBranchListInput routine.");
            ErrFound = true;
        }
        BCount = 0;
        for (Count = 1; Count <= data.NumOfBranchLists; ++Count) {
            CurrentModuleObject = "BranchList";
            inputProcessor->getObjectItem(CurrentModuleObject,
                                          Count,
                                          Alphas,
                                          NumAlphas,
                                          Numbers,
                                          NumNumbers,
                                          IOStat,
                                          lNumericBlanks,
                                          lAlphaBlanks,
                                          cAlphaFields,
                                          cNumericFields);
            if (UtilityRoutines::IsNameEmpty(Alphas(1), CurrentModuleObject, ErrFound)) continue;

            ++BCount;
            data.BranchList(BCount).Name = Alphas(1);
            data.BranchList(BCount).NumOfBranchNames = NumAlphas - 1;
            data.BranchList(BCount).BranchNames.allocate(NumAlphas - 1);
            if (data.BranchList(BCount).NumOfBranchNames == 0) {
                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + data.BranchList(BCount).Name + "\", No branch names entered.");
                ErrFound = true;
            } else {
                data.BranchList(BCount).BranchNames({1, NumAlphas - 1}) = Alphas({2, NumAlphas});
                for (Loop = 1; Loop <= data.BranchList(BCount).NumOfBranchNames; ++Loop) {
                    // If NumOfBranches = 0 then Branches havent been read yet.
                    if (data.NumOfBranches == 0) {
                        GetBranchInput(data);
                    }
                    if (!data.BranchList(BCount).BranchNames(Loop).empty()) {
                        Found = UtilityRoutines::FindItemInList(data.BranchList(BCount).BranchNames(Loop), data.Branch);
                        if (Found == 0) {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + data.BranchList(BCount).Name + "\", invalid data.");
                            ShowContinueError("..invalid Branch Name not found=\"" + data.BranchList(BCount).BranchNames(Loop) + "\".");
                            ErrFound = true;
                        }
                    }
                }
            }
        }

        // Check for duplicate names specified in Branch Lists
        for (Count = 1; Count <= data.NumOfBranchLists; ++Count) {
            if (data.BranchList(Count).NumOfBranchNames == 0) continue;
            TestName = data.BranchList(Count).BranchNames(1);
            for (Loop = 2; Loop <= data.BranchList(Count).NumOfBranchNames; ++Loop) {
                if (TestName != data.BranchList(Count).BranchNames(Loop)) continue;
                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + data.BranchList(BCount).Name + "\", invalid data.");
                ShowContinueError("..invalid: duplicate branch name specified in the list.");
                ShowContinueError("..Branch Name=" + TestName);
                ShowContinueError("..Branch Name #" + TrimSigDigits(Loop) + " is duplicate.");
                ErrFound = true;
            }
        }

        if (ErrFound) {
            ShowSevereError(RoutineName + " Invalid Input -- preceding condition(s) will likely cause termination.");
        }
        data.NumOfBranchLists = BCount;
        Alphas.deallocate();
        Numbers.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();
    }

    void GetConnectorListInput(BranchInputManagerData &data)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   October 1999
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains connector list input from IDF.
        // ConnectorList,
        //         \memo only two connectors allowed per loop
        //         \memo if two entered, one must be Connector:Splitter and one must be Connector:Mixer
        //     A1, \field Name
        //         \required-field
        //         \reference ConnectorLists
        //     A2, \field Connector 1 Object Type
        //         \required-field
        //         \key Connector:Splitter
        //         \key Connector:Mixer
        //     A3, \field Connector 1 Name
        //         \required-field
        //     A4, \field Connector 2 Object Type
        //         \key Connector:Splitter
        //         \key Connector:Mixer
        //     A5; \field Connector 2 Name
        //  This is in the process of possibly being extended, thus the code herein.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Count;               // Loop Counter
        int NumAlphas;           // Used to retrieve names from IDF
        Array1D_string Alphas;   // Used to retrieve names from IDF
        int NumNumbers;          // Used to retrieve numbers from IDF
        Array1D<Real64> Numbers; // Used to retrieve numbers from IDF
        Array1D_string cAlphaFields;
        Array1D_string cNumericFields;
        Array1D_bool lNumericBlanks;
        Array1D_bool lAlphaBlanks;
        int IOStat; // Could be used in the Get Routines, not currently checked
        int NumParams;
        int NumConnectors;
        int CCount;
        int Arg;
        int SplitNum;
        int MixerNum;
        Array1D_string BranchNames;
        int NumBranchNames;
        bool ErrorsFound;
        int Loop;
        int Loop1;
        int Loop2;
        bool CurMixer;
        bool CurSplitter;
        int TestNum;
        bool MatchFound;

        if (!data.GetConnectorListInputFlag) return;
        ErrorsFound = false;
        std::string CurrentModuleObject = "ConnectorList";
        data.NumOfConnectorLists = inputProcessor->getNumObjectsFound(CurrentModuleObject);
        data.ConnectorLists.allocate(data.NumOfConnectorLists);
        inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, NumParams, NumAlphas, NumNumbers);
        if (NumAlphas != 5 || NumNumbers != 0) {
            ShowWarningError("GetConnectorList: Illegal \"extension\" to " + CurrentModuleObject +
                             " object. Internal code does not support > 2 connectors (Connector:Splitter and Connector:Mixer)");
        }
        Alphas.allocate(NumAlphas);
        Numbers.dimension(NumNumbers, 0.0);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);
        for (Count = 1; Count <= data.NumOfConnectorLists; ++Count) {
            inputProcessor->getObjectItem(CurrentModuleObject,
                                          Count,
                                          Alphas,
                                          NumAlphas,
                                          Numbers,
                                          NumNumbers,
                                          IOStat,
                                          lNumericBlanks,
                                          lAlphaBlanks,
                                          cAlphaFields,
                                          cNumericFields);
            data.ConnectorLists(Count).Name = Alphas(1);
            NumConnectors = (NumAlphas - 1) / 2; // potential problem if puts in type but not name
            if (mod(NumAlphas - 1, 2) != 0) ++NumConnectors;
            data.ConnectorLists(Count).NumOfConnectors = NumConnectors;
            data.ConnectorLists(Count).ConnectorType.allocate(NumConnectors);
            data.ConnectorLists(Count).ConnectorName.allocate(NumConnectors);
            data.ConnectorLists(Count).ConnectorMatchNo.allocate(NumConnectors);
            data.ConnectorLists(Count).ConnectorType = "UNKNOWN";
            data.ConnectorLists(Count).ConnectorName = "UNKNOWN";
            data.ConnectorLists(Count).ConnectorMatchNo = 0;
            data.ConnectorLists(Count).NumOfSplitters = 0;
            data.ConnectorLists(Count).NumOfMixers = 0;

            CCount = 0;
            for (Arg = 2; Arg <= NumAlphas; Arg += 2) {
                ++CCount;
                if (UtilityRoutines::SameString(Alphas(Arg), cSPLITTER)) {
                    data.ConnectorLists(Count).ConnectorType(CCount) = Alphas(Arg).substr(0, 30);
                    ++data.ConnectorLists(Count).NumOfSplitters;
                } else if (UtilityRoutines::SameString(Alphas(Arg), cMIXER)) {
                    data.ConnectorLists(Count).ConnectorType(CCount) = Alphas(Arg).substr(0, 30);
                    ++data.ConnectorLists(Count).NumOfMixers;
                } else {
                    ShowWarningError("GetConnectorListInput: Invalid " + cAlphaFields(Arg) + '=' + Alphas(Arg) + " in " + CurrentModuleObject + '=' +
                                     Alphas(1));
                }
                data.ConnectorLists(Count).ConnectorName(CCount) = Alphas(Arg + 1);
            }
        }
        data.GetConnectorListInputFlag = false;
        Alphas.deallocate();
        Numbers.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        // Validity checks on Connector Lists
        if (data.GetSplitterInputFlag) {
            GetSplitterInput(data);
            data.GetSplitterInputFlag = false;
        }
        if (data.GetMixerInputFlag) {
            GetMixerInput(data);
            data.GetMixerInputFlag = false;
        }

        SplitNum = 0;
        MixerNum = 0;
        for (Count = 1; Count <= data.NumOfConnectorLists; ++Count) {
            if (data.ConnectorLists(Count).NumOfConnectors <= 1) continue; // Air Loop only has one.
            if (data.ConnectorLists(Count).NumOfConnectors > 2) continue;  // Rules not clear for this case
            for (Loop = 1; Loop <= data.ConnectorLists(Count).NumOfConnectors; ++Loop) {
                if (data.ConnectorLists(Count).ConnectorMatchNo(Loop) != 0) continue;
                if (UtilityRoutines::SameString(data.ConnectorLists(Count).ConnectorType(Loop), cSPLITTER)) {
                    CurSplitter = true;
                    CurMixer = false;
                    SplitNum = UtilityRoutines::FindItemInList(data.ConnectorLists(Count).ConnectorName(Loop), data.Splitters);
                    // Following code sets up branch names to be matched from Splitter/Mixer data structure
                    if (SplitNum == 0) {
                        ShowSevereError("Invalid Connector:Splitter(none)=" + data.ConnectorLists(Count).ConnectorName(Loop) + ", referenced by " +
                                        CurrentModuleObject + '=' + data.ConnectorLists(Count).Name);
                        ErrorsFound = true;
                        continue;
                    }
                    NumBranchNames = data.Splitters(SplitNum).NumOutletBranches;
                    BranchNames = data.Splitters(SplitNum).OutletBranchNames;
                } else if (UtilityRoutines::SameString(data.ConnectorLists(Count).ConnectorType(Loop), cMIXER)) {
                    CurSplitter = true;
                    CurMixer = false;
                    MixerNum = UtilityRoutines::FindItemInList(data.ConnectorLists(Count).ConnectorName(Loop), data.Mixers);
                    if (MixerNum == 0) {
                        ShowSevereError("Invalid Connector:Mixer(none)=" + data.ConnectorLists(Count).ConnectorName(Loop) + ", referenced by " +
                                        CurrentModuleObject + '=' + data.ConnectorLists(Count).Name);
                        ErrorsFound = true;
                        continue;
                    }
                    NumBranchNames = data.Mixers(MixerNum).NumInletBranches;
                    BranchNames = data.Mixers(MixerNum).InletBranchNames;
                } else {
                    continue;
                }
                // Try to match mixer to splitter
                for (Loop1 = Loop + 1; Loop1 <= data.ConnectorLists(Count).NumOfConnectors; ++Loop1) {
                    if (CurMixer && !UtilityRoutines::SameString(data.ConnectorLists(Count).ConnectorType(Loop1), cSPLITTER)) continue;
                    if (CurSplitter && !UtilityRoutines::SameString(data.ConnectorLists(Count).ConnectorType(Loop1), cMIXER)) continue;
                    if (data.ConnectorLists(Count).ConnectorMatchNo(Loop1) != 0) continue;
                    {
                        auto const SELECT_CASE_var(CurSplitter);
                        if (SELECT_CASE_var) {
                            // Current "item" is a splitter, candidate is a mixer.
                            MixerNum = UtilityRoutines::FindItemInList(data.ConnectorLists(Count).ConnectorName(Loop1), data.Mixers);
                            if (MixerNum == 0) continue;
                            if (data.Mixers(MixerNum).NumInletBranches != NumBranchNames) continue;
                            MatchFound = true;
                            for (Loop2 = 1; Loop2 <= data.Mixers(MixerNum).NumInletBranches; ++Loop2) {
                                TestNum = UtilityRoutines::FindItemInList(data.Mixers(MixerNum).InletBranchNames(Loop2), BranchNames, NumBranchNames);
                                if (TestNum == 0) {
                                    MatchFound = false;
                                    break;
                                }
                            }
                            if (MatchFound) {
                                data.ConnectorLists(Count).ConnectorMatchNo(Loop1) = MixerNum;
                                data.ConnectorLists(Count).ConnectorMatchNo(Loop) = SplitNum;
                            }
                        } else {
                            // Current "item" is a splitter, candidate is a mixer.
                            SplitNum = UtilityRoutines::FindItemInList(data.ConnectorLists(Count).ConnectorName(Loop1), data.Splitters);
                            if (SplitNum == 0) continue;
                            if (data.Splitters(SplitNum).NumOutletBranches != NumBranchNames) continue;
                            MatchFound = true;
                            for (Loop2 = 1; Loop2 <= data.Splitters(SplitNum).NumOutletBranches; ++Loop2) {
                                TestNum = UtilityRoutines::FindItemInList(data.Splitters(SplitNum).OutletBranchNames(Loop2), BranchNames, NumBranchNames);
                                if (TestNum == 0) {
                                    MatchFound = false;
                                    break;
                                }
                            }
                            if (MatchFound) {
                                data.ConnectorLists(Count).ConnectorMatchNo(Loop1) = SplitNum;
                                data.ConnectorLists(Count).ConnectorMatchNo(Loop) = MixerNum;
                            }
                        }
                    }
                }
                BranchNames.deallocate();
            }
        }

        for (Count = 1; Count <= data.NumOfConnectorLists; ++Count) {
            if (data.ConnectorLists(Count).NumOfConnectors <= 1) continue; // Air Loop only has one.
            if (data.ConnectorLists(Count).NumOfConnectors > 2) continue;  // Rules not clear
            for (Loop = 1; Loop <= data.ConnectorLists(Count).NumOfConnectors; ++Loop) {
                if (data.ConnectorLists(Count).ConnectorMatchNo(Loop) != 0) continue;
                //  = 0, not matched.
                ShowSevereError("For " + CurrentModuleObject + '=' + data.ConnectorLists(Count).Name);
                ShowContinueError("...Item=" + data.ConnectorLists(Count).ConnectorName(Loop) + ", Type=" + data.ConnectorLists(Count).ConnectorType(Loop) +
                                  " was not matched.");
                if (UtilityRoutines::SameString(data.ConnectorLists(Count).ConnectorType(Loop), "Connector:Splitter")) {
                    ShowContinueError(
                        "The BranchList for this Connector:Splitter does not match the BranchList for its corresponding Connector:Mixer.");
                } else {
                    ShowContinueError(
                        "The BranchList for this Connector:Mixer does not match the BranchList for its corresponding Connector:Splitter.");
                }
                ErrorsFound = true;
            }
        }

        if (ErrorsFound) {
            ShowFatalError("GetConnectorListInput: Program terminates for preceding conditions.");
        }
    }

    void GetSplitterInput(BranchInputManagerData &data)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   Sept 2005 (moved from GetLoopSplitter)
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Gets the Splitter data that is used in Loops.
        // IDD structure:
        // Connector:Splitter,
        //   \min-fields 3
        //        \extensible:1 Just duplicate last field and \ comments (changing numbering, please)
        //        \memo Split one air/water stream into N outlet streams.  Branch names cannot be duplicated
        //        \memo within a single Splitter list.
        //    A1, \field Name
        //         \required-field
        //    A2, \field Inlet Branch Name
        //         \required-field
        //         \type object-list
        //         \object-list Branches
        //    A3, \field Outlet Branch 1 Name
        //         \required-field
        //         \type object-list
        //         \object-list Branches
        //    A4, \field Outlet Branch 2 Name
        //         \type object-list
        //         \object-list Branches

        // Using/Aliasing
        using General::TrimSigDigits;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;           // Used to retrieve names from IDF
        Array1D_string Alphas;   // Used to retrieve names from IDF
        int NumNumbers;          // Used to retrieve numbers from IDF
        Array1D<Real64> Numbers; // Used to retrieve numbers from IDF
        Array1D_string cAlphaFields;
        Array1D_string cNumericFields;
        Array1D_bool lNumericBlanks;
        Array1D_bool lAlphaBlanks;
        int IOStat; // Could be used in the Get Routines, not currently checked
        int NumParams;
        int Loop;
        int Loop1;
        int Count;
        int Found;
        static bool ErrorsFound(false);
        std::string TestName;
        std::string BranchListName;
        std::string FoundSupplyDemandAir;
        std::string SaveSupplyDemandAir;
        std::string FoundLoop;
        std::string SaveLoop;
        bool MatchedLoop;

        if (!data.GetSplitterInputFlag) return;
        std::string CurrentModuleObject = cSPLITTER;
        data.NumSplitters = inputProcessor->getNumObjectsFound(CurrentModuleObject);
        data.Splitters.allocate(data.NumSplitters);
        inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, NumParams, NumAlphas, NumNumbers);
        Alphas.allocate(NumAlphas);
        Numbers.dimension(NumNumbers, 0.0);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);
        for (Count = 1; Count <= data.NumSplitters; ++Count) {
            inputProcessor->getObjectItem(CurrentModuleObject,
                                          Count,
                                          Alphas,
                                          NumAlphas,
                                          Numbers,
                                          NumNumbers,
                                          IOStat,
                                          lNumericBlanks,
                                          lAlphaBlanks,
                                          cAlphaFields,
                                          cNumericFields);
            data.Splitters(Count).Name = Alphas(1);
            data.Splitters(Count).InletBranchName = Alphas(2);
            data.Splitters(Count).NumOutletBranches = NumAlphas - 2;
            data.Splitters(Count).OutletBranchNames.allocate(data.Splitters(Count).NumOutletBranches);
            for (Loop = 1; Loop <= data.Splitters(Count).NumOutletBranches; ++Loop) {
                data.Splitters(Count).OutletBranchNames(Loop) = Alphas(2 + Loop);
            }
        }
        data.GetSplitterInputFlag = false;
        Alphas.deallocate();
        Numbers.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        // More validity -- check splitter "names" against branches.
        if (!data.GetBranchInputFlag) {
            GetBranchInput(data);
            data.GetBranchInputFlag = false;
        }
        for (Count = 1; Count <= data.NumSplitters; ++Count) {
            Found = UtilityRoutines::FindItemInList(data.Splitters(Count).InletBranchName, data.Branch);
            if (Found == 0) {
                ShowSevereError("GetSplitterInput: Invalid Branch=" + data.Splitters(Count).InletBranchName + ", referenced as Inlet Branch to " +
                                CurrentModuleObject + '=' + data.Splitters(Count).Name);
                ErrorsFound = true;
            }
            for (Loop = 1; Loop <= data.Splitters(Count).NumOutletBranches; ++Loop) {
                Found = UtilityRoutines::FindItemInList(data.Splitters(Count).OutletBranchNames(Loop), data.Branch);
                if (Found == 0) {
                    ShowSevereError("GetSplitterInput: Invalid Branch=" + data.Splitters(Count).OutletBranchNames(Loop) +
                                    ", referenced as Outlet Branch # " + TrimSigDigits(Loop) + " to " + CurrentModuleObject + '=' +
                                    data.Splitters(Count).Name);
                    ErrorsFound = true;
                }
            }
        }

        // Check for duplicate names specified in Splitters
        for (Count = 1; Count <= data.NumSplitters; ++Count) {
            TestName = data.Splitters(Count).InletBranchName;
            for (Loop = 1; Loop <= data.Splitters(Count).NumOutletBranches; ++Loop) {
                if (TestName != data.Splitters(Count).OutletBranchNames(Loop)) continue;
                ShowSevereError(CurrentModuleObject + '=' + data.Splitters(Count).Name + " specifies an outlet node name the same as the inlet node.");
                ShowContinueError("..Inlet Node=" + TestName);
                ShowContinueError("..Outlet Node #" + TrimSigDigits(Loop) + " is duplicate.");
                ErrorsFound = true;
            }
            for (Loop = 1; Loop <= data.Splitters(Count).NumOutletBranches; ++Loop) {
                for (Loop1 = Loop + 1; Loop1 <= data.Splitters(Count).NumOutletBranches; ++Loop1) {
                    if (data.Splitters(Count).OutletBranchNames(Loop) != data.Splitters(Count).OutletBranchNames(Loop1)) continue;
                    ShowSevereError(CurrentModuleObject + '=' + data.Splitters(Count).Name + " specifies duplicate outlet nodes in its outlet node list.");
                    ShowContinueError("..Outlet Node #" + TrimSigDigits(Loop) + " Name=" + data.Splitters(Count).OutletBranchNames(Loop));
                    ShowContinueError("..Outlet Node #" + TrimSigDigits(Loop) + " is duplicate.");
                    ErrorsFound = true;
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError("GetSplitterInput: Fatal Errors Found in " + CurrentModuleObject + ", program terminates.");
        }

        //  Everything supposed to be good.  Now make sure all branches in Splitter on same side of loop.
        SaveSupplyDemandAir = std::string();
        for (Count = 1; Count <= data.NumSplitters; ++Count) {
            // 2.  Find the branch name in branchlist
            TestName = data.Splitters(Count).InletBranchName;
            BranchListName = std::string();
            for (Loop1 = 1; Loop1 <= data.NumOfBranchLists; ++Loop1) {
                if (any_eq(data.BranchList(Loop1).BranchNames, TestName)) {
                    BranchListName = data.BranchList(Loop1).Name;
                    break;
                }
            }

            if (!BranchListName.empty()) {
                FoundSupplyDemandAir = std::string();
                FoundLoop = std::string();
                MatchedLoop = false;
                // 3.  Find the loop and type
                FindAirPlantCondenserLoopFromBranchList(BranchListName, FoundLoop, FoundSupplyDemandAir, MatchedLoop);
                if (MatchedLoop) {
                    SaveSupplyDemandAir = FoundSupplyDemandAir;
                    SaveLoop = FoundLoop;
                } else {
                    ShowSevereError("GetSplitterInput: Inlet Splitter Branch=\"" + TestName + "\" and BranchList=\"" + BranchListName +
                                    "\" not matched to a Air/Plant/Condenser Loop");
                    ShowContinueError("...and therefore, not a valid Loop Splitter.");
                    ShowContinueError("..." + CurrentModuleObject + '=' + data.Splitters(Count).Name);
                    ErrorsFound = true;
                }
            } else {
                ShowSevereError("GetSplitterInput: Inlet Splitter Branch=\"" + TestName + "\" not on BranchList");
                ShowContinueError("...and therefore, not a valid Loop Splitter.");
                ShowContinueError("..." + CurrentModuleObject + '=' + data.Splitters(Count).Name);
                ErrorsFound = true;
            }
            for (Loop = 1; Loop <= data.Splitters(Count).NumOutletBranches; ++Loop) {
                TestName = data.Splitters(Count).OutletBranchNames(Loop);
                BranchListName = std::string();
                for (Loop1 = 1; Loop1 <= data.NumOfBranchLists; ++Loop1) {
                    if (any_eq(data.BranchList(Loop1).BranchNames, TestName)) {
                        BranchListName = data.BranchList(Loop1).Name;
                        break;
                    }
                }

                if (!BranchListName.empty()) {
                    FoundSupplyDemandAir = std::string();
                    FoundLoop = std::string();
                    MatchedLoop = false;
                    // 3.  Find the loop and type
                    FindAirPlantCondenserLoopFromBranchList(BranchListName, FoundLoop, FoundSupplyDemandAir, MatchedLoop);
                    if (MatchedLoop) {
                        if (SaveSupplyDemandAir != FoundSupplyDemandAir || SaveLoop != FoundLoop) {
                            ShowSevereError("GetSplitterInput: Outlet Splitter Branch=\"" + TestName + "\" does not match types of Inlet Branch.");
                            ShowContinueError("...Inlet Branch is on \"" + SaveLoop + "\" on \"" + SaveSupplyDemandAir + "\" side.");
                            ShowContinueError("...Outlet Branch is on \"" + FoundLoop + "\" on \"" + FoundSupplyDemandAir + "\" side.");
                            ShowContinueError("...All branches in Loop Splitter must be on same kind of loop and supply/demand side.");
                            ShowContinueError("..." + CurrentModuleObject + '=' + data.Splitters(Count).Name);
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError("GetSplitterInput: Outlet Splitter Branch=\"" + TestName + "\" and BranchList=\"" + BranchListName +
                                        "\" not matched to a Air/Plant/Condenser Loop");
                        ShowContinueError("...and therefore, not a valid Loop Splitter.");
                        ShowContinueError("..." + CurrentModuleObject + '=' + data.Splitters(Count).Name);
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError("GetSplitterInput: Outlet Splitter Branch=\"" + TestName + "\" not on BranchList");
                    ShowContinueError("...and therefore, not a valid Loop Splitter");
                    ShowContinueError("..." + CurrentModuleObject + '=' + data.Splitters(Count).Name);
                    ErrorsFound = true;
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError("GetSplitterInput: Fatal Errors Found in " + CurrentModuleObject + ", program terminates.");
        }
    }

    void GetMixerInput(BranchInputManagerData &data)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   Sept 2005 (moved from GetLoopMixer)
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Gets the Mixer data that is used in Loops.
        // IDD Structure:
        // Connector:Mixer,
        //   \min-fields 3
        //        \extensible:1 Just duplicate last field and \ comments (changing numbering, please)
        //        \memo Mix N inlet air/water streams into one.  Branch names cannot be duplicated within
        //        \memo a single mixer list.
        //    A1 , \field Name
        //         \required-field
        //    A2 , \field Outlet Branch Name
        //         \required-field
        //         \type object-list
        //         \object-list Branches
        //    A3 , \field Inlet Branch 1 Name
        //         \required-field
        //         \type object-list
        //         \object-list Branches
        //    A4 , \field Inlet Branch 2 Name
        //         \type object-list
        //         \object-list Branches

        // Using/Aliasing
        using General::TrimSigDigits;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;           // Used to retrieve names from IDF
        Array1D_string Alphas;   // Used to retrieve names from IDF
        int NumNumbers;          // Used to retrieve numbers from IDF
        Array1D<Real64> Numbers; // Used to retrieve numbers from IDF
        Array1D_string cAlphaFields;
        Array1D_string cNumericFields;
        Array1D_bool lNumericBlanks;
        Array1D_bool lAlphaBlanks;
        int IOStat; // Could be used in the Get Routines, not currently checked
        int NumParams;
        int Loop;
        int Loop1;
        int Count;
        int Found;
        static bool ErrorsFound(false);
        std::string TestName;
        std::string BranchListName;
        std::string FoundSupplyDemandAir;
        std::string SaveSupplyDemandAir;
        std::string FoundLoop;
        std::string SaveLoop;
        bool MatchedLoop;

        if (!data.GetMixerInputFlag) return;

        std::string CurrentModuleObject = cMIXER;

        data.NumMixers = inputProcessor->getNumObjectsFound(CurrentModuleObject);
        data.Mixers.allocate(data.NumMixers);
        inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, NumParams, NumAlphas, NumNumbers);
        Alphas.allocate(NumAlphas);
        Numbers.dimension(NumNumbers, 0.0);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);
        for (Count = 1; Count <= data.NumMixers; ++Count) {
            inputProcessor->getObjectItem(CurrentModuleObject,
                                          Count,
                                          Alphas,
                                          NumAlphas,
                                          Numbers,
                                          NumNumbers,
                                          IOStat,
                                          lNumericBlanks,
                                          lAlphaBlanks,
                                          cAlphaFields,
                                          cNumericFields);
            data.Mixers(Count).Name = Alphas(1);
            data.Mixers(Count).OutletBranchName = Alphas(2);
            data.Mixers(Count).NumInletBranches = NumAlphas - 2;
            data.Mixers(Count).InletBranchNames.allocate(data.Mixers(Count).NumInletBranches);
            for (Loop = 1; Loop <= data.Mixers(Count).NumInletBranches; ++Loop) {
                data.Mixers(Count).InletBranchNames(Loop) = Alphas(2 + Loop);
            }
        }
        data.GetMixerInputFlag = false;
        Alphas.deallocate();
        Numbers.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        // More validity -- check mixer "names" against branches.
        if (!data.GetBranchInputFlag) {
            GetBranchInput(data);
            data.GetBranchInputFlag = false;
        }
        for (Count = 1; Count <= data.NumMixers; ++Count) {
            Found = UtilityRoutines::FindItemInList(data.Mixers(Count).OutletBranchName, data.Branch);
            if (Found == 0) {
                ShowSevereError("GetMixerInput: Invalid Branch=" + data.Mixers(Count).OutletBranchName + ", referenced as Outlet Branch in " +
                                CurrentModuleObject + '=' + data.Mixers(Count).Name);
                ErrorsFound = true;
            }
            for (Loop = 1; Loop <= data.Mixers(Count).NumInletBranches; ++Loop) {
                Found = UtilityRoutines::FindItemInList(data.Mixers(Count).InletBranchNames(Loop), data.Branch);
                if (Found == 0) {
                    ShowSevereError("GetMixerInput: Invalid Branch=" + data.Mixers(Count).InletBranchNames(Loop) + ", referenced as Inlet Branch # " +
                                    TrimSigDigits(Loop) + " in " + CurrentModuleObject + '=' + data.Mixers(Count).Name);
                    ErrorsFound = true;
                }
            }
        }

        // Check for duplicate names specified in Mixer
        for (Count = 1; Count <= data.NumMixers; ++Count) {
            TestName = data.Mixers(Count).OutletBranchName;
            for (Loop = 1; Loop <= data.Mixers(Count).NumInletBranches; ++Loop) {
                if (TestName != data.Mixers(Count).InletBranchNames(Loop)) continue;
                ShowSevereError(CurrentModuleObject + '=' + data.Mixers(Count).Name + " specifies an inlet node name the same as the outlet node.");
                ShowContinueError("..Outlet Node=" + TestName);
                ShowContinueError("..Inlet Node #" + TrimSigDigits(Loop) + " is duplicate.");
                ErrorsFound = true;
            }
            for (Loop = 1; Loop <= data.Mixers(Count).NumInletBranches; ++Loop) {
                for (Loop1 = Loop + 1; Loop1 <= data.Mixers(Count).NumInletBranches; ++Loop1) {
                    if (data.Mixers(Count).InletBranchNames(Loop) != data.Mixers(Count).InletBranchNames(Loop1)) continue;
                    ShowSevereError(CurrentModuleObject + '=' + data.Mixers(Count).Name + " specifies duplicate inlet nodes in its inlet node list.");
                    ShowContinueError("..Inlet Node #" + TrimSigDigits(Loop) + " Name=" + data.Mixers(Count).InletBranchNames(Loop));
                    ShowContinueError("..Inlet Node #" + TrimSigDigits(Loop) + " is duplicate.");
                    ErrorsFound = true;
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError("GetMixerInput: Fatal Errors Found in " + CurrentModuleObject + ", program terminates.");
        }

        //  Everything supposed to be good.  Now make sure all branches in Splitter on same side of loop.
        SaveSupplyDemandAir = std::string();
        for (Count = 1; Count <= data.NumMixers; ++Count) {
            // 2.  Find the branch name in branchlist
            TestName = data.Mixers(Count).OutletBranchName;
            BranchListName = std::string();
            for (Loop1 = 1; Loop1 <= data.NumOfBranchLists; ++Loop1) {
                if (any_eq(data.BranchList(Loop1).BranchNames, TestName)) {
                    BranchListName = data.BranchList(Loop1).Name;
                    break;
                }
            }

            if (!BranchListName.empty()) {
                FoundSupplyDemandAir = std::string();
                FoundLoop = std::string();
                MatchedLoop = false;
                // 3.  Find the loop and type
                FindAirPlantCondenserLoopFromBranchList(BranchListName, FoundLoop, FoundSupplyDemandAir, MatchedLoop);
                if (MatchedLoop) {
                    SaveSupplyDemandAir = FoundSupplyDemandAir;
                    SaveLoop = FoundLoop;
                } else {
                    ShowSevereError("GetMixerInput: Outlet Mixer Branch=\"" + TestName + "\" and BranchList=\"" + BranchListName +
                                    "\" not matched to a Air/Plant/Condenser Loop");
                    ShowContinueError("...and therefore, not a valid Loop Mixer.");
                    ShowContinueError("..." + CurrentModuleObject + '=' + data.Mixers(Count).Name);
                    ErrorsFound = true;
                }
            } else {
                ShowSevereError("GetMixerInput: Outlet Mixer Branch=\"" + TestName + "\" not on BranchList");
                ShowContinueError("...and therefore, not a valid Loop Mixer.");
                ShowContinueError("..." + CurrentModuleObject + '=' + data.Mixers(Count).Name);
                ErrorsFound = true;
            }
            for (Loop = 1; Loop <= data.Mixers(Count).NumInletBranches; ++Loop) {
                TestName = data.Mixers(Count).InletBranchNames(Loop);
                BranchListName = std::string();
                for (Loop1 = 1; Loop1 <= data.NumOfBranchLists; ++Loop1) {
                    if (any_eq(data.BranchList(Loop1).BranchNames, TestName)) {
                        BranchListName = data.BranchList(Loop1).Name;
                        break;
                    }
                }

                if (!BranchListName.empty()) {
                    FoundSupplyDemandAir = std::string();
                    FoundLoop = std::string();
                    MatchedLoop = false;
                    // 3.  Find the plant loop and type
                    FindAirPlantCondenserLoopFromBranchList(BranchListName, FoundLoop, FoundSupplyDemandAir, MatchedLoop);
                    if (MatchedLoop) {
                        if (SaveSupplyDemandAir != FoundSupplyDemandAir || SaveLoop != FoundLoop) {
                            ShowSevereError("GetMixerInput: Outlet Mixer Branch=\"" + TestName + "\" does not match types of Inlet Branch.");
                            ShowContinueError("...Outlet Branch is on \"" + SaveLoop + "\" on \"" + SaveSupplyDemandAir + "\" side.");
                            ShowContinueError("...Inlet Branch is on \"" + FoundLoop + "\" on \"" + FoundSupplyDemandAir + "\" side.");
                            ShowContinueError("...All branches in Loop Mixer must be on same kind of loop and supply/demand side.");
                            ShowContinueError("..." + CurrentModuleObject + '=' + data.Mixers(Count).Name);
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError("GetMixerInput: Inlet Mixer Branch=\"" + TestName + "\" and BranchList=\"" + BranchListName +
                                        "\" not matched to a Air/Plant/Condenser Loop");
                        ShowContinueError("...and therefore, not a valid Loop Mixer.");
                        ShowContinueError("..." + CurrentModuleObject + '=' + data.Mixers(Count).Name);
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError("GetMixerInput: Inlet Mixer Branch=\"" + TestName + "\" not on BranchList");
                    ShowContinueError("...and therefore, not a valid Loop Mixer");
                    ShowContinueError("..." + CurrentModuleObject + '=' + data.Mixers(Count).Name);
                    ErrorsFound = true;
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError("GetMixerInput: Fatal Errors Found in " + CurrentModuleObject + ", program terminates.");
        }
    }

    void FindPlantLoopBranchConnection(std::string const &BranchListName,
                                       std::string &FoundPlantLoopName,
                                       int &FoundPlantLoopNum,
                                       std::string &FoundSupplyDemand,
                                       Real64 &FoundVolFlowRate,
                                       bool &MatchedPlantLoop)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   October 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // An auxiliary routine locate a plant loop and type from a BranchListName

        // METHODOLOGY EMPLOYED:
        // Calls GetObject for PLANT LOOP

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Num;
        int NumPlantLoops;
        int NumParams;
        Array1D_string Alphas;
        int NumAlphas;
        Array1D<Real64> Numbers;
        int NumNumbers;
        int IOStat;

        // Get Inputs
        std::string CurrentModuleObject = "PlantLoop";

        NumPlantLoops = inputProcessor->getNumObjectsFound(CurrentModuleObject);
        inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, NumParams, NumAlphas, NumNumbers);
        Alphas.allocate(NumAlphas);
        Numbers.allocate(NumNumbers);

        for (Num = 1; Num <= NumPlantLoops; ++Num) {
            inputProcessor->getObjectItem(CurrentModuleObject, Num, Alphas, NumAlphas, Numbers, NumNumbers, IOStat);
            // Only looking for BranchList here.
            if (Alphas(8) == BranchListName) {
                FoundPlantLoopName = Alphas(1);
                FoundSupplyDemand = "Supply";
                FoundVolFlowRate = Numbers(3);
                FoundPlantLoopNum = Num;
                MatchedPlantLoop = true;
                break;
            } else if (Alphas(12) == BranchListName) {
                FoundPlantLoopName = Alphas(1);
                FoundSupplyDemand = "Demand";
                FoundVolFlowRate = Numbers(3);
                FoundPlantLoopNum = Num;
                MatchedPlantLoop = true;
                break;
            }
        }

        Alphas.deallocate();
        Numbers.deallocate();
    }

    void FindCondenserLoopBranchConnection(std::string const &BranchListName,
                                           std::string &FoundCondLoopName,
                                           int &FoundCondLoopNum,
                                           std::string &FoundSupplyDemand,
                                           Real64 &FoundVolFlowRate,
                                           bool &MatchedCondLoop)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   February 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // An auxiliary routine locate a condenser loop and type from a BranchListName

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Num;
        int NumCondLoops;
        int NumParams;
        Array1D_string Alphas;
        int NumAlphas;
        Array1D<Real64> Numbers;
        int NumNumbers;
        int IOStat;

        // Get Inputs
        std::string CurrentModuleObject = "CondenserLoop";

        NumCondLoops = inputProcessor->getNumObjectsFound(CurrentModuleObject);
        inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, NumParams, NumAlphas, NumNumbers);
        Alphas.allocate(NumAlphas);
        Numbers.allocate(NumNumbers);

        for (Num = 1; Num <= NumCondLoops; ++Num) {
            inputProcessor->getObjectItem(CurrentModuleObject, Num, Alphas, NumAlphas, Numbers, NumNumbers, IOStat);
            // Only looking for BranchList here.
            if (Alphas(8) == BranchListName) {
                FoundCondLoopName = Alphas(1);
                FoundSupplyDemand = "Supply";
                FoundVolFlowRate = Numbers(3);
                FoundCondLoopNum = Num;
                MatchedCondLoop = true;
                break;
            } else if (Alphas(12) == BranchListName) {
                FoundCondLoopName = Alphas(1);
                FoundSupplyDemand = "Demand";
                FoundVolFlowRate = Numbers(3);
                FoundCondLoopNum = Num;
                MatchedCondLoop = true;
                break;
            }
        }

        Alphas.deallocate();
        Numbers.deallocate();
    }

    void FindAirLoopBranchConnection(std::string const &BranchListName,
                                     std::string &FoundAirLoopName,
                                     int &FoundAirLoopNum,
                                     std::string &FoundAir,
                                     Real64 &FoundVolFlowRate,
                                     bool &MatchedAirLoop)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   February 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // An auxiliary routine locate a Airenser loop and type from a BranchListName

        // METHODOLOGY EMPLOYED:
        // calls GetObject for PRIMARY AIR LOOP

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Num;
        int NumAirLoops;
        int NumParams;
        Array1D_string Alphas;
        int NumAlphas;
        Array1D<Real64> Numbers;
        int NumNumbers;
        int IOStat;

        // Get Inputs
        std::string CurrentModuleObject = "AirLoopHVAC";
        NumAirLoops = inputProcessor->getNumObjectsFound(CurrentModuleObject);
        inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, NumParams, NumAlphas, NumNumbers);
        Alphas.allocate(NumAlphas);
        Numbers.allocate(NumNumbers);

        for (Num = 1; Num <= NumAirLoops; ++Num) {
            inputProcessor->getObjectItem(CurrentModuleObject, Num, Alphas, NumAlphas, Numbers, NumNumbers, IOStat);
            // Only looking for BranchList here.
            if (Alphas(4) == BranchListName) {
                FoundAirLoopName = Alphas(1);
                FoundAir = "Air";
                FoundVolFlowRate = Numbers(1);
                FoundAirLoopNum = Num;
                MatchedAirLoop = true;
                break;
            }
        }

        Alphas.deallocate();
        Numbers.deallocate();
    }

    void FindAirPlantCondenserLoopFromBranchList(std::string const &BranchListName, // Branch List Name
                                                 std::string &LoopType,             // LoopType (if found, Plant,Condenser or Air)
                                                 std::string &LoopSupplyDemandAir,  // Supply if "Supply" or Demand if "Demand" or Air if "Air"
                                                 bool &MatchedLoop                  // true if found
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   February 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Assist in validating Loop Splitter/Mixer connections.

        // METHODOLOGY EMPLOYED:
        // Call two previously written subroutines that match a Branch List Name to
        // Plant or Condenser Loop

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string FoundLoopName;
        int FoundLoopNum;
        Real64 FoundLoopVolFlowRate;

        LoopSupplyDemandAir = std::string();
        FoundLoopName = std::string();
        FoundLoopNum = 0;
        FoundLoopVolFlowRate = 0.0;
        MatchedLoop = false;
        LoopType = std::string();

        // Try Plant first
        FindPlantLoopBranchConnection(BranchListName, FoundLoopName, FoundLoopNum, LoopSupplyDemandAir, FoundLoopVolFlowRate, MatchedLoop);

        if (MatchedLoop) LoopType = "Plant";
        if (!MatchedLoop) { // Try Condenser Loop
            LoopSupplyDemandAir = std::string();
            FoundLoopName = std::string();
            FoundLoopNum = 0;
            FoundLoopVolFlowRate = 0.0;
            MatchedLoop = false;

            // Try Condenser
            FindCondenserLoopBranchConnection(BranchListName, FoundLoopName, FoundLoopNum, LoopSupplyDemandAir, FoundLoopVolFlowRate, MatchedLoop);
            if (MatchedLoop) LoopType = "Condenser";
        }

        if (!MatchedLoop) { // Try Air Loop
            LoopSupplyDemandAir = std::string();
            FoundLoopName = std::string();
            FoundLoopNum = 0;
            FoundLoopVolFlowRate = 0.0;
            MatchedLoop = false;

            // Try Air
            FindAirLoopBranchConnection(BranchListName, FoundLoopName, FoundLoopNum, LoopSupplyDemandAir, FoundLoopVolFlowRate, MatchedLoop);
            if (MatchedLoop) LoopType = "Air";
        }
    }

    //==================================================================================
    //   Routines that test branch integrity
    //==================================================================================

    void AuditBranches(BranchInputManagerData &data,
                       bool const mustprint,           // true if the warning should be printed.
                       Optional_string_const CompType, // when mustprint (ScanPlantLoop)  use CompType in error message and scan
                       Optional_string_const CompName  // when mustprint (ScanPlantLoop)  use CompName in error message and scan
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   November 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine will point out any "dangling branches" that are not included on a BranchList.
        // Warnings are produced as the user might clutter up the input file with unused branches.

        // Using/Aliasing
        using DataErrorTracking::TotalSevereErrors;
        using General::RoundSigDigits;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumDanglingCount;        // when mustprint not true, count and report
        int BlNum;                   // Branch List Counter
        int BrN;                     // Branch Counter
        int CpN;                     // Components on Branch
        int Found;                   // non-zero when found
        std::string FoundBranchName; // Branch matching compname/type
        bool NeverFound;

        NumDanglingCount = 0;
        NeverFound = true;
        for (BrN = 1; BrN <= data.NumOfBranches; ++BrN) {
            Found = 0;
            FoundBranchName = "";
            if (present(CompType) && present(CompName)) {
                for (CpN = 1; CpN <= data.Branch(BrN).NumOfComponents; ++CpN) {
                    if (!UtilityRoutines::SameString(CompType, data.Branch(BrN).Component(CpN).CType) ||
                        !UtilityRoutines::SameString(CompName, data.Branch(BrN).Component(CpN).Name))
                        continue;
                    FoundBranchName = data.Branch(BrN).Name;
                    NeverFound = false;
                }
            }
            for (BlNum = 1; BlNum <= data.NumOfBranchLists; ++BlNum) {
                Found = UtilityRoutines::FindItemInList(data.Branch(BrN).Name, data.BranchList(BlNum).BranchNames, data.BranchList(BlNum).NumOfBranchNames);
                if (Found != 0) break;
            }
            if (Found != 0) continue;
            ++NumDanglingCount;
            if (DisplayExtraWarnings || mustprint) {
                if (mustprint) {
                    ShowContinueError("AuditBranches: Branch=\"" + data.Branch(BrN).Name + "\" not found on any BranchLists.");
                    if (!FoundBranchName.empty()) {
                        ShowContinueError("Branch contains component, type=\"" + CompType + "\", name=\"" + CompName + "\"");
                    }
                } else {
                    ShowSevereMessage("AuditBranches: Branch=\"" + data.Branch(BrN).Name + "\" not found on any BranchLists.");
                    ++TotalSevereErrors;
                }
            }
        }
        if (mustprint && NeverFound) { // this may be caught during branch input, not sure
            ShowContinueError("Component, type=\"" + CompType + "\", name=\"" + CompName + "\" was not found on any Branch.");
            ShowContinueError("Look for mistyped branch or component names/types.");
        }
        if (!mustprint && NumDanglingCount > 0) {
            ShowSevereMessage("AuditBranches: There are " + RoundSigDigits(NumDanglingCount) + " branch(es) that do not appear on any BranchList.");
            TotalSevereErrors += NumDanglingCount;
            ShowContinueError("Use Output:Diagnostics,DisplayExtraWarnings; for detail of each branch not on a branch list.");
        }
    }

    void TestBranchIntegrity(BranchInputManagerData &data, OutputFiles &outputFiles, bool &ErrFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   November 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine tests branch integrity and displays the loop for each branch.
        // Also, input and output nodes.

        // Using/Aliasing
        using General::RoundSigDigits;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Loop;
        int Count;
        int MatchNode;                    // Node Number for match
        std::string MatchNodeName;        // Name for error message if not matched
        std::string BranchInletNodeName;  // Branch Inlet Node Name
        std::string BranchOutletNodeName; // Branch Outlet Node Name
        std::string BranchLoopName;       // Loop Name which Branch is part of
        std::string BranchLoopType;       // Loop Type which Branch is part of
        int NumErr;                       // Error Counter
        Array1D_bool BranchReported;
        int BCount;
        int Found;
        //  LOGICAL UniqueNodeError
        int NodeNum;
        int Loop2;
        bool IsAirBranch;
        int BranchFluidType;
        bool MixedFluidTypesOnBranchList;
        int InitialBranchFluidNode;
        Array1D_int BranchFluidNodes;
        Array1D_int FoundBranches;
        Array1D_int BranchPtrs;
        int NumNodesOnBranchList;
        int NumFluidNodes;
        std::string OriginalBranchFluidType;
        std::string cBranchFluidType;
        int Ptr;
        int EndPtr;

        struct BranchUniqueNodes
        {
            int NumNodes{0};
            Array1D_string UniqueNodeNames;
        };

        // Object Data
        Array1D<BranchUniqueNodes> BranchNodes;

        // Formats

        BranchReported.dimension(data.NumOfBranches, false);

        // Do by Branch Lists
        ShowMessage("Testing Individual Branch Integrity");
        ErrFound = false;

        BranchNodes.allocate(data.NumOfBranches);

        print(outputFiles.bnd, "{}\n", "! ===============================================================");
        static constexpr auto Format_700("! <#Branch Lists>,<Number of Branch Lists>");
        print(outputFiles.bnd, "{}\n", Format_700);
        print(outputFiles.bnd, " #Branch Lists,{}\n", data.NumOfBranchLists);
        static constexpr auto Format_702("! <Branch List>,<Branch List Count>,<Branch List Name>,<Loop Name>,<Loop Type>,<Number of Branches>");
        print(outputFiles.bnd, "{}\n", Format_702);
        static constexpr auto Format_704(
            "! <Branch>,<Branch Count>,<Branch Name>,<Loop Name>,<Loop Type>,<Branch Inlet Node Name>,<Branch Outlet Node Name>");
        print(outputFiles.bnd, "{}\n", Format_704);

        for (BCount = 1; BCount <= data.NumOfBranchLists; ++BCount) {
            print(outputFiles.bnd,
                  " Branch List,{},{},{},{},{}\n",
                  BCount,
                  data.BranchList(BCount).Name,
                  data.BranchList(BCount).LoopName,
                  data.BranchList(BCount).LoopType,
                  data.BranchList(BCount).NumOfBranchNames);

            IsAirBranch = false;
            BranchFluidType = NodeType_Unknown;
            MixedFluidTypesOnBranchList = false;
            NumNodesOnBranchList = 0;
            FoundBranches.allocate(data.BranchList(BCount).NumOfBranchNames);
            FoundBranches = 0;
            BranchPtrs.allocate(data.BranchList(BCount).NumOfBranchNames + 2);
            BranchPtrs = 0;
            for (Count = 1; Count <= data.BranchList(BCount).NumOfBranchNames; ++Count) {
                Found = UtilityRoutines::FindItemInList(data.BranchList(BCount).BranchNames(Count), data.Branch);
                if (Found > 0) {
                    NumNodesOnBranchList += data.Branch(Found).NumOfComponents * 2;
                    FoundBranches(Count) = Found;
                    BranchPtrs(Count) = NumNodesOnBranchList;
                } else {
                    ShowSevereError("Branch not found=" + data.BranchList(BCount).BranchNames(Count));
                    ErrFound = true;
                }
            }
            BranchPtrs(data.BranchList(BCount).NumOfBranchNames + 1) = BranchPtrs(data.BranchList(BCount).NumOfBranchNames) + 1;
            BranchFluidNodes.dimension(NumNodesOnBranchList, 0);
            OriginalBranchFluidType = std::string();
            NumFluidNodes = 0;
            for (Count = 1; Count <= data.BranchList(BCount).NumOfBranchNames; ++Count) {
                Found = FoundBranches(Count);
                if (Found == 0) {
                    print(outputFiles.bnd,
                          "   Branch,{},{},(not found),**Unknown**,**Unknown**,**Unknown**,**Unknown**\n",
                          Count,
                          data.BranchList(BCount).BranchNames(Count));
                    continue;
                }
                BranchReported(Found) = true;
                // Check Branch for connections

                MatchNode = 0;
                InitialBranchFluidNode = 0;
                if (data.Branch(Found).NumOfComponents > 0) {
                    MatchNode = data.Branch(Found).Component(1).InletNode;
                    MatchNodeName = data.Branch(Found).Component(1).InletNodeName;
                    BranchInletNodeName = data.Branch(Found).Component(1).InletNodeName;
                } else {
                    ShowWarningError("Branch has no components=" + data.Branch(Found).Name);
                }
                NumErr = 0;
                for (Loop = 1; Loop <= data.Branch(Found).NumOfComponents; ++Loop) {
                    if (Node(data.Branch(Found).Component(Loop).InletNode).FluidType == NodeType_Air) IsAirBranch = true;
                    if (BranchFluidType == NodeType_Unknown) {
                        ++NumFluidNodes;
                        BranchFluidNodes(NumFluidNodes) = data.Branch(Found).Component(Loop).InletNode;
                        BranchFluidType = Node(data.Branch(Found).Component(Loop).InletNode).FluidType;
                        InitialBranchFluidNode = data.Branch(Found).Component(Loop).InletNode;
                        OriginalBranchFluidType = ValidNodeFluidTypes(BranchFluidType);
                    } else if (BranchFluidType != Node(data.Branch(Found).Component(Loop).InletNode).FluidType &&
                               Node(data.Branch(Found).Component(Loop).InletNode).FluidType != NodeType_Unknown) {
                        ++NumFluidNodes;
                        BranchFluidNodes(NumFluidNodes) = data.Branch(Found).Component(Loop).InletNode;
                        MixedFluidTypesOnBranchList = true;
                    } else {
                        ++NumFluidNodes;
                        BranchFluidNodes(NumFluidNodes) = data.Branch(Found).Component(Loop).InletNode;
                    }
                    if (Node(data.Branch(Found).Component(Loop).OutletNode).FluidType == NodeType_Air) IsAirBranch = true;
                    if (BranchFluidType == NodeType_Unknown) {
                        ++NumFluidNodes;
                        BranchFluidNodes(NumFluidNodes) = data.Branch(Found).Component(Loop).InletNode;
                        BranchFluidType = Node(data.Branch(Found).Component(Loop).OutletNode).FluidType;
                        InitialBranchFluidNode = data.Branch(Found).Component(Loop).OutletNode;
                        OriginalBranchFluidType = ValidNodeFluidTypes(BranchFluidType);
                    } else if (BranchFluidType != Node(data.Branch(Found).Component(Loop).OutletNode).FluidType &&
                               Node(data.Branch(Found).Component(Loop).OutletNode).FluidType != NodeType_Unknown) {
                        ++NumFluidNodes;
                        BranchFluidNodes(NumFluidNodes) = data.Branch(Found).Component(Loop).OutletNode;
                        MixedFluidTypesOnBranchList = true;
                    } else {
                        ++NumFluidNodes;
                        BranchFluidNodes(NumFluidNodes) = data.Branch(Found).Component(Loop).OutletNode;
                    }
                    if (data.Branch(Found).Component(Loop).InletNode != MatchNode) {
                        ShowSevereError("Error Detected in BranchList=" + data.BranchList(BCount).Name);
                        ShowContinueError("Actual Error occurs in Branch=" + data.Branch(Found).Name);
                        ShowContinueError("Branch Outlet does not match Inlet, Outlet=" + MatchNodeName);
                        ShowContinueError("Inlet Name=" + data.Branch(Found).Component(Loop).InletNodeName);
                        ErrFound = true;
                        ++NumErr;
                    } else {
                        MatchNode = data.Branch(Found).Component(Loop).OutletNode;
                        MatchNodeName = data.Branch(Found).Component(Loop).OutletNodeName;
                    }
                }
                data.Branch(Found).FluidType = BranchFluidType;
                BranchOutletNodeName = MatchNodeName;
                if (data.Branch(Found).AssignedLoopName.empty()) {
                    BranchLoopName = "**Unknown**";
                    BranchLoopType = "**Unknown**";
                } else if (data.Branch(Found).AssignedLoopName == data.BranchList(BCount).LoopName) {
                    BranchLoopName = data.BranchList(BCount).LoopName;
                    BranchLoopType = data.BranchList(BCount).LoopType;
                } else {
                    BranchLoopName = data.Branch(Found).AssignedLoopName;
                    BranchLoopType = "**Unknown**";
                }
                print(outputFiles.bnd,
                      "   Branch,{},{},{},{},{},{}\n",
                      Count,
                      data.Branch(Found).Name,
                      BranchLoopName,
                      BranchLoopType,
                      BranchInletNodeName,
                      BranchOutletNodeName);
            }
            if (MixedFluidTypesOnBranchList) {
                ShowSevereError("BranchList=" + data.BranchList(BCount).Name + " has mixed fluid types in its nodes.");
                ErrFound = true;
                if (OriginalBranchFluidType.empty()) OriginalBranchFluidType = "**Unknown**";
                ShowContinueError("Initial Node=" + NodeID(InitialBranchFluidNode) + ", Fluid Type=" + OriginalBranchFluidType);
                ShowContinueError("BranchList Topology - Note nodes which do not match that fluid type:");
                Ptr = 1;
                EndPtr = BranchPtrs(1);
                for (Loop = 1; Loop <= data.BranchList(BCount).NumOfBranchNames; ++Loop) {
                    if (FoundBranches(Loop) != 0) {
                        ShowContinueError("..Branch=" + data.Branch(FoundBranches(Loop)).Name);
                    } else {
                        ShowContinueError("..Illegal Branch=" + data.BranchList(BCount).BranchNames(Loop));
                        continue;
                    }
                    for (Loop2 = Ptr; Loop2 <= EndPtr; ++Loop2) {
                        cBranchFluidType = ValidNodeFluidTypes(Node(BranchFluidNodes(Loop2)).FluidType);
                        if (cBranchFluidType.empty()) cBranchFluidType = "**Unknown**";
                        ShowContinueError("....Node=" + NodeID(BranchFluidNodes(Loop2)) + ", Fluid Type=" + cBranchFluidType);
                    }
                    Ptr = EndPtr + 1;
                    EndPtr = BranchPtrs(Loop + 1);
                }
            }
            BranchFluidNodes.deallocate();
            BranchPtrs.deallocate();
            FoundBranches.deallocate();
        }

        // Build node names in branches
        for (Count = 1; Count <= data.NumOfBranches; ++Count) {
            BranchNodes(Count).UniqueNodeNames.allocate(data.Branch(Count).NumOfComponents * 2);
            BranchNodes(Count).UniqueNodeNames = std::string();
            NodeNum = 0;
            for (Loop = 1; Loop <= data.Branch(Count).NumOfComponents; ++Loop) {
                Found = UtilityRoutines::FindItemInList(data.Branch(Count).Component(Loop).InletNodeName, BranchNodes(Count).UniqueNodeNames, NodeNum);
                if (Found == 0) {
                    ++NodeNum;
                    BranchNodes(Count).UniqueNodeNames(NodeNum) = data.Branch(Count).Component(Loop).InletNodeName;
                }
                Found = UtilityRoutines::FindItemInList(data.Branch(Count).Component(Loop).OutletNodeName, BranchNodes(Count).UniqueNodeNames, NodeNum);
                if (Found == 0) {
                    ++NodeNum;
                    BranchNodes(Count).UniqueNodeNames(NodeNum) = data.Branch(Count).Component(Loop).OutletNodeName;
                }
            }
            BranchNodes(Count).NumNodes = NodeNum;
        }
        // Check Uniqueness branch to branch
        for (Count = 1; Count <= data.NumOfBranches; ++Count) {
            for (Loop = Count + 1; Loop <= data.NumOfBranches; ++Loop) {
                for (Loop2 = 1; Loop2 <= BranchNodes(Count).NumNodes; ++Loop2) {
                    Found = UtilityRoutines::FindItemInList(
                        BranchNodes(Count).UniqueNodeNames(Loop2), BranchNodes(Loop).UniqueNodeNames, BranchNodes(Loop).NumNodes);
                    if (Found != 0) {
                        ShowSevereError("Non-unique node name found, name=" + BranchNodes(Count).UniqueNodeNames(Loop2));
                        ShowContinueError("..1st occurrence in Branch=" + data.Branch(Count).Name);
                        ShowContinueError("..duplicate occurrence in Branch=" + data.Branch(Loop).Name);
                        ErrFound = true;
                    }
                }
            }
        }
        for (Count = 1; Count <= data.NumOfBranches; ++Count) {
            BranchNodes(Count).UniqueNodeNames.deallocate();
        }
        BranchNodes.deallocate();

        BCount = 0;
        for (Count = 1; Count <= data.NumOfBranches; ++Count) {
            if (BranchReported(Count)) continue;
            ++BCount;
        }
        if (BCount > 0) {
            static constexpr auto Format_706("! <# Orphaned Branches>,<Number of Branches not on Branch Lists>");
            print(outputFiles.bnd, "{}\n", Format_706);
            print(outputFiles.bnd, " #Orphaned Branches,{}\n", BCount);
            ShowWarningError("There are orphaned Branches in input. See .bnd file for details.");

            BCount = 0;

            for (Count = 1; Count <= data.NumOfBranches; ++Count) {
                if (BranchReported(Count)) continue;
                ++BCount;
                ShowWarningError("Orphan Branch=\"" + data.Branch(Count).Name + "\".");

                if (data.Branch(Count).NumOfComponents > 0) {
                    MatchNode = data.Branch(Count).Component(1).InletNode;
                    MatchNodeName = data.Branch(Count).Component(1).InletNodeName;
                    BranchInletNodeName = data.Branch(Count).Component(1).InletNodeName;
                } else {
                    ShowWarningError("Branch has no components=" + data.Branch(Count).Name);
                }
                NumErr = 0;
                for (Loop = 1; Loop <= data.Branch(Count).NumOfComponents; ++Loop) {
                    if (data.Branch(Count).Component(Loop).InletNode != MatchNode) {
                        ShowSevereError("Error Detected in Branch=" + data.Branch(Count).Name);
                        ShowContinueError("Branch Outlet does not match Inlet, Outlet=" + MatchNodeName);
                        ShowContinueError("Inlet Name=" + data.Branch(Count).Component(Loop).InletNodeName);
                        ErrFound = true;
                        ++NumErr;
                    } else {
                        MatchNode = data.Branch(Count).Component(Loop).OutletNode;
                        MatchNodeName = data.Branch(Count).Component(Loop).OutletNodeName;
                    }
                }
                BranchOutletNodeName = MatchNodeName;
                if (data.Branch(Count).AssignedLoopName.empty()) {
                    BranchLoopName = "**Unknown**";
                    BranchLoopType = "**Unknown**";
                } else {
                    BranchLoopName = data.Branch(Count).AssignedLoopName;
                    BranchLoopType = "**Unknown**";
                }
                print(outputFiles.bnd,
                      " Branch,{},{},{},{},{},{}\n",
                      BCount,
                      data.Branch(Count).Name,
                      BranchLoopName,
                      BranchLoopType,
                      BranchInletNodeName,
                      BranchOutletNodeName);
            }
        }

        if (ErrFound) {
            ShowSevereError("Branch(es) did not pass integrity testing");
        } else {
            ShowMessage("All Branches passed integrity testing");
        }
    }

} // namespace BranchInputManager

} // namespace EnergyPlus
