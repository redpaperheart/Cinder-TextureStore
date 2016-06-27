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

#pragma once

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Thread.h"
#include "cinder/Utilities.h"
#include "cinder/ConcurrentCircularBuffer.h"

#include "rph/ConcurrentDeque.h"
#include "rph/ConcurrentMap.h"

namespace rph {

    class TextureStore {
      private:
        // singleton implementation
        TextureStore();
        ~TextureStore();
        TextureStore(TextureStore const&){};
        TextureStore& operator=(TextureStore const&){return *m_pInstance;};
        static TextureStore* m_pInstance;
        
      public:
        static TextureStore* getInstance();
        
        
        std::vector<ci::gl::TextureRef> loadImageDirectory(ci::fs::path path, ci::gl::Texture::Format fmt=ci::gl::Texture::Format(), bool isGarbageCollectable = true );
        std::vector<ci::gl::TextureRef> fetchImageDirectory(ci::fs::path path, ci::gl::Texture::Format fmt=ci::gl::Texture::Format(), bool isGarbageCollectable = true );
        
        //! synchronously loads an image into a texture, stores it and returns it
        ci::gl::TextureRef	load(const std::string &url, ci::gl::Texture::Format fmt=ci::gl::Texture::Format(), bool isGarbageCollectable = true, bool runGarbageCollector = true);
        //! asynchronously loads an image into a texture, returns immediately
        ci::gl::TextureRef	fetch(const std::string &url, ci::gl::Texture::Format fmt=ci::gl::Texture::Format(), bool isGarbageCollectable = true, bool runGarbageCollector = true);
        
        
        //! returns TRUE if image is scheduled for loading but has not been turned into a Texture yet
        bool isLoading(const std::string &url);
        //! returns TRUE if image has been turned into a Texture
        bool isLoaded(const std::string &url);
        
        //! removes Textures from memory if no longer in use
        void garbageCollect();
        
        // helpers:
        void drawAllStoredTextures( float width = 100.0f, float height = 100.0f );
        void status();
        
      protected:
        void loadImagesThreadFn();
        
        bool                                        mShouldQuit;
        std::shared_ptr<std::thread>                mThread;
        
        //! queue of textures to load asynchronously
        ConcurrentDeque<std::string>                mQueue;
        ConcurrentDeque<std::string>                mLoadingQueue;
        
        std::map<std::string, ci::gl::TextureRef>   mTextureRefs;
        ConcurrentMap<std::string, ci::Surface>     mSurfaces;
        
        //! list of Textures so they don't get garbage collected
    	//std::map<std::string, std::map<std::string, ci::gl::TextureRef>> mTempFetchTextureDirectory;
        std::map<std::string, ci::gl::TextureRef> mTextureRefsNonGarbageCollectable;

    };
    

    // helper functions for easier access
    
    //! synchronously loads all images from a directory
    inline std::vector<ci::gl::TextureRef>	loadImageDirectory(const std::string &url, ci::gl::Texture::Format fmt=ci::gl::Texture::Format(), bool garbageCollectable = true){ return TextureStore::getInstance()->loadImageDirectory(url, fmt, garbageCollectable); };
    
    //! asynchronously loads all images from a directory
    inline std::vector<ci::gl::TextureRef>	fetchImageDirectory(const std::string &url, ci::gl::Texture::Format fmt=ci::gl::Texture::Format(), bool garbageCollectable = true){ return TextureStore::getInstance()->fetchImageDirectory(url, fmt, garbageCollectable); };
    
    //! synchronously loads an image into a texture, stores it and returns it immediately
    inline ci::gl::TextureRef	loadTexture(const std::string &url, ci::gl::Texture::Format fmt=ci::gl::Texture::Format()){ return TextureStore::getInstance()->load(url, fmt); };

    //! asynchronously loads an image into a texture,  stores it and returns it once it's loaded
    inline ci::gl::TextureRef	fetchTexture(const std::string &url, ci::gl::Texture::Format fmt=ci::gl::Texture::Format()){ return TextureStore::getInstance()->fetch(url, fmt); };

} // namespace rph
