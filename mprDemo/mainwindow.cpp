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
        // 输出调试信息
                 qDebug() << "Picked position: (X:" << pickedPosition[0]
                          << ", Y:" << pickedPosition[1]
                          << ", Z:" << pickedPosition[2] << ")";
        if (mainWindow)
        {
            // 添加点到三视图和3D视图
            mainWindow->addPointToAxialView(pickedPosition[0], pickedPosition[1], pickedPosition[2]);
            mainWindow->addPointToSagittalView(pickedPosition[0], pickedPosition[1], pickedPosition[2]);
            mainWindow->addPointToCoronalView(pickedPosition[0], pickedPosition[1], pickedPosition[2]);
            mainWindow->addPointTo3DView(pickedPosition[0], pickedPosition[1], pickedPosition[2]);

            // 获取当前图像数据的维度和体素尺寸
              vtkImageData* imageData = mainWindow->dicomReader->GetOutput();
              int* dimensions = imageData->GetDimensions();
              double* spacing = imageData->GetSpacing(); // 获取体素尺寸

              // 计算各个视图的切片索引
              int axialSlice = std::round(pickedPosition[2] / spacing[2]);    // Z轴切片索引
              int sagittalSlice = std::round(pickedPosition[0] / spacing[0]); // X轴切片索引
              int coronalSlice = std::round(pickedPosition[1] / spacing[1]);  // Y轴切片索引

            // 限制切片索引在合法范围内
            axialSlice = std::clamp(axialSlice, 0, dimensions[2] - 1);
            sagittalSlice = std::clamp(sagittalSlice, 0, dimensions[0] - 1);
            coronalSlice = std::clamp(coronalSlice, 0, dimensions[1] - 1);

            // 使用 qDebug 输出每个视图的当前切片索引
            qDebug() << "Axial View Slice Index:" << axialSlice;
            qDebug() << "Sagittal View Slice Index:" << sagittalSlice;
            qDebug() << "Coronal View Slice Index:" << coronalSlice;
            // 同步更新滑动条的数值
            mainWindow->ui->slider_Axial->setValue(axialSlice);
            mainWindow->ui->slider_Sagittal->setValue(sagittalSlice);
            mainWindow->ui->slider_Coronal->setValue(coronalSlice);
            // 更新切片位置
            mainWindow->axialViewer->SetSlice(axialSlice);
            mainWindow->sagittalViewer->SetSlice(sagittalSlice);
            mainWindow->coronalViewer->SetSlice(coronalSlice);

            // 更新对应的3D视图中的平面切片
            mainWindow->planeWidgetZ->SetSliceIndex(axialSlice);
            mainWindow->planeWidgetX->SetSliceIndex(sagittalSlice);
            mainWindow->planeWidgetY->SetSliceIndex(coronalSlice);

            // 重新渲染视图
            mainWindow->axialViewer->Render();
            mainWindow->sagittalViewer->Render();
            mainWindow->coronalViewer->Render();
            mainWindow->ui->win_3D->renderWindow()->Render();
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
    // 连接 checkbox 和 slider 的变化事件

    connect(ui->slider_Axial, &QSlider::valueChanged, this, &MainWindow::on_slider_Axial_valueChanged);
    connect(ui->slider_Sagittal, &QSlider::valueChanged, this, &MainWindow::on_slider_Sagittal_valueChanged);
    connect(ui->slider_Coronal, &QSlider::valueChanged, this, &MainWindow::on_slider_Coronal_valueChanged);

    //三视图滑动条和spinbox
    // Connect slider and spinbox signals and slots
    connect(ui->slider_Axial, &QSlider::valueChanged, ui->spinBox_Axial, &QSpinBox::setValue);
    connect(ui->spinBox_Axial, QOverload<int>::of(&QSpinBox::valueChanged), ui->slider_Axial, &QSlider::setValue);

    connect(ui->slider_Sagittal, &QSlider::valueChanged, ui->spinBox_Sagittal, &QSpinBox::setValue);
    connect(ui->spinBox_Sagittal, QOverload<int>::of(&QSpinBox::valueChanged), ui->slider_Sagittal, &QSlider::setValue);

    connect(ui->slider_Coronal, &QSlider::valueChanged, ui->spinBox_Coronal, &QSpinBox::setValue);
    connect(ui->spinBox_Coronal, QOverload<int>::of(&QSpinBox::valueChanged), ui->slider_Coronal, &QSlider::setValue);
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
    // 获取体素间距
    double spacing[3];
    imageData->GetSpacing(spacing);
    // 输出体素间距以供调试
    qDebug() << "Voxel Spacing体素距离: (X:" << spacing[0] << ", Y:" << spacing[1] << ", Z:" << spacing[2] << ")";

    int* dimensions = imageData->GetDimensions();
    int numSlices = dimensions[2];
    int midSlice = numSlices / 2;
//    int numSlicesAxial = dimensions[2];
//    int numSlicesSagittal = dimensions[0];
//    int numSlicesCoronal = dimensions[1];

//    int midSliceAxial = numSlicesAxial / 2;
//    int midSliceSagittal = numSlicesSagittal / 2;
//    int midSliceCoronal = numSlicesCoronal / 2;
//    // 设置滑动条的范围
    ui->slider_Axial->setRange(0, 448);
    ui->slider_Axial->setValue(0);

    ui->slider_Sagittal->setRange(0, 512);
    ui->slider_Sagittal->setValue(0);

    ui->slider_Coronal->setRange(0, 512);
    ui->slider_Coronal->setValue(0);
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
    planeWidget->SetSliceIndex(0);
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
void MainWindow::addPointToAxialView(double x, double y, double z)
{
    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetCenter(x, y, z);
    sphereSource->SetRadius(10);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(sphereSource->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0.0, 0.0, 1.0);  // Blue for Axial

    axialViewer->GetRenderer()->AddActor(actor);
    axialViewer->Render();
}

void MainWindow::addPointToSagittalView(double x, double y, double z)
{
    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetCenter(x, y, z);
    sphereSource->SetRadius(10);

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
    sphereSource->SetRadius(10);

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

void MainWindow::on_slider_Axial_valueChanged(int value)
{
    // 更新Axial视图的切片
    axialViewer->SetSlice(value);
    axialViewer->Render();

    // 同步更新3D视图中的X平面切片
    planeWidgetZ->SetSliceIndex(value);
    ui->win_3D->renderWindow()->Render();

}

void MainWindow::on_slider_Sagittal_valueChanged(int value)
{
    // 更新Sagittal视图的切片
    sagittalViewer->SetSlice(value);
    sagittalViewer->Render();

    // 同步更新3D视图中的Y平面切片
    planeWidgetX->SetSliceIndex(value);
    ui->win_3D->renderWindow()->Render();
}

void MainWindow::on_slider_Coronal_valueChanged(int value)
{
    // 更新Coronal视图的切片
    coronalViewer->SetSlice(value);
    coronalViewer->Render();

    // 同步更新3D视图中的Z平面切片
    planeWidgetY->SetSliceIndex(value);
    ui->win_3D->renderWindow()->Render();
}

