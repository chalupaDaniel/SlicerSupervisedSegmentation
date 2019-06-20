# Supervised segmentation toolbox using machine learning for 3D Slicer
This 3D Slicer extension enables the user to quickly train a supervised classifier and use it to classify images. See the [Wiki page]( https://github.com/chalupaDaniel/SlicerSupervisedSegmentation/wiki/Supervised-Segmentation-Toolbox-for-3D-Slicer) for more info.

If you use this extension, please reference https://doi.org/10.3390/sym10110627

## Included classifiers:
 - C Support Vector Machine from shark[1] and dlib[2]
 - Nu Support Vector Machine from dlib[2]
 - Random Forest from shark[1]

## Included preprocessing methods:
 - Gaussian filter from VTK[3]
 - Median filter from VTK[3]
 - Sobel operator from VTK[3]

## References:
\[1\] IGEL, CH., HEIDRICH-MEISNER, V., GLASSMACHERS, T., [Shark](http://image.diku.dk/shark/index.html), 2008, Journal of Machine Learning Research, 993-996, 9

\[2\] KING, E. D., [Dlib-ml](http://dlib.net/): A Machine Learning Toolkit, 2009, Journal of Machine Learning Research, 1755-1758, 10

\[3\] SCHROEDER, W., MARTIN, K.; LORENSEN, B., [The Visualization Toolkit](http://www.vtk.org/) (4th ed.), Kitware, ISBN 978-1-930934-19-1
