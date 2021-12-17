#include "stdafx.h"

#include "Sample2D_Image.h"

#include <Graphic3d_Texture2Dmanual.hxx>
#include <Image_AlienPixMap.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Sample2D_Image,AIS_Shape)

Sample2D_Image::Sample2D_Image(TCollection_AsciiString& aFileName,
                               const Standard_Real X,
                               const Standard_Real Y,
                               const Standard_Real aScale)
    :AIS_Shape(TopoDS_Shape())
{
  myFilename = aFileName;
  myX = X;
  myY = Y;  
  myScale = aScale;
}
void Sample2D_Image::MakeShape()
{
  Standard_Real coeff = 1.0;
  Handle(Image_AlienPixMap) anImage = new Image_AlienPixMap();
  if (anImage->Load (myFilename))
  {
    coeff = Standard_Real(anImage->Height()) / Standard_Real(anImage->Width()) * myScale;
  }

  TopoDS_Edge E1 = BRepBuilderAPI_MakeEdge(gp_Pnt(myX,myY,0.), gp_Pnt(100*myScale+myX,myY,0.));
  TopoDS_Edge E2 = BRepBuilderAPI_MakeEdge(gp_Pnt(100*myScale+myX,myY,0.), gp_Pnt(100*myScale+myX,100*coeff+myY,0.));
  TopoDS_Edge E3 = BRepBuilderAPI_MakeEdge(gp_Pnt(100*myScale+myX,100*coeff+myY,0.), gp_Pnt(myX,100*coeff+myY,0.));
  TopoDS_Edge E4 = BRepBuilderAPI_MakeEdge(gp_Pnt(myX,100*coeff+myY,0.), gp_Pnt(myX,myY,0.));
  TopoDS_Wire anImageBounds = BRepBuilderAPI_MakeWire(E1,E2,E3,E4);
  myFace = BRepBuilderAPI_MakeFace(gp_Pln(gp_Pnt(0,0,0),gp_Dir(0,0,1)),anImageBounds);
}

void Sample2D_Image::SetContext(const Handle(AIS_InteractiveContext)& theContext) 
{
  if(theContext.IsNull() || theContext->CurrentViewer().IsNull()) return;
  AIS_InteractiveObject::SetContext(theContext);
  MakeShape();
  this->Set(TopoDS_Shape(myFace));
  myDrawer->SetShadingAspect (new Prs3d_ShadingAspect());
  Handle(Graphic3d_Texture2Dmanual) aTexture = new Graphic3d_Texture2Dmanual (myFilename);
  aTexture->DisableModulate();
  myDrawer->ShadingAspect()->Aspect()->SetTextureMap (aTexture);
  myDrawer->ShadingAspect()->Aspect()->SetTextureMapOn();
}
