#include "EndpointsPage.h"
#include "ui_EndpointsPage.h"
#include <QCheckBox>

EndpointsPage::EndpointsPage( EndpointCtrlInterface& epMgr_, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EndpointsPage),
    epMgr(epMgr_)
{
    ui->setupUi(this);

    ui->endpointsScrollAreaLayout->setAlignment( Qt::AlignTop );

    epMgr.registerForCallbacks( *this );

    epMgr.getAudioEndpoints( this, NULL );

}

EndpointsPage::~EndpointsPage()
{
    epMgr.unRegisterForCallbacks( *this );
    delete ui;
}

void EndpointsPage::getAudioEndpointsResponse( const AudioEndpointInfoList& endpoints, void* userData )
{
    endpoints_ = endpoints;
    QMetaObject::invokeMethod( this, "updateEndpoints", Qt::QueuedConnection );
}

void EndpointsPage::audioEndpointsUpdatedNtf()
{
    if ( this->isVisible() )
    {
        epMgr.getAudioEndpoints( this, NULL );
    }
}


void EndpointsPage::updateEndpoints()
{
    QLayoutItem *child;
    while ((child = ui->endpointsScrollAreaLayout->takeAt(0)) != 0)
    {
        delete child->widget();
        delete child;
    }

    AudioEndpointInfoList::const_iterator it = endpoints_.begin();
    for ( ; it != endpoints_.end(); it++ )
    {
        QCheckBox* ep = new QCheckBox();
        ep->setText( QString::fromStdString( (*it).id ) );
        ep->setChecked( (*it).active );
        connect( ep, SIGNAL(clicked(bool)), this, SLOT(endpointCheckbox_clicked(bool)));
        ui->endpointsScrollAreaLayout->addWidget( ep );
    }
}

void EndpointsPage::endpointCheckbox_clicked(bool checked)
{
    QCheckBox* cb = (QCheckBox*) QObject::sender();
    if ( checked )
        epMgr.addAudioEndpoint( cb->text().toStdString(), this, NULL );
    else
        epMgr.removeAudioEndpoint( cb->text().toStdString(), this, NULL );
}
