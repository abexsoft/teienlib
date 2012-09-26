class Floor < Bullet::BtMotionState
  def initialize(dynamicsWorld, sceneMgr)
    super()

    x = 10 # width
    y = 1   # depth
    z = 10 # height
    mass = 0.0

    # create a physics object
    @cshape = Bullet::BtBoxShape.new(Bullet::BtVector3.new(x, 1, z))
    inertia = Bullet::BtVector3.new()
    @cshape.calculateLocalInertia(mass, inertia)
    @rigid_body = Bullet::BtRigidBody.new(mass, self, @cshape, inertia)
    dynamicsWorld.addRigidBody(@rigid_body)

    # create a floor mesh resource
    Ogre::MeshManager::getSingleton().createPlane("floor", 
                                                  Ogre::ResourceGroupManager.DEFAULT_RESOURCE_GROUP_NAME,
                                                  Ogre::Plane.new(Ogre::Vector3.UNIT_Y, 0), 
                                                  x * 2, z * 2, 10, 10, true, 1, 10, 10, 
                                                  Ogre::Vector3.UNIT_Z)
    @entity = sceneMgr.createEntity("Floor", "floor")
    @entity.setMaterialName("Examples/Rockwall")
    @entity.setCastShadows(false)
    sceneMgr.getRootSceneNode().attachObject(@entity)

    # set this object's position
    @transform = Bullet::BtTransform.new()
    @transform.setIdentity()
    @transform.setOrigin(Bullet::BtVector3.new(0, -y, 0))
    @rigid_body.setCenterOfMassTransform(@transform)
  end

  def setWorldTransform(worldTrans)
  end

  def getWorldTransform(worldTrans)
  end

end
