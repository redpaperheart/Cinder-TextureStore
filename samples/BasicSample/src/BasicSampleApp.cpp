#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "rph/TextureStore.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicSampleApp : public AppNative {
  public:
	void setup() override;
	void update() override;
	void draw() override;
    
    gl::TextureRef mLoadedTexRef;
    gl::TextureRef mFetchedTexRef;
};

void BasicSampleApp::setup(){
    mLoadedTexRef = rph::loadTexture("artwork/rph_isometric_blue.jpg");
}


void BasicSampleApp::update(){
    while( !mFetchedTexRef ){
        mFetchedTexRef = rph::loadTexture("artwork/rph_isometric_yellow.jpg");
    }
}

void BasicSampleApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    rph::TextureStore::getInstance()->drawAllStoredTextures();
}

CINDER_APP_NATIVE( BasicSampleApp, RendererGl )
