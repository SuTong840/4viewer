//#include "crosshaircallback.h"
//#include <vtkRenderer.h>
//#include <vtkRenderWindow.h>
//#include <vtkRenderWindowInteractor.h>
//#include <vtkPointPicker.h>
//#include <vtkActor.h>
//#include <vtkPolyDataMapper.h>
//#include <vtkSphereSource.h>
//#include <vtkProperty.h>
//#include <vtkSmartPointer.h>

//vtkStandardNewMacro(CrosshairCallback);

//void CrosshairCallback::Execute(vtkObject* caller, unsigned long eventId, void* callData)
//{
//    vtkRenderWindowInteractor* interactor = static_cast<vtkRenderWindowInteractor*>(caller);

//    if (eventId == vtkCommand::MouseMoveEvent)
//    {
//        int* pos = interactor->GetEventPosition();

//        // Update the crosshair position
//        // For now, this is just a simple point representation
//        double displayCoords[3] = {static_cast<double>(pos[0]), static_cast<double>(pos[1]), 0.0};
//        double worldCoords[4];

//        renderer->SetDisplayPoint(displayCoords);
//        renderer->DisplayToWorld();
//        renderer->GetWorldPoint(worldCoords);

//        // Implement code to update crosshair appearance on screen if needed
//    }
//    else if (eventId == vtkCommand::LeftButtonPressEvent)
//    {
//        vtkSmartPointer<vtkPointPicker> picker = vtkSmartPointer<vtkPointPicker>::New();
//        picker->Pick(interactor->GetEventPosition()[0], interactor->GetEventPosition()[1], 0, renderer);

//        double pickedPosition[3];
//        picker->GetPickPosition(pickedPosition);

//        // Create a sphere to represent the picked point
//        vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
//        sphereSource->SetCenter(pickedPosition);
//        sphereSource->SetRadius(5.0);

//        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//        mapper->SetInputConnection(sphereSource->GetOutputPort());

//        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
//        actor->SetMapper(mapper);
//        actor->GetProperty()->SetColor(1.0, 0.0, 0.0); // Set the color of the point to red

//        renderer->AddActor(actor);
//        renderer->GetRenderWindow()->Render();
//    }
//}

//void CrosshairCallback::SetRenderer(vtkSmartPointer<vtkRenderer> ren)
//{
//    renderer = ren;
//}

//void CrosshairCallback::SetRenderWindow(vtkSmartPointer<vtkRenderWindow> rw)
//{
//    renderWindow = rw;
//}
