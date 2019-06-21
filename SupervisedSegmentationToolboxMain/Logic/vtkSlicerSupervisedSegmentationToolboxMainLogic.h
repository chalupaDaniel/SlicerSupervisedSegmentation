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

// .NAME vtkSlicerSupervisedSegmentationToolboxMainLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerSupervisedSegmentationToolboxMainLogic_h
#define __vtkSlicerSupervisedSegmentationToolboxMainLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerSupervisedSegmentationToolboxMainModuleLogicExport.h"


/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_SUPERVISEDSEGMENTATIONTOOLBOXMAIN_MODULE_LOGIC_EXPORT vtkSlicerSupervisedSegmentationToolboxMainLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerSupervisedSegmentationToolboxMainLogic *New();
  vtkTypeMacro(vtkSlicerSupervisedSegmentationToolboxMainLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkSlicerSupervisedSegmentationToolboxMainLogic();
  virtual ~vtkSlicerSupervisedSegmentationToolboxMainLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
private:

  vtkSlicerSupervisedSegmentationToolboxMainLogic(const vtkSlicerSupervisedSegmentationToolboxMainLogic&); // Not implemented
  void operator=(const vtkSlicerSupervisedSegmentationToolboxMainLogic&); // Not implemented
};

#endif
