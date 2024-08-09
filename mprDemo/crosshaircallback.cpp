#include "CrosshairCallback.h"
#include <vtkRenderWindowInteractor.h>
#include <vtkPicker.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkCommand.h>

void CrosshairCallback::Execute(vtkObject *caller, unsigned long, void*)
{
    vtkRenderWindowInteractor *interactor = static_cast<vtkRenderWindowInteractor*>(caller);
    int* clickPos = interactor->GetEventPosition();

    // 获取当前渲染窗口中的拾取器并执行拾取操作
    interactor->GetPicker()->Pick(clickPos[0], clickPos[1], 0, imageViewer[0]->GetRenderer());
    double* picked = interactor->GetPicker()->GetPickPosition();

    // 计算切片索引
    int sliceIndex[3];
    for (int i = 0; i < 3; ++i)
    {
        sliceIndex[i] = static_cast<int>(picked[i]);
    }

    // 更新每个视图的切片索引
    imageViewer[0]->SetSlice(sliceIndex[2]);
    imageViewer[1]->SetSlice(sliceIndex[0]);
    imageViewer[2]->SetSlice(sliceIndex[1]);

    for (int i = 0; i < 3; ++i)
    {
        imageViewer[i]->Render();
    }
}

void CrosshairCallback::SetImageViewer(vtkImageViewer2* viewer[3])
{
    for (int i = 0; i < 3; ++i)
    {
        imageViewer[i] = viewer[i];
    }
}
