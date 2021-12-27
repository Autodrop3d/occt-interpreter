// Created on: 1995-02-23
// Created by: Remi LEQUETTE
// Copyright (c) 1995-1999 Matra Datavision
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

#include <Draw_Interpretor.hxx>

#include <Draw_Appli.hxx>
#include <Message.hxx>
#include <Message_PrinterOStream.hxx>
#include <OSD.hxx>
#include <OSD_File.hxx>
#include <OSD_Path.hxx>
#include <OSD_Process.hxx>
#include <Standard_SStream.hxx>
#include <Standard_RangeError.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Macro.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_ExtendedString.hxx>

#include <string.h>
#include "./Draw.cxx"

// logging helpers
namespace {
  void dumpArgs (Standard_OStream& os, int argc, const char *argv[])
  {
    for (int i=0; i < argc; i++)
      os << argv[i] << " ";
    os << std::endl;
  }

  void flush_standard_streams ()
  {
    std::cerr << std::flush;
    std::cout << std::flush;
  }

} // anonymous namespace

static Standard_Integer CommandCmd (ClientData theClientData, Tcl_Interp* interp, Standard_Integer argc, const char* argv[])
{

  return 0;
}

static void CommandDelete (ClientData theClientData)
{
  Draw_Interpretor::CallBackData* aCallback = (Draw_Interpretor::CallBackData* )theClientData;
  delete aCallback;
}

//=======================================================================
//function : Draw_Interpretor
//purpose  :
//=======================================================================
Draw_Interpretor::Draw_Interpretor()
: // the tcl interpreter is not created immediately as it is kept
  // by a global variable and created and deleted before the main()
  myInterp (NULL),
  isAllocated (Standard_False),
  myDoLog (Standard_False),
  myDoEcho (Standard_False),
  myToColorize (Standard_True),
  myFDLog (-1)
{
  //
}

//=======================================================================
//function : Draw_Interpretor
//purpose  :
//=======================================================================
Draw_Interpretor::Draw_Interpretor (const Draw_PInterp& theInterp)
: myInterp (theInterp),
  isAllocated (Standard_False),
  myDoLog (Standard_False),
  myDoEcho (Standard_False),
  myToColorize (Standard_True),
  myFDLog (-1)
{
  //
}

//=======================================================================
//function : Init
//purpose  : It is necessary to call this function
//=======================================================================

void Draw_Interpretor::Init()
{
  isAllocated=Standard_True;
}

//=======================================================================
//function : SetToColorize
//purpose  :
//=======================================================================
void Draw_Interpretor::SetToColorize (Standard_Boolean theToColorize)
{
  myToColorize = theToColorize;
  for (Message_SequenceOfPrinters::Iterator aPrinterIter (Message::DefaultMessenger()->Printers());
       aPrinterIter.More(); aPrinterIter.Next())
  {
    if (Handle(Message_PrinterOStream) aPrinter = Handle(Message_PrinterOStream)::DownCast (aPrinterIter.Value()))
    {
      aPrinter->SetToColorize (theToColorize);
    }
  }
}

//=======================================================================
//function : add
//purpose  :
//=======================================================================
void Draw_Interpretor::add (const Standard_CString          theCommandName,
                            const Standard_CString          theHelp,
                            const Standard_CString          theFileName,
                            Draw_Interpretor::CallBackData* theCallback,
                            const Standard_CString          theGroup)
{
  Standard_ASSERT_RAISE (myInterp != NULL, "Attempt to add command to Null interpretor");

  Standard_PCharacter aName  = (Standard_PCharacter )theCommandName;
  Standard_PCharacter aHelp  = (Standard_PCharacter )theHelp;
  Standard_PCharacter aGroup = (Standard_PCharacter )theGroup;
  this->callbacks[theCommandName] = theCallback;
  this->help[theCommandName] = theHelp;
}

//=======================================================================
//function : Remove
//purpose  : 
//=======================================================================

Standard_Boolean Draw_Interpretor::Remove(Standard_CString const n)
{
  return true;
}

//=======================================================================
//function : Result
//purpose  : 
//=======================================================================

Standard_CString Draw_Interpretor::Result() const
{
  return "cool"
}

//=======================================================================
//function : Reset
//purpose  : 
//=======================================================================

void Draw_Interpretor::Reset()
{
}

//=======================================================================
//function : Append
//purpose  : 
//=======================================================================

Draw_Interpretor& Draw_Interpretor::Append(const Standard_CString s)
{
  std::cout << s;
  return *this;
}

//=======================================================================
//function : Append
//purpose  : 
//=======================================================================

Draw_Interpretor& Draw_Interpretor::Append(const TCollection_AsciiString& s)
{
  return Append (s.ToCString());
}

//=======================================================================
//function : Append
//purpose  : 
//=======================================================================

Draw_Interpretor& Draw_Interpretor::Append(const TCollection_ExtendedString& theString)
{
  std::cout << s;
  return *this;
}

//=======================================================================
//function : Append
//purpose  : 
//=======================================================================

Draw_Interpretor& Draw_Interpretor::Append(const Standard_Integer i)
{
  std::cout << i;
  return *this;
}

//=======================================================================
//function : Append
//purpose  : 
//=======================================================================

Draw_Interpretor& Draw_Interpretor::Append(const Standard_Real r)
{
  std::cout << r;
  return *this;
}

//=======================================================================
//function : Append
//purpose  : 
//=======================================================================

Draw_Interpretor& Draw_Interpretor::Append(const Standard_SStream& s)
{
  return Append (s.str().c_str());
}

//=======================================================================
//function : AppendElement
//purpose  : 
//=======================================================================

void Draw_Interpretor::AppendElement(const Standard_CString s)
{

}

//=======================================================================
//function : Eval
//purpose  : 
//=======================================================================

Standard_Integer Draw_Interpretor::Eval(const Standard_CString line)
{
}


//=======================================================================
//function : Eval
//purpose  : 
//=======================================================================

Standard_Integer Draw_Interpretor::RecordAndEval(const Standard_CString line,
						 const Standard_Integer flags)
{
}

//=======================================================================
//function : EvalFile
//purpose  : 
//=======================================================================

Standard_Integer Draw_Interpretor::EvalFile(const Standard_CString fname)
{
  return 0;
}

//=======================================================================
//function : PrintHelp
//purpose  :
//=======================================================================

Standard_Integer Draw_Interpretor::PrintHelp (const Standard_CString theCommandName)
{
  return 0;
}

//=======================================================================
//function :Complete
//purpose  : 
//=======================================================================

Standard_Boolean Draw_Interpretor::Complete(const Standard_CString line)
{
  return true;
}

//=======================================================================
//function : Destroy
//purpose  : 
//=======================================================================

Draw_Interpretor::~Draw_Interpretor()
{
  SetDoLog (Standard_False);
}

//=======================================================================
//function : Logging
//purpose  : 
//=======================================================================

void Draw_Interpretor::SetDoLog (Standard_Boolean doLog)
{
  if (myDoLog == doLog)
    return;
  myDoLog = doLog;
}

void Draw_Interpretor::SetDoEcho (Standard_Boolean doEcho)
{
  myDoEcho = doEcho;
}

Standard_Boolean Draw_Interpretor::GetDoLog () const
{
  return myDoLog;
}

Standard_Boolean Draw_Interpretor::GetDoEcho () const
{
  return myDoEcho;
}

void Draw_Interpretor::ResetLog ()
{
  if (myFDLog < 0)
    return;

}

void Draw_Interpretor::AddLog (const Standard_CString theStr)
{
  if (myFDLog < 0 || ! theStr || ! theStr[0])
    return;

  // flush cerr and cout, for the case if they are bound to the log
  flush_standard_streams();

  // write as plain bytes
  std::cout << theStr << std::endl;
}

TCollection_AsciiString Draw_Interpretor::GetLog ()
{
  TCollection_AsciiString aLog;
  if (myFDLog < 0)
    return aLog;

  return aLog;
}
