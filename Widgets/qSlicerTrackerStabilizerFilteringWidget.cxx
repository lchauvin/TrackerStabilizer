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
  QTimer* RefreshTimer;

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
  this->RefreshTimer = new QTimer();
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

  connect(d->RefreshTimer, SIGNAL(timeout()),
	  this, SLOT(onRefreshTimeout()));

  connect(d->FilteringBox, SIGNAL(toggled(bool)),
	  this, SLOT(onFilteringToggled(bool)));
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
    connect(d->InputTransformWidget, SIGNAL(nodeActivated(vtkMRMLNode*)),
            this, SLOT(onInputNodeActivated(vtkMRMLNode*)));
    }

  if (d->OutputTransformWidget)
    {
    d->OutputTransformWidget->setMRMLScene(newScene);
    connect(d->OutputTransformWidget, SIGNAL(nodeActivated(vtkMRMLNode*)),
            this, SLOT(onOutputNodeActivated(vtkMRMLNode*)));
    }
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerFilteringWidget
::onInputNodeActivated(vtkMRMLNode* newInputNode)
{
  Q_D(qSlicerTrackerStabilizerFilteringWidget);

  if (newInputNode == d->InputTransformNode)
    {
    return;
    }

  // Set new input node
  vtkMRMLLinearTransformNode* newTransformNode = 
    vtkMRMLLinearTransformNode::SafeDownCast(newInputNode);
  
  if (newTransformNode)
    {
    qvtkDisconnect(d->InputTransformNode, vtkMRMLLinearTransformNode::TransformModifiedEvent,
                   this, SLOT(onInputTransformModified()));

    if (newTransformNode == d->OutputTransformNode)
      {
      // Output node cannot be used as input node
      d->InputTransformWidget->setCurrentNode(NULL);
      }
    else
      {
      d->InputTransformNode = newTransformNode;
      if (d->OutputTransformNode)
	{
	vtkSmartPointer<vtkMatrix4x4> inputMatrix =
	  vtkSmartPointer<vtkMatrix4x4>::New();
	d->InputTransformNode->GetMatrixTransformToWorld(inputMatrix.GetPointer());
	d->OutputTransformNode->SetMatrixTransformToParent(inputMatrix);
	}
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

  if (newOutputNode == d->OutputTransformNode)
    {
    return;
    }

  // Set new output node
  vtkMRMLLinearTransformNode* newTransformNode = 
    vtkMRMLLinearTransformNode::SafeDownCast(newOutputNode);

  if (newTransformNode)
    {
    if (newTransformNode == d->InputTransformNode)
      {
      // Input node cannot be used as output node
      d->OutputTransformWidget->setCurrentNode(NULL);
      }
    else
      {
      d->OutputTransformNode = newTransformNode;
      }
    }

  // Initialize output node
  if (d->OutputTransformNode)
    {
    vtkSmartPointer<vtkMatrix4x4> identityMatrix =
      vtkSmartPointer<vtkMatrix4x4>::New();
    identityMatrix->Identity();

    d->OutputTransformNode->SetMatrixTransformToParent(identityMatrix);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerFilteringWidget
::onRefreshTimeout()
{
  Q_D(qSlicerTrackerStabilizerFilteringWidget);

  this->applyFilter(d->InputTransformNode, d->OutputTransformNode);
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerFilteringWidget
::onInputTransformModified()
{
  Q_D(qSlicerTrackerStabilizerFilteringWidget);

  // Update Output transform when filter is deactivated
  if (d->FilteringBox->isChecked())
    {
    return;
    }

  if (!d->InputTransformNode || !d->OutputTransformNode)
    {
    return;
    }

  vtkSmartPointer<vtkMatrix4x4> inputMatrix =
    vtkSmartPointer<vtkMatrix4x4>::New();
  d->InputTransformNode->GetMatrixTransformToParent(inputMatrix.GetPointer());
  d->OutputTransformNode->SetMatrixTransformToParent(inputMatrix);
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerFilteringWidget
::onFilteringToggled(bool filter)
{
  Q_D(qSlicerTrackerStabilizerFilteringWidget);

  if (filter)
    {
    d->RefreshTimer->start(50);
    }
  else
    {
    d->RefreshTimer->stop();

    if (d->InputTransformNode && d->OutputTransformNode)
      {
      vtkSmartPointer<vtkMatrix4x4> inputMatrix =
	vtkSmartPointer<vtkMatrix4x4>::New();
      d->InputTransformNode->GetMatrixTransformToWorld(inputMatrix.GetPointer());
      d->OutputTransformNode->SetMatrixTransformToParent(inputMatrix);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerFilteringWidget
::applyFilter(vtkMRMLLinearTransformNode* input, vtkMRMLLinearTransformNode* output)
{
  Q_D(qSlicerTrackerStabilizerFilteringWidget);

  if (!input || !output)
    {
    return;
    }

  double unfiltPosOr[6], filtPosOr[6];
  double dt = 0.015; // 15ms

  // Get Matrixes
  vtkSmartPointer<vtkMatrix4x4> inputMatrix =
    vtkSmartPointer<vtkMatrix4x4>::New();
  input->GetMatrixTransformToParent(inputMatrix);

  vtkSmartPointer<vtkMatrix4x4> outputMatrix =
    vtkSmartPointer<vtkMatrix4x4>::New();
  output->GetMatrixTransformToParent(outputMatrix);

  unfiltPosOr[0] = inputMatrix->GetElement(0,3);
  unfiltPosOr[1] = inputMatrix->GetElement(1,3);
  unfiltPosOr[2] = inputMatrix->GetElement(2,3);

  // Unfiltered Beta
  double unfilteredRadius = pow(pow(inputMatrix->GetElement(0,0),2)+pow(inputMatrix->GetElement(1,0),2),0.5);
  unfiltPosOr[4] =
    atan2(-inputMatrix->GetElement(2,0), unfilteredRadius); 

  // Unfiltered Gamma
  unfiltPosOr[5] =
    atan2(inputMatrix->GetElement(2,1)/cos(unfiltPosOr[4]),inputMatrix->GetElement(2,2)/cos(unfiltPosOr[4]));

  // Unfiltered Alpha
  unfiltPosOr[3] =
    atan2(inputMatrix->GetElement(1,0)/cos(unfiltPosOr[4]),inputMatrix->GetElement(0,0)/cos(unfiltPosOr[4]));

  filtPosOr[0] = outputMatrix->GetElement(0,3);
  filtPosOr[1] = outputMatrix->GetElement(1,3);
  filtPosOr[2] = outputMatrix->GetElement(2,3);

  // Accumulated Beta
  double filteredRadius = pow(pow(outputMatrix->GetElement(0,0),2)+pow(outputMatrix->GetElement(1,0),2),0.5);
  filtPosOr[4] =
    atan2(-outputMatrix->GetElement(2,0), filteredRadius);

  // Accumulated Gamma
  filtPosOr[5] =
    atan2(outputMatrix->GetElement(2,1)/cos(filtPosOr[4]),outputMatrix->GetElement(2,2)/cos(filtPosOr[4]));
  
  // Accumulated Alpha
  filtPosOr[3] =
    atan2(outputMatrix->GetElement(1,0)/cos(filtPosOr[4]),outputMatrix->GetElement(0,0)/cos(filtPosOr[4]));

  double cutoff_frequency = d->FilteringValueWidget->value();
  for (int i = 0; i < 6; i++)
    {
    // Low pass filter with w_cutoff frequency
    filtPosOr[i]  = (filtPosOr[i] + dt*cutoff_frequency*unfiltPosOr[i])/(1 + cutoff_frequency*dt);
    }

  // Alpha, Beta and Gamma angles
  double a = filtPosOr[3];
  double b  = filtPosOr[4];
  double g = filtPosOr[5];

  // Create the transformation again
  vtkSmartPointer<vtkMatrix4x4> transformationMatrix =
    vtkSmartPointer<vtkMatrix4x4> ::New();

  transformationMatrix->SetElement(0,0,cos(a)*cos(b));
  transformationMatrix->SetElement(0,1,cos(a)*sin(b)*sin(g)-sin(a)*cos(g));
  transformationMatrix->SetElement(0,2,cos(a)*sin(b)*cos(g)+sin(a)*sin(g));
  transformationMatrix->SetElement(0,3,filtPosOr[0]);

  transformationMatrix->SetElement(1,0,sin(a)*cos(b));
  transformationMatrix->SetElement(1,1,sin(a)*sin(b)*sin(g)+cos(a)*cos(g));
  transformationMatrix->SetElement(1,2,sin(a)*sin(b)*cos(g)-cos(a)*sin(g));
  transformationMatrix->SetElement(1,3,filtPosOr[1]);

  transformationMatrix->SetElement(2,0,-sin(b));
  transformationMatrix->SetElement(2,1,cos(b)*sin(g));
  transformationMatrix->SetElement(2,2,cos(b)*cos(g));
  transformationMatrix->SetElement(2,3,filtPosOr[2]);

  transformationMatrix->SetElement(3,0,0);
  transformationMatrix->SetElement(3,1,0);
  transformationMatrix->SetElement(3,2,0);
  transformationMatrix->SetElement(3,3,1);

  // Setting the TransformNode
  output->SetMatrixTransformToParent(transformationMatrix);
}
