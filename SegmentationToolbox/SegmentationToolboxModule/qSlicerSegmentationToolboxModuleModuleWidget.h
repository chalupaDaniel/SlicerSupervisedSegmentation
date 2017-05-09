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

#ifndef __qSlicerSegmentationToolboxModuleModuleWidget_h
#define __qSlicerSegmentationToolboxModuleModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerSegmentationToolboxModuleModuleExport.h"

class qSlicerSegmentationToolboxModuleModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_SEGMENTATIONTOOLBOXMODULE_EXPORT qSlicerSegmentationToolboxModuleModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerSegmentationToolboxModuleModuleWidget(QWidget *parent=0);
  virtual ~qSlicerSegmentationToolboxModuleModuleWidget();

public slots:

protected:
  QScopedPointer<qSlicerSegmentationToolboxModuleModuleWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerSegmentationToolboxModuleModuleWidget);
  Q_DISABLE_COPY(qSlicerSegmentationToolboxModuleModuleWidget);
};

#endif
