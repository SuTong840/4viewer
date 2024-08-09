#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <vtkDICOMImageReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkImagePlaneWidget.h>
#include <vtkProperty.h>
#include <vtkImageViewer2.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    dicomReader = vtkSmartPointer<vtkDICOMImageReader>::New();
    axialViewer = vtkSmartPointer<vtkImageViewer2>::New();
    sagittalViewer = vtkSmartPointer<vtkImageViewer2>::New();
    coronalViewer = vtkSmartPointer<vtkImageViewer2>::New();

    setupViews();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadDicom()
{
    QString dicomPath = "E:/Data/Dicom/volume-325"; // Update this path as needed
    dicomReader->SetDirectoryName(dicomPath.toStdString().c_str());
    dicomReader->Update();
}

void MainWindow::setupViews()
{
    loadDicom(); // Load the DICOM files first

    vtkImageData* imageData = dicomReader->GetOutput();

    // Set up the Axial view
    axialViewer->SetInputData(imageData);
    axialViewer->SetSliceOrientationToXY();
    axialViewer->SetRenderWindow(ui->win_Axial->renderWindow());
    axialViewer->Render();

    // Set up the Sagittal view
    sagittalViewer->SetInputData(imageData);
    sagittalViewer->SetSliceOrientationToYZ();
    sagittalViewer->SetRenderWindow(ui->win_Sagittal->renderWindow());
    sagittalViewer->Render();

    // Set up the Coronal view
    coronalViewer->SetInputData(imageData);
    coronalViewer->SetSliceOrientationToXZ();
    coronalViewer->SetRenderWindow(ui->win_Coronal->renderWindow());
    coronalViewer->Render();

    // Set up the 3D view with orthogonal slices
//    ui->win_3D->renderWindow()->AddRenderer(renderer3D);
//    ui->win_3D->renderWindow()->SetInteractor(interactor3D);

//    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
//    interactor3D->SetInteractorStyle(style);

//    // Configure the plane widgets
//    planeWidgetX->SetInteractor(interactor3D);
//    planeWidgetX->SetInputData(imageData);
//    planeWidgetX->SetPlaneOrientationToXAxes();
//    planeWidgetX->SetSliceIndex(imageData->GetDimensions()[0] / 2);
//    planeWidgetX->GetPlaneProperty()->SetColor(1, 0, 0); // Red
//    planeWidgetX->On();

//    planeWidgetY->SetInteractor(interactor3D);
//    planeWidgetY->SetInputData(imageData);
//    planeWidgetY->SetPlaneOrientationToYAxes();
//    planeWidgetY->SetSliceIndex(imageData->GetDimensions()[1] / 2);
//    planeWidgetY->GetPlaneProperty()->SetColor(0, 1, 0); // Green
//    planeWidgetY->On();

//    planeWidgetZ->SetInteractor(interactor3D);
//    planeWidgetZ->SetInputData(imageData);
//    planeWidgetZ->SetPlaneOrientationToZAxes();
//    planeWidgetZ->SetSliceIndex(imageData->GetDimensions()[2] / 2);
//    planeWidgetZ->GetPlaneProperty()->SetColor(0, 0, 1); // Blue
//    planeWidgetZ->On();

//    ui->win_3D->renderWindow()->Render();
//    interactor3D->Start();
}
