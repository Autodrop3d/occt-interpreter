// Created on: 1993-08-13
// Created by: Bruno DUMORTIER
// Copyright (c) 1993-1999 Matra Datavision
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


#include <Draw.hxx>
#include <Draw_Interpretor.hxx>
#include <Draw_ProgressIndicator.hxx>
#include <gp_Pnt2d.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <Message_PrinterOStream.hxx>
#include <OSD.hxx>
#include <OSD_Environment.hxx>
#include <OSD_File.hxx>
#include <OSD_Process.hxx>
#include <OSD_SharedLibrary.hxx>
#include <OSD_Timer.hxx>
#include <Resource_Manager.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Stream.hxx>
#include <Standard_Version.hxx>
#include <TCollection_AsciiString.hxx>

#include <Standard_WarningDisableFunctionCast.hxx>

extern Standard_Boolean Draw_ParseFailed;


Standard_EXPORT Draw_Interpretor theCommands;
Standard_EXPORT Standard_Boolean ErrorMessages = Standard_True;


static Handle(Draw_ProgressIndicator) global_Progress = NULL;

Standard_EXPORT Standard_Boolean Draw_Interprete(const char* command);


//=======================================================================
//function : GetInterpretor
//purpose  :
//=======================================================================
Draw_Interpretor& Draw::GetInterpretor()
{
  return theCommands;
}

//=======================================================================
//function :
//purpose  : Set/Get Progress Indicator
//=======================================================================
void Draw::SetProgressBar(const Handle(Draw_ProgressIndicator)& theProgress)
{
  global_Progress = theProgress;
}

Handle(Draw_ProgressIndicator) Draw::GetProgressBar()
{
  return global_Progress;
}



// User functions called before and after each command
void (*Draw_BeforeCommand)() = NULL;
void (*Draw_AfterCommand)(Standard_Integer) = NULL;

Standard_Boolean Draw_Interprete(const char* com)
{

  static Standard_Boolean first = Standard_True;



  if (!theCommands.Complete(com))
    return Standard_False;

  // *******************************************************************
  // Command interpreter
  // *******************************************************************

//  Standard_Integer i = 0;
//  Standard_Integer j = 0;




  if (Draw_BeforeCommand) (*Draw_BeforeCommand) ();

  Standard_Integer c;
  
  Standard_CString command;
  c = theCommands.RecordAndEval(command);

  if (Draw_AfterCommand) (*Draw_AfterCommand)(c);

  if (*theCommands.Result())
  {
    if (c > 0 && theCommands.ToColorize())
    {
      Message_PrinterOStream::SetConsoleTextColor (&std::cout, Message_ConsoleColor_Red, true);
    }
    std::cout << theCommands.Result() << std::endl;
    if (c > 0 && theCommands.ToColorize())
    {
      Message_PrinterOStream::SetConsoleTextColor (&std::cout, Message_ConsoleColor_Default, false);
    }
  }

  return Standard_True;
}



//
// for debug call
//



Standard_Integer  Draw_Call (char *c)
{
   Standard_Integer r = theCommands.Eval(c);
   std::cout << theCommands.Result() << std::endl;
   return r;
}


//=======================================================================
//function : ParseOnOff
//purpose  :
//=======================================================================
Standard_Boolean Draw::ParseOnOff (Standard_CString  theArg,
                                   Standard_Boolean& theIsOn)
{
  TCollection_AsciiString aFlag(theArg);
  aFlag.LowerCase();
  if (aFlag == "on"
   || aFlag == "1")
  {
    theIsOn = Standard_True;
    return Standard_True;
  }
  else if (aFlag == "off"
        || aFlag == "0")
  {
    theIsOn = Standard_False;
    return Standard_True;
  }
  return Standard_False;
}

//=======================================================================
//function : ParseOnOffIterator
//purpose  :
//=======================================================================
Standard_Boolean Draw::ParseOnOffIterator (Standard_Integer  theArgsNb,
                                           const char**      theArgVec,
                                           Standard_Integer& theArgIter)
{
  Standard_Boolean isOn = Standard_True;
  if (theArgIter + 1 < theArgsNb
   && Draw::ParseOnOff (theArgVec[theArgIter + 1], isOn))
  {
    ++theArgIter;
  }
  return isOn;
}

//=======================================================================
//function : ParseOnOffNoIterator
//purpose  :
//=======================================================================
Standard_Boolean Draw::ParseOnOffNoIterator (Standard_Integer  theArgsNb,
                                             const char**      theArgVec,
                                             Standard_Integer& theArgIter)
{
  Standard_Boolean toReverse = strncasecmp (theArgVec[theArgIter], "no", 2) == 0
                            || strncasecmp (theArgVec[theArgIter], "-no", 3) == 0;
  Standard_Boolean isOn = Draw::ParseOnOffIterator (theArgsNb, theArgVec, theArgIter);
  return toReverse ? !isOn : isOn;
}
