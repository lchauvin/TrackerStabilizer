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

// TrackerStabilizer MRML includes
#include "vtkMRMLTrackerStabilizerNode.h"

// Other MRML includes
#include "vtkMRMLLinearTransformNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkIntArray.h>
#include <vtkCommand.h>

// Other includes
#include <sstream>

// Constants
static const char* INPUT_TRANSFORM_ROLE = "inputTransformNode";
static const char* FILTERED_TRANSFORM_ROLE = "filteredTransformNode";

//-----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTrackerStabilizerNode);

vtkMRMLTrackerStabilizerNode
::vtkMRMLTrackerStabilizerNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );

  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkCommand::ModifiedEvent );

  this->AddNodeReferenceRole( INPUT_TRANSFORM_ROLE, NULL, events.GetPointer() );
  this->AddNodeReferenceRole( FILTERED_TRANSFORM_ROLE, NULL, events.GetPointer() );

  this->CutOffFrequency = 7.5;
  this->FilterActivated = false;
}

//-----------------------------------------------------------------------------
vtkMRMLTrackerStabilizerNode
::~vtkMRMLTrackerStabilizerNode()
{
}

//-----------------------------------------------------------------------------
void vtkMRMLTrackerStabilizerNode
::WriteXML( ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << " cutoffFrequency=\"" << this->CutOffFrequency << "\"";
  of << indent << " filterActivated=\"" << ( this->FilterActivated ? "true" : "false" ) << "\"";
}

//-----------------------------------------------------------------------------
void vtkMRMLTrackerStabilizerNode
::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "cutoffFrequency"))
      {
      std::stringstream ss;
      ss << attValue;
      double val;
      ss >> val;
      this->CutOffFrequency = val;
      }
    else if(!strcmp(attName, "filterActivated"))
      {
      if (!strcmp(attValue,"true"))
	{
	this->FilterActivated = true;
	}
      else
	{
	this->FilterActivated = false;
	}
      }
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLTrackerStabilizerNode
::Copy( vtkMRMLNode *anode )
{
  Superclass::Copy( anode );

  vtkMRMLTrackerStabilizerNode *node = ( vtkMRMLTrackerStabilizerNode* ) anode;

  this->CutOffFrequency = node->CutOffFrequency;
  this->FilterActivated = node->FilterActivated;

  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkMRMLTrackerStabilizerNode
::PrintSelf( ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os, indent);

  os << indent << "InputTransformNodeID: " << this->GetInputTransformNode()->GetID() << std::endl;
  os << indent << "FilteredTransformNodeID: " << this->GetFilteredTransformNode()->GetID() << std::endl;
  os << indent << "CutOff Frequency: " << this->CutOffFrequency << std::endl;
  os << indent << "Filter Activated: " << this->FilterActivated << std::endl;
}

//-----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTrackerStabilizerNode
::GetInputTransformNode()
{
  vtkMRMLLinearTransformNode* inputNode = vtkMRMLLinearTransformNode::SafeDownCast(
    this->GetNodeReference( INPUT_TRANSFORM_ROLE ) );
  return inputNode;
}

//-----------------------------------------------------------------------------
void vtkMRMLTrackerStabilizerNode
::SetAndObserveInputTransformNodeID( const char* inputNodeId )
{
  // SetAndObserveNodeReferenceID does not handle nicely setting of the same
  // node (it should simply ignore the request, but it adds another observer instead)
  // so check for node equality here.
  const char* currentNodeId = this->GetNodeReferenceID(INPUT_TRANSFORM_ROLE);
  if (inputNodeId != NULL && currentNodeId != NULL)
    {
    if (strcmp(inputNodeId, currentNodeId) == 0)
      {
      // not changed
      return;
      }
    }
  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkCommand::ModifiedEvent );
  this->SetAndObserveNodeReferenceID( INPUT_TRANSFORM_ROLE, inputNodeId, events.GetPointer() );
  this->InvokeEvent(InputDataModifiedEvent); // This will tell the logic to update
}

//-----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTrackerStabilizerNode
::GetFilteredTransformNode()
{
  vtkMRMLLinearTransformNode* filteredNode = vtkMRMLLinearTransformNode::SafeDownCast(
    this->GetNodeReference( FILTERED_TRANSFORM_ROLE ) );
  return filteredNode;
}

//-----------------------------------------------------------------------------
void vtkMRMLTrackerStabilizerNode
::SetAndObserveFilteredTransformNodeID( const char* filteredNodeId )
{
  // SetAndObserveNodeReferenceID does not handle nicely setting of the same
  // node (it should simply ignore the request, but it adds another observer instead)
  // so check for node equality here.
  const char* currentNodeId = this->GetNodeReferenceID(FILTERED_TRANSFORM_ROLE);
  if (filteredNodeId != NULL && currentNodeId != NULL)
    {
    if (strcmp(filteredNodeId, currentNodeId) == 0)
      {
      // not changed
      return;
      }
    }
  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkCommand::ModifiedEvent );
  this->SetAndObserveNodeReferenceID( FILTERED_TRANSFORM_ROLE, filteredNodeId, events.GetPointer() );
  this->InvokeEvent(InputDataModifiedEvent); // This will tell the logic to update
}

//-----------------------------------------------------------------------------
void vtkMRMLTrackerStabilizerNode
::ProcessMRMLEvents( vtkObject *caller, unsigned long /*event*/, void* /*callData*/ )
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if ( callerNode == NULL ) return;

  if (this->GetInputTransformNode() && this->GetInputTransformNode() == caller)
    {
    this->InvokeEvent(InputDataModifiedEvent); // This will tell the logic to update
    }
  else if (this->GetFilteredTransformNode() && this->GetFilteredTransformNode() == caller)
    {
    this->InvokeEvent(InputDataModifiedEvent); // This will tell the logic to update
    }
}
