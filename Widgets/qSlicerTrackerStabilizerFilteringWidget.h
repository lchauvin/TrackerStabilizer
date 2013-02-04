/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Laurent Chauvin (SNR), Jayender Jagadeesan (SPL)
  and was partially funded by NIH grant 3P41RR013218-12S1

  ==============================================================================*/

#ifndef __qSlicerTrackerStabilizerFilteringWidget_h
#define __qSlicerTrackerStabilizerFilteringWidget_h

// Qt includes
#include <QWidget>
#include <ctkVTKObject.h>

// FilteredTransform Widgets includes
#include "qSlicerTrackerStabilizerModuleWidgetsExport.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"

// VTK includes
#include "vtkMatrix4x4.h"
#include "vtkSmartPointer.h"

class qSlicerTrackerStabilizerFilteringWidgetPrivate;
class vtkMRMLScene;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_TrackerStabilizer
class Q_SLICER_MODULE_TRACKERSTABILIZER_WIDGETS_EXPORT qSlicerTrackerStabilizerFilteringWidget
: public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

 public:
  typedef QWidget Superclass;
  qSlicerTrackerStabilizerFilteringWidget(QWidget *parent=0);
  virtual ~qSlicerTrackerStabilizerFilteringWidget();

  protected slots:
    virtual void setMRMLScene(vtkMRMLScene *newScene);
    void onInputNodeActivated(vtkMRMLNode* newInputNode);
    void onOutputNodeActivated(vtkMRMLNode* newOutputNode);
    void onInputTransformModified();
    void applyFiltering(vtkMRMLLinearTransformNode* input, vtkMRMLLinearTransformNode* output);

 protected:
  QScopedPointer<qSlicerTrackerStabilizerFilteringWidgetPrivate> d_ptr;

 private:
  Q_DECLARE_PRIVATE(qSlicerTrackerStabilizerFilteringWidget);
  Q_DISABLE_COPY(qSlicerTrackerStabilizerFilteringWidget);
};

#endif
