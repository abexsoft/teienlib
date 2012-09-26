#
# Key Listener
#
class KeyListener < OIS::KeyListener
  def initialize(listener)
    super()
    @listener = listener
  end
  
  def keyPressed(keyEvent)
    return @listener.key_pressed(keyEvent)
  end
  
  def keyReleased(keyEvent)
    return @listener.key_released(keyEvent)
  end
end

#
# Mouse Listener
#
class MouseListener < OIS::MouseListener
  def initialize(listener)
    super()
    @listener = listener
  end
  
  def mouseMoved(evt)
    return @listener.mouse_moved(evt)
  end
  
  def mousePressed(mouseEvent, mouseButtonID)
    return @listener.mouse_pressed(mouseEvent, mouseButtonID)
  end
  
  def mouseReleased(mouseEvent, mouseButtonID)
    return @listener.mouse_released(mouseEvent, mouseButtonID)
  end
end

#
# Tray Listener
#
class TrayListener < OgreBites::SdkTrayListener
  def initialize(listener)
    super()
    @listener = listener
  end

  def buttonHit(button)
    @listener.button_hit(button)
  end

  def itemSelected(menu)
    @listener.item_selected(menu)
  end

  def yesNoDialogClosed(name, bl)
    @listener.yes_no_dialog_closed(name, bl)
  end

  def okDialogClosed(name)
    @listener.ok_dialog_closed(name)
  end
end
