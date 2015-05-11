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

// Qt includes
#include <QDebug>
#include <QMessageBox>
#include <QTimer>

// SlicerQt includes
#include "qSlicerTrackerStabilizerModuleWidget.h"
#include "ui_qSlicerTrackerStabilizerModuleWidget.h"

#include "vtkSlicerTrackerStabilizerLogic.h"

#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTrackerStabilizerNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerTrackerStabilizerModuleWidgetPrivate: public Ui_qSlicerTrackerStabilizerModuleWidget
{
  Q_DECLARE_PUBLIC( qSlicerTrackerStabilizerModuleWidget );

protected:
  qSlicerTrackerStabilizerModuleWidget* const q_ptr;
public:
  qSlicerTrackerStabilizerModuleWidgetPrivate( qSlicerTrackerStabilizerModuleWidget& object );
  ~qSlicerTrackerStabilizerModuleWidgetPrivate();
  vtkSlicerTrackerStabilizerLogic* logic() const;

  QTimer* RefreshTimer;
};

//-----------------------------------------------------------------------------
// qSlicerTrackerStabilizerModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerTrackerStabilizerModuleWidgetPrivate::qSlicerTrackerStabilizerModuleWidgetPrivate( qSlicerTrackerStabilizerModuleWidget& object) : q_ptr( &object )
{
  this->RefreshTimer = new QTimer();
}

//-----------------------------------------------------------------------------
qSlicerTrackerStabilizerModuleWidgetPrivate::~qSlicerTrackerStabilizerModuleWidgetPrivate()
{
  if (this->RefreshTimer)
    {
    this->RefreshTimer->stop();
    this->RefreshTimer->deleteLater();
    }
}

//-----------------------------------------------------------------------------
vtkSlicerTrackerStabilizerLogic* qSlicerTrackerStabilizerModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerTrackerStabilizerModuleWidget );
  return vtkSlicerTrackerStabilizerLogic::SafeDownCast( q->logic() );
}

//-----------------------------------------------------------------------------
// qSlicerTrackerStabilizerModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerTrackerStabilizerModuleWidget::qSlicerTrackerStabilizerModuleWidget(QWidget* _parent)
  : Superclass( _parent )
    , d_ptr( new qSlicerTrackerStabilizerModuleWidgetPrivate( *this ) )
{
}

//-----------------------------------------------------------------------------
qSlicerTrackerStabilizerModuleWidget::~qSlicerTrackerStabilizerModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerModuleWidget::setup()
{
  Q_D(qSlicerTrackerStabilizerModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  this->setMRMLScene( d->logic()->GetMRMLScene() );

  connect(d->ModuleNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
	  this, SLOT(onModuleNodeChanged()));

  connect(d->RefreshTimer, SIGNAL(timeout()),
	  this, SLOT(onRefreshTimeout()));

  connect(d->FilteringBox, SIGNAL(toggled(bool)),
	  this, SLOT(onFilteringToggled(bool)));

  connect(d->InputTransformWidget, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
	  this, SLOT(onInputNodeChanged()));

  connect(d->OutputTransformWidget, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
	  this, SLOT(onOutputNodeChanged()));

  connect(d->FilteringValueWidget, SIGNAL(valueChanged(double)),
	  this, SLOT(onCutOffFrequencyChanged(double)));

  this->UpdateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerModuleWidget::enter()
{
  Q_D(qSlicerTrackerStabilizerModuleWidget);

  if (this->mrmlScene() == NULL)
    {
    qCritical() << "Invalid scene!";
    return;
    }

  // Create a module MRML node if there is none in the scene.
  vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLTrackerStabilizerNode");
  if (node == NULL)
    {
    vtkSmartPointer< vtkMRMLTrackerStabilizerNode > newNode =
      vtkSmartPointer< vtkMRMLTrackerStabilizerNode >::New();
    this->mrmlScene()->AddNode(newNode);
    }

  node = this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLTrackerStabilizerNode");
  if (node == NULL)
    {
    qCritical("Failed to create module node");
    return;
    }

  // For convenience, select a default node
  if (d->ModuleNodeComboBox->currentNode() == NULL)
    {
    d->ModuleNodeComboBox->setCurrentNodeID(node->GetID());
    }

  // Start timer if not already started
  if (d->RefreshTimer->isActive() == false)
    {
    d->RefreshTimer->start(50);
    }

  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerTrackerStabilizerModuleWidget);

  this->Superclass::setMRMLScene(scene);
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerModuleWidget::onSceneImportedEvent()
{
  this->enter();
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerModuleWidget::onModuleNodeChanged()
{
  Q_D(qSlicerTrackerStabilizerModuleWidget);

  this->UpdateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerModuleWidget::onRefreshTimeout()
{
  Q_D(qSlicerTrackerStabilizerModuleWidget);

  if (this->mrmlScene() == NULL)
    {
    return;
    }

  vtkCollection* filteringNodes = this->mrmlScene()->GetNodesByClass("vtkMRMLTrackerStabilizerNode");
  for (int i = 0; i < filteringNodes->GetNumberOfItems(); ++i)
    {
    vtkMRMLTrackerStabilizerNode* tsNode = vtkMRMLTrackerStabilizerNode::SafeDownCast(
      filteringNodes->GetItemAsObject(i));
    if (tsNode == NULL)
      {
      return;
      }

    d->logic()->Filter(tsNode);
    }

  filteringNodes->Delete();
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerModuleWidget::onFilteringToggled(bool filter)
{
  Q_D(qSlicerTrackerStabilizerModuleWidget);

  vtkMRMLTrackerStabilizerNode* tsNode = vtkMRMLTrackerStabilizerNode::SafeDownCast(
    d->ModuleNodeComboBox->currentNode());
  if (tsNode == NULL)
    {
    qCritical("Filter has been toggled with no module node selection");
    return;
    }

  tsNode->SetFilterActivated(filter);
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerModuleWidget::onInputNodeChanged()
{
  Q_D(qSlicerTrackerStabilizerModuleWidget);

  vtkMRMLTrackerStabilizerNode* tsNode = vtkMRMLTrackerStabilizerNode::SafeDownCast(
    d->ModuleNodeComboBox->currentNode());
  if (tsNode == NULL)
    {
    qCritical("Input node has changed with no module node selection");
    return;
    }

  vtkMRMLLinearTransformNode* inputNode = vtkMRMLLinearTransformNode::SafeDownCast(
    d->InputTransformWidget->currentNode());
  tsNode->SetAndObserveInputTransformNodeID( (inputNode!=NULL) ? inputNode->GetID() : NULL);
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerModuleWidget::onOutputNodeChanged()
{
  Q_D(qSlicerTrackerStabilizerModuleWidget);

  vtkMRMLTrackerStabilizerNode* tsNode = vtkMRMLTrackerStabilizerNode::SafeDownCast(
    d->ModuleNodeComboBox->currentNode());
  if (tsNode == NULL)
    {
    qCritical("Output node has changed with no module node selection");
    return;
    }

  vtkMRMLLinearTransformNode* outputNode = vtkMRMLLinearTransformNode::SafeDownCast(
    d->OutputTransformWidget->currentNode());

  // Sanity check: If the output transform is already selected as output in another filter
  // with a different input, the output results will be the average of both inputs.
  // User should be warned in this case.

  bool differentFiltersUsingSameOutput = false;
  bool useSameOutputForMultipleFilters = false;
  vtkCollection* filteringNodes = this->mrmlScene()->GetNodesByClass("vtkMRMLTrackerStabilizerNode");
  for (int i = 0; i < filteringNodes->GetNumberOfItems(); ++i)
    {
    vtkMRMLTrackerStabilizerNode* tmpNode = vtkMRMLTrackerStabilizerNode::SafeDownCast(
      filteringNodes->GetItemAsObject(i));
    if (tmpNode == NULL)
      {
      continue;
      }

    vtkMRMLLinearTransformNode* filterOutputNode = tmpNode->GetFilteredTransformNode();
    if (filterOutputNode != NULL && filterOutputNode == outputNode)
      {
      // Same output node has been found in another filter in the scene
      differentFiltersUsingSameOutput = true;

      QMessageBox warningMsg;
      std::stringstream ss;
      ss << "Another filter (ID: " << tmpNode->GetID() << ") has been found in the scene with the same output transform. Having two or more filters with the same output will result in the average of all inputs of these filters. Are you sure you want to select this transform as output for this filter ?";
      warningMsg.setText(ss.str().c_str());
      warningMsg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
      warningMsg.setDefaultButton(QMessageBox::No);
      int ret = warningMsg.exec();

      useSameOutputForMultipleFilters = (ret == QMessageBox::Yes);

      break;
      }
    }
  filteringNodes->Delete();

  if (differentFiltersUsingSameOutput == true && useSameOutputForMultipleFilters == false)
    {
    tsNode->SetAndObserveFilteredTransformNodeID("");
    this->UpdateFromMRMLNode();
    return;
    }

  tsNode->SetAndObserveFilteredTransformNodeID( (outputNode!=NULL) ? outputNode->GetID() : NULL);
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerModuleWidget::onCutOffFrequencyChanged(double cutoff)
{
  Q_D(qSlicerTrackerStabilizerModuleWidget);

  vtkMRMLTrackerStabilizerNode* tsNode = vtkMRMLTrackerStabilizerNode::SafeDownCast(
    d->ModuleNodeComboBox->currentNode());
  if (tsNode == NULL)
    {
    qCritical("Cutoff Frequency changed with no module node selection");
    return;
    }

  tsNode->SetCutOffFrequency(cutoff);
}

//-----------------------------------------------------------------------------
void qSlicerTrackerStabilizerModuleWidget::UpdateFromMRMLNode()
{
  Q_D(qSlicerTrackerStabilizerModuleWidget);

  vtkMRMLTrackerStabilizerNode* tsNode = vtkMRMLTrackerStabilizerNode::SafeDownCast(
    d->ModuleNodeComboBox->currentNode());
  if (tsNode == NULL)
    {
    d->InputTransformWidget->setCurrentNodeID("");
    d->OutputTransformWidget->setCurrentNodeID("");
    d->FilteringBox->setChecked(false);
    return;
    }

  d->InputTransformWidget->setCurrentNode(tsNode->GetInputTransformNode());
  d->OutputTransformWidget->setCurrentNode(tsNode->GetFilteredTransformNode());

  d->FilteringBox->setChecked(tsNode->GetFilterActivated());
  d->FilteringValueWidget->setValue(tsNode->GetCutOffFrequency());
}

