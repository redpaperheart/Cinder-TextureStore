/*
 Copyright (c) 2014 Red Paper Heart Inc.

 TextureStore is based on work by Paul Houx
 Copyright (c) 2010-2012, Paul Houx - All rights reserved.

 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

#include "rph/TextureStore.h"

namespace rph {
    
    TextureStore* TextureStore::m_pInstance = NULL;
    TextureStore* TextureStore::getInstance(){
        if (!m_pInstance){ // Only allow one instance of class to be generated.
            m_pInstance = new TextureStore;
            //m_pInstance->setup();
        }
        return m_pInstance;
    }
    
    TextureStore::TextureStore(){
        // initialize buffers
        mTextureRefs.clear();
        mSurfaces.clear();
        mShouldQuit = false;
        // create and launch the thread
        mThread = std::shared_ptr<std::thread>( new std::thread( std::bind( &TextureStore::loadImagesThreadFn, this ) ) );
    }

    TextureStore::~TextureStore(){
        // stop thread
        mShouldQuit = true;
        try{
            mThread->join();
        }catch(...){}
        // clear buffers
        mSurfaces.clear();
        mTextureRefs.clear();
        mLoadingQueue.clear();
        mQueue.clear();
    }
    

    
    bool TextureStore::isLoading(const std::string &url){
        return mLoadingQueue.contains(url);
    }
    
    bool TextureStore::isLoaded(const std::string &url){
        return (mTextureRefs.find( url ) != mTextureRefs.end());
    }
    
    ci::gl::TextureRef TextureStore::load(const std::string &url, ci::gl::Texture::Format fmt, bool isGarbageCollectable, bool runGarbageCollector)
    {
        // if texture already exists, return it immediately
        if (mTextureRefs.find( url ) != mTextureRefs.end())
            return mTextureRefs[ url ];
        
        // otherwise, check if the image has loaded and create a texture for it
        ci::Surface surface;
        if( mSurfaces.try_pop(url, surface) ) {
            // done loading
            mLoadingQueue.erase(url);
            
            // perform garbage collection to make room for new textures
            if(runGarbageCollector)garbageCollect();
            
//            ci::app::console() << ci::app::getElapsedSeconds() << ": creating Texture for '" << url << "'." << std::endl;

            ci::gl::TextureRef texRef = ci::gl::Texture::create(surface, fmt);
            mTextureRefs[ url ] = texRef;
            return texRef;
        }
        
        // load texture and add to TextureList
        //ci::app::console() << "Loading Texture '" << url << "'." << std::endl;

        try
        {
            ci::ImageSourceRef img = ci::loadImage( ci::app::loadResource( url ) );
            ci::gl::TextureRef t = ci::gl::Texture::create( img, fmt );
            mTextureRefs[ url ] = t;
            if(!isGarbageCollectable){
                mTextureRefsNonGarbageCollectable[ url ] = t;
            }
            return t;
        }
        catch(...){}
        
        try
        {
            ci::ImageSourceRef img = ci::loadImage( url );
            ci::gl::TextureRef t = ci::gl::Texture::create( img, fmt );
            mTextureRefs[ url ] = t;
            if(!isGarbageCollectable){
                mTextureRefsNonGarbageCollectable[ url ] = t;
            }
            return t;
        }
        catch(...){}
        
        try
        {
            ci::ImageSourceRef img = ci::loadImage( loadUrl( ci::Url(url) ) );
            ci::gl::TextureRef t = ci::gl::Texture::create( img, fmt );
            mTextureRefs[ url ] = t;
            if(!isGarbageCollectable){
                mTextureRefsNonGarbageCollectable[ url ] = t;
            }
            return t;
        }
        catch(...){}
        
        // perform garbage collection to make room for new textures
        garbageCollect();
        
        // did not succeed
        ci::app::console() << ci::app::getElapsedSeconds() << ": error loading texture '" << url << "'!" << std::endl;

        return NULL; //ci::gl::Texture::create( ci::gl::Texture() ); // return NULL?
    }
    
    std::vector<ci::gl::TextureRef> TextureStore::loadImageDirectory(ci::fs::path dir, ci::gl::Texture::Format fmt, bool isGarbageCollectable ){
        
        //ci::app::console() << "rph::TextureStore::loadImageDirectory" << std::endl;
        
        std::vector<ci::gl::TextureRef> textureRefs;
        textureRefs.clear();
        
        if( !ci::fs::exists( dir ) ){
            //ci::app::console() << "rph::TextureStore::loadImageDirectory - WARNING - ("<< dir << ") Folder does not Exist!" << std::endl;
            dir = ci::app::Platform::get()->getResourcePath("") / dir;
            if( !ci::fs::exists(dir) ){
                ci::app::console() << "rph::TextureStore::loadImageDirectory - ERROR - ("<< dir << ") Folder does not Exist!" << std::endl;
                return textureRefs;
            }
        }
        for ( ci::fs::directory_iterator it( dir ); it != ci::fs::directory_iterator(); ++it ){
            if ( ci::fs::is_regular_file( *it ) ){
                // -- Perhaps there is a better way to ignore hidden files
                std::string fileName = it->path().filename().string();
                
                if( !( fileName.compare( ".DS_Store" ) == 0 ) ){
                    
                    ci::gl::TextureRef t = load( dir.string() +"/"+ fileName , fmt, isGarbageCollectable, false );
                    textureRefs.push_back( t );
                    
                }
            }
        }
        garbageCollect();
        return textureRefs;
    }
    
    
    ci::gl::TextureRef TextureStore::fetch(const std::string &url, ci::gl::Texture::Format fmt, bool isGarbageCollectable, bool runGarbageCollector)
    {
        // if texture already exists, return it immediately
        if (mTextureRefs.find( url ) != mTextureRefs.end())
            return mTextureRefs[ url ];

        // otherwise, check if the image has loaded and create a texture for it
        ci::Surface surface;
        if( mSurfaces.try_pop(url, surface) ) {
            // done loading
            mLoadingQueue.erase(url);

            // perform garbage collection to make room for new textures
            if(runGarbageCollector)garbageCollect();

            //ci::app::console() << ci::app::getElapsedSeconds() << ": creating Texture for '" << url << "'." << std::endl;
            
            ci::gl::TextureRef texRef = ci::gl::Texture::create(surface, fmt);
            mTextureRefs[ url ] = texRef;
            if(!isGarbageCollectable){
                mTextureRefsNonGarbageCollectable[ url ] = texRef;
            }
            return texRef;
        }
        
        // add to list of currently loading/scheduled files
        if( mLoadingQueue.push_back(url, true) ) {
            // hand over to threaded loader
            if( mQueue.push_back(url, true) ) {
                //ci::app::console() << ci::app::getElapsedSeconds() << ": queueing Texture '" << url << "' for loading." << std::endl;
            }
        }
        return NULL;
    }
    
    std::vector<ci::gl::TextureRef> TextureStore::fetchImageDirectory(ci::fs::path dir, ci::gl::Texture::Format fmt, bool isGarbageCollectable){
        
        int notYetLoadedCount = 0;
        std::vector<ci::gl::TextureRef> textureRefs;
        textureRefs.clear();
        
        if( !ci::fs::exists( dir ) ){
            //ci::app::console() << "rph::TextureStore::loadImageDirectory - WARNING - ("<< dir << ") Folder does not Exist!" << std::endl;
            dir = ci::app::Platform::get()->getResourcePath("") / dir;
            if( !ci::fs::exists(dir) ){
                ci::app::console() << "rph::TextureStore::loadImageDirectory - ERROR - ("<< dir << ") Folder does not Exist!" << std::endl;
                return textureRefs;
            }
        }
        for ( ci::fs::directory_iterator it( dir ); it != ci::fs::directory_iterator(); ++it ){
            if ( ci::fs::is_regular_file( *it ) ){
                // -- Perhaps there is a better way to ignore hidden files
                std::string fileName = it->path().filename().string();
                
                if( !( fileName.compare( ".DS_Store" ) == 0 ) ){
                    ci::gl::TextureRef t = fetch( dir.string() +"/"+ fileName , fmt, false, false );
                    if( !t ){
                        notYetLoadedCount++;
                        break;
                    }
                    textureRefs.push_back( t );
                }
            }
        }
        if( notYetLoadedCount > 0 ) {
            textureRefs.clear();
        } else {
            garbageCollect();
        }
        return textureRefs;
    }
    
    
    void TextureStore::loadImagesThreadFn()
    {
        ci::ThreadSetup threadSetup; // instantiate this if you're talking to Cinder from a secondary thread
        
        bool                succeeded;
        ci::Surface			surface;
        ci::ImageSourceRef	image;
        std::string			url;
        
        ci::app::console() << "TEXTURESTORE THREAD STARTED" << std::endl;
        // run until interrupted
        while( ( ! mShouldQuit ) ) {
            if( mSurfaces.size() > 5 ) continue;
            mQueue.wait_and_pop_front(url);
            
            // try to load image
            succeeded = false;
            
            // try to load from FILE (fastest)
            if(!succeeded) try {
                image = ci::loadImage( ci::loadFile( url ) );
                succeeded = true;
            } catch(...) {}
            
            // try to load from RESOURCES (fast)
            if(!succeeded) try {
                image = ci::loadImage( ci::app::loadResource( url ) );
                succeeded = true;
            } catch(...) {}
            
            // try to load from ASSET (fast)
            if(!succeeded) try {
                image = ci::loadImage( ci::app::loadAsset( url ) );
                succeeded = true;
            } catch(...) {}
            
            // try to load from URL (slow)
            if(!succeeded) try {
                image = ci::loadImage( ci::loadUrl( ci::Url(url) ) );
                succeeded = true;
            } catch(...) {}
            
            // do NOT continue if not succeeded (yeah, it's confusing, I know)
            if(!succeeded) continue;
            
            // create Surface from the image
            try {
                surface = ci::Surface(image);
                
// resize image if larger than 4096 px
//            Area source = surface.getBounds();
//            Area dest(0, 0, 4096, 4096);
//            Area fit = Area::proportionalFit(source, dest, false, false);
//            if(source.getSize() != fit.getSize())
//                surface = ci::ip::resizeCopy(surface, source, fit.getSize());

                // copy to main thread
                mSurfaces.push(url, surface);
            }catch(...){}
        }
        ci::app::console() << "TEXTURESTORE THREAD STOPPED" << std::endl;
    }
    
    void TextureStore::garbageCollect(){
//        int s = mTextureRefs.size();
        for(std::map<std::string, ci::gl::TextureRef>::iterator itr=mTextureRefs.begin();itr!=mTextureRefs.end();){
            if(itr->second.use_count() < 2){
                ci::app::console() << ci::app::getElapsedSeconds() << ": removing texture '" << itr->first << "' because it is no longer in use." << std::endl;
                mTextureRefs.erase(itr++);
            } else {
                ++itr;
            }
        }
//        ci::app::console() << ci::app::getElapsedSeconds() << "TextureStore::garbageCollect() removed: " << (s-mTextureRefs.size()) << std::endl;
    }
    
    void TextureStore::drawAllStoredTextures(float width, float height){
        
        int numOfColumns = ci::math<float>::floor( ci::app::getWindowWidth() / width );
        int count = 0;
        int rows = 0;
        for( std::map<std::string, ci::gl::TextureRef>::iterator iter = mTextureRefs.begin(); iter != mTextureRefs.end(); iter++){
            ci::gl::pushMatrices();
            ci::gl::translate( (count++ % numOfColumns) * width, height * rows );
            if( count % numOfColumns == 0) rows++;
            ci::gl::draw( (*iter).second, ci::Rectf(0,0,width,height));
            //ci::gl::drawString(ci::toString( (*iter).second.use_count(), vec2(0,10));
            ci::gl::drawString( ci::toString( (*iter).second.use_count() ), ci::vec2(10,10) );
            ci::gl::popMatrices();
        }
    }
    
    void TextureStore::status(){
        ci::app::console() << "-------------------------" << std::endl;
        ci::app::console() << "mTextureRefs[ "<< mTextureRefs.size() << " ]" << std::endl;
        ci::app::console() << "mTextureRefsNonGarbageCollectable[ "<< mTextureRefsNonGarbageCollectable.size() << " ]" << std::endl;
    }
} // namespace rph
