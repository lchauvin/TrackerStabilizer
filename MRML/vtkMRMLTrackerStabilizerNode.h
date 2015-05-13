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

#ifndef __vtkMRMLTrackerStabilizerNode_h
#define __vtkMRMLTrackerStabilizerNode_h

#include "vtkCommand.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"

// TrackerStabilizer includes
#include "vtkSlicerTrackerStabilizerModuleMRMLExport.h"

class vtkMRMLLinearTransformNode;

class
VTK_SLICER_TRACKERSTABILIZER_MODULE_MRML_EXPORT
vtkMRMLTrackerStabilizerNode
: public vtkMRMLNode
{
public:

  enum Events
  {
    // vtkCommand::UserEvent + 777 is just a random value that is very unlikely to be used for anything else in this class
    InputDataModifiedEvent = vtkCommand::UserEvent + 777
  };

  vtkTypeMacro( vtkMRMLTrackerStabilizerNode, vtkMRMLNode);

  // Standard MRML node methods

  static vtkMRMLTrackerStabilizerNode *New();

  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "TrackerStabilizer"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int nIndent );
  virtual void Copy( vtkMRMLNode *anode );

protected:
  
  // Constructor/destructor methods
  
  vtkMRMLTrackerStabilizerNode();
  virtual ~vtkMRMLTrackerStabilizerNode();
  vtkMRMLTrackerStabilizerNode( const vtkMRMLTrackerStabilizerNode& );
  void operator=( const vtkMRMLTrackerStabilizerNode& );

public:
  
  vtkGetMacro( CutOffFrequency, double );
  vtkSetMacro( CutOffFrequency, double );

  vtkGetMacro( FilterActivated, bool );
  vtkSetMacro( FilterActivated, bool );
  vtkBooleanMacro( FilterActivated, bool );
  
  vtkMRMLLinearTransformNode* GetInputTransformNode();
  void SetAndObserveInputTransformNodeID( const char* inputNodeId );

  vtkMRMLLinearTransformNode* GetFilteredTransformNode();
  void SetAndObserveFilteredTransformNodeID( const char* filteredNodeId );  

  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData );

private:
  
  double CutOffFrequency;
  bool FilterActivated;

};

#endif
