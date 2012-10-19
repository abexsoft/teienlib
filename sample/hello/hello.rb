
$LOAD_PATH.push(File.dirname(File.expand_path(__FILE__)) + "/../../lib")

require "bullet"
require "ogre"
require "ois"
require "ogrebites"
require "procedural"
require "ogre_config"

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
    init_physics()

    # view
    init_view()
  end

  def init_physics()
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
    @dynamics_world.set_gravity(gravity)
  end

  def init_view()
    # initialize Root
    @root = Ogre::Root.new("")

    # load Plugins
    cfg = Ogre::ConfigFile.new
    cfg.load("#{$file_path}/plugins.cfg")
    pluginDir = cfg.get_setting("PluginFolder")
    pluginDir += "/" if (pluginDir.length > 0) && (pluginDir[-1] != '/') 
    cfg.each_settings {|secName, keyName, valueName|
      fullPath = pluginDir + valueName
      fullPath.sub!("<SystemPluginFolder>", OgreConfig::get_plugin_folder)
      @root.load_plugin(fullPath) if (keyName == "Plugin")
    }

    # initialize Resources
    cfg = Ogre::ConfigFile.new
    cfg.load("#{$file_path}/resources.cfg")
    resourceDir = cfg.get_setting("ResourceFolder")
    resourceDir += "/" if (resourceDir.length > 0) && (resourceDir[-1] != '/')
    cfg.each_settings {|secName, keyName, valueName|
      next if (keyName == "ResourceFolder")
      fullPath = resourceDir + valueName
      fullPath.sub!("<SystemResourceFolder>", OgreConfig::get_resource_folder)
      Ogre::ResourceGroupManager::get_singleton().add_resource_location(fullPath, 
                                                                        keyName, 
                                                                        secName)
    }

    # create a window to draw.
    return false unless @root.show_config_dialog()
    @window = @root.initialise(true, "Sinbad")
    @root.add_frame_listener(self)

    # initialize Managers
    ## InputManager
    windowHnd = Ogre::Intp.new
    @window.get_custom_attribute("WINDOW", windowHnd)
    windowHndStr = sprintf("%d", windowHnd.value())
    pl = Ois::ParamList.new
    pl["WINDOW"] = windowHndStr
    @inputManager = Ois::InputManager::create_input_system(pl)
    @keyboard = @inputManager.create_input_object(Ois::OISKeyboard, true).to_keyboard()
    @mouse = @inputManager.create_input_object(Ois::OISMouse, true).to_mouse()

    ## TrayManager
    Ogre::ResourceGroupManager::get_singleton().initialise_resource_group("Essential")
    @tray_mgr = Ogrebites::SdkTrayManager.new("Base", @window, @mouse);
    ms = @mouse.get_mouse_state()
    ms.width = @window.get_width()
    ms.height = @window.get_height()

    ## SceneMgr
    @sceneMgr = @root.create_scene_manager(Ogre::ST_GENERIC)
    @sceneMgr.set_shadow_technique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE)

    # initialize Listeners
    @keyListener = KeyListener.new(self)
    @keyboard.set_event_callback(@keyListener)

    @mouseListener = MouseListener.new(self)
    @mouse.set_event_callback(@mouseListener)

    @trayListener = TrayListener.new(self)
    @tray_mgr.set_listener(@trayListener)

    # initialize Camera
    @camera = @sceneMgr.create_camera("FixCamera")
    # Create one viewport, entire window
    @vp = @window.add_viewport(@camera);
    @vp.set_background_colour(Ogre::ColourValue.new(0, 0, 0))
    # Alter the camera aspect ratio to match the viewport
    @camera.set_aspect_ratio(Float(@vp.get_actual_width()) / Float(@vp.get_actual_height()))

    # load "General" group resources into ResourceGroupManager.
    @tray_mgr.show_loading_bar(1, 0)
    Ogre::ResourceGroupManager::get_singleton().initialise_resource_group("General")
    @tray_mgr.hide_loading_bar()
    Ogre::TextureManager::get_singleton().set_default_num_mipmaps(5)

    @tray_mgr.show_frame_stats(Ogrebites::TL_BOTTOMLEFT)
    @tray_mgr.show_logo(Ogrebites::TL_BOTTOMRIGHT)
    @tray_mgr.hide_cursor()
  end

  def finalize()
    # patch for GC.
    @dynamics_world.remove_rigid_body(@floor.rigid_body)
    @boxes.each{|box|
      @dynamics_world.remove_rigid_body(box.rigid_body)
    }
    @boxes = []
  end

  def run
    # set background and some fog
    @vp.set_background_colour(Ogre::ColourValue.new(1.0, 1.0, 0.8))
    @sceneMgr.set_fog(Ogre::FOG_LINEAR, Ogre::ColourValue.new(1.0, 1.0, 0.8), 0, 15, 100)

    # set shadow properties
    @sceneMgr.set_shadow_technique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE)
    @sceneMgr.set_shadow_colour(Ogre::ColourValue.new(0.5, 0.5, 0.5))
    @sceneMgr.set_shadow_texture_size(1024)
    @sceneMgr.set_shadow_texture_count(1)

    # use a small amount of ambient lighting
    @sceneMgr.set_ambient_light(Ogre::ColourValue.new(0.3, 0.3, 0.3))

    # add a bright light above the scene
    @light = @sceneMgr.create_light()
    @light.set_type(Ogre::Light::LT_POINT)
    @light.set_position(-10, 40, 20)
    @light.set_specular_colour(Ogre::ColourValue.White)

    @camera_mover = CameraMover.new(@camera)
    @camera_mover.set_position(Ogre::Vector3.new(20, 10, 20))
    @camera_mover.look_at(Ogre::Vector3.new(0, 10, 0))

    items = []
    items.push("Help")
    @help = @tray_mgr.create_params_panel(Ogrebites::TL_TOPLEFT, "HelpMessage", 100, items)
    @help.set_param_value(Ogre::UTFString.new("Help"), Ogre::UTFString.new("H / F1"))

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

    @root.start_rendering()
    finalize()
  end

  def shot_box()
    cam_pos_ogre = @camera.get_position()
    cam_dir_ogre = @camera.get_direction()

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
  def frame_rendering_queued(evt)
    # update bullet(physics)
    @dynamics_world.step_simulation(evt.timeSinceLastFrame)

    # update ogre3d(view)
    @keyboard.capture()
    @mouse.capture()
    @tray_mgr.frame_rendering_queued(evt)

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
    when Ois::KC_ESCAPE
      @quit =true
    when Ois::KC_H, Ois::KC_F1
      if (!@tray_mgr.is_dialog_visible() && @info["Help"] != "") 
        @tray_mgr.show_ok_dialog(Ogre::UTFString.new("Help"), Ogre::UTFString.new(@info["Help"]))
      else 
        @tray_mgr.close_dialog()
      end

    when Ois::KC_E
      @camera_mover.move_forward(true)
    when Ois::KC_D
      @camera_mover.move_backward(true)
    when Ois::KC_S
      @camera_mover.move_left(true)
    when Ois::KC_F
      @camera_mover.move_right(true)
    end

    return true
  end
  
  def key_released(keyEvent)
    case keyEvent.key
    when Ois::KC_ESCAPE
      @quit =true
    when Ois::KC_E
      @camera_mover.move_forward(false)
    when Ois::KC_D
      @camera_mover.move_backward(false)
    when Ois::KC_S
      @camera_mover.move_left(false)
    when Ois::KC_F
      @camera_mover.move_right(false)
    end

    return true
  end
  
  def mouse_moved(mouseEvent)
    return true if @tray_mgr.inject_mouse_move(mouseEvent)
    @camera_mover.mouse_moved(mouseEvent)
    return true
  end

  def mouse_pressed(mouseEvent, mouseButtonID)
    return true if @tray_mgr.inject_mouse_down(mouseEvent, mouseButtonID)
    @camera_mover.mouse_pressed(mouseEvent, mouseButtonID)

    if (mouseButtonID == Ois::MB_Left)
      shot_box()
    end

    return true
  end

  def mouse_released(mouseEvent, mouseButtonID)
    return true if @tray_mgr.inject_mouse_up(mouseEvent, mouseButtonID)
    @camera_mover.mouse_released(mouseEvent, mouseButtonID)
    return true
  end

  def ok_dialog_closed(name)
    return true
  end
end


world = World.new()
world.run
