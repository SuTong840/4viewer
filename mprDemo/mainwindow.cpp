#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <vtkDICOMImageReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkImagePlaneWidget.h>
#include <vtkImageViewer2.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkPointPicker.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkSphereSource.h>
#include <vtkProperty.h>
#include <vtkRendererCollection.h>
// 回调函数类，用于处理鼠标点击事件
class PointPickerCallback : public vtkCommand
{
public:
    static PointPickerCallback* New()
    {
        return new PointPickerCallback;
    }

    void Execute(vtkObject* caller, unsigned long eventId, void* callData) override
    {
        vtkRenderWindowInteractor* interactor = static_cast<vtkRenderWindowInteractor*>(caller);
        vtkRenderer* renderer = interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();

        // 使用PointPicker来拾取点
        vtkSmartPointer<vtkPointPicker> picker = vtkSmartPointer<vtkPointPicker>::New();
        picker->Pick(interactor->GetEventPosition()[0], interactor->GetEventPosition()[1], 0, renderer);

        double pickedPosition[3];
        picker->GetPickPosition(pickedPosition);

        // 创建一个球体来表示点
        vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
        sphereSource->SetCenter(pickedPosition);
        sphereSource->SetRadius(10); // 点的大小

        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(sphereSource->GetOutputPort());

        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(1.0, 0.0, 0.0); // 设置点的颜色为红色

        renderer->AddActor(actor);
        renderer->GetRenderWindow()->Render();
    }
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
    loadDicom(); // 首先加载DICOM文件

    vtkImageData* imageData = dicomReader->GetOutput();

    // 设置轴位（Axial）视图
    axialViewer->SetInputData(imageData);
    axialViewer->SetSliceOrientationToXY();
    axialViewer->SetRenderWindow(ui->win_Axial->renderWindow());
    axialViewer->SetSlice(0); // 设置初始切片
    axialViewer->GetRenderer()->ResetCamera();
    axialViewer->GetRenderer()->GetActiveCamera()->SetViewUp(0, 1, 0); // 使视图方向正确
    axialViewer->Render();

    // 设置矢状位（Sagittal）视图
    sagittalViewer->SetInputData(imageData);
    sagittalViewer->SetSliceOrientationToYZ();
    sagittalViewer->SetRenderWindow(ui->win_Sagittal->renderWindow());
    sagittalViewer->SetSlice(0); // 设置初始切片
    sagittalViewer->GetRenderer()->ResetCamera();
    sagittalViewer->GetRenderer()->GetActiveCamera()->SetViewUp(0, 1, 0); // 使视图方向正确
    sagittalViewer->Render();

    // 设置冠状位（Coronal）视图
    coronalViewer->SetInputData(imageData);
    coronalViewer->SetSliceOrientationToXZ();
    coronalViewer->SetRenderWindow(ui->win_Coronal->renderWindow());
    coronalViewer->SetSlice(0); // 设置初始切片
    coronalViewer->GetRenderer()->ResetCamera();
    coronalViewer->GetRenderer()->GetActiveCamera()->SetViewUp(0, 1, 0); // 使视图方向正确
    coronalViewer->Render();

    // 启用交互器
    ui->win_Axial->renderWindow()->GetInteractor()->Initialize();
    ui->win_Sagittal->renderWindow()->GetInteractor()->Initialize();
    ui->win_Coronal->renderWindow()->GetInteractor()->Initialize();
}

void MainWindow::on_btn_point_clicked()
{
    // 为三个视图窗口添加鼠标点击事件处理
    vtkSmartPointer<PointPickerCallback> pointCallback = vtkSmartPointer<PointPickerCallback>::New();

    // Axial 视图
    ui->win_Axial->renderWindow()->GetInteractor()->AddObserver(vtkCommand::LeftButtonPressEvent, pointCallback);

    // Sagittal 视图
    ui->win_Sagittal->renderWindow()->GetInteractor()->AddObserver(vtkCommand::LeftButtonPressEvent, pointCallback);

    // Coronal 视图
    ui->win_Coronal->renderWindow()->GetInteractor()->AddObserver(vtkCommand::LeftButtonPressEvent, pointCallback);
}
