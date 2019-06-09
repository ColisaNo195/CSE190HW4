/************************************************************************************

Authors     :   Bradley Austin Davis <bdavis@saintandreas.org>
Copyright   :   Copyright Brad Davis. All Rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/
#include "pch.h" // for server
#include "rpc/client.h"
#include "serverData.h"

#include "Avatar.cpp" // Oculus Avatar

#include <iostream>
#include <memory>
#include <exception>
#include <algorithm>
#include <string>

#include <Windows.h>

#define __STDC_FORMAT_MACROS 1

#define FAIL(X) throw std::runtime_error(X)

///////////////////////////////////////////////////////////////////////////////
//
// GLM is a C++ math library meant to mirror the syntax of GLSL 
//

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Skybox.h"

// Import the most commonly used types into the default namespace
using glm::ivec3;
using glm::ivec2;
using glm::uvec2;
using glm::mat3;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::quat;

///////////////////////////////////////////////////////////////////////////////
//
// GLEW gives cross platform access to OpenGL 3.x+ functionality.  
//

#include <GL/glew.h>

bool checkFramebufferStatus(GLenum target = GL_FRAMEBUFFER)
{
  GLuint status = glCheckFramebufferStatus(target);
  switch (status)
  {
  case GL_FRAMEBUFFER_COMPLETE:
    return true;
    break;

  case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
    std::cerr << "framebuffer incomplete attachment" << std::endl;
    break;

  case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
    std::cerr << "framebuffer missing attachment" << std::endl;
    break;

  case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
    std::cerr << "framebuffer incomplete draw buffer" << std::endl;
    break;

  case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
    std::cerr << "framebuffer incomplete read buffer" << std::endl;
    break;

  case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
    std::cerr << "framebuffer incomplete multisample" << std::endl;
    break;

  case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
    std::cerr << "framebuffer incomplete layer targets" << std::endl;
    break;

  case GL_FRAMEBUFFER_UNSUPPORTED:
    std::cerr << "framebuffer unsupported internal format or image" << std::endl;
    break;

  default:
    std::cerr << "other framebuffer error" << std::endl;
    break;
  }

  return false;
}

bool checkGlError()
{
  GLenum error = glGetError();
  if (!error)
  {
    return false;
  }
  else
  {
    switch (error)
    {
    case GL_INVALID_ENUM:
      std::cerr <<
        ": An unacceptable value is specified for an enumerated argument.The offending command is ignored and has no other side effect than to set the error flag.";
      break;
    case GL_INVALID_VALUE:
      std::cerr <<
        ": A numeric argument is out of range.The offending command is ignored and has no other side effect than to set the error flag";
      break;
    case GL_INVALID_OPERATION:
      std::cerr <<
        ": The specified operation is not allowed in the current state.The offending command is ignored and has no other side effect than to set the error flag..";
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      std::cerr <<
        ": The framebuffer object is not complete.The offending command is ignored and has no other side effect than to set the error flag.";
      break;
    case GL_OUT_OF_MEMORY:
      std::cerr <<
        ": There is not enough memory left to execute the command.The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
      break;
    case GL_STACK_UNDERFLOW:
      std::cerr <<
        ": An attempt has been made to perform an operation that would cause an internal stack to underflow.";
      break;
    case GL_STACK_OVERFLOW:
      std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to overflow.";
      break;
    }
    return true;
  }
}

void glDebugCallbackHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg,
                            GLvoid* data)
{
  OutputDebugStringA(msg);
  std::cout << "debug call: " << msg << std::endl;
}

//////////////////////////////////////////////////////////////////////
//
// GLFW provides cross platform window creation
//

#include <GLFW/glfw3.h>

namespace glfw
{
  inline GLFWwindow* createWindow(const uvec2& size, const ivec2& position = ivec2(INT_MIN))
  {
    GLFWwindow* window = glfwCreateWindow(size.x, size.y, "glfw", nullptr, nullptr);
    if (!window)
    {
      FAIL("Unable to create rendering window");
    }
    if ((position.x > INT_MIN) && (position.y > INT_MIN))
    {
      glfwSetWindowPos(window, position.x, position.y);
    }
    return window;
  }
}

// A class to encapsulate using GLFW to handle input and render a scene
class GlfwApp
{
protected:
  uvec2 windowSize;
  ivec2 windowPosition;
  GLFWwindow* window{nullptr};
  unsigned int frame{0};

public:
  GlfwApp()
  {
    // Initialize the GLFW system for creating and positioning windows
    if (!glfwInit())
    {
      FAIL("Failed to initialize GLFW");
    }
    glfwSetErrorCallback(ErrorCallback);
  }

  virtual ~GlfwApp()
  {
    if (nullptr != window)
    {
      glfwDestroyWindow(window);
    }
    glfwTerminate();
  }

  virtual int run()
  {
    preCreate();

    window = createRenderingTarget(windowSize, windowPosition);

    if (!window)
    {
      std::cout << "Unable to create OpenGL window" << std::endl;
      return -1;
    }

    postCreate();

    initGl();

    while (!glfwWindowShouldClose(window))
    {
      ++frame;
      glfwPollEvents();
      update();
      draw();
      finishFrame();
    }

    shutdownGl();

    return 0;
  }

protected:
  virtual GLFWwindow* createRenderingTarget(uvec2& size, ivec2& pos) = 0;

  virtual void draw() = 0;

  void preCreate()
  {
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
  }

  void postCreate()
  {
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwMakeContextCurrent(window);

    // Initialize the OpenGL bindings
    // For some reason we have to set this experminetal flag to properly
    // init GLEW if we use a core context.
    glewExperimental = GL_TRUE;
    if (0 != glewInit())
    {
      FAIL("Failed to initialize GLEW");
    }
    glGetError();

    if (GLEW_KHR_debug)
    {
      GLint v;
      glGetIntegerv(GL_CONTEXT_FLAGS, &v);
      if (v & GL_CONTEXT_FLAG_DEBUG_BIT)
      {
        //glDebugMessageCallback(glDebugCallbackHandler, this);
      }
    }
  }

  virtual void initGl()
  {
  }

  virtual void shutdownGl()
  {
  }

  virtual void finishFrame()
  {
    glfwSwapBuffers(window);
  }

  virtual void destroyWindow()
  {
    glfwSetKeyCallback(window, nullptr);
    glfwSetMouseButtonCallback(window, nullptr);
    glfwDestroyWindow(window);
  }

  virtual void onKey(int key, int scancode, int action, int mods)
  {
    if (GLFW_PRESS != action)
    {
      return;
    }

    switch (key)
    {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window, 1);
      return;
    }
  }

  virtual void update()
  {
  }

  virtual void onMouseButton(int button, int action, int mods)
  {
  }

protected:
  virtual void viewport(const ivec2& pos, const uvec2& size)
  {
    glViewport(pos.x, pos.y, size.x, size.y);
  }

private:

  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
  {
    GlfwApp* instance = (GlfwApp *)glfwGetWindowUserPointer(window);
    instance->onKey(key, scancode, action, mods);
  }

  static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
  {
    GlfwApp* instance = (GlfwApp *)glfwGetWindowUserPointer(window);
    instance->onMouseButton(button, action, mods);
  }

  static void ErrorCallback(int error, const char* description)
  {
    FAIL(description);
  }
};

//////////////////////////////////////////////////////////////////////
//
// The Oculus VR C API provides access to information about the HMD
//

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

namespace ovr
{
  // Convenience method for looping over each eye with a lambda
  template <typename Function>
  inline void for_each_eye(Function function)
  {
    for (ovrEyeType eye = ovrEyeType::ovrEye_Left;
         eye < ovrEyeType::ovrEye_Count;
         eye = static_cast<ovrEyeType>(eye + 1))
    {
      function(eye);
    }
  }

  inline mat4 toGlm(const ovrMatrix4f& om)
  {
    return glm::transpose(glm::make_mat4(&om.M[0][0]));
  }

  inline mat4 toGlm(const ovrFovPort& fovport, float nearPlane = 0.01f, float farPlane = 10000.0f)
  {
    return toGlm(ovrMatrix4f_Projection(fovport, nearPlane, farPlane, true));
  }

  inline vec3 toGlm(const ovrVector3f& ov)
  {
    return glm::make_vec3(&ov.x);
  }

  inline vec2 toGlm(const ovrVector2f& ov)
  {
    return glm::make_vec2(&ov.x);
  }

  inline uvec2 toGlm(const ovrSizei& ov)
  {
    return uvec2(ov.w, ov.h);
  }

  inline quat toGlm(const ovrQuatf& oq)
  {
    return glm::make_quat(&oq.x);
  }

  inline mat4 toGlm(const ovrPosef& op)
  {
    mat4 orientation = glm::mat4_cast(toGlm(op.Orientation));
    mat4 translation = glm::translate(mat4(), ovr::toGlm(op.Position));
    return translation * orientation;
  }

  inline ovrMatrix4f fromGlm(const mat4& m)
  {
    ovrMatrix4f result;
    mat4 transposed(glm::transpose(m));
    memcpy(result.M, &(transposed[0][0]), sizeof(float) * 16);
    return result;
  }

  inline ovrVector3f fromGlm(const vec3& v)
  {
    ovrVector3f result;
    result.x = v.x;
    result.y = v.y;
    result.z = v.z;
    return result;
  }

  inline ovrVector2f fromGlm(const vec2& v)
  {
    ovrVector2f result;
    result.x = v.x;
    result.y = v.y;
    return result;
  }

  inline ovrSizei fromGlm(const uvec2& v)
  {
    ovrSizei result;
    result.w = v.x;
    result.h = v.y;
    return result;
  }

  inline ovrQuatf fromGlm(const quat& q)
  {
    ovrQuatf result;
    result.x = q.x;
    result.y = q.y;
    result.z = q.z;
    result.w = q.w;
    return result;
  }
}

class RiftManagerApp
{
protected:
  ovrSession _session;
  ovrHmdDesc _hmdDesc;
  ovrGraphicsLuid _luid;

public:
  RiftManagerApp()
  {
    if (!OVR_SUCCESS(ovr_Create(&_session, &_luid)))
    {
      FAIL("Unable to create HMD session");
    }

    _hmdDesc = ovr_GetHmdDesc(_session);
  }

  ~RiftManagerApp()
  {
    ovr_Destroy(_session);
    _session = nullptr;
  }
};

class RiftApp : public GlfwApp, public RiftManagerApp
{
public:

private:
  GLuint _fbo{0};
  GLuint _depthBuffer{0};
  ovrTextureSwapChain _eyeTexture;

  GLuint _mirrorFbo{0};
  ovrMirrorTexture _mirrorTexture;

  ovrEyeRenderDesc _eyeRenderDescs[2];

  mat4 _eyeProjections[2];

  ovrLayerEyeFov _sceneLayer;
  ovrViewScaleDesc _viewScaleDesc;

  uvec2 _renderTargetSize;
  uvec2 _mirrorSize;

  // for avatar
  std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();
  ovrAvatarPacket* playbackPacket = nullptr;
  float playbackTime = 0;

public:

  RiftApp()
  {
    using namespace ovr;
    _viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

    memset(&_sceneLayer, 0, sizeof(ovrLayerEyeFov));
    _sceneLayer.Header.Type = ovrLayerType_EyeFov;
    _sceneLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

    ovr::for_each_eye([&](ovrEyeType eye)
    {
      ovrEyeRenderDesc& erd = _eyeRenderDescs[eye] = ovr_GetRenderDesc(_session, eye, _hmdDesc.DefaultEyeFov[eye]);
      ovrMatrix4f ovrPerspectiveProjection =
        ovrMatrix4f_Projection(erd.Fov, 0.01f, 1000.0f, ovrProjection_ClipRangeOpenGL);
      _eyeProjections[eye] = ovr::toGlm(ovrPerspectiveProjection);
      _viewScaleDesc.HmdToEyePose[eye] = erd.HmdToEyePose;

      ovrFovPort& fov = _sceneLayer.Fov[eye] = _eyeRenderDescs[eye].Fov;
      auto eyeSize = ovr_GetFovTextureSize(_session, eye, fov, 1.0f);
      _sceneLayer.Viewport[eye].Size = eyeSize;
      _sceneLayer.Viewport[eye].Pos = {(int)_renderTargetSize.x, 0};

      _renderTargetSize.y = std::max(_renderTargetSize.y, (uint32_t)eyeSize.h);
      _renderTargetSize.x += eyeSize.w;
    });
    // Make the on screen window 1/4 the resolution of the render target
    _mirrorSize = _renderTargetSize;
    _mirrorSize /= 4;
  }

protected:
  GLFWwindow* createRenderingTarget(uvec2& outSize, ivec2& outPosition) override
  {
    return glfw::createWindow(_mirrorSize);
  }

  void initGl() override
  {
    GlfwApp::initGl();

    // Disable the v-sync for buffer swap
    glfwSwapInterval(0);

    ovrTextureSwapChainDesc desc = {};
    desc.Type = ovrTexture_2D;
    desc.ArraySize = 1;
    desc.Width = _renderTargetSize.x;
    desc.Height = _renderTargetSize.y;
    desc.MipLevels = 1;
    desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc.SampleCount = 1;
    desc.StaticImage = ovrFalse;
    ovrResult result = ovr_CreateTextureSwapChainGL(_session, &desc, &_eyeTexture);
    _sceneLayer.ColorTexture[0] = _eyeTexture;
    if (!OVR_SUCCESS(result))
    {
      FAIL("Failed to create swap textures");
    }

    int length = 0;
    result = ovr_GetTextureSwapChainLength(_session, _eyeTexture, &length);
    if (!OVR_SUCCESS(result) || !length)
    {
      FAIL("Unable to count swap chain textures");
    }
    for (int i = 0; i < length; ++i)
    {
      GLuint chainTexId;
      ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, i, &chainTexId);
      glBindTexture(GL_TEXTURE_2D, chainTexId);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // Set up the framebuffer object
    glGenFramebuffers(1, &_fbo);
    glGenRenderbuffers(1, &_depthBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _renderTargetSize.x, _renderTargetSize.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	
    ovrMirrorTextureDesc mirrorDesc;
    memset(&mirrorDesc, 0, sizeof(mirrorDesc));
    mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
    mirrorDesc.Width = _mirrorSize.x;
    mirrorDesc.Height = _mirrorSize.y;
    if (!OVR_SUCCESS(ovr_CreateMirrorTextureGL(_session, &mirrorDesc, &_mirrorTexture)))
    {
      FAIL("Could not create mirror texture");
    }
    glGenFramebuffers(1, &_mirrorFbo);
  }

  void onKey(int key, int scancode, int action, int mods) override
  {
    if (GLFW_PRESS == action)
      switch (key)
      {
      case GLFW_KEY_R:
        ovr_RecenterTrackingOrigin(_session);
        return;
      }

    GlfwApp::onKey(key, scancode, action, mods);
  }

  bool pressA = false;

  void draw() final override
  {
	  /* avatar section tracing */
	  // Compute how much time has elapsed since the last frame
	  std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
	  std::chrono::duration<float> deltaTime = currentTime - lastTime;
	  float deltaSeconds = deltaTime.count();
	  lastTime = currentTime;

	  // Convert the OVR inputs into Avatar SDK inputs
	  ovrInputState touchState;
	  ovr_GetInputState(_session, ovrControllerType_Active, &touchState);
	  ovrTrackingState trackingState = ovr_GetTrackingState(_session, 0.0, false);

	  glm::vec3 hmdP = _glmFromOvrVector(trackingState.HeadPose.ThePose.Position);
	  glm::quat hmdQ = _glmFromOvrQuat(trackingState.HeadPose.ThePose.Orientation);
	  glm::vec3 leftP = _glmFromOvrVector(trackingState.HandPoses[ovrHand_Left].ThePose.Position);
	  glm::quat leftQ = _glmFromOvrQuat(trackingState.HandPoses[ovrHand_Left].ThePose.Orientation);
	  glm::vec3 rightP = _glmFromOvrVector(trackingState.HandPoses[ovrHand_Right].ThePose.Position);
	  glm::quat rightQ = _glmFromOvrQuat(trackingState.HandPoses[ovrHand_Right].ThePose.Orientation);

	  ovrAvatarTransform hmd;
	  _ovrAvatarTransformFromGlm(hmdP, hmdQ, glm::vec3(1.0f), &hmd);

	  ovrAvatarTransform left;
	  _ovrAvatarTransformFromGlm(leftP, leftQ, glm::vec3(1.0f), &left);

	  ovrAvatarTransform right;
	  _ovrAvatarTransformFromGlm(rightP, rightQ, glm::vec3(1.0f), &right);

	  ovrAvatarHandInputState inputStateLeft;
	  _ovrAvatarHandInputStateFromOvr(left, touchState, ovrHand_Left, &inputStateLeft);

	  ovrAvatarHandInputState inputStateRight;
	  _ovrAvatarHandInputStateFromOvr(right, touchState, ovrHand_Right, &inputStateRight);

	  _runAvatar(deltaSeconds, hmd, inputStateLeft, inputStateRight, playbackPacket, &playbackTime); // end of avatar update

	  ovrInputState inputState;
	  if (OVR_SUCCESS(ovr_GetInputState(_session, ovrControllerType_Touch, &inputState))) {
		  // start game
		  if (inputState.Buttons & ovrButton_A) {
			  if (pressButton(pressA)) {
				  startGame();
			  }
		  }
		  else { pressA = false; }
	  }

    ovrPosef eyePoses[2];
    ovr_GetEyePoses(_session, frame, true, _viewScaleDesc.HmdToEyePose, eyePoses, &_sceneLayer.SensorSampleTime);

    int curIndex;
    ovr_GetTextureSwapChainCurrentIndex(_session, _eyeTexture, &curIndex);
    GLuint curTexId;
    ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, curIndex, &curTexId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ovr::for_each_eye([&](ovrEyeType eye)
    {
      const auto& vp = _sceneLayer.Viewport[eye];
      glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
      _sceneLayer.RenderPose[eye] = eyePoses[eye];

	  glm::vec3 eyeWorld = _glmFromOvrVector(eyePoses[eye].Position); // for Avatar
      renderScene(_eyeProjections[eye], ovr::toGlm(eyePoses[eye]), eyeWorld);
    });
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    ovr_CommitTextureSwapChain(_session, _eyeTexture);
    ovrLayerHeader* headerList = &_sceneLayer.Header;
    ovr_SubmitFrame(_session, frame, &_viewScaleDesc, &headerList, 1);

    GLuint mirrorTextureId;
    ovr_GetMirrorTextureBufferGL(_session, _mirrorTexture, &mirrorTextureId);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _mirrorFbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0);
    glBlitFramebuffer(0, 0, _mirrorSize.x, _mirrorSize.y, 0, _mirrorSize.y, _mirrorSize.x, 0, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  }

  // allow triggering the status once when a button is pressed.
  bool pressButton(bool& button) {
	  if (!button) {
		  button = true;
		  return true;
	  }
	  else {
		  return false;
	  }
  }

  virtual void renderScene(const glm::mat4& projection, const glm::mat4& headPose, const glm::vec3& viewPos) = 0;
  virtual void startGame() = 0;
};

//////////////////////////////////////////////////////////////////////
//
// The remainder of this code is specific to the scene we want to 
// render.  I use glfw to render an array of cubes, but your 
// application would perform whatever rendering you want
//

#include <vector>
#include "shader.h"
#include "Cube.h"

// a class for building and rendering cubes
class Scene
{
  // Program
  std::vector<glm::mat4> instance_positions;
  GLuint instanceCount;
  GLuint shaderID;

  std::unique_ptr<TexturedCube> cube;
  std::unique_ptr<Skybox> skybox;

  const unsigned int GRID_SIZE{5};

public:
  Scene()
  {
    // Create two cube
    instance_positions.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -0.3)));
    instance_positions.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -0.9)));

    instanceCount = instance_positions.size();

    // Shader Program 
    shaderID = LoadShaders("skybox.vert", "skybox.frag");

    cube = std::make_unique<TexturedCube>("cube"); 

	// 10m wide sky box: size doesn't matter though
    skybox = std::make_unique<Skybox>("skybox");
	skybox->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
  }

  void render(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& viewPos)
  {
    // Render two cubes
    for (int i = 0; i < instanceCount; i++)
    {
      // Scale to 20cm: 200cm * 0.1
      cube->toWorld = instance_positions[i] * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
      cube->draw(shaderID, projection, view);
    }

	// Render Skybox : remove view translation
	//skybox->draw(shaderID, projection, view);
  }
};

// for sound effect
#include "fmod.hpp"
#include "common.h"

// for font display
#include "text.cpp"

// An example application that renders a simple cube
class ExampleApp : public RiftApp
{
  std::shared_ptr<Scene> scene;
  rpc::client * client;

  // initialize sound
  FMOD::System     *system;
  FMOD::Sound      *sound, *sound_to_play;
  FMOD::Channel    *channel = 0;
  FMOD_RESULT       result;
  unsigned int      version;
  void             *extradriverdata = 0;
  int               numsubsounds;

  // local status of game
  bool started = false;
  bool ended = false; // true after a game ends
  bool victory = true; // if the current player wins
  GLuint textshader;
public:
  ExampleApp(rpc::client * c)
  {
	  client = c;
  }

protected:
  void initGl() override
  {
    RiftApp::initGl();
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    ovr_RecenterTrackingOrigin(_session);
    scene = std::shared_ptr<Scene>(new Scene());

	// intialize fonts
	init_font();
	textshader = LoadShaders("font.vert", "font.frag");

	// initialize avatar
	_initAvatar();
	_initAvatarShader();

	/* music */
	//Common_Init(&extradriverdata);

	//Create a System object and initialize.
	result = FMOD::System_Create(&system);
	result = system->getVersion(&version);
	result = system->init(32, FMOD_INIT_NORMAL, extradriverdata);
	
	//load file here (Note: function uses ../media/ as file path)
	result = system->createStream(Common_MediaPath("rainforest_ambience.wav"), FMOD_LOOP_NORMAL | FMOD_2D, 0, &sound);
	result = sound->getNumSubSounds(&numsubsounds);
	if (numsubsounds)
	{
		sound->getSubSound(0, &sound_to_play);
	}
	else
	{
		sound_to_play = sound;
	}

	//Play the sound.
	result = system->playSound(sound_to_play, 0, false, &channel);

  }

  void shutdownGl() override
  {
    scene.reset();

	// shut down avatar
	_shutdownAvatar();
	
	// shut down music
	result = sound->release();  /* Release the parent, not the sound that was retrieved with getSubSound. */
	result = system->close();
	result = system->release();
	Common_Close();

  }

  void renderScene(const glm::mat4& projection, const glm::mat4& headPose, const glm::vec3& viewPos) override
  {
	glm::mat4 view = glm::inverse(headPose);

    scene->render(projection, view, viewPos);

	// if game has started, render countdown time (Please render before Avatar)
	if (started) {
		int remain_time = client->call("remainTime").as<int>();
		if (remain_time >= 0) {
			renderText(textshader, std::to_string(remain_time), -1.2f, 0.8f, -0.7f, 0.05f, glm::vec3(0.5, 0.8f, 0.2f), projection, view);
		}
		// if the countdown time has past, updated status for started
		else { started = false; ended = true; }
	}
	else if (ended) {
		if (victory) {
			renderText(textshader, "You win!", -1.2f, 0.8f, -0.7f, 0.04f, glm::vec3(0.5, 0.8f, 0.2f), projection, view);
		}
		else {
			renderText(textshader, "Time up", -1.2f, 0.8f, -0.7f, 0.04f, glm::vec3(0.5, 0.8f, 0.2f), projection, view);
		}
		renderText(textshader, "Press A to restart", -2.0f, 0.3f, -1.0f, 0.02f, glm::vec3(0.5, 0.8f, 0.2f), projection, view);
	}

	// Render avatar
	renderAvatar(view, projection, viewPos, false);

	// Render opposing avatar through server
	avatarPos p;
	p.view = glm::mat4(view);
	p.viewPos = glm::vec3(viewPos);
	
	p = client->call("mirrorPos", p).as<avatarPos>();
	renderAvatar(p.view, projection, p.viewPos, true);

	
  }

  void startGame() override
  {
	  if (!started) {
		  started = true;
		  ended = false;
		  client->call("startgame");
	  }
  }
};

using namespace std;
int main(int argc, char** argv)
{
  int result = -1;

  // Setup an rpc client that connects to "localhost:8080"
  rpc::client c("localhost", 8080);
  std::cout << "Connected" << std::endl;

  // testing program for rpc here
  string output = c.call("test").as<string>();
  cout << output;

  // VR app here
  if (!OVR_SUCCESS(ovr_Initialize(nullptr)))
  {
    FAIL("Failed to initialize the Oculus SDK");
  }
  result = ExampleApp(&c).run();
 
  ovr_Shutdown();
  return result;
}
