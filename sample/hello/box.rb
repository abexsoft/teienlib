class Box < Bullet::BtMotionState
  @@cnt = 0

  attr_accessor :id

  def initialize(dynamics_world, scene_mgr, mass, x, y, z, color)
    super() # needed by Bullet::BtMotionState

    @dynamics_world = dynamics_world
    @scene_mgr = scene_mgr
    @x = x
    @y = y
    @z = z
    @color = color
    @@cnt += 1
    @id = @@cnt
    @name = "box_#{@id}"

    # create a physics object.
    @colObj = Bullet::BtBoxShape.new(Bullet::BtVector3.new(x, y, z))
    inertia = Bullet::BtVector3.new()
    @colObj.calculateLocalInertia(mass, inertia)
    @rigid_body = Bullet::BtRigidBody.new(mass, self, @colObj, inertia)
    dynamics_world.addRigidBody(@rigid_body)

    # create a view object.
    gen = Procedural::BoxGenerator.new()
    gen.setSizeX(x * 2.0).setSizeY(y * 2.0).setSizeZ(z * 2.0)
    gen.setNumSegX(1).setNumSegY(1).setNumSegZ(1)
    gen.setUTile(1.0).setVTile(1.0).realizeMesh(@name)
    @entity = scene_mgr.createEntity(@name)
    @entity.setCastShadows(true)
    @entity.setMaterialName("Examples/BumpyMetal")
    @scene_node = scene_mgr.getRootSceneNode().createChildSceneNode()
    @scene_node.attachObject(@entity)

    # position/rotation
    @transform = Bullet::BtTransform.new()
    @transform.setIdentity()
  end

  # called through Bullet::BtMotionState when this rigid_body transform is changed 
  # on bullet simulation step.
  def setWorldTransform(worldTrans)
    @transform = Bullet::BtTransform.new(worldTrans)
    newPos = @transform.getOrigin()
    newRot = @transform.getRotation()
    return if (newRot.x.nan?)

    @scene_node.setPosition(newPos.x, newPos.y, newPos.z) 
    @scene_node.setOrientation(newRot.w, newRot.x, newRot.y, newRot.z)
  end

  # called by Bullet::BtMotionState
  def getWorldTransform(worldTrans)
  end

  def set_position(x, y, z)
    @scene_node.setPosition(x, y, z)
    @transform.setOrigin(Bullet::BtVector3.new(x, y, z))
    @rigid_body.setCenterOfMassTransform(@transform)
  end

  def apply_impulse(imp, rel = Bullet::BtVector3.new(0, 0, 0))
    @rigid_body.activate(true)
    @rigid_body.applyImpulse(imp, rel)
  end
end
