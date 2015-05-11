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

// .NAME vtkSlicerTrackerStabilizerLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerTrackerStabilizerLogic_h
#define __vtkSlicerTrackerStabilizerLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTrackerStabilizerNode.h"

// STD includes
#include <cstdlib>

#include "vtkSlicerTrackerStabilizerModuleLogicExport.h"

class vtkMatrix4x4;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_TRACKERSTABILIZER_MODULE_LOGIC_EXPORT vtkSlicerTrackerStabilizerLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerTrackerStabilizerLogic *New();
  vtkTypeMacro(vtkSlicerTrackerStabilizerLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData);

  void Filter(vtkMRMLTrackerStabilizerNode* tsNode);

protected:
  vtkSlicerTrackerStabilizerLogic();
  virtual ~vtkSlicerTrackerStabilizerLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

  void Slerp(double* result, double t, double* from, double* to, bool adjustSign = true);
  void GetInterpolatedTransform(vtkMatrix4x4* itemAmatrix, vtkMatrix4x4* itemBmatrix,
				double itemAweight, double itemBweight,
				vtkMatrix4x4* interpolatedMatrix);

private:

  vtkSlicerTrackerStabilizerLogic(const vtkSlicerTrackerStabilizerLogic&); // Not implemented
  void operator=(const vtkSlicerTrackerStabilizerLogic&);               // Not implemented
};

#endif
