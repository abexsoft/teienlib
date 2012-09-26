class CameraMover
  CS_FREELOOK = 0
  CS_ORBIT = 1
  CS_MANUAL = 2
  CS_TPS = 3

  CAM_HEIGHT = 5.0

  attr_accessor :height
  attr_accessor :camera_pivot
  attr_accessor :camera

  def initialize(cam)
    @camera = cam
    @camera.setPosition(0, 0, 0)
    @camera.setNearClipDistance(0.1)

    @style = CS_FREELOOK

    # CS_FREELOOK, CS_ORBIT, CS_MANUAL
    @sdk_camera_man = OgreBites::SdkCameraMan.new(@camera)
    @evt_frame = Ogre::FrameEvent.new

    # CS_TPS
    @height = CAM_HEIGHT
    @camera_pivot = cam.getSceneManager().getRootSceneNode().createChildSceneNode()
    @camera_goal = @camera_pivot.createChildSceneNode(Ogre::Vector3.new(0, 0, 5))

    @camera_pivot.setFixedYawAxis(true)
    @camera_goal.setFixedYawAxis(true)

    @pivot_pitch = 0

  end

  def set_style(style)
    @style = style
    case @style
    when CS_FREELOOK
      @sdk_camera_man.setStyle(OgreBites::CS_FREELOOK)
    when CS_ORBIT
      @sdk_camera_man.setStyle(OgreBites::CS_ORBIT)
    else  # CS_MANUAL, CS_TPS
      @sdk_camera_man.setStyle(OgreBites::CS_MANUAL)
    end
  end

  def set_target(target)
    @target = target
    if @style == CS_TPS
      @camera.setAutoTracking(false)
      @camera.moveRelative(Ogre::Vector3.new(0, 0, 0))
      update_camera(1.0)
    else
      @sdk_camera_man.setTarget(target.pivotSceneNode)
    end
  end

  def set_position(pos)
    @camera.setPosition(pos) if @style == CS_FREELOOK
  end

  def look_at(pos)
    @camera.lookAt(pos) if @style == CS_FREELOOK
  end

  def set_yaw_pitch_dist(yaw, pitch, dist)
    @sdk_camera_man.setYawPitchDist(yaw, pitch, dist) if @style == CS_ORBIT
  end

  def move_forward(bl)
    evt = OIS::KeyEvent.new(nil, OIS::KC_W, 0)
    if bl
      @sdk_camera_man.injectKeyDown(evt)
    else
      @sdk_camera_man.injectKeyUp(evt)
    end
  end

  def move_backward(bl)
    evt = OIS::KeyEvent.new(nil, OIS::KC_S, 0)
    if bl
      @sdk_camera_man.injectKeyDown(evt)
    else
      @sdk_camera_man.injectKeyUp(evt)
    end
  end

  def move_left(bl)
    evt = OIS::KeyEvent.new(nil, OIS::KC_A, 0)
    if bl
      @sdk_camera_man.injectKeyDown(evt)
    else
      @sdk_camera_man.injectKeyUp(evt)
    end
  end

  def move_right(bl)
    evt = OIS::KeyEvent.new(nil, OIS::KC_D, 0)
    if bl
      @sdk_camera_man.injectKeyDown(evt)
    else
      @sdk_camera_man.injectKeyUp(evt)
    end
  end


  def update(delta)
    if (@style == CS_TPS)
      update_camera(delta)
    else
      @evt_frame.timeSinceLastFrame = delta
      @sdk_camera_man.frameRenderingQueued(@evt_frame)
    end
  end

  #
  # This method moves this camera position to the goal position smoothly.
  # In general, should be called in the frameRenderingQueued handler.
  #
  def update_camera(deltaTime)
    # place the camera pivot roughly at the character's shoulder
    @camera_pivot.setPosition(@target.get_position() + Ogre::Vector3.UNIT_Y * @height)
    # move the camera smoothly to the goal
    goalOffset = @camera_goal._getDerivedPosition() - @camera.getPosition()
    @camera.move(goalOffset * deltaTime * 9.0)
    # always look at the pivot
    @camera.lookAt(@camera_pivot._getDerivedPosition())
  end

  def mouse_moved(evt)
    if @style == CS_TPS

      # deal with a warp.
      if evt.state.X.rel.abs > 300
        #puts "#{evt.state.X.rel}, #{evt.state.X.abs}"
        return true
      end

      update_camera_goal(-0.05 * evt.state.X.rel, 
                         -0.05 * evt.state.Y.rel, 
                         -0.0005 * evt.state.Z.rel)
    else
      @sdk_camera_man.injectMouseMove(evt)      
    end    
    return true
  end

  #
  # This method updates the goal position, which this camera should be placed finally.
  # In general, should be called when the mouse is moved.
  # *deltaYaw*::_float_, degree value.
  # *deltaPitch*::_float_, degree value.
  # *deltaZoom*::_float_, zoom 
  #
  def update_camera_goal(deltaYaw, deltaPitch, deltaZoom)

    @camera_pivot.yaw(Ogre::Radian.new(Ogre::Degree.new(deltaYaw)), Ogre::Node::TS_WORLD);

    # bound the pitch
    if (!(@pivot_pitch + deltaPitch > 25 && deltaPitch > 0) &&
        !(@pivot_pitch + deltaPitch < -60 && deltaPitch < 0))
      @camera_pivot.pitch(Ogre::Radian.new(Ogre::Degree.new(deltaPitch)), Ogre::Node::TS_LOCAL)
      @pivot_pitch += deltaPitch;
    end
    dist = @camera_goal._getDerivedPosition().distance(@camera_pivot._getDerivedPosition())
    distChange = deltaZoom * dist;

#    puts "dist: #{dist}:#{distChange}"

    # bound the zoom
    if (!(dist + distChange < 8 && distChange < 0) &&
        !(dist + distChange > 25 && distChange > 0))

      @camera_goal.translate(Ogre::Vector3.new(0, 0, distChange), Ogre::Node::TS_LOCAL)
    end
  end

  def mouse_pressed(mouseEvent, mouseButtonID)
    @sdk_camera_man.injectMouseDown(mouseEvent, mouseButtonID) if @style == CS_ORBIT
    return true
  end

  def mouse_released(mouseEvent, mouseButtonID)
    @sdk_camera_man.injectMouseUp(mouseEvent, mouseButtonID) if @style == CS_ORBIT
    return true
  end
  
end

