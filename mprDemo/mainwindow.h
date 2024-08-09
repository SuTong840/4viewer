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
#include <vtkCamera.h>
#include <vtkProperty.h>

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

private slots:


    void on_btn_point_clicked();

private:
    Ui::MainWindow *ui;
    vtkSmartPointer<vtkDICOMImageReader> dicomReader;
    vtkSmartPointer<vtkImageViewer2> axialViewer;
    vtkSmartPointer<vtkImageViewer2> sagittalViewer;
    vtkSmartPointer<vtkImageViewer2> coronalViewer;

    vtkSmartPointer<vtkImagePlaneWidget> planeWidgetX;
    vtkSmartPointer<vtkImagePlaneWidget> planeWidgetY;
    vtkSmartPointer<vtkImagePlaneWidget> planeWidgetZ;

};

#endif // MAINWINDOW_H
