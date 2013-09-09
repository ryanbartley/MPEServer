#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "MPEServer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class MPEApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
    mpe::MPEServerRef mMPE;
};

void MPEApp::setup()
{
    mMPE = mpe::MPEServer::create( 9001, 3, 2 );
}

void MPEApp::mouseDown( MouseEvent event )
{
}

void MPEApp::update()
{
}

void MPEApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( MPEApp, RendererGl )
