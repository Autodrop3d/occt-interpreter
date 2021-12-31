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

#include <Draw.hxx>
#include <Draw_ProgressIndicator.hxx>
#include <Message.hxx>
#include <NCollection_Array1.hxx>
#include <NCollection_Map.hxx>
#include <Standard_SStream.hxx>
#include <Standard_Stream.hxx>
#include <Standard_NotImplemented.hxx>
#include <TCollection_AsciiString.hxx>

#include <errno.h>

#include <OSD_Environment.hxx>

CStringMap<Standard_Real> Draw::reals;

Standard_Boolean Draw_ParseFailed = Standard_True;

//=======================================================================
// Atof and Atoi
//=======================================================================

static Standard_Boolean Numeric(char c)
{
  return (c == '.' || (c >= '0' && c <= '9'));
}

static Standard_Boolean Alphabetic(char c)
{
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_'));
}

static Standard_Real Parse(char*&);

static Standard_Real ParseValue (char*& theName)
{
  while (*theName == ' ' || *theName == '\t') { ++theName; }
  Standard_Real x = 0;
  switch (*theName)
  {
    case '\0':
    {
      break;
    }
    case '(':
    {
      ++theName;
      x = Parse (theName);
      if (*theName != ')')
      {
        std::cout << "Mismatched parenthesis" << std::endl;
      }
      ++theName;
      break;
    }
    case '+':
    {
      ++theName;
      x = ParseValue (theName);
      break;
    }
    case '-':
    {
      ++theName;
      x = - ParseValue (theName);
      break;
    }
    default:
    {
      // process a string
      char* p = theName;
      while (Numeric (*p)) { ++p; }
      // process scientific notation
      if ((*p == 'e') || (*p == 'E'))
      {
        if (Numeric (*(p+1)) || *(p+1) == '+' || *(p+1) == '-')
        {
          p+= 2;
        }
      }
      while (Numeric (*p) || Alphabetic (*p)) { p++; }
      char c = *p;
      *p = '\0';

      if (Numeric (*theName))   // numeric literal
      {
        x = Atof (theName);
      }
      else if (!Draw::Get ((Standard_CString )theName, x)) // variable
      {
        // search for a function ...
        *p = c;
        // count arguments
        Standard_Integer argc = 1;
        char* q = p;
        while ((*q == ' ') || (*q == '\t')) { ++q; }
        if (*q == '(')
        {
          Standard_Integer pc = 1;
          argc = 2;
          q++;
          while ((pc > 0) && *q)
          {
            if (*q == '(') { ++pc; }
            if (*q == ')') { --pc; }
            if ((pc == 1) && (*q == ',')) { ++argc; }
            ++q;
          }
          if (pc > 0)
          {
            std::cout << "Unclosed parenthesis"<< std::endl;
            x = 0;
          }
          else
          {
            // build function call
            // replace , and first and last () by space
            if (argc > 1)
            {
              Standard_Integer i = 2;
              while (*p != '(') { ++p; }
              *p = ' ';
              ++p;
              pc = 1;
              while (pc > 0)
              {
                if (*p == '(') { ++pc; }
                if (*p == ')') { --pc; }
                if ((pc == 1) && (*p == ','))
                {
                  *p = ' ';
                  ++p;
                  ++i;
                }
                else
                {
                  ++p;
                }
              }
              *(p-1) = '\0';
              c = *p;

              Draw_Interpretor& aCommands = Draw::GetInterpretor();

              // call the function, save the current result
              TCollection_AsciiString sv (aCommands.Result());
              if (*aCommands.Result())
              {
                aCommands.Reset();
              }
              if (aCommands.Eval (theName) != 0)
              {
                std::cout << "Call of function " << theName << " failed" << std::endl;
                x = 0;
              }
              else
              {
                x = Atof (aCommands.Result());
              }
              aCommands.Reset();
              if (!sv.IsEmpty())
              {
                aCommands << sv;
              }
            }
          }
        }
        else
        {
          Draw_ParseFailed = Standard_True;
        }
      }
      *p = c;
      theName = p;
    }
    break;
  }

  while (*theName == ' ' || *theName == '\t')
  {
    ++theName;
  }
  return x;
}


static Standard_Real ParseFactor(char*& name)
{
  Standard_Real x = ParseValue(name);

  for(;;) {
    char c = *name;
    if (c == '\0') return x;
    name++;
    switch (c) {

    case '*' :
      x *= ParseValue(name);
      break;

    case '/' :
      x /= ParseValue(name);
      break;

      default :
	name--;
	return x;
      
    }
  }
}

static Standard_Real Parse(char*& name)
{
  Standard_Real x = ParseFactor(name);

  for(;;) {
    char c = *name;
    if (c == '\0') return x;
    name++;
    switch (c) {

    case '+' :
      x += ParseFactor(name);
      break;

    case '-' :
      x -= ParseFactor(name);
      break;

      default :
  name--;
  return x;
      
    }
  }
}

//=======================================================================
// function : Atof
// purpose  :
//=======================================================================
Standard_Real Draw::Atof(const Standard_CString theName)
{
  // copy the string
  NCollection_Array1<char> aBuff (0, (Standard_Integer )strlen (theName));
  char* n = &aBuff.ChangeFirst();
  strcpy (n, theName);
  Draw_ParseFailed = Standard_False;
  Standard_Real x = Parse(n);
  while ((*n == ' ') || (*n == '\t')) n++;
  if (*n) Draw_ParseFailed = Standard_True;
  return x;
}

//=======================================================================
// function : ParseReal
// purpose  :
//=======================================================================
bool Draw::ParseReal (const Standard_CString theExpressionString, Standard_Real& theParsedRealValue)
{
  const Standard_Real aParsedRealValue = Atof (theExpressionString);
  if (Draw_ParseFailed)
  {
    Draw_ParseFailed = Standard_False;
    return false;
  }
  theParsedRealValue = aParsedRealValue;
  return true;
}

//=======================================================================
// function : Atoi
// purpose  :
//=======================================================================
Standard_Integer Draw::Atoi(const Standard_CString name)
{
  return (Standard_Integer) Draw::Atof(name);
}

//=======================================================================
// function : ParseInteger
// purpose  :
//=======================================================================
bool Draw::ParseInteger (const Standard_CString theExpressionString, Standard_Integer& theParsedIntegerValue)
{
  Standard_Real aParsedRealValue = 0.0;
  if (!ParseReal (theExpressionString, aParsedRealValue))
  {
    return false;
  }
  const Standard_Integer aParsedIntegerValue = static_cast<Standard_Integer> (aParsedRealValue);
  if (static_cast<Standard_Real> (aParsedIntegerValue) != aParsedRealValue)
  {
    return false;
  }
  theParsedIntegerValue = aParsedIntegerValue;
  return true;
}

//=======================================================================
//function : Set
//purpose  : set a TCL var
//=======================================================================
void Draw::Set(const Standard_CString Name, const Standard_CString val)
{
  Standard_PCharacter pName, pVal;
  //
  pName=(Standard_PCharacter)Name;
  pVal=(Standard_PCharacter)val;
  //
  // Tcl_SetVar(Draw::GetInterpretor().Interp(), pName, pVal, 0);
  //
  throw 'unimplemeneted';
}

void Draw::Set(const Standard_CString theName, const Standard_Real theValue)
{
  Draw::reals[theName] = theValue;
}

Standard_Boolean Draw::Get (const Standard_CString theName,
                            Standard_Real& theValue)
{
  if (Draw::reals.count(theName)) {
    theValue = Draw::reals[theName];
    return Standard_True;
  } else {
    return Standard_False;
  }
}

//=======================================================================
// Command management
// refresh the screen
//=======================================================================

static void before()
{
}


extern void (*Draw_BeforeCommand)();
extern void (*Draw_AfterCommand)(Standard_Integer);

