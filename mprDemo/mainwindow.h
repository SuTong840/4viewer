#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageViewer2.h>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkImagePlaneWidget.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadDicom();
    void setupViews();

private:
    Ui::MainWindow *ui;
    vtkSmartPointer<vtkDICOMImageReader> dicomReader;
    vtkSmartPointer<vtkImageViewer2> axialViewer;
    vtkSmartPointer<vtkImageViewer2> sagittalViewer;
    vtkSmartPointer<vtkImageViewer2> coronalViewer;

    vtkSmartPointer<vtkImagePlaneWidget> planeWidgetX;
    vtkSmartPointer<vtkImagePlaneWidget> planeWidgetY;
    vtkSmartPointer<vtkImagePlaneWidget> planeWidgetZ;

    vtkSmartPointer<vtkRenderer> renderer3D;
    vtkSmartPointer<vtkRenderWindowInteractor> interactor3D;
};

#endif // MAINWINDOW_H
