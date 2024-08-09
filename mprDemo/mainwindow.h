#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageViewer2.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>
#include <vtkImagePlaneWidget.h>
#include <QVTKOpenGLNativeWidget.h>
namespace Ui {
class MainWindow;
}

class MyInteractorStyle : public vtkInteractorStyleImage
{
public:
    static MyInteractorStyle* New();
    vtkTypeMacro(MyInteractorStyle, vtkInteractorStyleImage);

    virtual void OnLeftButtonDown() override;

private:
    void PrintClickPosition();
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_btn_point_clicked();

private:
    Ui::MainWindow *ui;

    vtkSmartPointer<vtkDICOMImageReader> dicomReader;
    vtkSmartPointer<vtkImageViewer2> axialViewer;
    vtkSmartPointer<vtkImageViewer2> sagittalViewer;
    vtkSmartPointer<vtkImageViewer2> coronalViewer;
    vtkSmartPointer<vtkRenderer> renderer3D;
    vtkSmartPointer<vtkImagePlaneWidget> planeWidgetX;
    vtkSmartPointer<vtkImagePlaneWidget> planeWidgetY;
    vtkSmartPointer<vtkImagePlaneWidget> planeWidgetZ;
    void setupPlaneWidget(vtkSmartPointer<vtkImagePlaneWidget>& planeWidget, QVTKOpenGLNativeWidget* view, char activationKey, std::vector<double> color, vtkImageData* imageData, int orientation);
    void loadDicom();
    void setupViews();
    void  setup3DView(vtkImageData* imageData);
};

#endif // MAINWINDOW_H
