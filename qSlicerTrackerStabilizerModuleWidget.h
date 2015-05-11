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

#ifndef __qSlicerTrackerStabilizerModuleWidget_h
#define __qSlicerTrackerStabilizerModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerTrackerStabilizerModuleExport.h"

class qSlicerTrackerStabilizerModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_TRACKERSTABILIZER_EXPORT qSlicerTrackerStabilizerModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerTrackerStabilizerModuleWidget(QWidget *parent=0);
  virtual ~qSlicerTrackerStabilizerModuleWidget();

public slots:

  virtual void setMRMLScene( vtkMRMLScene* scene );
  void onSceneImportedEvent();

protected slots:

  void onModuleNodeChanged();

  void onRefreshTimeout();
  void onFilteringToggled(bool filter);
  void onInputNodeChanged();
  void onOutputNodeChanged();
  void onCutOffFrequencyChanged(double cutoff);
  void UpdateFromMRMLNode();

protected:
  QScopedPointer<qSlicerTrackerStabilizerModuleWidgetPrivate> d_ptr;
  
  virtual void setup();
  virtual void enter();

private:
  Q_DECLARE_PRIVATE(qSlicerTrackerStabilizerModuleWidget);
  Q_DISABLE_COPY(qSlicerTrackerStabilizerModuleWidget);
};

#endif
