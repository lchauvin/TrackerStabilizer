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

  This file was originally developed by Laurent Chauvin and
  Jayender Jagadeesan, Brigham and Women's Hospital.
  The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 8P41EB015898, 3P41RR013218
 
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

  /// Apply a low-pass filter on the input and return filtered transform in the output
  void applyFilter(vtkMRMLLinearTransformNode* input, vtkMRMLLinearTransformNode* output);

  protected slots:
    virtual void setMRMLScene(vtkMRMLScene *newScene);
    void onInputNodeActivated(vtkMRMLNode* newInputNode);
    void onOutputNodeActivated(vtkMRMLNode* newOutputNode);
    void onInputTransformModified();

 protected:
  QScopedPointer<qSlicerTrackerStabilizerFilteringWidgetPrivate> d_ptr;

 private:
  Q_DECLARE_PRIVATE(qSlicerTrackerStabilizerFilteringWidget);
  Q_DISABLE_COPY(qSlicerTrackerStabilizerFilteringWidget);
};

#endif
