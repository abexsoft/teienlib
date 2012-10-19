class Box < Bullet::BtMotionState
  @@cnt = 0

  attr_accessor :id
  attr_accessor :rigid_body

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
    @colObj.calculate_local_inertia(mass, inertia)
    @rigid_body = Bullet::BtRigidBody.new(mass, self, @colObj, inertia)
    dynamics_world.add_rigid_body(@rigid_body)

    # create a view object.
    gen = Procedural::BoxGenerator.new()
    gen.set_size_x(x * 2.0).set_size_y(y * 2.0).set_size_z(z * 2.0)
    gen.set_num_seg_x(1).set_num_seg_y(1).set_num_seg_z(1)
    gen.set_utile(1.0).set_vtile(1.0).realize_mesh(@name)
    @entity = scene_mgr.create_entity(@name)
    @entity.set_cast_shadows(true)
    @entity.set_material_name("Examples/BumpyMetal")
    @scene_node = scene_mgr.get_root_scene_node().create_child_scene_node()
    @scene_node.attach_object(@entity)

    # position/rotation
    @transform = Bullet::BtTransform.new()
    @transform.set_identity()
  end

  # called through Bullet::BtMotionState when this rigid_body transform is changed 
  # on bullet simulation step.
  def set_world_transform(worldTrans)
    @transform = Bullet::BtTransform.new(worldTrans)
    newPos = @transform.get_origin()
    newRot = @transform.get_rotation()
    return if (newRot.x.nan?)

    @scene_node.set_position(newPos.x, newPos.y, newPos.z) 
    @scene_node.set_orientation(newRot.w, newRot.x, newRot.y, newRot.z)
  end

  # called by Bullet::BtMotionState
  def get_world_transform(worldTrans)
  end

  def set_position(x, y, z)
    @scene_node.set_position(x, y, z)
    @transform.set_origin(Bullet::BtVector3.new(x, y, z))
    @rigid_body.set_center_of_mass_transform(@transform)
  end

  def apply_impulse(imp, rel = Bullet::BtVector3.new(0, 0, 0))
    @rigid_body.activate(true)
    @rigid_body.apply_impulse(imp, rel)
  end
end
