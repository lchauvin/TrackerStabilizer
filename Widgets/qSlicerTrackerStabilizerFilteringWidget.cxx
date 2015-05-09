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

// FilteredTransform Widgets includes
#include "qSlicerTrackerStabilizerFilteringWidget.h"
#include "ui_qSlicerTrackerStabilizerFilteringWidget.h"

#include <vtkMath.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TrackerStabilizer
class qSlicerTrackerStabilizerFilteringWidgetPrivate
  : public Ui_qSlicerTrackerStabilizerFilteringWidget
{
  Q_DECLARE_PUBLIC(qSlicerTrackerStabilizerFilteringWidget);
protected:
  qSlicerTrackerStabilizerFilteringWidget* const q_ptr;

  vtkMRMLLinearTransformNode* InputTransformNode;
  vtkMRMLLinearTransformNode* OutputTransformNode;

public:
  qSlicerTrackerStabilizerFilteringWidgetPrivate(
    qSlicerTrackerStabilizerFilteringWidget& object);
  virtual void setupUi(qSlicerTrackerStabilizerFilteringWidget*);
};

// --------------------------------------------------------------------------
qSlicerTrackerStabilizerFilteringWidgetPrivate
::qSlicerTrackerStabilizerFilteringWidgetPrivate(
  qSlicerTrackerStabilizerFilteringWidget& object)
  : q_ptr(&object)
{
  this->InputTransformNode = NULL;
  this->OutputTransformNode = NULL;
}

// --------------------------------------------------------------------------
void qSlicerTrackerStabilizerFilteringWidgetPrivate
::setupUi(qSlicerTrackerStabilizerFilteringWidget* widget)
{
  this->Ui_qSlicerTrackerStabilizerFilteringWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerTrackerStabilizerFilteringWidget methods

//-----------------------------------------------------------------------------
qSlicerTrackerStabilizerFilteringWidget
::qSlicerTrackerStabilizerFilteringWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
    , d_ptr( new qSlicerTrackerStabilizerFilteringWidgetPrivate(*this) )
{
  Q_D(qSlicerTrackerStabilizerFilteringWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerTrackerStabilizerFilteringWidget
::~qSlicerTrackerStabilizerFilteringWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerFilteringWidget
::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qSlicerTrackerStabilizerFilteringWidget);

  if (d->InputTransformWidget)
    {
    d->InputTransformWidget->setMRMLScene(newScene);
    connect(d->InputTransformWidget, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
            this, SLOT(onInputNodeActivated(vtkMRMLNode*)));
    }

  if (d->OutputTransformWidget)
    {
    d->OutputTransformWidget->setMRMLScene(newScene);
    connect(d->OutputTransformWidget, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
            this, SLOT(onOutputNodeActivated(vtkMRMLNode*)));
    }
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerFilteringWidget
::onInputNodeActivated(vtkMRMLNode* newInputNode)
{
  Q_D(qSlicerTrackerStabilizerFilteringWidget);

  // Set new input node
  vtkMRMLLinearTransformNode* newTransformNode = 
    vtkMRMLLinearTransformNode::SafeDownCast(newInputNode);
  
  if (newTransformNode)
    {
    qvtkDisconnect(d->InputTransformNode, vtkMRMLLinearTransformNode::TransformModifiedEvent,
                   this, SLOT(onInputTransformModified()));

    d->InputTransformNode = newTransformNode;

    if (d->OutputTransformNode && d->InputTransformNode)
      {
      // If InputNode is changed, we set OutputNode to InputNode 
      // to initialize filtering
      vtkSmartPointer<vtkMatrix4x4> matrixCurrent = vtkSmartPointer<vtkMatrix4x4>::New();
      d->InputTransformNode->GetMatrixTransformToParent(matrixCurrent);  
      d->OutputTransformNode->SetMatrixTransformToParent(matrixCurrent);
      }

    qvtkConnect(d->InputTransformNode, vtkMRMLLinearTransformNode::TransformModifiedEvent,
                this, SLOT(onInputTransformModified()));
    }
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerFilteringWidget
::onOutputNodeActivated(vtkMRMLNode* newOutputNode)
{
  Q_D(qSlicerTrackerStabilizerFilteringWidget);

  // Set new output node
  vtkMRMLLinearTransformNode* newTransformNode = 
    vtkMRMLLinearTransformNode::SafeDownCast(newOutputNode);

  if (newTransformNode)
    {
    if (newTransformNode != d->InputTransformNode)
      {
      d->OutputTransformNode = newTransformNode;
      }
    else
      {
      d->InputTransformWidget->setCurrentNode(d->OutputTransformNode);
      }
    }

  if (d->OutputTransformNode)
    {
    d->OutputTransformNode->GetMatrixTransformToParent()->Identity();
    }
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerFilteringWidget
::onInputTransformModified()
{
  Q_D(qSlicerTrackerStabilizerFilteringWidget);

  if (d->FilteringBox && d->InputTransformNode && d->OutputTransformNode)
    {
    if (d->FilteringBox->isChecked() && d->FilteringValueWidget)
      {
      this->applyFilter(d->InputTransformNode, d->OutputTransformNode);
      }
    else
      {
      // No filter applied. Just change pointer to avoid copying data.
      vtkMatrix4x4* inputMatrixToParent = d->InputTransformNode->GetMatrixTransformToParent();
      if (inputMatrixToParent)
        {
        d->OutputTransformNode->SetAndObserveMatrixTransformToParent(inputMatrixToParent);
        }
      }
    }
}

//----------------------------------------------------------------------------
// Spherical linear interpolation between two rotation quaternions.
// t is a value between 0 and 1 that interpolates between from and to (t=0 means the results is the same as "from").
// Precondition: no aliasing problems to worry about ("result" can be "from" or "to" param).
// Parameters: adjustSign - If true, then slerp will operate by adjusting the sign of the slerp to take shortest path. True is recommended, otherwise the interpolation sometimes give unexpected results. 
// References: From Adv Anim and Rendering Tech. Pg 364
void Slerp(double *result, double t, double *from, double *to, bool adjustSign = true)   
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
void GetInterpolatedTransform(vtkMatrix4x4* itemAmatrix, vtkMatrix4x4* itemBmatrix, double itemAweight, double itemBweight, vtkMatrix4x4* interpolatedMatrix)
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
    interpolatedMatrix->Element[i][3] = itemAmatrix->GetElement(i,3)*itemAweightNormalized + itemBmatrix->GetElement(i,3)*itemBweightNormalized;
  }
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerFilteringWidget
::applyFilter(vtkMRMLLinearTransformNode* input, vtkMRMLLinearTransformNode* output)
{
  Q_D(qSlicerTrackerStabilizerFilteringWidget);

  // Compute weights (low-pass filter with w_cutoff frequency)
  const double dt = 0.015; // 15ms TODO: get it from timestamp
  const double cutoff_frequency = d->FilteringValueWidget->value();
  const double weightPrevious = 1;
  const double weightCurrent = dt*cutoff_frequency; 

  // Get matrices
  vtkSmartPointer<vtkMatrix4x4> matrixCurrent = vtkSmartPointer<vtkMatrix4x4>::New();
  input->GetMatrixTransformToParent(matrixCurrent);

  vtkSmartPointer<vtkMatrix4x4> matrixPrevious = vtkSmartPointer<vtkMatrix4x4>::New();
  output->GetMatrixTransformToParent(matrixPrevious);
  
  // Compute interpolated matrix
  vtkSmartPointer<vtkMatrix4x4> matrixNew = vtkSmartPointer<vtkMatrix4x4>::New();
  GetInterpolatedTransform(matrixPrevious, matrixCurrent, weightPrevious, weightCurrent, matrixNew);

  // Setting the TransformNode
  output->SetMatrixTransformToParent(matrixNew);
}
