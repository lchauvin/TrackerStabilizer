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

// TrackerStabilizer Logic includes
#include "vtkSlicerTrackerStabilizerLogic.h"

// MRML includes

// VTK includes
#include <vtkIntArray.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTrackerStabilizerLogic);

//----------------------------------------------------------------------------
vtkSlicerTrackerStabilizerLogic::vtkSlicerTrackerStabilizerLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerTrackerStabilizerLogic::~vtkSlicerTrackerStabilizerLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerTrackerStabilizerLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerTrackerStabilizerLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerTrackerStabilizerLogic::RegisterNodes()
{
  if( !this->GetMRMLScene() )
    {
    vtkWarningMacro( "MRML scene not yet created" );
    return;
    }

  this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLTrackerStabilizerNode >::New() );
}

//---------------------------------------------------------------------------
void vtkSlicerTrackerStabilizerLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerTrackerStabilizerLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
    {
    vtkWarningMacro( "OnMRMLSceneNodeAdded: Invalid MRML scene or node" );
    return;
    }

  if ( node->IsA( "vtkMRMLTrackerStabilizerNode" ) )
    {
    vtkDebugMacro( "OnMRMLSceneNodeAdded: Module node added." );
    vtkUnObserveMRMLNodeMacro( node );
    vtkNew<vtkIntArray> events;
    events->InsertNextValue( vtkCommand::ModifiedEvent );
    events->InsertNextValue( vtkMRMLTrackerStabilizerNode::InputDataModifiedEvent );
    vtkObserveMRMLNodeEventsMacro( node, events.GetPointer() );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerTrackerStabilizerLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
    {
    vtkWarningMacro( "OnMRMLSceneNodeRemoved: Invalid MRML scene or node" );
    return;
    }

  if ( node->IsA( "vtkMRMLTrackerStabilizer" ) )
    {
    vtkDebugMacro( "OnMRMLSceneNodeRemoved" );
    vtkUnObserveMRMLNodeMacro( node );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerTrackerStabilizerLogic
::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* /*callData*/)
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if ( callerNode == NULL )
    {
    return;
    }

  vtkMRMLTrackerStabilizerNode* tsNode = vtkMRMLTrackerStabilizerNode::SafeDownCast( callerNode );
  if ( tsNode == NULL )
    {
    return;
    }

  if ( event == vtkMRMLTrackerStabilizerNode::InputDataModifiedEvent )
    {
    }
}

//----------------------------------------------------------------------------
// Spherical linear interpolation between two rotation quaternions.
// t is a value between 0 and 1 that interpolates between from and to (t=0 means the results is the same as "from").
// Precondition: no aliasing problems to worry about ("result" can be "from" or "to" param).
// Parameters: adjustSign - If true, then slerp will operate by adjusting the sign of the slerp to take shortest path. True is recommended, otherwise the interpolation sometimes give unexpected results. 
// References: From Adv Anim and Rendering Tech. Pg 364

void vtkSlicerTrackerStabilizerLogic
::Slerp(double *result, double t, double *from, double *to, bool adjustSign)
{
  const double* p = from; // just an alias to match q

  // calc cosine theta
  double cosom = from[0]*to[0]+from[1]*to[1]+from[2]*to[2]+from[3]*to[3]; // dot( from, to )

  // adjust signs (if necessary)
  double q[4];
  if (adjustSign && (cosom < (double)0.0))
  {
    cosom = -cosom;
    q[0] = -to[0];   // Reverse all signs
    q[1] = -to[1];
    q[2] = -to[2];
    q[3] = -to[3];
  }
  else
  {
    q[0] = to[0];
    q[1] = to[1];
    q[2] = to[2];
    q[3] = to[3];
  }

  // Calculate coefficients
  double sclp, sclq;
  if (((double)1.0 - cosom) > (double)0.0001) // 0.0001 -> some epsillon
  {
    // Standard case (slerp)
    double omega, sinom;
    omega = acos( cosom ); // extract theta from dot product's cos theta
    sinom = sin( omega );
    sclp  = sin( ((double)1.0 - t) * omega ) / sinom;
    sclq  = sin( t * omega ) / sinom;
  }
  else
  {
    // Very close, do linear interp (because it's faster)
    sclp = (double)1.0 - t;
    sclq = t;
  }

  for (int i=0; i<4; i++)
  {
    result[i] = sclp * p[i] + sclq * q[i];
  }
}

//----------------------------------------------------------------------------
// Interpolate the matrix for the given timestamp from the two nearest
// transforms in the buffer.
// The rotation is interpolated with SLERP interpolation, and the
// position is interpolated with linear interpolation.
// The flags correspond to the closest element.

void vtkSlicerTrackerStabilizerLogic
::GetInterpolatedTransform(vtkMatrix4x4* itemAmatrix, vtkMatrix4x4* itemBmatrix, 
			   double itemAweight, double itemBweight,
			   vtkMatrix4x4* interpolatedMatrix)
{
  double itemAweightNormalized=itemAweight/(itemAweight+itemBweight);
  double itemBweightNormalized=itemBweight/(itemAweight+itemBweight);

  double matrixA[3][3]={{0,0,0},{0,0,0},{0,0,0}};
  for (int i = 0; i < 3; i++)
  {
    matrixA[i][0] = itemAmatrix->GetElement(i,0);
    matrixA[i][1] = itemAmatrix->GetElement(i,1);
    matrixA[i][2] = itemAmatrix->GetElement(i,2);
  }

  double matrixB[3][3] = {{0,0,0}, {0,0,0}, {0,0,0}};
  for (int i = 0; i < 3; i++)
  {
    matrixB[i][0] = itemBmatrix->GetElement(i,0);
    matrixB[i][1] = itemBmatrix->GetElement(i,1);
    matrixB[i][2] = itemBmatrix->GetElement(i,2);
  }

  double matrixAquat[4]= {0,0,0,0};
  vtkMath::Matrix3x3ToQuaternion(matrixA, matrixAquat);
  double matrixBquat[4]= {0,0,0,0};
  vtkMath::Matrix3x3ToQuaternion(matrixB, matrixBquat);
  double interpolatedRotationQuat[4]= {0,0,0,0};
  Slerp(interpolatedRotationQuat, itemBweightNormalized, matrixAquat, matrixBquat);
  double interpolatedRotation[3][3] = {{0,0,0},{0,0,0},{0,0,0}};
  vtkMath::QuaternionToMatrix3x3(interpolatedRotationQuat, interpolatedRotation);

  for (int i = 0; i < 3; i++)
  {
    interpolatedMatrix->Element[i][0] = interpolatedRotation[i][0];
    interpolatedMatrix->Element[i][1] = interpolatedRotation[i][1];
    interpolatedMatrix->Element[i][2] = interpolatedRotation[i][2];
    interpolatedMatrix->Element[i][3] = itemAmatrix->GetElement(i,3)*itemAweightNormalized + 
      itemBmatrix->GetElement(i,3)*itemBweightNormalized;
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerTrackerStabilizerLogic
::Filter(vtkMRMLTrackerStabilizerNode* tsNode)
{
  if ( tsNode == NULL )
    {
    return;
    }

  vtkMRMLLinearTransformNode* inputNode = tsNode->GetInputTransformNode();
  vtkMRMLLinearTransformNode* outputNode = tsNode->GetFilteredTransformNode();

  if ( inputNode == NULL || outputNode == NULL)
    {
    return;
    }

  // Compute weights (low-pass filter with w_cutoff frequency)
  const double dt = 0.015; // 15ms TODO: get it from timestamp
  const double cutoff_frequency = tsNode->GetCutOffFrequency();
  const double weightPrevious = 1;
  const double weightCurrent = dt*cutoff_frequency;

  // Get matrices
  vtkSmartPointer<vtkMatrix4x4> matrixCurrent = vtkSmartPointer<vtkMatrix4x4>::New();
  inputNode->GetMatrixTransformToParent(matrixCurrent);

  vtkSmartPointer<vtkMatrix4x4> matrixPrevious = vtkSmartPointer<vtkMatrix4x4>::New();
  outputNode->GetMatrixTransformToParent(matrixPrevious);

  vtkSmartPointer<vtkMatrix4x4> matrixOutput = vtkSmartPointer<vtkMatrix4x4>::New();
  if (tsNode->GetFilterActivated() == false)
    {
    // No filter. Output Transform = Input Transform
    inputNode->GetMatrixTransformToParent(matrixOutput);
    }
  else
    {
    // Compute interpolated matrix
    GetInterpolatedTransform(matrixPrevious, matrixCurrent, weightPrevious, weightCurrent, matrixOutput);
    }

  // Setting the TransformNode
  outputNode->SetMatrixTransformToParent(matrixOutput);
}
