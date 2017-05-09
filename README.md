# Supervised segmentation toolbox using machine learning for 3D Slicer
This 3D Slicer extension enables the user to quickly train a supervised classifier and use it to classify images.

## Included classifiers:
 - C Support Vector Machine from shark[1] and dlib[2]
 - Nu Support Vector Machine from dlib[2]
 - Random Forest from shark[1]

## Included preprocessing methods:
 - Gaussian filter from VTK[3]
 - Median filter from VTK[3]
 - Sobel operator from VTK[3]
  
## 3D Slicer users:
If you want to use this extension in 3D Slicer, check the [lib folder](SegmentationToolbox/lib/) and below to see supported 3D Slicer versions and operating systems combinations. After downloading the selected libraries, go to Edit->Application Settings->Modules in 3D Slicer and add the path to the module into 'Additional module paths:'

Lib folder includes:
 - Windows 10 64-bit, 3D Slicer 4.6.0

## Developers:
TODO
  
## References:
\[1\] IGEL, CH., HEIDRICH-MEISNER, V., GLASSMACHERS, T., [Shark](http://image.diku.dk/shark/index.html), 2008, Journal of Machine Learning Research, 993-996, 9

\[2\] KING, E. D., [Dlib-ml](http://dlib.net/): A Machine Learning Toolkit, 2009, Journal of Machine Learning Research, 1755-1758, 10

\[3\] SCHROEDER, W., MARTIN, K.; LORENSEN, B., [The Visualization Toolkit](http://www.vtk.org/) (4th ed.), Kitware, ISBN 978-1-930934-19-1
