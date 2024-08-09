#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkImagePlaneWidget.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkPointPicker.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkSphereSource.h>
#include <vtkProperty.h>
#include <vtkRendererCollection.h>
#include <QDebug>

// 回调函数类，用于处理鼠标点击事件
class PointPickerCallback : public vtkCommand
{
public:
    static PointPickerCallback* New()
    {
        return new PointPickerCallback;
    }

    void SetMainWindow(MainWindow* window)
    {
        this->mainWindow = window;
    }

    void Execute(vtkObject* caller, unsigned long eventId, void* callData) override
    {
        vtkRenderWindowInteractor* interactor = static_cast<vtkRenderWindowInteractor*>(caller);
        vtkRenderer* renderer = interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();

        vtkSmartPointer<vtkPointPicker> picker = vtkSmartPointer<vtkPointPicker>::New();
        picker->Pick(interactor->GetEventPosition()[0], interactor->GetEventPosition()[1], 0, renderer);

        double pickedPosition[3];
        picker->GetPickPosition(pickedPosition);

        if (mainWindow)
        {
            mainWindow->addPointToSagittalView(pickedPosition[0], pickedPosition[1], pickedPosition[2]);
            mainWindow->addPointToCoronalView(pickedPosition[0], pickedPosition[1], pickedPosition[2]);
            mainWindow->addPointTo3DView(pickedPosition[0], pickedPosition[1], pickedPosition[2]);
        }
    }

private:
    MainWindow* mainWindow = nullptr;
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    dicomReader = vtkSmartPointer<vtkDICOMImageReader>::New();
    axialViewer = vtkSmartPointer<vtkImageViewer2>::New();
    sagittalViewer = vtkSmartPointer<vtkImageViewer2>::New();
    coronalViewer = vtkSmartPointer<vtkImageViewer2>::New();
    renderer3D = vtkSmartPointer<vtkRenderer>::New();

    setupViews();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadDicom()
{
    QString dicomPath = "E:/Data/Dicom/volume-325"; // 根据需要更新此路径
    dicomReader->SetDirectoryName(dicomPath.toStdString().c_str());
    dicomReader->Update();
}

void MainWindow::setupViews()
{
    loadDicom();

    vtkImageData* imageData = dicomReader->GetOutput();
    if (!imageData) {
        qDebug() << "Failed to load image data";
        return;
    }

    int* dimensions = imageData->GetDimensions();
    int numSlices = dimensions[2];
    int midSlice = numSlices / 2;

    // 设置轴位（Axial）视图
    axialViewer->SetInputData(imageData);
    axialViewer->SetSliceOrientationToXY();
    axialViewer->SetRenderWindow(ui->win_Axial->renderWindow());
    axialViewer->SetSlice(midSlice);
    axialViewer->GetRenderer()->ResetCamera();
    axialViewer->Render();

    vtkSmartPointer<vtkInteractorStyleImage> styleAxial = vtkSmartPointer<vtkInteractorStyleImage>::New();
    ui->win_Axial->renderWindow()->GetInteractor()->SetInteractorStyle(styleAxial);

    // 设置矢状位（Sagittal）视图
    sagittalViewer->SetInputData(imageData);
    sagittalViewer->SetSliceOrientationToYZ();
    sagittalViewer->SetRenderWindow(ui->win_Sagittal->renderWindow());
    sagittalViewer->SetSlice(midSlice);
    sagittalViewer->GetRenderer()->ResetCamera();
    sagittalViewer->Render();
    sagittalViewer->GetRenderer()->GetActiveCamera()->SetViewUp(0, 0, -1);
    vtkSmartPointer<vtkInteractorStyleImage> styleSagittal = vtkSmartPointer<vtkInteractorStyleImage>::New();
    ui->win_Sagittal->renderWindow()->GetInteractor()->SetInteractorStyle(styleSagittal);

    // 设置冠状位（Coronal）视图

    // 设置冠状位（Coronal）视图
    coronalViewer->SetInputData(imageData);
    coronalViewer->SetSliceOrientationToXZ();
    coronalViewer->SetRenderWindow(ui->win_Coronal->renderWindow());
    coronalViewer->SetSlice(midSlice);
    coronalViewer->GetRenderer()->ResetCamera();
    coronalViewer->Render();

    // 获取摄像机对象
    vtkCamera* camera = coronalViewer->GetRenderer()->GetActiveCamera();
    camera->SetViewUp(0, 1, 0);
    camera->Azimuth(180);
    camera->Roll(45);
    coronalViewer->Render();

    vtkSmartPointer<vtkInteractorStyleImage> styleCoronal = vtkSmartPointer<vtkInteractorStyleImage>::New();
    ui->win_Coronal->renderWindow()->GetInteractor()->SetInteractorStyle(styleCoronal);

    // 初始化渲染窗口
    ui->win_Axial->renderWindow()->GetInteractor()->Initialize();
    ui->win_Sagittal->renderWindow()->GetInteractor()->Initialize();
    ui->win_Coronal->renderWindow()->GetInteractor()->Initialize();

    // 设置3D视图
    setup3DView(imageData);
}

void MainWindow::setup3DView(vtkImageData* imageData)
{
    ui->win_3D->renderWindow()->AddRenderer(renderer3D);

    setupPlaneWidget(planeWidgetX, ui->win_3D, 'x', {1.0, 0.0, 0.0}, imageData, 0); // Red for X
    setupPlaneWidget(planeWidgetY, ui->win_3D, 'y', {0.0, 1.0, 0.0}, imageData, 1); // Green for Y
    setupPlaneWidget(planeWidgetZ, ui->win_3D, 'z', {0.0, 0.0, 1.0}, imageData, 2); // Blue for Z

    renderer3D->ResetCamera();
    ui->win_3D->renderWindow()->Render();
}

void MainWindow::setupPlaneWidget(vtkSmartPointer<vtkImagePlaneWidget>& planeWidget, QVTKOpenGLNativeWidget* view, char activationKey, std::vector<double> color, vtkImageData* imageData, int orientation)
{
    planeWidget = vtkSmartPointer<vtkImagePlaneWidget>::New();
    planeWidget->SetInteractor(view->interactor());

    if (color.size() < 3) {
        qDebug() << "Color vector must have at least 3 elements.";
        return;
    }

    planeWidget->GetPlaneProperty()->SetColor(color.data());
    planeWidget->SetTexturePlaneProperty(vtkSmartPointer<vtkProperty>::New());
    planeWidget->TextureInterpolateOn();
    planeWidget->SetResliceInterpolateToLinear();
    planeWidget->SetInputData(imageData);
    planeWidget->SetPlaneOrientation(orientation);
    planeWidget->SetSliceIndex(imageData->GetDimensions()[orientation] / 2);
    planeWidget->On();

    qDebug() << "Plane widget setup completed for orientation:" << orientation;
}

void MainWindow::on_btn_point_clicked()
{
    vtkSmartPointer<PointPickerCallback> pointCallback = vtkSmartPointer<PointPickerCallback>::New();
    pointCallback->SetMainWindow(this);

    ui->win_Axial->renderWindow()->GetInteractor()->AddObserver(vtkCommand::LeftButtonPressEvent, pointCallback);
    ui->win_Sagittal->renderWindow()->GetInteractor()->AddObserver(vtkCommand::LeftButtonPressEvent, pointCallback);
    ui->win_Coronal->renderWindow()->GetInteractor()->AddObserver(vtkCommand::LeftButtonPressEvent, pointCallback);
}
void MainWindow::addPointToSagittalView(double x, double y, double z)
{
    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetCenter(x, y, z);
    sphereSource->SetRadius(3.0);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(sphereSource->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(1.0, 0.0, 0.0);

    sagittalViewer->GetRenderer()->AddActor(actor);
    sagittalViewer->Render();
}

void MainWindow::addPointToCoronalView(double x, double y, double z)
{
    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetCenter(x, y, z);
    sphereSource->SetRadius(3.0);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(sphereSource->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0.0, 1.0, 0.0);  // Green for Coronal

    coronalViewer->GetRenderer()->AddActor(actor);
    coronalViewer->Render();
}

void MainWindow::addPointTo3DView(double x, double y, double z)
{
    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetCenter(x, y, z);
    sphereSource->SetRadius(10);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(sphereSource->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0.0, 0.0, 1.0);  // Blue for 3D

    renderer3D->AddActor(actor);
    ui->win_3D->renderWindow()->Render();
}
