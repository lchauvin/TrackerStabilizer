/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Laurent Chauvin, Brigham and Women's
  Hospital. The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 8P41EB015898
 
==============================================================================*/

// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerTrackerStabilizerModuleWidget.h"
#include "ui_qSlicerTrackerStabilizerModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerTrackerStabilizerModuleWidgetPrivate: public Ui_qSlicerTrackerStabilizerModuleWidget
{
public:
  qSlicerTrackerStabilizerModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerTrackerStabilizerModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerTrackerStabilizerModuleWidgetPrivate::qSlicerTrackerStabilizerModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerTrackerStabilizerModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerTrackerStabilizerModuleWidget::qSlicerTrackerStabilizerModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerTrackerStabilizerModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerTrackerStabilizerModuleWidget::~qSlicerTrackerStabilizerModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerModuleWidget::setup()
{
  Q_D(qSlicerTrackerStabilizerModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  if (d->FilteringWidget)
    {
    connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
	    d->FilteringWidget, SLOT(setMRMLScene(vtkMRMLScene*)));
    }
}

