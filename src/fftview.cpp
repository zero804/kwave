#include <math.h>
#include <limits.h>
#include <qdir.h>
#include <qpainter.h>
#include <qcursor.h>
#include <qkeycode.h>
#include <libkwave/kwavesignal.h>
#include <libkwave/dialogoperation.h>
#include <libkwave/dynamicloader.h>
#include <libkwave/interpolation.h>
#include <libkwave/parser.h>
#include "../libgui/kwavedialog.h"
#include "signalmanager.h"
#include "fftview.h"
#include "main.h"

extern KApplication *app;
extern KwavePlugin **dialogplugins;
//****************************************************************************
FFTContainer::FFTContainer (QWidget *parent): QWidget (parent)
{
  this->view=0;
}
//****************************************************************************
void FFTContainer::setObjects (FFTWidget *view,ScaleWidget *x,ScaleWidget *y,CornerPatchWidget *corner)
{
  this->view=view;
  this->xscale=x;
  this->yscale=y;
  this->corner=corner;
}
//****************************************************************************
FFTContainer::~FFTContainer ()
{
}
//****************************************************************************
void FFTContainer::resizeEvent  (QResizeEvent *)
{
  if (view)
    {
      int bsize=(QPushButton("test",this).sizeHint()).height();
      view->setGeometry (bsize,0,width()-bsize,height()-bsize);  
      xscale->setGeometry       (bsize,height()-bsize,width()-bsize,bsize);  
      yscale->setGeometry       (0,0,bsize,height()-bsize);
      corner->setGeometry       (0,height()-bsize,bsize,bsize);
    }
}
//****************************************************************************
FFTWindow::FFTWindow (QString *name) : KTopLevelWidget (name->data())
{
  QPopupMenu *fft=      new QPopupMenu ();
  QPopupMenu *view=     new QPopupMenu ();
	      cursor=   new QPopupMenu ();
  QPopupMenu *edit=     new QPopupMenu ();
  QPopupMenu *dbmenu=   new QPopupMenu ();
  KMenuBar   *bar=      new KMenuBar (this); 

  bar->insertItem       (klocale->translate("&Spectral Data"),fft);
  bar->insertItem       (klocale->translate("&Edit"),edit);
  bar->insertItem       (klocale->translate("&View"),view);
  bar->insertItem       (klocale->translate("&Cursor"),cursor);

  status=new KStatusBar (this,"Frequencies Status Bar");
  status->insertItem ("Frequency:          0 Hz     ",1);
  status->insertItem ("Amplitude:    0 %      ",2);
  status->insertItem ("Phase:    0        ",3);  
  status->insertItem ("Note:    0        ",4);  

  mainwidget=new FFTContainer (this);

  fftview=new FFTWidget (mainwidget);
  xscale=new ScaleWidget (mainwidget,0,100,"Hz");
  yscale=new ScaleWidget (mainwidget,100,0,"%");
  corner=new CornerPatchWidget (mainwidget);

  mainwidget->setObjects (fftview,xscale,yscale,corner);

  edit->insertItem      (klocale->translate("Multiply with graph"),fftview,SLOT(amplify()));
  edit->insertItem      (klocale->translate("Multiply with formant pattern"),fftview,SLOT(formant()));
  edit->insertItem      (klocale->translate("Smooth"),fftview,SLOT(smooth()));
  edit->insertSeparator ();
  edit->insertItem      (klocale->translate("Kill phase"),fftview,SLOT(killPhase()));
  cursor->insertItem    (klocale->translate("find Maximum"),fftview,SLOT(findMaxPeak()),Key_M);
  cursor->insertItem    (klocale->translate("find Minimum"),fftview,SLOT(findMinimum()),SHIFT+Key_M);
  findPeakID = cursor->insertItem (klocale->translate("find nearest Peak"),this,SLOT(findPeak()),Key_Tab);
  cursor->setCheckable( TRUE );
  cursor->setItemChecked( findPeakID, true );


  fft->insertItem       (klocale->translate("Inverse FFT"),fftview,SLOT(iFFT()));

  view->insertItem      (klocale->translate("Amplitude in %"),this,SLOT(percentMode()));
  view->insertItem      (klocale->translate("Amplitude in dB"),dbmenu);

  for (int i=0;i<110;i+=10)
    {
      char buf[10];
      sprintf (buf,"0-%d dB",i+50);
      dbmenu->insertItem (buf);
    }
  connect (dbmenu,SIGNAL (activated(int)),this,SLOT(dbMode(int)));

  view->insertItem (klocale->translate("Phase"),this,SLOT(phaseMode()));

  connect (fftview,SIGNAL(freqInfo(int,int)),this,SLOT(setFreqInfo(int,int)));
  connect (fftview,SIGNAL(ampInfo(int,int)),this,SLOT(setAmpInfo(int,int)));
  connect (fftview,SIGNAL(noteInfo(int,int)),this,SLOT(setNoteInfo(int,int)));
  connect (fftview,SIGNAL(dbInfo(int,int)),this,SLOT(setDBInfo(int,int)));
  connect (fftview,SIGNAL(phaseInfo(int,int)),this,SLOT(setPhaseInfo(int,int)));
  setView (mainwidget);
  setStatusBar (status);
  setMenu (bar);

 QString *windowname=new QString (QString ("Frequencies of ")+QString(name->data()));
  setCaption (windowname->data()); 
  resize (480,300);
  setMinimumSize (480,300);
}
//****************************************************************************
void FFTWindow::phaseMode  ()
{
  fftview->phaseMode ();
  yscale->setMaxMin (180,-180);
  yscale->setUnit   ("�");
}
//****************************************************************************
void FFTWindow::percentMode  ()
{
  fftview->percentMode ();
  yscale->setMaxMin (0,100);
  yscale->setUnit   ("%");
}
//****************************************************************************
void FFTWindow::dbMode  (int mode)
{
  fftview->dbMode (50+mode*10);
  yscale->setMaxMin (-(50+mode*10),0);
  yscale->setUnit   ("db");
}
//****************************************************************************
void FFTWindow::setSignal (complex *data,double max,int size, int rate)
{
  fftview->setSignal (data,size,rate);
  xscale ->setMaxMin (rate/2,0); 
}
//****************************************************************************
void FFTWindow::findPeak  ()
{
  bool snap = 0;
  fftview->togglefindPeak (&snap);
  cursor->setItemChecked( findPeakID, snap );
  if (snap) {
    fftview->repaint (false);
  }
}
//****************************************************************************
void FFTWindow::askFreqRange ()
{
}
//****************************************************************************
FFTWindow::~FFTWindow ()
{
}
//****************************************************************************
void FFTWindow::setFreqInfo  (int hz,int err)
{
  char buf[64];

  sprintf (buf,"Frequency: %d +/-%d Hz",hz,err);
  status->changeItem (buf,1);
}
//****************************************************************************
void FFTWindow::setAmpInfo  (int amp,int err)
{
  char buf[64];

  sprintf (buf,"Amplitude: %d +/-%d %%",amp,err);
  status->changeItem (buf,2);
}
//****************************************************************************
void FFTWindow::setDBInfo  (int amp,int err)
{
  char buf[64];
  sprintf (buf,"Amplitude: %d dB",amp);
  status->changeItem (buf,2);
}
//****************************************************************************
void FFTWindow::setPhaseInfo  (int ph,int err)
{
  char buf[64];

  sprintf (buf,"Phase: %d +/- %d",ph,err);
  status->changeItem (buf,3);
}
//****************************************************************************
//Note detection contributed by Gerhard Zintel
void FFTWindow::setNoteInfo  (int hz,int x)
{
  char buf[64];
  float BaseNoteA = 440.0; //We should be able to set this in a menu
  static char *notename[] = {
     "C", "C#/Db", "D", "D#/Eb", "E", "F", "F#/Gb", "G", "G#/Ab", "A", "A#/Bb", "B"
  };
  int octave, note; // note = round(57 + 12 * (log(hz) - log(440)) / log(2))
  note = 45 + (int) (12.0*(log(hz)-log(BaseNoteA))/log(2)+0.5);
  if (note < 0) note = 0;
  octave = note / 12;
  note = note % 12;

  sprintf (buf,"Note: %s%d ",notename[note],octave);
  status->changeItem (buf,4);
}



