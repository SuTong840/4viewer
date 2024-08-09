#ifndef CROSSHAIRCALLBACK_H
#define CROSSHAIRCALLBACK_H

#include <vtkCommand.h>
#include <vtkSmartPointer.h>
#include <vtkImageViewer2.h>

class CrosshairCallback : public vtkCommand
{
public:
    static CrosshairCallback *New()
    {
        return new CrosshairCallback;
    }

    void Execute(vtkObject *caller, unsigned long eventId, void* callData) override;
    void SetImageViewer(vtkImageViewer2* viewer[3]);

private:
    vtkImageViewer2* imageViewer[3];
};

#endif // CROSSHAIRCALLBACK_H
