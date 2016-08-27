#include "ofxImGui.h"

ofxImGui::ofxImGui()
{
    lastTime = 0.0f;
    engine = NULL;
    theme = NULL;
}

void ofxImGui::setup(BaseTheme* theme_)
{
    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2((float)ofGetWidth(), (float)ofGetHeight());
    io.MouseDrawCursor = false;

#if defined(TARGET_OPENGLES)
    engine = new EngineOpenGLES();
#else  
    engine = new EngineGLFW();
#endif

    engine->setup();

    if (theme_)
    {
        setTheme(theme_);
    }
    else
    {
        setTheme(new BaseTheme());
    }

	mouseListeners.push_back(ofEvents().mouseDragged.newListener(this, &ofxImGui::mouseEvent, OF_EVENT_ORDER_BEFORE_APP));
	mouseListeners.push_back(ofEvents().mouseEntered.newListener(this, &ofxImGui::mouseEvent, OF_EVENT_ORDER_BEFORE_APP));
	mouseListeners.push_back(ofEvents().mouseExited.newListener(this, &ofxImGui::mouseEvent, OF_EVENT_ORDER_BEFORE_APP));
	mouseListeners.push_back(ofEvents().mouseMoved.newListener(this, &ofxImGui::mouseEvent, OF_EVENT_ORDER_BEFORE_APP));
	mouseListeners.push_back(ofEvents().mousePressed.newListener(this, &ofxImGui::mouseEvent, OF_EVENT_ORDER_BEFORE_APP));
	mouseListeners.push_back(ofEvents().mouseReleased.newListener(this, &ofxImGui::mouseEvent, OF_EVENT_ORDER_BEFORE_APP));
	mouseListeners.push_back(ofEvents().mouseScrolled.newListener(this, &ofxImGui::mouseEvent, OF_EVENT_ORDER_BEFORE_APP));
}

void ofxImGui::setTheme(BaseTheme* theme_)
{
    if(theme)
    {
        delete theme;
        theme = NULL;
    }
    theme = theme_;
    theme->updateColors();
}

void ofxImGui::openThemeColorWindow()
{
    theme->themeColorsWindow(true);
}

GLuint ofxImGui::loadPixels(ofPixels& pixels)
{
    return engine->loadTextureImage2D(pixels.getData(),
                                      pixels.getWidth(),
                                      pixels.getHeight());
}

GLuint ofxImGui::loadPixels(string imagePath)
{
    if(!engine) return -1;
    ofPixels pixels;
    ofLoadImage(pixels, imagePath);
    return loadPixels(pixels);
}

GLuint ofxImGui::loadImage(ofImage& image)
{
    if(!engine) return -1;
    return loadPixels(image.getPixels());
}

GLuint ofxImGui::loadImage(string imagePath)
{
    return loadPixels(imagePath);
}

GLuint ofxImGui::loadTexture(string imagePath)
{
    ofDisableArbTex();
    ofTexture* texture  = new ofTexture();
    ofLoadImage(*texture, imagePath);
    ofEnableArbTex();
    loadedTextures.push_back(texture);
    return texture->getTextureData().textureID;
}

GLuint ofxImGui::loadTexture(ofTexture& texture, string imagePath)
{
    bool isUsingArb = ofGetUsingArbTex();
    if (isUsingArb)
    {
        ofDisableArbTex();

    }
    ofLoadImage(texture, imagePath);
    if (isUsingArb)
    {
        ofEnableArbTex();
    }
    return texture.getTextureData().textureID;
}

void ofxImGui::begin()
{
	if(!firstFrame){
		//ImGui::Render();
	}
	firstFrame = false;
    if(!engine)
    {
        ofLogError(__FUNCTION__) << "setup call required - calling it for you";
        setup();
    }

    ImGuiIO& io = ImGui::GetIO();

    float currentTime = ofGetElapsedTimef();
    if(lastTime > 0.f)
    {
        io.DeltaTime = currentTime - lastTime;
    }
    else
    {
        io.DeltaTime = 1.0f / 60.f;
    }
    lastTime = currentTime;

	ImGui::NewFrame();
}

bool ofxImGui::mouseEvent(ofMouseEventArgs & mouse){
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(mouse.x, mouse.y);
	ofRectangle guiRect{guiPos.x, guiPos.y, guiSize.x, guiSize.y};
	switch (mouse.type) {
		case ofMouseEventArgs::Pressed:
			io.MouseDown[mouse.button] = true;
			mouseEventStartedOnGui = guiRect.inside(mouse);
			return mouseEventStartedOnGui;
		case ofMouseEventArgs::Released:{
			io.MouseDown[mouse.button] = false;
			io.MouseReleased[mouse.button] = true;
			auto attended = mouseEventStartedOnGui;
			mouseEventStartedOnGui = false;
			return attended;
		}
		case ofMouseEventArgs::Scrolled:
			io.MouseWheel = mouse.scrollY;
			return guiRect.inside(mouse);
		default:
		return mouseEventStartedOnGui;
	}
}

void ofxImGui::end()
{
	//TODO: this is wrong, the size of the last window or
	// something but can't be used for capturing mouse events
	guiPos = ImGui::GetWindowPos();
	guiSize = ImGui::GetWindowSize();
	ImGui::Render();
}

void ofxImGui::close()
{
    if(engine)
    {
        delete engine;
        engine = NULL;
    }
    //if(io)
    //{
    //    io->Fonts->TexID = 0;
    //    io = nullptr;
    //}
    if(theme)
    {
        delete theme;
        theme = NULL;
    }
    for(size_t i=0; i<loadedTextures.size(); i++)
    {
        delete loadedTextures[i];
    }
    loadedTextures.clear();
}

ofxImGui::~ofxImGui()
{
    close();
    ImGui::Shutdown();
}
