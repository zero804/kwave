
#include <qpushbt.h>
#include "libgui/ScaleWidget.h"
#include "libgui/CornerPatchWidget.h"

#include "PitchWidget.h"
#include "PitchContainer.h"

PitchContainer::PitchContainer (QWidget *parent): QWidget (parent)
{
  this->view=0;
}
//****************************************************************************
void PitchContainer::setObjects (PitchWidget *view,ScaleWidget *x,ScaleWidget *y,CornerPatchWidget *corner)
{
  this->view=view;
  this->xscale=x;
  this->yscale=y;
  this->corner=corner;
}
//****************************************************************************
PitchContainer::~PitchContainer ()
{
}
//****************************************************************************
void PitchContainer::resizeEvent (QResizeEvent *)
{
  if (view)
    {
      int bsize=(QPushButton("test",this).sizeHint()).height();
      view->setGeometry	 (bsize,0,width()-bsize,height()-bsize);  
      xscale->setGeometry(bsize,height()-bsize,width()-bsize,bsize);  
      yscale->setGeometry(0,0,bsize,height()-bsize);
      corner->setGeometry(0,height()-bsize,bsize,bsize);
    }
}
//****************************************************************************
