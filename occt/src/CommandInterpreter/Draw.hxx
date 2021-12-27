// Created on: 1991-04-24
// Created by: Arnaud BOUZY
// Copyright (c) 1991-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#ifndef _Draw_HeaderFile
#define _Draw_HeaderFile

#include <Draw_Interpretor.hxx>
#include <NCollection_Map.hxx>
#include <Standard_Handle.hxx>

class BRepTools_History;
class Draw_ProgressIndicator;

//! MAQUETTE DESSIN MODELISATION
class Draw 
{
public:

  DEFINE_STANDARD_ALLOC


public: //! @name Tcl variables management tools
  
  //! Sets a numeric variable.
  Standard_EXPORT static void Set (const Standard_CString Name, const Standard_Real val);

  //! Returns main DRAW interpretor.
  Standard_EXPORT static Draw_Interpretor& GetInterpretor();


  //! Gets a   numeric  variable. Returns  True   if the
  //! variable exist.
  Standard_EXPORT static Standard_Boolean Get (const Standard_CString Name, Standard_Real& val);
  
  //! Sets a TCL string variable
  Standard_EXPORT static void Set (const Standard_CString Name, const Standard_CString val);

public: //! @name argument parsing tools
  
  //! Converts numeric expression, that can involve DRAW
  //! variables, to real value.
  Standard_EXPORT static Standard_Real Atof (const Standard_CString Name);

  //! Converts the numeric expression, that can involve DRAW variables, to a real value
  //! @param theExpressionString the strings that contains the expression involving DRAW variables to be parsed
  //! @param theParsedRealValue a real value that is a result of parsing
  //! @return true if parsing was successful, or false otherwise
  Standard_EXPORT static bool ParseReal (const Standard_CString theExpressionString, Standard_Real& theParsedRealValue);

  //! Converts numeric expression, that can involve DRAW
  //! variables, to integer value.
  //! Implemented as cast of Atof() to integer.
  Standard_EXPORT static Standard_Integer Atoi (const Standard_CString Name);

  //! Converts the numeric expression, that can involve DRAW variables, to an integer value
  //! @param theExpressionString the strings that contains the expression involving DRAW variables to be parsed
  //! @param theParsedIntegerValue an integer value that is a result of parsing
  //! @return true if parsing was successful, or false otherwise
  Standard_EXPORT static bool ParseInteger (const Standard_CString theExpressionString,
                                            Standard_Integer&      theParsedIntegerValue);


  //! Parses boolean argument. Handles either flag specified by 0|1 or on|off.
  //!
  //! Usage code sample for command argument in form "cmd -usefeature [on|off|1|0]=on":
  //! @code
  //!   for (int anArgIter = 1; anArgIter < theNbArgs; ++anArgIter)
  //!   {
  //!     TCollection_AsciiString aParam (theArgVec[anArgIter]);
  //!     aParam.LowerCase();
  //!     if (aParam == "-usefeature")
  //!     {
  //!       bool toUseFeature = true;
  //!       if (anArgIter + 1 < theNbArgs && Draw::ParseOnOff (theArgVec[anArgIter + 1]))
  //!       {
  //!         ++anArgIter;
  //!       }
  //!       // process feature
  //!     }
  //!   }
  //! @endcode
  //!
  //! @param theArg   [in] argument value
  //! @param theIsOn [out] decoded Boolean flag
  //! @return FALSE on syntax error
  Standard_EXPORT static Standard_Boolean ParseOnOff (Standard_CString  theArg,
                                                      Standard_Boolean& theIsOn);

  //! Parses boolean argument at specified iterator position with optional on/off coming next.
  //!
  //! Usage code sample for command argument in form "cmd -usefeature [on|off|1|0]=on":
  //! @code
  //!   for (int anArgIter = 1; anArgIter < theNbArgs; ++anArgIter)
  //!   {
  //!     if (strcasecmp (theArgVec[anArgIter], "-usefeature") == 0)
  //!     {
  //!       bool toUseFeature = Draw::ParseOnOffIterator (theNbArgs, theArgVec, anArgIter);
  //!       // process feature
  //!     }
  //!   }
  //! @endcode
  //!
  //! @param theArgsNb [in] overall number of arguments
  //! @param theArgVec [in] vector of arguments
  //! @param theArgIter [in] [out] argument position to parse
  //! @return flag value
  Standard_EXPORT static Standard_Boolean ParseOnOffIterator (Standard_Integer  theArgsNb,
                                                              const char**      theArgVec,
                                                              Standard_Integer& theArgIter);

  //! Parses boolean argument at specified iterator position with optional on/off coming next.
  //! Similar to ParseOnOffIterator() but also reverses returned value if argument name starts with "no" prefix.
  //! E.g. if nominal argument is "cmd -usefeature [on|off|1|0]=on", then "-nousefeature" argument will return FALSE.
  //! @param theArgsNb [in] overall number of arguments
  //! @param theArgVec [in] vector of arguments
  //! @param theArgIter [in] [out] argument position to parse
  //! @return flag value
  Standard_EXPORT static Standard_Boolean ParseOnOffNoIterator (Standard_Integer  theArgsNb,
                                                                const char**      theArgVec,
                                                                Standard_Integer& theArgIter);

public:

  //! Returns last graphic selection description.
  Standard_EXPORT static void LastPick (Standard_Integer& view, Standard_Integer& X, Standard_Integer& Y, Standard_Integer& button);
  
    //! sets progress indicator
  Standard_EXPORT static void SetProgressBar (const Handle(Draw_ProgressIndicator)& theProgress);
  
  //! gets progress indicator
  Standard_EXPORT static Handle(Draw_ProgressIndicator) GetProgressBar();

public: //! @name methods loading standard command sets

  //! Defines all Draw commands
  Standard_EXPORT static void Commands (Draw_Interpretor& I);
  
  //! Defines Draw basic commands
  Standard_EXPORT static void BasicCommands (Draw_Interpretor& I);
  
  //! Defines Draw message commands
  Standard_EXPORT static void MessageCommands (Draw_Interpretor& I);

  //! Defines Draw variables handling commands.
  Standard_EXPORT static void VariableCommands (Draw_Interpretor& I);
  
  //! Defines Draw variables handling commands.
  Standard_EXPORT static void GraphicCommands (Draw_Interpretor& I);
  
  //! Defines Loads Draw plugins commands.
  Standard_EXPORT static void PloadCommands (Draw_Interpretor& I);
  
  //! Defines Draw unit commands
  Standard_EXPORT static void UnitCommands (Draw_Interpretor& I);

  static std::map<Standard_CString, Handle(BRepTools_History)> History;

protected:

  static std::map<Standard_CString, Standard_Real> reals;

};

#endif // _Draw_HeaderFile
