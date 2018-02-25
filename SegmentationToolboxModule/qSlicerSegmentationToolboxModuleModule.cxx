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

==============================================================================*/

// SegmentationToolboxModule Logic includes
#include <vtkSlicerSegmentationToolboxModuleLogic.h>

// SegmentationToolboxModule includes
#include "qSlicerSegmentationToolboxModuleModule.h"
#include "qSlicerSegmentationToolboxModuleModuleWidget.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerSegmentationToolboxModuleModule, qSlicerSegmentationToolboxModuleModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerSegmentationToolboxModuleModulePrivate
{
public:
  qSlicerSegmentationToolboxModuleModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerSegmentationToolboxModuleModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerSegmentationToolboxModuleModulePrivate::qSlicerSegmentationToolboxModuleModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerSegmentationToolboxModuleModule methods

//-----------------------------------------------------------------------------
qSlicerSegmentationToolboxModuleModule::qSlicerSegmentationToolboxModuleModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSegmentationToolboxModuleModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerSegmentationToolboxModuleModule::~qSlicerSegmentationToolboxModuleModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerSegmentationToolboxModuleModule::helpText() const
{
  return "This module allows using machine learning to segment preprocessed images using one of the included algorithms.";
}

//-----------------------------------------------------------------------------
QString qSlicerSegmentationToolboxModuleModule::acknowledgementText() const
{
  return "";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSegmentationToolboxModuleModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Daniel Chalupa");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerSegmentationToolboxModuleModule::icon() const
{
  return QIcon(":/Icons/SegmentationToolboxModule.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerSegmentationToolboxModuleModule::categories() const
{
  return QStringList() << "Segmentation";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSegmentationToolboxModuleModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationToolboxModuleModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerSegmentationToolboxModuleModule
::createWidgetRepresentation()
{
  return new qSlicerSegmentationToolboxModuleModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerSegmentationToolboxModuleModule::createLogic()
{
  return vtkSlicerSegmentationToolboxModuleLogic::New();
}
