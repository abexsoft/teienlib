
$LOAD_PATH.push(File.dirname(File.expand_path(__FILE__)) + "/../../lib")

require "Ogre"
require "OIS"
require "OgreBites"
require "Procedural"
require "Bullet"

require "OgreConfig"

require_relative "ui_listener"
require_relative "camera_mover"
require_relative "box"
require_relative "floor"

class World < Ogre::FrameListener
  def initialize()
    super() # needed by Ogre::FrameListener

    @delta_sum = 0
    @quit = false
    @info = {}
    @info["Help"] = 
      " Use the ESDF keys to move the camera. \n" + 
      " Use the left mouse button to shot a box. \n" + 
      " Press Q to quit."

    $file_path = File.dirname(File.expand_path(__FILE__))

    # physics
    initPhysics()

    # view
    initView()
  end

  def initPhysics()
    # initialize bullet dynamics world.
    @collision_config = Bullet::BtDefaultCollisionConfiguration.new();
    @collision_dispatcher = Bullet::BtCollisionDispatcher.new(@collision_config)
    world_aabb_min = Bullet::BtVector3.new(-3000.0,-500.0, -3000.0)
    world_aabb_max = Bullet::BtVector3.new(3000.0, 500.0, 3000.0)
    max_proxies = 1024 * 4
    @aabb_cache = Bullet::BtAxisSweep3.new(world_aabb_min, world_aabb_max, max_proxies)
    @solver = Bullet::BtSequentialImpulseConstraintSolver.new();
    @dynamics_world = Bullet::BtDiscreteDynamicsWorld.new(@collision_dispatcher, @aabb_cache,
                                                         @solver, @collision_config)

    gravity = Bullet::BtVector3.new(0.0, -9.8, 0.0)
    @dynamics_world.setGravity(gravity)
  end

  def initView()
    # initialize Root
    @root = Ogre::Root.new("")

    # load Plugins
    cfg = Ogre::ConfigFile.new
    cfg.load("#{$file_path}/plugins.cfg")
    pluginDir = cfg.getSetting("PluginFolder")
    pluginDir += "/" if (pluginDir.length > 0) && (pluginDir[-1] != '/') 
    cfg.each_Settings {|secName, keyName, valueName|
      fullPath = pluginDir + valueName
      fullPath.sub!("<SystemPluginFolder>", OgreConfig::getPluginFolder)
      @root.loadPlugin(fullPath) if (keyName == "Plugin")
    }

    # initialize Resources
    cfg = Ogre::ConfigFile.new
    cfg.load("#{$file_path}/resources.cfg")
    resourceDir = cfg.getSetting("ResourceFolder")
    resourceDir += "/" if (resourceDir.length > 0) && (resourceDir[-1] != '/')
    cfg.each_Settings {|secName, keyName, valueName|
      next if (keyName == "ResourceFolder")
      fullPath = resourceDir + valueName
      fullPath.sub!("<SystemResourceFolder>", OgreConfig::getResourceFolder)
      Ogre::ResourceGroupManager::getSingleton().addResourceLocation(fullPath, 
                                                                     keyName, 
                                                                     secName)
    }

    # create a window to draw.
    return false unless @root.showConfigDialog()
    @window = @root.initialise(true, "Sinbad")
    @root.addFrameListener(self)

    # initialize Managers
    ## InputManager
    windowHnd = Ogre::Intp.new
    @window.getCustomAttribute("WINDOW", windowHnd)
    windowHndStr = sprintf("%d", windowHnd.value())
    pl = OIS::ParamList.new
    pl["WINDOW"] = windowHndStr
    @inputManager = OIS::InputManager::createInputSystem(pl)
    @keyboard = @inputManager.createInputObject(OIS::OISKeyboard, true).toKeyboard()
    @mouse = @inputManager.createInputObject(OIS::OISMouse, true).toMouse()

    ## TrayManager
    Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Essential")
    @tray_mgr = OgreBites::SdkTrayManager.new("Base", @window, @mouse);
    ms = @mouse.getMouseState()
    ms.width = @window.getWidth()
    ms.height = @window.getHeight()

    ## SceneMgr
    @sceneMgr = @root.createSceneManager(Ogre::ST_GENERIC)
    @sceneMgr.setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE)

    # initialize Listeners
    @keyListener = KeyListener.new(self)
    @keyboard.setEventCallback(@keyListener)

    @mouseListener = MouseListener.new(self)
    @mouse.setEventCallback(@mouseListener)

    @trayListener = TrayListener.new(self)
    @tray_mgr.setListener(@trayListener)

    # initialize Camera
    @camera = @sceneMgr.createCamera("FixCamera")
    # Create one viewport, entire window
    @vp = @window.addViewport(@camera);
    @vp.setBackgroundColour(Ogre::ColourValue.new(0, 0, 0))
    # Alter the camera aspect ratio to match the viewport
    @camera.setAspectRatio(Float(@vp.getActualWidth()) / Float(@vp.getActualHeight()))

    # load "General" group resources into ResourceGroupManager.
    @tray_mgr.showLoadingBar(1, 0)
    Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("General")
    @tray_mgr.hideLoadingBar()
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5)

    @tray_mgr.showFrameStats(OgreBites::TL_BOTTOMLEFT)
    @tray_mgr.showLogo(OgreBites::TL_BOTTOMRIGHT)
    @tray_mgr.hideCursor()
  end

  def run
    # set background and some fog
    @vp.setBackgroundColour(Ogre::ColourValue.new(1.0, 1.0, 0.8))
    @sceneMgr.setFog(Ogre::FOG_LINEAR, Ogre::ColourValue.new(1.0, 1.0, 0.8), 0, 15, 100)

    # set shadow properties
    @sceneMgr.setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE)
    @sceneMgr.setShadowColour(Ogre::ColourValue.new(0.5, 0.5, 0.5))
    @sceneMgr.setShadowTextureSize(1024)
    @sceneMgr.setShadowTextureCount(1)

    # use a small amount of ambient lighting
    @sceneMgr.setAmbientLight(Ogre::ColourValue.new(0.3, 0.3, 0.3))

    # add a bright light above the scene
    @light = @sceneMgr.createLight()
    @light.setType(Ogre::Light::LT_POINT)
    @light.setPosition(-10, 40, 20)
    @light.setSpecularColour(Ogre::ColourValue.White)

    @camera_mover = CameraMover.new(@camera)
    @camera_mover.set_position(Ogre::Vector3.new(20, 10, 20))
    @camera_mover.look_at(Ogre::Vector3.new(0, 10, 0))

    items = []
    items.push("Help")
    @help = @tray_mgr.createParamsPanel(OgreBites::TL_TOPLEFT, "HelpMessage", 100, items)
    @help.setParamValue(Ogre::UTFString.new("Help"), Ogre::UTFString.new("H / F1"))

    # creates objects
    @floor = Floor.new(@dynamics_world, @sceneMgr)

    @boxes = []
    11.times {|x|
      3.times {|y|
        box = Box.new(@dynamics_world, @sceneMgr, 1.0, 1, 1, 1, 'blue')
        box.set_position(-10 + x * 2, 5 + y * 2, 0)
        @boxes.push(box)
      }
    }

    @root.startRendering()
  end

  def shot_box()
    cam_pos_ogre = @camera.getPosition()
    cam_dir_ogre = @camera.getDirection()

    cam_pos = Bullet::BtVector3.new(cam_pos_ogre.x, cam_pos_ogre.y, cam_pos_ogre.z)
    cam_dir = Bullet::BtVector3.new(cam_dir_ogre.x, cam_dir_ogre.y, cam_dir_ogre.z)

    box = Box.new(@dynamics_world, @sceneMgr, 1.0, 1, 1, 1, 'grey')
    box_pos = cam_pos + (cam_dir * 5.0)
    box.set_position(box_pos.x, box_pos.y, box_pos.z)
    
    force = cam_dir * Bullet::BtVector3.new(100.0, 100.0, 100.0)
    box.apply_impulse(force)

    @boxes.push(box)
  end

  #
  # Event handlers
  #

  # implementation of Ogre::FrameListener interface.
  def frameRenderingQueued(evt)
    # update bullet(physics)
    @dynamics_world.stepSimulation(evt.timeSinceLastFrame)

    # update ogre3d(view)
    @keyboard.capture()
    @mouse.capture()
    @tray_mgr.frameRenderingQueued(evt)

    # update model
    @camera_mover.update(evt.timeSinceLastFrame)
    
    @delta_sum += evt.timeSinceLastFrame
    # create a new box per 3sec.
    if (@delta_sum > 3.0) 
      box = Box.new(@dynamics_world, @sceneMgr, 1.0, 1, 1, 1, 'orange')
      box.set_position(-10 + ((box.id - 1) % 11) * 2, 20, 0)
      @boxes.push(box)
      @delta_sum = 0
    end

    return !@quit
  end

  def key_pressed(keyEvent)
    case keyEvent.key 
    when OIS::KC_ESCAPE
      @quit =true
    when OIS::KC_H, OIS::KC_F1
      if (!@tray_mgr.isDialogVisible() && @info["Help"] != "") 
        @tray_mgr.showOkDialog(Ogre::UTFString.new("Help"), Ogre::UTFString.new(@info["Help"]))
      else 
        @tray_mgr.closeDialog()
      end

    when OIS::KC_E
      @camera_mover.move_forward(true)
    when OIS::KC_D
      @camera_mover.move_backward(true)
    when OIS::KC_S
      @camera_mover.move_left(true)
    when OIS::KC_F
      @camera_mover.move_right(true)
    end

    return true
  end
  
  def key_released(keyEvent)
    case keyEvent.key
    when OIS::KC_ESCAPE
      @quit =true
    when OIS::KC_E
      @camera_mover.move_forward(false)
    when OIS::KC_D
      @camera_mover.move_backward(false)
    when OIS::KC_S
      @camera_mover.move_left(false)
    when OIS::KC_F
      @camera_mover.move_right(false)
    end

    return true
  end
  
  def mouse_moved(mouseEvent)
    return true if @tray_mgr.injectMouseMove(mouseEvent)
    @camera_mover.mouse_moved(mouseEvent)
    return true
  end

  def mouse_pressed(mouseEvent, mouseButtonID)
    return true if @tray_mgr.injectMouseDown(mouseEvent, mouseButtonID)
    @camera_mover.mouse_pressed(mouseEvent, mouseButtonID)

    if (mouseButtonID == OIS::MB_Left)
      shot_box()
    end

    return true
  end

  def mouse_released(mouseEvent, mouseButtonID)
    return true if @tray_mgr.injectMouseUp(mouseEvent, mouseButtonID)
    @camera_mover.mouse_released(mouseEvent, mouseButtonID)
    return true
  end

  def ok_dialog_closed(name)
    return true
  end
end


world = World.new()
world.run
