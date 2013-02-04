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

  if(d->InputTransformWidget)
    {
    d->InputTransformWidget->setMRMLScene(newScene);
    connect(d->InputTransformWidget, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
            this, SLOT(onInputNodeActivated(vtkMRMLNode*)));
    }

  if(d->OutputTransformWidget)
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
  vtkMRMLLinearTransformNode* newTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(newInputNode);
  if(newTransformNode)
    {
    std::cerr << "Disconnect" << std::endl;
    qvtkDisconnect(d->InputTransformNode, vtkMRMLLinearTransformNode::TransformModifiedEvent,
                   this, SLOT(onInputTransformModified()));

    d->InputTransformNode = newTransformNode;

    if(d->OutputTransformNode && d->InputTransformNode)
      {
      // If InputNode is changed, we set OutputNode to InputNode to restart filtering to avoid filtering with 2 different inputs
      d->OutputTransformNode->GetMatrixTransformToParent()->DeepCopy(d->InputTransformNode->GetMatrixTransformToParent());
      }

    std::cerr << "Connect" << std::endl;
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
  vtkMRMLLinearTransformNode* newTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(newOutputNode);
  if(newTransformNode)
    {
    if(newTransformNode != d->InputTransformNode)
      {
      d->OutputTransformNode = newTransformNode;
      }
    else
      {
      d->InputTransformWidget->setCurrentNode(d->OutputTransformNode);
      }
    }

  if(d->OutputTransformNode)
    {
    d->OutputTransformNode->GetMatrixTransformToParent()->Identity();
    }
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerFilteringWidget
::onInputTransformModified()
{
  Q_D(qSlicerTrackerStabilizerFilteringWidget);

  if(d->FilteringBox && d->InputTransformNode && d->OutputTransformNode)
    {
    if(d->FilteringBox->isChecked() && d->FilteringValueWidget)
      {
      this->applyFiltering(d->InputTransformNode, d->OutputTransformNode);
      }
    else
      {
      // No filter applied. Just change pointer to avoid copying data.
      d->OutputTransformNode->SetAndObserveMatrixTransformToParent(d->InputTransformNode->GetMatrixTransformToParent());

      std::cerr << "No Filtering" << std::endl;
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerFilteringWidget
::applyFiltering(vtkMRMLLinearTransformNode* input, vtkMRMLLinearTransformNode* output)
{
  Q_D(qSlicerTrackerStabilizerFilteringWidget);

  double UnfiltPosOr[6], FiltPosOr[6];
  double dt = 0.015; // 15ms

  // Get Matrixes
  vtkSmartPointer<vtkMatrix4x4> inputMatrix =
    input->GetMatrixTransformToParent();

  vtkSmartPointer<vtkMatrix4x4> outputMatrix =
    output->GetMatrixTransformToParent();

  // Filter
  UnfiltPosOr[0] = inputMatrix->GetElement(0,3);
  UnfiltPosOr[1] = inputMatrix->GetElement(1,3);
  UnfiltPosOr[2] = inputMatrix->GetElement(2,3);

  UnfiltPosOr[4] =
    atan2(-inputMatrix->GetElement(2,0),pow(pow(inputMatrix->GetElement(0,0),2)+pow(inputMatrix->GetElement(1,0),2),0.5)
    ); //beta

  UnfiltPosOr[5] =
    atan2(inputMatrix->GetElement(2,1)/cos(UnfiltPosOr[4]),inputMatrix->GetElement(2,2)/cos(UnfiltPosOr[4]));
  // gamma

  UnfiltPosOr[3] =
    atan2(inputMatrix->GetElement(1,0)/cos(UnfiltPosOr[4]),inputMatrix->GetElement(0,0)/cos(UnfiltPosOr[4]));
  //alpha

  FiltPosOr[0] = outputMatrix->GetElement(0,3);
  FiltPosOr[1] = outputMatrix->GetElement(1,3);
  FiltPosOr[2] = outputMatrix->GetElement(2,3);

  FiltPosOr[4] =
    atan2(-outputMatrix->GetElement(2,0),pow(pow(outputMatrix->GetElement(0,0),2)+pow(outputMatrix->GetElement(1,0),2),0.5)
    ); //beta

  FiltPosOr[5] =
    atan2(outputMatrix->GetElement(2,1)/cos(FiltPosOr[4]),outputMatrix->GetElement(2,2)/cos(FiltPosOr[4]));
  // gamma

  FiltPosOr[3] =
    atan2(outputMatrix->GetElement(1,0)/cos(FiltPosOr[4]),outputMatrix->GetElement(0,0)/cos(FiltPosOr[4]));
  // alpha

  double cutoff_frequency = d->FilteringValueWidget->value();
  for(int i = 0; i < 6; i++)
    {
    // Low pass filter with w_cutoff frequency
    FiltPosOr[i]  = (FiltPosOr[i] + dt*cutoff_frequency*UnfiltPosOr[i])/(1 + cutoff_frequency*dt);
    }

  double a = FiltPosOr[3];
  double b = FiltPosOr[4];
  double g = FiltPosOr[5];

  // Create the transformation again
  vtkSmartPointer<vtkMatrix4x4> TransformationMatrix =
    vtkSmartPointer<vtkMatrix4x4> ::New();

  TransformationMatrix->SetElement(0,0,cos(a)*cos(b));
  TransformationMatrix->SetElement(0,1,cos(a)*sin(b)*sin(g)-sin(a)*cos(g));
  TransformationMatrix->SetElement(0,2,cos(a)*sin(b)*cos(g)+sin(a)*sin(g));
  TransformationMatrix->SetElement(0,3,FiltPosOr[0]);

  TransformationMatrix->SetElement(1,0,sin(a)*cos(b));
  TransformationMatrix->SetElement(1,1,sin(a)*sin(b)*sin(g)+cos(a)*cos(g));
  TransformationMatrix->SetElement(1,2,sin(a)*sin(b)*cos(g)-cos(a)*sin(g));
  TransformationMatrix->SetElement(1,3,FiltPosOr[1]);

  TransformationMatrix->SetElement(2,0,-sin(b));
  TransformationMatrix->SetElement(2,1,cos(b)*sin(g));
  TransformationMatrix->SetElement(2,2,cos(b)*cos(g));
  TransformationMatrix->SetElement(2,3,FiltPosOr[2]);

  TransformationMatrix->SetElement(3,0,0);
  TransformationMatrix->SetElement(3,1,0);
  TransformationMatrix->SetElement(3,2,0);
  TransformationMatrix->SetElement(3,3,1);

  // Setting the TransformNode
  output->SetAndObserveMatrixTransformToParent(TransformationMatrix);
}
