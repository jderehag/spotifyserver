#ifndef ENDPOINTSPAGE_H
#define ENDPOINTSPAGE_H

#include "EndpointManager/EndpointManagerCtrlInterface.h"
#include <QWidget>

namespace Ui {
class EndpointsPage;
}

class EndpointsPage : public QWidget, IEndpointCtrlCallbackSubscriber
{
    Q_OBJECT

public:
    explicit EndpointsPage(EndpointCtrlInterface& epMgr_, QWidget *parent = 0);
    ~EndpointsPage();

public slots:
    void updateEndpoints();
    void endpointCheckbox_clicked(bool checked);

private:
    Ui::EndpointsPage *ui;

    virtual void getAudioEndpointsResponse( const AudioEndpointInfoList& endpoints, void* userData );
    virtual void audioEndpointsUpdatedNtf();

    EndpointCtrlInterface& epMgr;
    AudioEndpointInfoList endpoints_;

};

#endif // ENDPOINTSPAGE_H
